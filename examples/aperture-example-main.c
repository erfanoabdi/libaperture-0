/*
 * Copyright (C) 2020 James Westman
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      James Westman <james@flyingpimonster.net>
 */


#include <stdio.h>
#include <aperture/aperture.h>
#include <handy.h>


typedef struct {
  ApertureViewfinder *viewfinder;
  ApertureShutterButton *shutter;
  ApertureGallery *gallery;
  gboolean countdown_active;
} UI;

void
on_camera_changed (ApertureCameraSwitcherButton *switcher,
                   ApertureCamera               *camera,
                   ApertureViewfinder           *viewfinder)
{
  aperture_viewfinder_set_camera (viewfinder, camera);
}

gchar *
get_file(GUserDirectory user_dir, gchar *extension)
{
  GDateTime *date = g_date_time_new_now_local ();
  const gchar *dir = g_get_user_special_dir (user_dir);
  gchar *filename = g_date_time_format (date, "%F_%T");

  gchar *path = g_strconcat (dir, "/", filename, ".", extension, NULL);

  for (int i = 1; g_file_test (path, G_FILE_TEST_EXISTS); i ++) {
    g_free (path);
    path = g_strdup_printf ("%s/%s_%d.jpg", dir, filename, i);
  }

  g_date_time_unref (date);
  g_free (filename);

  return path;
}

void
on_take_picture (GtkButton *button,
                 UI        *ui)
{
  gchar *path = get_file (G_USER_DIRECTORY_PICTURES, "jpg");
  aperture_viewfinder_take_picture (ui->viewfinder, path);
  g_free (path);

  /*if (aperture_viewfinder_get_state (ui->viewfinder) == APERTURE_STATE_RECORDING) {
    aperture_shutter_button_set_mode (ui->shutter, APERTURE_SHUTTER_BUTTON_MODE_VIDEO);
    aperture_viewfinder_stop_recording (ui->viewfinder);
  } else {
    aperture_shutter_button_set_mode (ui->shutter, APERTURE_SHUTTER_BUTTON_MODE_RECORDING);
    gchar *path = get_file (G_USER_DIRECTORY_VIDEOS, "mkv");
    aperture_viewfinder_start_recording (ui->viewfinder, path);
    g_free (path);
  }*/
}

void
on_picture_taken (ApertureViewfinder *viewfinder,
                  GdkPixbuf          *pixbuf,
                  ApertureGallery    *gallery)
{
  GError *error = NULL;
  gchar *path = get_file (G_USER_DIRECTORY_PICTURES, "jpg");

  aperture_gallery_add_image (gallery, pixbuf);

  if (error) {
    g_error_free (error);
  }

  g_free (path);
}

static void
on_test_clicked (GtkButton *button, UI *ui)
{
  ui->countdown_active = !ui->countdown_active;
  if (ui->countdown_active) {
    aperture_shutter_button_start_countdown (ui->shutter);
  } else {
    aperture_shutter_button_stop_countdown (ui->shutter);
  }
}

int
main (int argc, char **argv)
{
  GtkWidget *window;
  ApertureCameraSwitcherButton *switcher;
  GtkWidget *grid;
  ApertureGalleryButton *gallery_button;
  GtkWidget *button2;
  GtkWidget *headerbar;
  UI ui;
  const gchar *desktop;

  desktop = g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
  g_setenv ("GST_DEBUG_DUMP_DOT_DIR", desktop, TRUE);

  gtk_init (&argc, &argv);
  aperture_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  grid = gtk_grid_new ();
  switcher = aperture_camera_switcher_button_new ();
  gtk_widget_set_sensitive (GTK_WIDGET (switcher), TRUE);
  ui.shutter = aperture_shutter_button_new ();
  button2 = gtk_button_new_with_label ("Test");
  ui.gallery = aperture_gallery_new ();
  gallery_button = aperture_gallery_button_new ();
  ui.viewfinder = aperture_viewfinder_new ();

  gtk_widget_set_size_request (GTK_WIDGET (ui.viewfinder), 200, 200);
  gtk_widget_set_size_request (GTK_WIDGET (ui.shutter), 44, 44);
  gtk_widget_set_size_request (GTK_WIDGET (gallery_button), 56, 56);
  aperture_shutter_button_set_mode (ui.shutter, APERTURE_SHUTTER_BUTTON_MODE_PICTURE);
  aperture_shutter_button_set_countdown (ui.shutter, 5);
  g_object_set (ui.shutter, "margin", 12, NULL);
  aperture_gallery_button_set_gallery (gallery_button, ui.gallery);
  gtk_widget_set_halign (GTK_WIDGET (gallery_button), GTK_ALIGN_CENTER);
  gtk_widget_set_valign (GTK_WIDGET (gallery_button), GTK_ALIGN_CENTER);

  g_signal_connect (switcher, "camera-changed", G_CALLBACK (on_camera_changed), ui.viewfinder);
  g_signal_connect (ui.shutter, "clicked", G_CALLBACK (on_take_picture), &ui);
  g_signal_connect (ui.viewfinder, "picture-taken", G_CALLBACK (on_picture_taken), ui.gallery);
  g_signal_connect (window, "destroy", gtk_main_quit, NULL);
  g_signal_connect (button2, "clicked", G_CALLBACK (on_test_clicked), &ui);

  aperture_gallery_set_viewfinder (ui.gallery, ui.viewfinder);
  gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (ui.gallery), 0, 0, 3, 1);
  gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (button2), 0, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (switcher), 1, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (gallery_button), 2, 1, 1, 1);
  gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (grid));

  headerbar = hdy_header_bar_new ();
  hdy_header_bar_set_show_close_button (HDY_HEADER_BAR (headerbar), TRUE);
  hdy_header_bar_set_custom_title (HDY_HEADER_BAR (headerbar), GTK_WIDGET (ui.shutter));
  gtk_window_set_titlebar (GTK_WINDOW (window), headerbar);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}

