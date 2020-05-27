/* aperture-viewfinder.c
 *
 * Copyright 2020 James Westman <james@flyingpimonster.net>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * SECTION:aperture-viewfinder
 * @title: ApertureViewfinder
 * @short_description: A GTK widget for displaying a camera feed and taking
 * pictures and videos from it
 *
 * The #ApertureViewfinder is the main widget in Aperture. It is responsible
 * for displaying a camera feed in your UI, and for using that camera feed to
 * do useful things, like taking pictures, recording video, and detecting
 * barcodes.
 *
 * #ApertureViewfinder does not contain camera controls, however--just the
 * camera feed. You'll need to build a controls UI yourself.
 */

/**
 * ApertureViewfinderState:
 * @APERTURE_VIEWFINDER_STATE_LOADING: The #ApertureViewfinder is still loading.
 * @APERTURE_VIEWFINDER_STATE_READY: The #ApertureViewfinder is ready to be used.
 * @APERTURE_VIEWFINDER_STATE_RECORDING: The #ApertureViewfinder is recording a video.
 * @APERTURE_VIEWFINDER_STATE_TAKING_PICTURE: The #ApertureViewfinder is taking a picture.
 * @APERTURE_VIEWFINDER_STATE_NO_CAMERAS: The #ApertureViewfinder could not find any cameras to use.
 * @APERTURE_VIEWFINDER_STATE_ERROR: An error has occurred and the viewfinder is not usable.
 *
 * Indicates what the viewfinder is currently doing. Many tasks, like taking
 * a picture, recording video, or switching cameras, requires the viewfinder
 * to be in a particular state.
 *
 * Since: 0.1
 */

/**
 * ApertureViewfinderError:
 * @APERTURE_VIEWFINDER_ERROR_UNSPECIFIED: An unknown error occurred.
 * @APERTURE_VIEWFINDER_ERROR_MISSING_PLUGIN: A plugin is missing that is needed to build the GStreamer pipeline. This is probably a software packaging issue.
 * @APERTURE_VIEWFINDER_ERROR_PIPELINE_ERROR: An error occurred somewhere in the GStreamer pipeline.
 * @APERTURE_VIEWFINDER_ERROR_COULD_NOT_TAKE_PICTURE: The picture could not be taken. It might have been successfully saved to a file, or it might not have.
 *
 * Indicates what type of error occurred.
 *
 * Since: 0.1
 */


#include "private/aperture-private.h"
#include "private/aperture-device-manager-private.h"
#include "aperture-utils.h"
#include "aperture-viewfinder.h"
#include "aperture-gst-widget.h"
#include "pipeline/aperture-pipeline-tee.h"


struct _ApertureViewfinder
{
  GtkBin parent_instance;

  ApertureDeviceManager *devices;

  int camera;
  ApertureViewfinderState state;

  GstElement *branch_zbar;

  ApertureGstWidget *gst_widget;
  GstElement *camerabin;
  AperturePipelineTee *tee;
  GstElement *pipeline;
};

G_DEFINE_TYPE (ApertureViewfinder, aperture_viewfinder, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_CAMERA,
  PROP_STATE,
  PROP_DETECT_BARCODES,
  N_PROPS,
};
static GParamSpec *props[N_PROPS];

enum {
  SIGNAL_PICTURE_TAKEN,
  SIGNAL_VIDEO_TAKEN,
  SIGNAL_BARCODE_DETECTED,
  SIGNAL_ERROR,
  N_SIGNALS,
};
static guint signals[N_SIGNALS];


static GstElement *
create_zbar_bin ()
{
  GstElement *bin = gst_bin_new (NULL);
  g_autoptr(GstPad) pad = NULL;
  GstPad *ghost_pad;

  GstElement *videoconvert;
  GstElement *zbar;
  GstElement *fakesink;

  videoconvert = gst_element_factory_make ("videoconvert", NULL);
  zbar = gst_element_factory_make ("zbar", NULL);
  fakesink = gst_element_factory_make ("fakesink", NULL);

  g_object_set (zbar, "cache", TRUE, NULL);

  gst_bin_add_many (GST_BIN (bin), videoconvert, zbar, fakesink, NULL);
  gst_element_link_many (videoconvert, zbar, fakesink, NULL);

  pad = gst_element_get_static_pad (videoconvert, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (bin, ghost_pad);

  return bin;
}


static void
set_state (ApertureViewfinder *self, ApertureViewfinderState state)
{
  if (self->state == state) {
    return;
  }

  self->state = state;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);
}


static void
set_error (ApertureViewfinder *self, ApertureViewfinderError error, gboolean recoverable)
{
  if (!recoverable) {
    set_state (self, APERTURE_VIEWFINDER_STATE_ERROR);
  }

  g_signal_emit (self, signals[SIGNAL_ERROR], 0, error, recoverable);
}


/* Creates an element. Puts the viewfinder in the error state if that fails. */
static GstElement *
create_element (ApertureViewfinder *self, const char *type)
{
  GstElement *element = gst_element_factory_make (type, NULL);

  if (element == NULL) {
    g_critical ("Failed to create element %s", type);
    set_error (self, APERTURE_VIEWFINDER_ERROR_MISSING_PLUGIN, FALSE);
  }

  return element;
}


static void
on_finish_taking_picture_on_new_pixbuf (GObject *source, GAsyncResult *res, gpointer user_data)
{
  ApertureViewfinder *self = APERTURE_VIEWFINDER (user_data);
  g_autoptr(GError) err = NULL;
  g_autoptr(GdkPixbuf) pixbuf = gdk_pixbuf_new_from_stream_finish (res, &err);

  set_state (self, APERTURE_VIEWFINDER_STATE_READY);

  if (err) {
    g_critical ("Could not take picture: %s", err->message);
    set_error (self, APERTURE_VIEWFINDER_ERROR_COULD_NOT_TAKE_PICTURE, TRUE);
    return;
  }

  g_signal_emit (self, signals[SIGNAL_PICTURE_TAKEN], 0, pixbuf);
}


static void
on_finish_taking_picture_on_file_read (GObject *source, GAsyncResult *res, gpointer user_data)
{
  GFile *file = G_FILE (source);
  ApertureViewfinder *self = APERTURE_VIEWFINDER (user_data);
  g_autoptr(GError) err = NULL;
  g_autoptr(GFileInputStream) stream = g_file_read_finish (file, res, &err);

  if (err) {
    g_critical ("Could not take picture: %s", err->message);
    set_error (self, APERTURE_VIEWFINDER_ERROR_COULD_NOT_TAKE_PICTURE, TRUE);
    return;
  }

  gdk_pixbuf_new_from_stream_async (G_INPUT_STREAM (stream), NULL, on_finish_taking_picture_on_new_pixbuf, self);
}


/* Called when a picture has been taken on the pipeline. Reads the picture from
 * the file it was saved to and then emits ::picture-taken */
static void
on_finish_taking_picture (ApertureViewfinder *self)
{
  g_autofree char *file_location = NULL;
  g_autoptr(GFile) file = NULL;

  g_object_get (self->camerabin, "location", &file_location, NULL);
  file = g_file_new_for_path (file_location);
  g_file_read_async (file, G_PRIORITY_DEFAULT, NULL, on_finish_taking_picture_on_file_read, self);
}


/* Bus message handler for the pipeline */
static gboolean
on_bus_message_async (GstBus *bus, GstMessage *message, gpointer user_data)
{
  ApertureViewfinder *self = APERTURE_VIEWFINDER (user_data);
  g_autoptr(GError) err = NULL;
  g_autofree char *debug_info = NULL;
  const char *code_type_str = NULL;
  ApertureBarcode code_type;
  const char *data = NULL;
  const GstStructure *structure;

  switch (message->type) {
  case GST_MESSAGE_ERROR:
    gst_message_parse_error (message, &err, &debug_info);
    g_critical ("Error received from element %s: %s", message->src->name, err->message);
    g_debug ("Debugging information: %s", debug_info ? debug_info : "none");

    /* FIXME: This might actually be a different error. For example, passing
     * an invalid filename to aperture_viewfinder_take_picture_to_file will
     * cause an error here, not in on_finish_taking_picture(). */
    set_error (self, APERTURE_VIEWFINDER_ERROR_PIPELINE_ERROR, FALSE);
    break;
  case GST_MESSAGE_ELEMENT:
    if (gst_message_has_name (message, "image-done")) {
      on_finish_taking_picture (self);
    } else if (gst_message_has_name (message, "video-done")) {
      set_state (self, APERTURE_VIEWFINDER_STATE_READY);
      g_signal_emit (self, signals[SIGNAL_VIDEO_TAKEN], 0);
    } else if (gst_message_has_name (message, "barcode")) {
      structure = gst_message_get_structure (message);
      code_type_str = gst_structure_get_string (structure, "type");
      code_type = aperture_barcode_type_from_string (code_type_str);
      data = gst_structure_get_string (structure, "symbol");

      g_signal_emit (self, signals[SIGNAL_BARCODE_DETECTED], 0, code_type, data);
    }
    break;
  default:
    break;
  }

  return G_SOURCE_CONTINUE;
}


/* Handler for when a camera is removed (unplugged, etc). If that was our
 * current camera, switch to a different camera. */
static void
on_camera_removed (ApertureViewfinder *self, int camera_index, ApertureDeviceManager *devices)
{
  if (camera_index == self->camera) {
    int num_cameras = aperture_device_manager_get_num_cameras (self->devices);
    if (num_cameras == 0) {
      set_state (self, APERTURE_VIEWFINDER_STATE_NO_CAMERAS);
    } else if (camera_index >= num_cameras) {
      aperture_viewfinder_set_camera (self, 0);
    } else {
      aperture_viewfinder_set_camera (self, camera_index);
    }
  }
}


/* VFUNCS */


static void
aperture_viewfinder_finalize (GObject *object)
{
  ApertureViewfinder *self = APERTURE_VIEWFINDER (object);

  g_clear_object (&self->devices);
  g_clear_object (&self->pipeline);
  g_clear_object (&self->tee);

  G_OBJECT_CLASS (aperture_viewfinder_parent_class)->finalize (object);
}


static void
aperture_viewfinder_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  ApertureViewfinder *self = APERTURE_VIEWFINDER (object);

  switch (prop_id) {
  case PROP_CAMERA:
    g_value_set_int (value, aperture_viewfinder_get_camera (self));
    break;
  case PROP_STATE:
    g_value_set_enum (value, aperture_viewfinder_get_state (self));
    break;
  case PROP_DETECT_BARCODES:
    g_value_set_boolean (value, aperture_viewfinder_get_detect_barcodes (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


static void
aperture_viewfinder_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  ApertureViewfinder *self = APERTURE_VIEWFINDER (object);

  switch (prop_id) {
  case PROP_CAMERA:
    aperture_viewfinder_set_camera (self, g_value_get_int (value));
    break;
  case PROP_DETECT_BARCODES:
    aperture_viewfinder_set_detect_barcodes (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


/* Starts the pipeline when the widget is realized */
static void
aperture_viewfinder_realize (GtkWidget *widget)
{
  ApertureViewfinder *self = APERTURE_VIEWFINDER (widget);

  GTK_WIDGET_CLASS (aperture_viewfinder_parent_class)->realize (widget);

  gst_element_set_state (self->pipeline, GST_STATE_PLAYING);
}


/* Stops the pipeline when the widget is unrealized */
static void
aperture_viewfinder_unrealize (GtkWidget *widget)
{
  ApertureViewfinder *self = APERTURE_VIEWFINDER (widget);

  GTK_WIDGET_CLASS (aperture_viewfinder_parent_class)->unrealize (widget);

  gst_element_set_state (self->pipeline, GST_STATE_NULL);
}


/* INIT */


static void
aperture_viewfinder_class_init (ApertureViewfinderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = aperture_viewfinder_finalize;
  object_class->get_property = aperture_viewfinder_get_property;
  object_class->set_property = aperture_viewfinder_set_property;
  widget_class->realize = aperture_viewfinder_realize;
  widget_class->unrealize = aperture_viewfinder_unrealize;

  /**
   * ApertureViewfinder:camera:
   *
   * The index of the camera device that is currently being used.
   *
   * Use #ApertureDeviceManager to get the number of available cameras. Setting
   * this property will switch cameras.
   *
   * To successfully switch cameras, the #ApertureViewfinder must be in the
   * %APERTURE_VIEWFINDER_STATE_READY state. This is because switching camera
   * sources would interrupt any picture or video that is being taken.
   *
   * Since: 0.1
   */
  props [PROP_CAMERA] =
    g_param_spec_int ("camera",
                      "Camera",
                      "The index of the camera to use",
                      -1, G_MAXINT, -1,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * ApertureViewfinder:state:
   *
   * What the viewfinder is currently doing.
   *
   * The state indicates what the viewfinder is currently doing, or sometimes
   * that an error occurred. Many operations, like taking a picture or starting
   * a recording, require that the #ApertureViewfinder be in the
   * %APERTURE_VIEWFINDER_STATE_READY state.
   *
   * Since: 0.1
   */
  props [PROP_STATE] =
    g_param_spec_enum ("state",
                       "State",
                       "What the viewfinder is currently doing",
                       APERTURE_TYPE_VIEWFINDER_STATE,
                       APERTURE_VIEWFINDER_STATE_LOADING,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * ApertureViewfinder:detect-barcodes:
   *
   * Whether the #ApertureViewfinder should detect barcodes.
   *
   * When a barcode is detected, the ::barcode-detected signal will be
   * emitted.
   *
   * This only works if barcode detection is enabled. See
   * aperture_is_barcode_detection_enabled(). If barcode detection is not
   * available, the value of this property will always be %FALSE, even if you
   * try to set it to %TRUE.
   *
   * Since: 0.1
   */
  props [PROP_DETECT_BARCODES] =
    g_param_spec_boolean ("detect-barcodes",
                          "Detect barcodes",
                          "Whether to detect barcodes in the camera feed",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, N_PROPS, props);

  /**
   * ApertureViewfinder::picture-taken:
   * @self: the #ApertureViewfinder instance
   * @pixbuf: the image that was taken
   *
   * Emitted when a picture is done being taken.
   *
   * When the signal is emitted, the image will already be saved to the file
   * that was specified when aperture_viewfinder_take_picture_to_file() was called.
   * You can access that file for more details, but the image is also loaded
   * for you into a pixbuf for easier access.
   *
   * Since: 0.1
   */
  signals[SIGNAL_PICTURE_TAKEN] =
    g_signal_new ("picture-taken",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1, GDK_TYPE_PIXBUF);

  /**
   * ApertureViewfinder::video-taken:
   * @self: the #ApertureViewfinder
   *
   * Emitted when a video is done being taken.
   *
   * When the signal is emitted, the video will already be saved to the file
   * that was specified when aperture_viewfinder_start_recording_to_file() was
   * called.
   *
   * Since: 0.1
   */
  signals[SIGNAL_VIDEO_TAKEN] =
    g_signal_new ("video-taken",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);

  /**
   * ApertureViewfinder::barcode-detected:
   * @self: the #ApertureViewfinder
   * @barcode_type: the type of barcode
   * @data: the data encoded in the barcode
   *
   * Emitted when a barcode is detected in the camera feed.
   *
   * This will only be emitted if #ApertureViewfinder:detect-barcodes is %TRUE.
   *
   * Barcodes are only detected when they appear on the feed, not on every
   * frame when they are visible.
   *
   * Since: 0.1
   */
  signals[SIGNAL_BARCODE_DETECTED] =
    g_signal_new ("barcode-detected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2, APERTURE_TYPE_BARCODE, G_TYPE_STRING);

  /**
   * ApertureViewfinder::error:
   * @self: the #ApertureViewfinder
   * @error: the type of error
   * @recoverable: %TRUE if the viewfinder can still be used, otherwise %FALSE
   *
   * Emitted whenever an error occurs.
   *
   * Sometimes errors are recoverable, meaning the viewfinder is still usable,
   * at least partially. (For example, if a picture is taken but the storage
   * destination is out of space, you could still take a picture and save it
   * elsewhere). If the error was not recoverable, #ApertureViewfinder:state
   * will be %APERTURE_VIEWFINDER_STATE_ERROR and @recoverable will be %FALSE.
   * Do not try to use a viewfinder in that state.
   *
   * Since: 0.1
   */
  signals[SIGNAL_ERROR] =
    g_signal_new ("error",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2, APERTURE_TYPE_VIEWFINDER_ERROR, G_TYPE_BOOLEAN);
}


static void
aperture_viewfinder_init (ApertureViewfinder *self)
{
  GstElement *sink;

  aperture_private_ensure_initialized ();

  self->gst_widget = aperture_gst_widget_new ();
  gtk_widget_set_hexpand (GTK_WIDGET (self->gst_widget), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (self->gst_widget), TRUE);
  gtk_widget_show (GTK_WIDGET (self->gst_widget));
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (self->gst_widget));

  self->pipeline = gst_pipeline_new (NULL);
  gst_bus_add_watch (gst_pipeline_get_bus (GST_PIPELINE (self->pipeline)), on_bus_message_async, self);

  self->tee = aperture_pipeline_tee_new ();

  self->camerabin = create_element (self, "camerabin");
  g_object_set (self->camerabin, "viewfinder-sink", self->tee, NULL);
  sink = g_object_ref (aperture_gst_widget_get_sink (self->gst_widget));
  aperture_pipeline_tee_add_branch (self->tee, sink);
  gst_bin_add (GST_BIN (self->pipeline), self->camerabin);

  self->camera = -1;
  self->devices = aperture_device_manager_get_instance ();

  g_signal_connect_object (self->devices,
                           "camera-removed",
                           G_CALLBACK (on_camera_removed),
                           self,
                           G_CONNECT_SWAPPED);

  if (aperture_device_manager_get_num_cameras (self->devices) > 0) {
    set_state (self, APERTURE_VIEWFINDER_STATE_READY);
    aperture_viewfinder_set_camera (self, 0);
  } else {
    set_state (self, APERTURE_VIEWFINDER_STATE_NO_CAMERAS);
  }
}


/* PUBLIC */


/**
 * aperture_viewfinder_new:
 *
 * Creates a new #ApertureViewfinder.
 *
 * Returns: (transfer full): a new #ApertureViewfinder
 * Since: 0.1
 */
ApertureViewfinder *
aperture_viewfinder_new (void)
{
  return g_object_new (APERTURE_TYPE_VIEWFINDER, NULL);
}


/**
 * aperture_viewfinder_set_camera:
 * @self: an #ApertureViewfinder
 * @camera: a camera index
 *
 * Sets the camera that the #ApertureViewfinder will use. See
 * #ApertureViewfinder:camera.
 *
 * This cannot be called if the viewfinder is not in the
 * %APERTURE_VIEWFINDER_STATE_READY state. The camera source cannot be changed
 * while a picture is being taken or a video is being recorded, or if an error
 * has occurred.
 *
 * Since: 0.1
 */
void
aperture_viewfinder_set_camera (ApertureViewfinder *self, int camera)
{
  g_autoptr(GstElement) wrapper = NULL;
  g_autoptr(GstElement) camera_src = NULL;

  g_return_if_fail (APERTURE_IS_VIEWFINDER (self));
  g_return_if_fail (camera >= -1 && camera < aperture_device_manager_get_num_cameras (self->devices));
  g_return_if_fail (self->state == APERTURE_VIEWFINDER_STATE_READY);

  if (self->camera == camera) {
    return;
  }

  self->camera = camera;

  if (camera != -1) {
    wrapper = create_element (self, "wrappercamerabinsrc");
    camera_src = aperture_device_manager_get_video_source (self->devices, self->camera);
    g_object_set (wrapper, "video-source", camera_src, NULL);
    g_object_set (self->camerabin, "camera-source", wrapper, NULL);
  }

  /* Must change camerabin to NULL and back to PLAYING for the change to take
   * effect */
  gst_element_set_state (self->camerabin, GST_STATE_NULL);
  if (gtk_widget_get_realized (GTK_WIDGET (self->gst_widget))) {
    gst_element_set_state (self->camerabin, GST_STATE_PLAYING);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAMERA]);
}


/**
 * aperture_viewfinder_get_camera:
 * @self: an #ApertureViewfinder
 *
 * Gets the index of the camera that the #ApertureViewfinder is currently using. See
 * #ApertureViewfinder:camera.
 *
 * Returns: the index of the current camera
 * Since: 0.1
 */
int
aperture_viewfinder_get_camera (ApertureViewfinder *self)
{
  g_return_val_if_fail (APERTURE_IS_VIEWFINDER (self), 0);
  return self->camera;
}


/**
 * aperture_viewfinder_get_state:
 * @self: an #ApertureViewfinder
 *
 * Gets the state of the #ApertureViewfinder. See #ApertureViewfinder:state.
 *
 * Returns: the viewfinder's state
 * Since: 0.1
 */
ApertureViewfinderState
aperture_viewfinder_get_state (ApertureViewfinder *self)
{
  g_return_val_if_fail (APERTURE_IS_VIEWFINDER (self), 0);
  return self->state;
}


/**
 * aperture_viewfinder_set_detect_barcodes:
 * @self: an #ApertureViewfinder
 * @detect_barcodes: %TRUE to detect barcodes, otherwise %FALSE
 *
 * Sets whether the #ApertureViewfinder should look for barcodes in its camera
 * feed. See #ApertureViewfinder:detect-barcodes.
 *
 * Before calling this function, use aperture_is_barcode_detection_enabled()
 * to make sure the barcode detection feature is enabled.
 *
 * Since: 0.1
 */
void
aperture_viewfinder_set_detect_barcodes (ApertureViewfinder *self, gboolean detect_barcodes)
{
  g_return_if_fail (APERTURE_IS_VIEWFINDER (self));
  detect_barcodes = !!detect_barcodes;
  g_return_if_fail (!detect_barcodes || aperture_is_barcode_detection_enabled ());

  if (aperture_viewfinder_get_detect_barcodes (self) == detect_barcodes) {
    return;
  }

  if (detect_barcodes) {
    self->branch_zbar = create_zbar_bin ();
    aperture_pipeline_tee_add_branch (self->tee, GST_ELEMENT (self->branch_zbar));
  } else {
    aperture_pipeline_tee_remove_branch (self->tee, GST_ELEMENT (self->branch_zbar));
    g_clear_object (&self->branch_zbar);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DETECT_BARCODES]);
}


/**
 * aperture_viewfinder_get_detect_barcodes:
 * @self: an #ApertureViewfinder
 *
 * Gets whether the #ApertureViewfinder is looking for barcodes in its camera
 * feed.
 *
 * Returns: %TRUE if the viewfinder is looking for barcodes, otherwise %FALSE
 * Since: 0.1
 */
gboolean
aperture_viewfinder_get_detect_barcodes (ApertureViewfinder *self)
{
  g_return_val_if_fail (APERTURE_IS_VIEWFINDER (self), FALSE);
  return self->branch_zbar != NULL;
}


/**
 * aperture_viewfinder_take_picture_to_file:
 * @self: an #ApertureViewfinder
 * @file: file path to save the picture to
 *
 * Takes a picture and saves it to a file.
 *
 * This may take a while. The resolution might be changed temporarily,
 * autofocusing might take place, etc. Basically everything you'd expect
 * to happen when you click the photo button on the camera app.
 *
 * When the picture has been taken and saved,
 * #ApertureViewfinder::picture_taken will be emitted.
 *
 * This cannot be called if the viewfinder is not in the
 * %APERTURE_VIEWFINDER_STATE_READY state. A picture cannot be taken if the
 * viewfinder is already taking a picture or recording a video, or if an error
 * has occurred.
 *
 * Since: 0.1
 */
void
aperture_viewfinder_take_picture_to_file (ApertureViewfinder *self, const char *file)
{
  g_return_if_fail (APERTURE_IS_VIEWFINDER (self));
  g_return_if_fail (file != NULL);
  g_return_if_fail (self->state == APERTURE_VIEWFINDER_STATE_READY);

  set_state (self, APERTURE_VIEWFINDER_STATE_TAKING_PICTURE);

  g_object_set (self->camerabin,
                "mode", 1,
                "location", file,
                NULL);

  g_signal_emit_by_name (self->camerabin, "start-capture");
}


/**
 * aperture_viewfinder_start_recording_to_file:
 * @self: an #ApertureViewfinder
 * @file: file path to save the video to
 *
 * Starts recording a video. The video will be saved to @file.
 *
 * This cannot be called if the viewfinder is not in the
 * %APERTURE_VIEWFINDER_STATE_READY state. Recording cannot start if the
 * viewfinder is already taking a picture or recording a video, or if an error
 * has occurred.
 *
 * Since: 0.1
 */
void
aperture_viewfinder_start_recording_to_file (ApertureViewfinder *self, const char *file)
{
  g_return_if_fail (APERTURE_IS_VIEWFINDER (self));
  g_return_if_fail (file != NULL);
  g_return_if_fail (self->state == APERTURE_VIEWFINDER_STATE_READY);

  set_state (self, APERTURE_VIEWFINDER_STATE_RECORDING);

  g_object_set (self->camerabin,
                "mode", 2,
                "location", file,
                NULL);

  g_signal_emit_by_name (self->camerabin, "start-capture");
}


/**
 * aperture_viewfinder_stop_recording:
 * @self: an #ApertureViewfinder
 *
 * Stop recording video. The #ApertureViewfinder::video_taken signal will be
 * emitted when this is done.
 *
 * The viewfinder must be recording when this is called. If it is,
 * #ApertureViewfinder:state will be %APERTURE_VIEWFINDER_STATE_RECORDING.
 *
 * Since: 0.1
 */
void
aperture_viewfinder_stop_recording (ApertureViewfinder *self)
{
  g_return_if_fail (APERTURE_IS_VIEWFINDER (self));
  g_return_if_fail (self->state == APERTURE_VIEWFINDER_STATE_RECORDING);

  g_signal_emit_by_name (self->camerabin, "stop-capture");
}
