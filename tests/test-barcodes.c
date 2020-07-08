/* test-barcodes.c
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
test_barcodes_enum ()
{
  g_assert_cmpint (aperture_barcode_type_from_string ("COMPOSITE"), ==, APERTURE_BARCODE_COMPOSITE);
  g_assert_cmpint (aperture_barcode_type_from_string ("DataBar"), ==, APERTURE_BARCODE_DATABAR);
  g_assert_cmpint (aperture_barcode_type_from_string ("QR-Code"), ==, APERTURE_BARCODE_QR);
  g_assert_cmpint (aperture_barcode_type_from_string ("I2/5"), ==, APERTURE_BARCODE_I25);
  g_assert_cmpint (aperture_barcode_type_from_string ("three zebras walking into a bar"), ==, APERTURE_BARCODE_UNKNOWN);
}


static void
test_barcodes_enabled ()
{
  g_test_summary ("Test that barcode detection is enabled in the test environment");

#ifdef BARCODE_TESTS_SKIPPABLE
  if (!aperture_is_barcode_detection_enabled ()) {
    g_test_skip ("Skipping test that requires barcode detection, because it is not available");
    return;
  }
#endif

  g_assert_true (aperture_is_barcode_detection_enabled ());
}


static void
barcode_detected_cb (TestUtilsCallback *cb, ApertureBarcode barcode, char *data)
{
  g_assert_cmpstr (data, ==, "hello world");
  g_assert_cmpint (barcode, ==, APERTURE_BARCODE_QR);
  testutils_callback_call (cb);
}


static void
test_barcodes_detection ()
{
  g_autoptr(DummyDeviceProvider) provider = DUMMY_DEVICE_PROVIDER (gst_device_provider_factory_get_by_name ("dummy-device-provider"));
  ApertureViewfinder *viewfinder;
  GtkWidget *window;
  TestUtilsCallback detected_callback;
  DummyDevice *device;

#ifdef BARCODE_TESTS_SKIPPABLE
  if (!aperture_is_barcode_detection_enabled ()) {
    g_test_skip ("Skipping test that requires barcode detection, because it is not available");
    return;
  }
#endif

  testutils_callback_init (&detected_callback);

  device = dummy_device_provider_add (provider);
  dummy_device_set_image (device, "/aperture/helloworld.png");

  viewfinder = aperture_viewfinder_new ();
  g_signal_connect_swapped (viewfinder, "barcode-detected", G_CALLBACK (barcode_detected_cb), &detected_callback);

  aperture_viewfinder_set_detect_barcodes (viewfinder, TRUE);
  /* make sure that worked */
  g_assert_true (aperture_viewfinder_get_detect_barcodes (viewfinder));

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (viewfinder));

  gtk_widget_show_all (window);

  testutils_callback_assert_called (&detected_callback, 1000);

  aperture_viewfinder_set_detect_barcodes (viewfinder, FALSE);
  aperture_viewfinder_set_detect_barcodes (viewfinder, TRUE);
  testutils_callback_assert_called (&detected_callback, 1000);

  gtk_widget_destroy (window);
}


void
add_barcodes_tests ()
{
  g_test_add_func ("/barcodes/enum", test_barcodes_enum);
  g_test_add_func ("/barcodes/enabled", test_barcodes_enabled);
  g_test_add_func ("/barcodes/detection", test_barcodes_detection);
}
