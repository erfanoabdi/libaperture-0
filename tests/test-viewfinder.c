/* test-viewfinder.c
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


#include <glib.h>
#include <aperture.h>

#include "dummy-device-provider.h"
#include "utils.h"


static void
test_viewfinder_no_camera_state ()
{
  g_autoptr(ApertureDeviceManager) manager = aperture_device_manager_get_instance ();
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  g_autoptr(ApertureViewfinder) viewfinder = NULL;

  /* make sure there are no cameras to start */
  g_assert_cmpint (aperture_device_manager_get_num_cameras (manager), ==, 0);

  /* if there are no cameras, new viewfinders should be in NO_CAMERAS state */
  viewfinder = aperture_viewfinder_new ();
  g_object_ref_sink (viewfinder);
  g_assert_cmpint (aperture_viewfinder_get_state (viewfinder), ==, APERTURE_VIEWFINDER_STATE_NO_CAMERAS);

  dummy_device_provider_add (provider);
  testutils_wait_for_device_change (manager);
  g_assert_cmpint (aperture_viewfinder_get_state (viewfinder), ==, APERTURE_VIEWFINDER_STATE_READY);
  g_assert_cmpint (aperture_viewfinder_get_camera (viewfinder), ==, 0);

  dummy_device_provider_remove (provider);
  testutils_wait_for_device_change (manager);
  g_assert_cmpint (aperture_viewfinder_get_state (viewfinder), ==, APERTURE_VIEWFINDER_STATE_NO_CAMERAS);
}


static void
on_picture_taken (ApertureViewfinder *source, GAsyncResult *res, TestUtilsCallback *callback)
{
  g_autoptr(GError) err = NULL;
  g_autoptr(GdkPixbuf) pixbuf = NULL;

  pixbuf = aperture_viewfinder_take_picture_finish (source, res, &err);

  g_assert_no_error (err);
  testutils_assert_quadrants_pixbuf (pixbuf);

  testutils_callback_call (callback);
}


static void
test_viewfinder_take_picture ()
{
  g_autoptr(ApertureDeviceManager) manager = aperture_device_manager_get_instance ();
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  ApertureViewfinder *viewfinder;
  GtkWidget *window;
  TestUtilsCallback picture_callback;
  DummyDevice *device;

  testutils_callback_init (&picture_callback);

  device = dummy_device_provider_add (provider);
  dummy_device_set_image (device, "/aperture/quadrants.png");
  testutils_wait_for_device_change (manager);

  viewfinder = aperture_viewfinder_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (viewfinder));
  gtk_widget_show_all (window);

  aperture_viewfinder_take_picture_async (viewfinder, NULL, (GAsyncReadyCallback) on_picture_taken, &picture_callback);

  testutils_callback_assert_called (&picture_callback, 1000);

  gtk_widget_destroy (window);
}


static void
simultaneous_operations_on_picture_taken_1 (ApertureViewfinder *source, GAsyncResult *res, TestUtilsCallback *callback)
{
  testutils_callback_call (callback);
}


static void
simultaneous_operations_on_picture_taken_2 (ApertureViewfinder *source, GAsyncResult *res, TestUtilsCallback *callback)
{
  g_autoptr(GError) err = NULL;
  g_autoptr(GdkPixbuf) pixbuf = NULL;

  pixbuf = aperture_viewfinder_take_picture_finish (source, res, &err);

  g_assert_null (pixbuf);
  g_assert_error (err, APERTURE_MEDIA_CAPTURE_ERROR, APERTURE_MEDIA_CAPTURE_ERROR_OPERATION_IN_PROGRESS);

  testutils_callback_call (callback);
}


static void
test_viewfinder_simultaneous_operations ()
{
  g_autoptr(ApertureDeviceManager) manager = aperture_device_manager_get_instance ();
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  ApertureViewfinder *viewfinder;
  GtkWidget *window;
  g_autoptr(GError) err1 = NULL;
  g_autoptr(GError) err2 = NULL;
  TestUtilsCallback picture_callback_1;
  TestUtilsCallback picture_callback_2;

  testutils_callback_init (&picture_callback_1);
  testutils_callback_init (&picture_callback_2);

  dummy_device_provider_add (provider);
  dummy_device_provider_add (provider);
  testutils_wait_for_device_change (manager);
  testutils_wait_for_device_change (manager);

  viewfinder = aperture_viewfinder_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (viewfinder));
  gtk_widget_show_all (window);

  /* this is the operation that will block the others */
  aperture_viewfinder_take_picture_async (viewfinder, NULL, (GAsyncReadyCallback) simultaneous_operations_on_picture_taken_1, &picture_callback_1);

  /* setting the camera should not work */
  aperture_viewfinder_set_camera (viewfinder, 1, &err1);
  g_assert_error (err1, APERTURE_MEDIA_CAPTURE_ERROR, APERTURE_MEDIA_CAPTURE_ERROR_OPERATION_IN_PROGRESS);

  /* starting a video should not work */
  aperture_viewfinder_start_recording_to_file (viewfinder, "not_a_real_filename", &err2);
  g_assert_error (err2, APERTURE_MEDIA_CAPTURE_ERROR, APERTURE_MEDIA_CAPTURE_ERROR_OPERATION_IN_PROGRESS);

  /* taking another picture should not work */
  aperture_viewfinder_take_picture_async (viewfinder, NULL, (GAsyncReadyCallback) simultaneous_operations_on_picture_taken_2, &picture_callback_2);

  testutils_callback_assert_called (&picture_callback_1, 1000);
  testutils_callback_assert_called (&picture_callback_2, 1000);

  gtk_widget_destroy (window);
}


static void
disconnect_on_picture_taken (ApertureViewfinder *source, GAsyncResult *res, TestUtilsCallback *callback)
{
  g_autoptr(GError) err = NULL;
  g_autoptr(GdkPixbuf) pixbuf = NULL;

  pixbuf = aperture_viewfinder_take_picture_finish (source, res, &err);

  g_assert_null (pixbuf);
  g_assert_error (err, APERTURE_MEDIA_CAPTURE_ERROR, APERTURE_MEDIA_CAPTURE_ERROR_CAMERA_DISCONNECTED);

  testutils_callback_call (callback);
}


static void
test_viewfinder_disconnect_camera ()
{
  g_autoptr(ApertureDeviceManager) manager = aperture_device_manager_get_instance ();
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  ApertureViewfinder *viewfinder;
  GtkWidget *window;
  TestUtilsCallback picture_callback;

  testutils_callback_init (&picture_callback);

  g_test_summary ("Test error handling when the active camera is disconnected during an operation");

  dummy_device_provider_add (provider);
  testutils_wait_for_device_change (manager);
  g_assert_cmpint (aperture_device_manager_get_num_cameras (manager), ==, 1);

  viewfinder = aperture_viewfinder_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (viewfinder));
  gtk_widget_show_all (window);

  aperture_viewfinder_take_picture_async (viewfinder, NULL, (GAsyncReadyCallback) disconnect_on_picture_taken, &picture_callback);

  dummy_device_provider_remove (provider);
  testutils_wait_for_device_change (manager);
  testutils_callback_assert_called (&picture_callback, 1000);

  gtk_widget_destroy (window);
}


void
add_viewfinder_tests ()
{
  g_test_add_func ("/viewfinder/no_camera", test_viewfinder_no_camera_state);
  g_test_add_func ("/viewfinder/take_picture", test_viewfinder_take_picture);
  g_test_add_func ("/viewfinder/simultaneous_operations", test_viewfinder_simultaneous_operations);
  g_test_add_func ("/viewfinder/disconnect_camera", test_viewfinder_disconnect_camera);
}
