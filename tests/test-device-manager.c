/* test-device-manager.c
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

#include "private/aperture-camera-private.h"
#include "dummy-device-provider.h"
#include "utils.h"


static void
test_device_manager_refcounting ()
{
  /* Test that managers are destroyed when they are not in use anymore, and
   * that if a device manager does exist, it is returned rather than a new
   * instance */

  ApertureDeviceManager *manager1 = aperture_device_manager_get_instance ();
  ApertureDeviceManager *manager2 = aperture_device_manager_get_instance ();
  ApertureDeviceManager *manager3 = NULL;

  g_assert_nonnull (manager1);
  g_assert_nonnull (manager2);

  /* make sure the managers are the same, since the first was still active
   * when the second was requested */
  g_assert_true (manager1 == manager2);

  /* Set some data on the manager, so that later we can make sure we have a
   * different manager object */
  g_object_set_data (G_OBJECT (manager1), "test", "Hello, world!");
  g_assert_cmpstr (g_object_get_data (G_OBJECT (manager1), "test"), ==, "Hello, world!");

  g_object_unref (manager1);
  g_assert_finalize_object (G_OBJECT (manager2));

  manager3 = aperture_device_manager_get_instance ();
  g_assert_nonnull (manager3);

  /* the old device manager was destroyed. make sure we got a new one */
  g_assert_null (g_object_get_data (G_OBJECT (manager3), "test"));

  g_assert_finalize_object (G_OBJECT (manager3));
}


/*
 * Determines whether the device manager contains the test device.
 */
static gboolean
manager_contains_test_device (ApertureDeviceManager *manager)
{
  g_autoptr(ApertureCamera) camera = NULL;
  int num_dummy_cameras = 0;
  int num_cameras = aperture_device_manager_get_num_cameras (manager);
  int i;

  for (i = 0; i < num_cameras; i ++) {
    g_set_object (&camera, aperture_device_manager_get_camera (manager, i));
    if (APERTURE_IS_CAMERA (camera)) {
      g_autoptr(GstElement) element = aperture_camera_get_source_element (camera, NULL);
      if (g_str_has_prefix (gst_object_get_name (GST_OBJECT (element)), "videotestsrc")) {
        num_dummy_cameras ++;
      }
    }
  }

  return num_dummy_cameras == 1;
}


static void
test_device_manager_works ()
{
  /* Test that device managers properly list devices obtained from GStreamer */

  g_autoptr(ApertureDeviceManager) manager = NULL;
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  int num_cameras;

  dummy_device_provider_add (provider);

  manager = aperture_device_manager_get_instance ();

  g_object_get (manager, "num-cameras", &num_cameras, NULL);
  g_assert_cmpint (num_cameras, ==, 1);

  /* make sure one of the devices is the dummy one */
  g_assert_true (manager_contains_test_device (manager));
}


static void
test_device_manager_monitoring ()
{
  g_test_summary ("Test that the ::camera-added and ::camera-removed signals work");

  g_autoptr(ApertureDeviceManager) manager = aperture_device_manager_get_instance ();
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  TestUtilsCallback added_callback, removed_callback;

  testutils_callback_init (&added_callback);
  testutils_callback_init (&removed_callback);

  g_signal_connect_swapped (manager, "camera-added", G_CALLBACK (testutils_callback_call), &added_callback);
  g_signal_connect_swapped (manager, "camera-removed", G_CALLBACK (testutils_callback_call), &removed_callback);

  dummy_device_provider_add (provider);
  testutils_callback_assert_called (&added_callback, 1000);

  g_assert_true (manager_contains_test_device (manager));
  g_assert_cmpint (aperture_device_manager_get_num_cameras (manager), ==, 1);

  dummy_device_provider_remove (provider);
  testutils_callback_assert_called (&removed_callback, 1000);

  g_assert_false (manager_contains_test_device (manager));
  g_assert_cmpint (aperture_device_manager_get_num_cameras (manager), ==, 0);
}


static void
test_device_manager_next_camera ()
{
  g_autoptr(ApertureDeviceManager) manager = aperture_device_manager_get_instance ();
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  int num_cameras;
  int camera = 0;
  TestUtilsCallback added_callback;

  /* make sure there are multiple cameras, or the test won't work */
  testutils_callback_init (&added_callback);
  g_signal_connect_swapped (manager, "camera-added", G_CALLBACK (testutils_callback_call), &added_callback);
  dummy_device_provider_add (provider);
  dummy_device_provider_add (provider);
  testutils_callback_assert_called (&added_callback, 1000);
  testutils_callback_assert_called (&added_callback, 1000);

  num_cameras = aperture_device_manager_get_num_cameras (manager);
  g_assert_cmpint (num_cameras, ==, 2);

  camera = aperture_device_manager_next_camera (manager, camera);
  g_assert_cmpint (camera, ==, 1);

  camera = aperture_device_manager_next_camera (manager, num_cameras - 1);
  g_assert_cmpint (camera, ==, 0);

  camera = aperture_device_manager_next_camera (manager, num_cameras);
  g_assert_cmpint (camera, ==, 0);

  camera = aperture_device_manager_next_camera (manager, num_cameras + 100);
  g_assert_cmpint (camera, ==, 0);
}


void
add_device_manager_tests ()
{
  g_test_add_func ("/device-manager/refcounting", test_device_manager_refcounting);
  g_test_add_func ("/device-manager/works", test_device_manager_works);
  g_test_add_func ("/device-manager/monitoring", test_device_manager_monitoring);
  g_test_add_func ("/device-manager/next-camera", test_device_manager_next_camera);
}
