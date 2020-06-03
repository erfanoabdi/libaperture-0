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

#include "private/aperture-device-manager-private.h"
#include "dummy-device-provider.h"
#include "utils.h"


static void
test_viewfinder_no_camera_state ()
{
  g_autoptr(ApertureDeviceManager) manager = aperture_device_manager_get_instance ();
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  g_autoptr(ApertureViewfinder) viewfinder;

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


void
add_viewfinder_tests ()
{
  g_test_add_func ("/viewfinder/no_camera", test_viewfinder_no_camera_state);
}
