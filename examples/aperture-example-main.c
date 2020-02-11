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


static void
callback (ApertureDeviceManager *device_manager, ApertureDevice *device)
{
  const gchar *name;
  GstDevice *gst_device;
  gchar *class;
  GstStructure *props;

  name = aperture_device_get_name (device);
  gst_device = aperture_device_get_device (device);
  props = gst_device_get_properties (gst_device);
  class = gst_device_get_device_class (gst_device);

  if (APERTURE_IS_CAMERA (device)) {
    printf ("Camera: %s (%s)\n", name, class);
  } else {
    printf ("Microphone: %s (%s)\n", name, class);
  }

  aperture_pretty_print_structure (props);

  if (class) g_free (class);
  if (props) gst_structure_free (props);
}

int
main (int argc, char **argv)
{
  GMainLoop *loop;
  ApertureDeviceManager *device_manager;

  loop = g_main_loop_new (NULL, TRUE);
  device_manager = aperture_device_manager_new ();

  g_signal_connect (device_manager, "camera-added", G_CALLBACK (callback), NULL);
  g_signal_connect (device_manager, "microphone-added", G_CALLBACK (callback), NULL);

  aperture_device_manager_start (device_manager);

  g_main_loop_run (loop);
  g_main_loop_unref (loop);

  return 0;
}

