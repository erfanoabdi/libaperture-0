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


void
on_camera_changed (ApertureCameraSwitcherButton *switcher,
                   ApertureCamera *camera,
                   gpointer user_data)
{
  ApertureWidget *widget = user_data;

  aperture_widget_set_camera (widget, camera);
}

int
main (int argc, char **argv)
{
  GtkWidget *window;
  ApertureWidget *widget;
  ApertureCameraSwitcherButton *switcher;
  GtkWidget *grid;

  // Aperture does not work on Wayland yet
  g_setenv ("GDK_BACKEND", "x11", TRUE);

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  grid = gtk_grid_new ();
  switcher = aperture_camera_switcher_button_new ();
  gtk_widget_set_sensitive (GTK_WIDGET (switcher), TRUE);
  widget = aperture_widget_new ();

  gtk_widget_set_size_request (GTK_WIDGET (widget), 200, 200);

  g_signal_connect (switcher, "camera-changed", G_CALLBACK (on_camera_changed), widget);
  g_signal_connect (window, "destroy", gtk_main_quit, NULL);

  gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (widget), 0, 0, 2, 1);
  gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (switcher), 1, 1, 1, 1);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (grid));

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}

