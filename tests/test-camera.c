/* test-camera.c
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

#include "utils.h"
#include "dummy-device-provider.h"


static void
on_camera_flash (ApertureCamera *source, GAsyncResult *res, TestUtilsCallback *callback)
{
  g_autoptr(GError) err = NULL;

  gboolean result = aperture_camera_do_flash_finish (source, res, &err);
  g_assert_no_error (err);

  // default implementation returns %FALSE since there's no flash to enable
  g_assert_false (result);

  testutils_callback_call (callback);
}


static void
test_camera_flash ()
{
  g_autoptr(ApertureDeviceManager) manager = aperture_device_manager_get_instance ();
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  g_autoptr(ApertureCamera) camera = NULL;
  TestUtilsCallback flash_callback;

  testutils_callback_init (&flash_callback);

  dummy_device_provider_add (provider);
  testutils_wait_for_device_change (manager);

  camera = aperture_device_manager_next_camera (manager, NULL);
  g_assert_nonnull (camera);

  aperture_camera_do_flash_async (camera, NULL, (GAsyncReadyCallback) on_camera_flash, &flash_callback);
  testutils_callback_assert_called (&flash_callback, 1000);

  aperture_camera_set_torch (camera, TRUE);
  aperture_camera_set_torch (camera, FALSE);
}


void
add_camera_tests ()
{
  g_test_add_func ("/camera/flash", test_camera_flash);
}
