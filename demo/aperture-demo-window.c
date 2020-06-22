/* aperture-demo-window.c
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


#include "aperture-demo-window.h"

struct _ApertureDemoWindow
{
  GtkApplicationWindow parent_instance;

  GtkStack *stack;

  GtkWidget *controls;
  ApertureViewfinder *viewfinder;
  GtkStack *controls_stack;
  GtkStack *no_cameras_stack;
  GtkButton *switch_camera;

  gboolean recording;
  gboolean taking_picture;
};

G_DEFINE_TYPE (ApertureDemoWindow, aperture_demo_window, GTK_TYPE_APPLICATION_WINDOW)


static char *
get_file (GUserDirectory user_dir, gchar *extension)
{
  /* Get a file path for an image or video. The filename is based on the
   * date and time. If that file already exists, a number will be added to
   * the end so that a new file is created. */

  g_autoptr(GDateTime) date = g_date_time_new_now_local ();
  const char *dir = g_get_user_special_dir (user_dir);
  g_autofree char *filename = g_date_time_format (date, "%F_%T");
  g_autofree char *basename = g_strdup_printf ("%s.%s", filename, extension);
  char *path = g_build_filename (dir, basename, NULL);

  for (int i = 1; g_file_test (path, G_FILE_TEST_EXISTS); i ++) {
    g_free (path);
    g_free (basename);

    basename = g_strdup_printf ("%s_%d.%s", filename, i, extension);
    path = g_build_filename (dir, basename, NULL);
  }

  return path;
}


static void
update_ui (ApertureDemoWindow *self)
{
  /* Update the UI depending on the state of the viewfinder and the current
   * operation.
   *
   * If it's recording, show the stop button, otherwise show the regular
   * controls. If it's not in the READY state, make the controls insensitive
   * so they can't be used. If there are no cameras, show that screen
   * instead. */

  ApertureViewfinderState state = aperture_viewfinder_get_state (self->viewfinder);
  gtk_widget_set_sensitive (self->controls, state == APERTURE_VIEWFINDER_STATE_READY && !(self->recording || self->taking_picture));

  gtk_stack_set_visible_child_name (self->controls_stack, self->recording ? "video" : "main");

  gtk_stack_set_visible_child_name (self->no_cameras_stack,
                                    state == APERTURE_VIEWFINDER_STATE_NO_CAMERAS
                                      ? "no_cameras"
                                      : "main");
}


static void
on_viewfinder_state_notify (ApertureDemoWindow *self)
{
  update_ui (self);
}


static void
on_photo_taken (ApertureViewfinder *source, GAsyncResult *res, ApertureDemoWindow *self)
{
  g_autofree char *file = get_file (G_USER_DIRECTORY_PICTURES, "jpg");
  g_autoptr(GError) err = NULL;
  g_autoptr(GdkPixbuf) pixbuf = aperture_viewfinder_take_picture_finish (source, res, &err);

  if (err) {
    g_critical ("%s", err->message);
  } else {
    gdk_pixbuf_save (pixbuf, file, "jpeg", &err, NULL);
    if (err) {
      g_critical ("%s", err->message);
    } else {
      g_debug ("Saved picture to %s", (char *) file);
    }
  }

  self->taking_picture = FALSE;
  update_ui (self);
}


static void
on_take_photo_clicked (GtkButton *button, ApertureDemoWindow *self)
{
  self->taking_picture = TRUE;
  update_ui (self);
  aperture_viewfinder_take_picture_async (self->viewfinder, NULL, (GAsyncReadyCallback) on_photo_taken, self);
}


static void
on_take_video_clicked (GtkButton *button, ApertureDemoWindow *self)
{
  g_autofree char *file = get_file (G_USER_DIRECTORY_VIDEOS, "mp4");
  g_autoptr(GError) err = NULL;

  self->recording = TRUE;
  update_ui (self);

  aperture_viewfinder_start_recording_to_file (self->viewfinder, file, &err);

  if (err) {
    g_critical ("%s", err->message);
  }
}


static void
on_video_done (ApertureViewfinder *source, GAsyncResult *res, ApertureDemoWindow *self)
{
  self->recording = FALSE;
  update_ui (self);
}


static void
on_stop_video_clicked (GtkButton *button, ApertureDemoWindow *self)
{
  aperture_viewfinder_stop_recording_async (self->viewfinder, NULL, (GAsyncReadyCallback) on_video_done, self);
}


static void
on_barcode_detected (ApertureViewfinder *viewfinder, ApertureBarcode barcode_type, const char *data)
{
  const char *type = g_enum_to_string (APERTURE_TYPE_BARCODE, barcode_type);
  g_print ("Barcode detected (%s): %s\n", type, data);
}


static void
on_switch_camera_clicked (GtkButton *button, ApertureDemoWindow *self)
{
  /* When the "switch camera" button is clicked, use
   * aperture_device_manager_next_camera() to find the next camera, and
   * start using that. */

  int camera = aperture_viewfinder_get_camera (self->viewfinder);
  g_autoptr(ApertureDeviceManager) manager = aperture_device_manager_get_instance ();
  g_autoptr(GError) err = NULL;

  camera = aperture_device_manager_next_camera (manager, camera);
  aperture_viewfinder_set_camera (self->viewfinder, camera, &err);

  if (err) {
    g_critical ("%s", err->message);
  }
}


/* INIT */


static void
aperture_demo_window_class_init (ApertureDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/io/gnome/Aperture/Demo/ui/demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, ApertureDemoWindow, viewfinder);
  gtk_widget_class_bind_template_child (widget_class, ApertureDemoWindow, controls);
  gtk_widget_class_bind_template_child (widget_class, ApertureDemoWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, ApertureDemoWindow, controls_stack);
  gtk_widget_class_bind_template_child (widget_class, ApertureDemoWindow, switch_camera);
  gtk_widget_class_bind_template_child (widget_class, ApertureDemoWindow, no_cameras_stack);
  gtk_widget_class_bind_template_callback (widget_class, on_barcode_detected);
  gtk_widget_class_bind_template_callback (widget_class, on_stop_video_clicked);
  gtk_widget_class_bind_template_callback (widget_class, on_switch_camera_clicked);
  gtk_widget_class_bind_template_callback (widget_class, on_take_photo_clicked);
  gtk_widget_class_bind_template_callback (widget_class, on_take_video_clicked);
  gtk_widget_class_bind_template_callback (widget_class, on_viewfinder_state_notify);
}


static void
aperture_demo_window_init (ApertureDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  /* Make sure the UI is in the correct state */
  on_viewfinder_state_notify (self);
}


/* PUBLIC */


ApertureDemoWindow *
aperture_demo_window_new (GtkApplication *app)
{
  g_return_val_if_fail (GTK_IS_APPLICATION (app), NULL);

  return g_object_new (APERTURE_TYPE_DEMO_WINDOW, "application", app, NULL);
}

