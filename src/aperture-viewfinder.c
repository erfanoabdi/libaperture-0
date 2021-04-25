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
 * APERTURE_MEDIA_CAPTURE_ERROR:
 *
 * Error domain for errors that occur while using an #ApertureViewfinder.
 */

/**
 * ApertureMediaCaptureError:
 * @APERTURE_MEDIA_CAPTURE_ERROR_OPERATION_IN_PROGRESS: Another operation is in progress. Wait for it to finish before starting another operation.
 * @APERTURE_MEDIA_CAPTURE_ERROR_NO_RECORDING_TO_STOP: There is no recording to stop (applies to aperture_viewfinder_stop_recording_async()).
 * @APERTURE_MEDIA_CAPTURE_ERROR_CAMERA_DISCONNECTED: The active camera was disconnected during the operation.
 * @APERTURE_MEDIA_CAPTURE_ERROR_INTERRUPTED: The operation was interrupted by an unknown error.
 * @APERTURE_MEDIA_CAPTURE_ERROR_NOT_READY: The viewfinder is not in the %APERTURE_VIEWFINDER_STATE_READY #ApertureViewfinder:state.
 *
 * Indicates the error that caused an operation to fail.
 *
 * Note that functions might set errors from other domains as well. For
 * example, if an error occurs in the GStreamer pipeline during the operation,
 * that error will be passed directly to your async handler.
 *
 * Since: 0.1
 */


#include "pipeline/aperture-pipeline-tee.h"
#include "private/aperture-camera-private.h"
#include "private/aperture-private.h"
#include "aperture-camera.h"
#include "aperture-device-manager.h"
#include "aperture-utils.h"
#include "aperture-viewfinder.h"


typedef struct {
  GstElement *bin;
  GstElement *valve;
  GstElement *gdkpixbufsink;
} ApertureViewfinderPhotoCapture;

struct _ApertureViewfinder
{
  GtkBin parent_instance;

  ApertureDeviceManager *devices;

  ApertureCamera *camera;
  GstElement *camera_src;
  ApertureViewfinderState state;

  GstElement *branch_zbar;

  GstElement *gtksink;
  GstElement *vf_csp;
  GstElement *vf_vc;
  GstElement *filesink;
  GtkWidget *sink_widget;

  GstElement *camerabin;
  AperturePipelineTee *tee;
  GstElement *pipeline;

  GTask *task_take_picture;

  gboolean recording_video;
  GTask *task_take_video;

  ApertureViewfinderPhotoCapture capture;
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
  SIGNAL_BARCODE_DETECTED,
  N_SIGNALS,
};
static guint signals[N_SIGNALS];


static void
end_take_photo_operation (ApertureViewfinder *self)
{
  g_clear_object (&self->task_take_picture);
}


static void
end_take_video_operation (ApertureViewfinder *self)
{
  self->recording_video = FALSE;
  g_clear_object (&self->task_take_video);
}


/* Cancels any ongoing operations. Called when an error occurs, or when the
 * current camera is unplugged. @err is copied, so you still need to unref it
 * afterward. */
static void
cancel_current_operation (ApertureViewfinder *self, GError *err)
{
  if (self->task_take_picture) {
    g_task_return_error (self->task_take_picture, g_error_copy (err));
    end_take_photo_operation (self);
  } else if (self->task_take_video) {
    g_task_return_error (self->task_take_video, g_error_copy (err));
    end_take_video_operation (self);
  }
}


static void
set_state (ApertureViewfinder *self, ApertureViewfinderState state)
{
  g_autoptr(GError) err = NULL;

  if (self->state == state) {
    return;
  }

  if (state != APERTURE_VIEWFINDER_STATE_READY) {
    if (state == APERTURE_VIEWFINDER_STATE_NO_CAMERAS) {
      err = g_error_new (APERTURE_MEDIA_CAPTURE_ERROR,
                         APERTURE_MEDIA_CAPTURE_ERROR_CAMERA_DISCONNECTED,
                         "The active camera was disconnected during the operation");
    } else {
      err = g_error_new (APERTURE_MEDIA_CAPTURE_ERROR,
                         APERTURE_MEDIA_CAPTURE_ERROR_INTERRUPTED,
                         "An error occurred during the operation");
    }

    cancel_current_operation (self, err);
  }

  self->state = state;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);
}


/* Creates an element. Puts the viewfinder in the error state if that fails.
 * Thus, should only be used where the viewfinder doesn't work without the
 * element (otherwise, use gst_element_factory_make()). */
static GstElement *
create_element (ApertureViewfinder *self, const char *type)
{
  GstElement *element = gst_element_factory_make (type, NULL);

  if (element == NULL) {
    g_critical ("Element %s is not installed", type);
    set_state (self, APERTURE_VIEWFINDER_STATE_ERROR);
  }

  return element;
}


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
create_photo_capture_bin (ApertureViewfinder *self, ApertureViewfinderPhotoCapture *result)
{
  GstElement *bin = gst_bin_new (NULL);
  g_autoptr(GstPad) pad = NULL;
  GstPad *ghost_pad;

  GstElement *valve;
  GstElement *videoconvert;
  GstElement *gdkpixbufsink;

  valve = create_element (self, "valve");
  videoconvert = create_element (self, "videoconvert");
  gdkpixbufsink = create_element (self, "gdkpixbufsink");

  g_object_set (valve, "drop", FALSE, NULL);

  gst_bin_add_many (GST_BIN (bin), valve, videoconvert, gdkpixbufsink, NULL);
  gst_element_link_many (valve, videoconvert, gdkpixbufsink, NULL);

  pad = gst_element_get_static_pad (valve, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (bin, ghost_pad);

  result->bin = bin;
  result->valve = valve;
  result->gdkpixbufsink = gdkpixbufsink;
}


/* If an operation (take photo, take video, switch camera) is in progress,
 * set @err. */
static void
get_current_operation (ApertureViewfinder *self, GError **err)
{
  /* for convenience, do nothing if there's already an error */
  if (err && *err) {
    return;
  }

  if (self->task_take_picture) {
    g_set_error (err,
                 APERTURE_MEDIA_CAPTURE_ERROR,
                 APERTURE_MEDIA_CAPTURE_ERROR_OPERATION_IN_PROGRESS,
                 "Operation in progress: Take picture");
  } else if (self->task_take_video || self->recording_video) {
    g_set_error (err,
                 APERTURE_MEDIA_CAPTURE_ERROR,
                 APERTURE_MEDIA_CAPTURE_ERROR_OPERATION_IN_PROGRESS,
                 "Operation in progress: Video recording");
  }
}


static void
set_error_if_not_ready (ApertureViewfinder *self, GError **err)
{
  /* for convenience, do nothing if there's already an error */
  if (err && *err) {
    return;
  }

  if (aperture_viewfinder_get_state (self) != APERTURE_VIEWFINDER_STATE_READY) {
    g_set_error (err,
                 APERTURE_MEDIA_CAPTURE_ERROR,
                 APERTURE_MEDIA_CAPTURE_ERROR_NOT_READY,
                 "The viewfinder is not in the READY state.");
  }
}


static void
on_pipeline_error (ApertureViewfinder *self, GstMessage *message)
{
  g_autoptr(GError) err = NULL;
  g_autofree char *debug_info = NULL;

  gst_message_parse_error (message, &err, &debug_info);
  g_prefix_error (&err, "Error received from element %s: ", message->src->name);

  cancel_current_operation (self, err);
  g_debug ("Debugging information: %s", debug_info ? debug_info : "none");

  if (GST_ELEMENT (self->camerabin)->current_state != GST_STATE_PLAYING) {
    set_state (self, APERTURE_VIEWFINDER_STATE_ERROR);
    g_critical ("%s", err->message);
  }
}


static void
on_gdk_pixbuf_preroll (ApertureViewfinder *self)
{
  g_object_set (self->capture.valve, "drop", TRUE, NULL);
}


static void
on_gdk_pixbuf (ApertureViewfinder *self, GstMessage *message)
{
  const GstStructure *structure = gst_message_get_structure (message);
  GdkPixbuf *pixbuf;

  if (self->task_take_picture) {
    gst_structure_get (structure, "pixbuf", GDK_TYPE_PIXBUF, &pixbuf, NULL);

    g_task_return_pointer (self->task_take_picture, pixbuf, g_object_unref);
    end_take_photo_operation (self);

    g_object_set (self->capture.valve, "drop", TRUE, NULL);
  }
}


static void
on_video_done (ApertureViewfinder *self)
{
  g_task_return_boolean (self->task_take_video, TRUE);
  end_take_video_operation (self);
}


static void
on_barcode_detected (ApertureViewfinder *self, GstMessage *message)
{
  const char *code_type_str = NULL;
  ApertureBarcode code_type;
  const char *data = NULL;
  const GstStructure *structure;

  structure = gst_message_get_structure (message);
  code_type_str = gst_structure_get_string (structure, "type");
  code_type = aperture_barcode_type_from_string (code_type_str);
  data = gst_structure_get_string (structure, "symbol");

  g_signal_emit (self, signals[SIGNAL_BARCODE_DETECTED], 0, code_type, data);
}


/* Bus message handler for the pipeline */
static gboolean
on_bus_message_async (GstBus *bus, GstMessage *message, gpointer user_data)
{
  ApertureViewfinder *self = APERTURE_VIEWFINDER (user_data);

  switch (message->type) {
  case GST_MESSAGE_ERROR:
    on_pipeline_error (self, message);
    break;

  case GST_MESSAGE_ELEMENT:
    if (gst_message_has_name (message, "pixbuf")) {
      on_gdk_pixbuf (self, message);
    } else if (gst_message_has_name (message, "preroll-pixbuf")) {
      on_gdk_pixbuf_preroll (self);
    } else if (gst_message_has_name (message, "video-done")) {
      on_video_done (self);
    } else if (gst_message_has_name (message, "barcode")) {
      on_barcode_detected (self, message);
    }
    break;

  default:
    break;
  }

  return G_SOURCE_CONTINUE;
}

/* VFUNCS */


static void
aperture_viewfinder_finalize (GObject *object)
{
  ApertureViewfinder *self = APERTURE_VIEWFINDER (object);

  g_clear_object (&self->devices);
  g_clear_object (&self->pipeline);
  g_clear_object (&self->camerabin);
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
    g_value_set_object (value, aperture_viewfinder_get_camera (self));
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
    aperture_viewfinder_set_camera (self, g_value_get_object (value), NULL);
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
   * The camera that is currently being used.
   *
   * Use #ApertureDeviceManager to obtain #ApertureCamera objects.
   *
   * To successfully switch cameras, the #ApertureViewfinder must be in the
   * %APERTURE_VIEWFINDER_STATE_READY state. This is because switching camera
   * sources would interrupt any picture or video that is being taken.
   *
   * Since: 0.1
   */
  props [PROP_CAMERA] =
    g_param_spec_object ("camera",
                         "Camera",
                         "The camera to use",
                         APERTURE_TYPE_CAMERA,
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
}


static void
aperture_viewfinder_init (ApertureViewfinder *self)
{
  g_autoptr(ApertureCamera) camera = NULL;

  aperture_private_ensure_initialized ();

  self->pipeline = gst_pipeline_new(NULL);
  self->camerabin = create_element(self, "droidcamsrc");
  self->vf_csp = create_element(self, "capsfilter");

  self->gtksink = create_element (self, "gtksink");
  g_object_get (self->gtksink, "widget", &self->sink_widget, NULL);
  gtk_widget_set_hexpand (self->sink_widget, TRUE);
  gtk_widget_set_vexpand (self->sink_widget, TRUE);
  gtk_widget_show (self->sink_widget);
  gtk_container_add (GTK_CONTAINER (self), self->sink_widget);
  self->tee = aperture_pipeline_tee_new ();
  aperture_pipeline_tee_add_branch (self->tee, self->gtksink);

  self->vf_vc = create_element(self, "videoconvert");

  gst_bin_add_many(GST_BIN(self->pipeline), self->camerabin, self->vf_csp, self->tee, self->vf_vc, NULL);

  gst_element_link_pads(self->camerabin, "vfsrc", self->vf_csp, "sink");
  gst_element_link_many(self->vf_csp, self->vf_vc, self->tee, NULL);

  //create_photo_capture_bin(self, &self->capture);
  //aperture_pipeline_tee_add_branch (self->tee, self->capture.bin);

  self->camera = NULL;
  self->devices = aperture_device_manager_get_instance ();

  if (aperture_device_manager_get_num_cameras (self->devices) > 0) {
    set_state (self, APERTURE_VIEWFINDER_STATE_READY);
    camera = aperture_device_manager_get_camera (self->devices, 0);
    aperture_viewfinder_set_camera (self, camera, NULL);
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
 * @error: a location for a #GError, or %NULL
 *
 * Sets the camera that the #ApertureViewfinder will use. See
 * #ApertureViewfinder:camera.
 *
 * Since: 0.1
 */
void
aperture_viewfinder_set_camera (ApertureViewfinder *self, ApertureCamera *camera, GError **error)
{
  g_autoptr(GstElement) droidcam = NULL;
  GError *err = NULL;
  int idx = 0;

  g_return_if_fail (APERTURE_IS_VIEWFINDER (self));
  g_return_if_fail (camera == NULL || APERTURE_IS_CAMERA (camera));

  get_current_operation (self, &err);
  set_error_if_not_ready (self, &err);
  if (err) {
    g_propagate_error (error, err);
    return;
  }

  if (self->camera == camera) {
    return;
  }

  g_set_object (&self->camera, camera);

  /* Must change camerabin to NULL and back to PLAYING for the change to take
   * effect */
  gst_element_set_state (self->pipeline, GST_STATE_NULL);

  if (camera != NULL) {
    idx = aperture_camera_get_source_element(camera);

    g_object_set(self->camerabin, "camera-device", idx, NULL);
  }

  if (gtk_widget_get_realized (GTK_WIDGET (self->sink_widget))) {
    gst_element_set_state (self->pipeline, GST_STATE_PLAYING);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAMERA]);
}


/**
 * aperture_viewfinder_get_camera:
 * @self: an #ApertureViewfinder
 *
 * Gets the camera that the #ApertureViewfinder is currently using. See
 * #ApertureViewfinder:camera.
 *
 * Returns: (transfer none): the current camera
 * Since: 0.1
 */
ApertureCamera *
aperture_viewfinder_get_camera (ApertureViewfinder *self)
{
  g_return_val_if_fail (APERTURE_IS_VIEWFINDER (self), NULL);
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
    self->branch_zbar = NULL;
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
 * aperture_viewfinder_take_picture_async:
 * @self: an #ApertureViewfinder
 * @cancellable: (nullable): a #GCancellable
 * @callback: a #GAsyncReadyCallback to execute upon completion
 * @user_data: closure data for @callback
 *
 * Takes a picture.
 *
 * This may take a while. The resolution might be changed temporarily,
 * autofocusing might take place, etc. Basically everything you'd expect
 * to happen when you click the photo button in a camera app.
 *
 * When the picture has been taken, @callback will be called. Use
 * aperture_viewfinder_take_picture_finish() to get the picture as a
 * #GdkPixbuf.
 *
 * Since: 0.1
 */
void
aperture_viewfinder_take_picture_async (ApertureViewfinder *self,
                                        GCancellable *cancellable,
                                        GAsyncReadyCallback callback,
                                        gpointer user_data)
{
  GTask *task = NULL;
  GError *err = NULL;

  g_return_if_fail (APERTURE_IS_VIEWFINDER (self));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  /* Set up task */
  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, aperture_viewfinder_take_picture_async);

  set_error_if_not_ready (self, &err);
  get_current_operation (self, &err);
  if (err) {
    g_task_return_error (task, err);
    g_object_unref (task);
    return;
  }

  self->task_take_picture = task;

  /* Start the picture taking process */
  g_object_set (self->capture.valve, "drop", FALSE, NULL);
}


/**
 * aperture_viewfinder_take_picture_finish:
 * @self: an #ApertureViewfinder
 * @result: a #GAsyncResult provided to callback
 * @error: a location for a #GError, or %NULL
 *
 * Finishes an operation started by
 * aperture_viewfinder_take_picture_async().
 *
 * Returns: (transfer full): the image that was taken, or %NULL if there was an
 * error
 * Since: 0.1
 */
GdkPixbuf *
aperture_viewfinder_take_picture_finish (ApertureViewfinder *self,
                                         GAsyncResult *result,
                                         GError **error)
{
  g_return_val_if_fail (APERTURE_IS_VIEWFINDER (self), NULL);
  g_return_val_if_fail (G_IS_TASK (result), FALSE);

  return g_task_propagate_pointer (G_TASK (result), error);
}


/**
 * aperture_viewfinder_start_recording_to_file:
 * @self: an #ApertureViewfinder
 * @file: file path to save the video to
 * @error: a location for a #GError, or %NULL
 *
 * Starts recording a video. The video will be saved to @file.
 *
 * Call aperture_viewfinder_stop_recording_async() to stop recording.
 *
 * Since: 0.1
 */
void
aperture_viewfinder_start_recording_to_file (ApertureViewfinder *self, const char *file, GError **error)
{
  GError *err = NULL;
  GstCaps *caps;

  g_return_if_fail (APERTURE_IS_VIEWFINDER (self));
  g_return_if_fail (file != NULL);

  set_error_if_not_ready (self, &err);
  get_current_operation (self, &err);
  if (err) {
    g_propagate_error (error, err);
    return;
  }

  self->recording_video = TRUE;

  g_object_set(self->filesink, "location", file, NULL);

  caps = gst_caps_from_string("video/mpeg, mpegversion=4, framerate=30/1");
  gst_element_link_pads_filtered(self->camerabin, "vidsrc", self->filesink, "sink", caps);
  gst_caps_unref(caps);

  g_object_set (self->camerabin,
                "mode", 2,
                NULL);

  g_signal_emit_by_name (self->camerabin, "start-capture");
}


/**
 * aperture_viewfinder_stop_recording_async:
 * @self: an #ApertureViewfinder
 * @cancellable: (nullable): a #GCancellable
 * @callback: a #GAsyncReadyCallback to execute upon completion
 * @user_data: closure data for @callback
 *
 * Stop recording video. @callback will be called when this is complete.
 *
 * Since: 0.1
 */
void
aperture_viewfinder_stop_recording_async (ApertureViewfinder *self,
                                          GCancellable *cancellable,
                                          GAsyncReadyCallback callback,
                                          gpointer user_data)
{
  GTask *task = NULL;

  g_return_if_fail (APERTURE_IS_VIEWFINDER (self));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  /* Set up the task */
  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, aperture_viewfinder_stop_recording_async);

  /* Make sure there's an ongoing recording and that we're not already
   * stopping it*/
  if (!self->recording_video) {
    g_task_return_new_error (task,
                             APERTURE_MEDIA_CAPTURE_ERROR,
                             APERTURE_MEDIA_CAPTURE_ERROR_NO_RECORDING_TO_STOP,
                             "There is no recording to stop");
  }
  if (self->task_take_video) {
    g_task_return_new_error (task,
                             APERTURE_MEDIA_CAPTURE_ERROR,
                             APERTURE_MEDIA_CAPTURE_ERROR_OPERATION_IN_PROGRESS,
                             "Operation in progress: Stop recording");
  }

  self->task_take_video = task;

  g_signal_emit_by_name (self->camerabin, "stop-capture");
}


/**
 * aperture_viewfinder_stop_recording_finish:
 * @self: an #ApertureViewfinder
 * @result: a #GAsyncResult provided to callback
 * @error: a location for a #GError, or %NULL
 *
 * Finishes an operation started by aperture_viewfinder_stop_recording_async().
 *
 * Returns: %TRUE if the process succeeded, otherwise %FALSE
 * Since: 0.1
 */
gboolean
aperture_viewfinder_stop_recording_finish (ApertureViewfinder *self,
                                           GAsyncResult *result,
                                           GError **error)
{
  g_return_val_if_fail (APERTURE_IS_VIEWFINDER (self), FALSE);
  g_return_val_if_fail (G_IS_TASK (result), FALSE);

  return g_task_propagate_boolean (G_TASK (result), error);
}

G_DEFINE_QUARK (APERTURE_MEDIA_CAPTURE_ERROR, aperture_media_capture_error);

