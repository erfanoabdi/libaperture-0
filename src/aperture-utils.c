/* aperture-utils.c
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


#include <gst/gst.h>
#include <gtk/gtk.h>

#include "aperture-build-info.h"
#include "aperture-utils.h"


#define BOOL_STR(x) (x ? "TRUE" : "FALSE")


static gboolean initialized = FALSE;
static gboolean init_warning = FALSE;


/**
 * SECTION:aperture-utils
 * @title: Miscellaneous Utilities
 * @short_description: Miscellaneous utility functions and enums
 */


/**
 * aperture_init:
 * @argc: (inout) (optional): pointer to the number of command line arguments,
 *        or %NULL
 * @argv: (array length=argc) (inout) (nullable) (optional) (transfer none):
 *        pointer to the program's command line arguments, or %NULL
 *
 * Initializes the Aperture library, if it hasn't been initialized already.
 *
 * This will initialize GStreamer for you. If you don't want this, initialize
 * GStreamer yourself before calling aperture_init().
 *
 * Since: 0.1
 */
void
aperture_init (int *argc, char ***argv)
{
  if (initialized) {
    return;
  }

  if (!gst_is_initialized ()) {
    gst_init (argc, argv);
  }

  initialized = TRUE;
}


/**
 * aperture_is_initialized:
 *
 * Gets whether Aperture is initialized.
 *
 * Returns: %TRUE if the library is initialized, otherwise %FALSE
 * Since: 0.1
 */
gboolean
aperture_is_initialized (void)
{
  return initialized;
}


/**
 * PRIVATE:aperture_private_ensure_initialized:
 *
 * Emits an error on the console if Aperture is not initialized.
 *
 * Since: 0.1
 */
void
aperture_private_ensure_initialized (void)
{
  if (!initialized && !init_warning) {
    g_critical ("Aperture is not initialized! Please call aperture_init() before using the rest of the library to avoid errors and crashes.");
    init_warning = TRUE;
  }
}


/**
 * aperture_is_barcode_detection_enabled:
 *
 * Determines whether the barcode detection features of Aperture are
 * enabled.
 *
 * This is based on whether the `zbar` element is available to GStreamer.
 * It is part of the gst-plugins-bad package. Note that many distributions
 * don't enable the zbar component of gst-plugins-bad by default, because
 * it needs an extra dependency (the zbar library). You may need to find
 * a gst-plugins-bad-extras package or similar, or compile that particular
 * plugin yourself. For a Flatpak example, see the demo application in
 * Aperture's source code.
 *
 * Note that Aperture itself does *not* need to be recompiled to enable
 * barcode detection. It is based solely on whether the GStreamer plugin
 * is available.
 *
 * Returns: %TRUE if barcode detection is available, otherwise %FALSE
 * Since: 0.1
 */
gboolean
aperture_is_barcode_detection_enabled (void)
{
  g_autoptr(GstElementFactory) factory = gst_element_factory_find ("zbar");
  return factory != NULL;
}


/* Convenience function to read the contents of a file, and fail gracefully by
 * returning an empty string. */
static char *
read_file (const char *filename) {
  g_autoptr(GFile) file = g_file_new_for_path (filename);
  g_autoptr(GError) err = NULL;
  char *contents = NULL;

  g_file_load_contents (file, NULL, &contents, NULL, NULL, &err);

  if (err) {
    return g_strdup ("");
  } else {
    return contents;
  }
}


/**
 * aperture_get_diagnostic_info:
 *
 * Gets a string containing useful debugging information, suitable for
 * including in bug reports, for example. This could include versions of
 * relevant libraries, basic software and hardware information, etc.
 *
 * No guarantees are made about the format of the string.
 *
 * Returns: (transfer full): the diagnostic string. Free with g_free().
 * Since: 0.1
 */
char *
aperture_get_diagnostic_info (void)
{
  g_autoptr(GstDeviceMonitor) monitor = NULL;
  g_autolist(GstDevice) devices = NULL;
  g_autoptr(GString) device_info = g_string_new (NULL);
  g_autofree char *etc_os_release = read_file ("/etc/os-release");

  if (gst_is_initialized ()) {
    int n = 0;

    monitor = gst_device_monitor_new ();
    gst_device_monitor_add_filter (monitor, "Source/Video", NULL);
    gst_device_monitor_start (monitor);
    devices = gst_device_monitor_get_devices (monitor);
    gst_device_monitor_stop (monitor);

    for (GList *l = devices; l != NULL; l = l->next) {
      GstDevice *device = l->data;
      g_autoptr(GstStructure) props = gst_device_get_properties (device);
      g_autoptr(GstCaps) caps = gst_device_get_caps (device);

      g_string_append_printf (
        device_info,
        "  [devices.%d]\n"
        "    name = %s\n"
        "    properties = %s\n"
        "    caps = %s\n"
        ,
        n ++,
        gst_object_get_name (GST_OBJECT (device)),
        gst_structure_to_string (props),
        gst_caps_to_string (caps)
      );
    }
  }

  return g_strdup_printf (
    "[/etc/os-release]\n"
    "%s\n"
    "[GLib]\n"
    "  version = %d.%d.%d\n"

    "[GTK]\n"
    "  version = %d.%d.%d\n"

    "[GStreamer]\n"
    "  version = %d.%d.%d\n"
    "  initialized = %s\n"

    "[Aperture]\n"
    "  version = %d.%d.%d\n"
    "  initialized = %s\n"
    "  zbar_enabled = %s\n"
    "%s"
    ,
    etc_os_release,
    GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION,
    GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION,
    GST_VERSION_MAJOR, GST_VERSION_MINOR, GST_VERSION_MICRO,
    BOOL_STR (gst_is_initialized ()),
    APERTURE_MAJOR_VERSION, APERTURE_MINOR_VERSION, APERTURE_MICRO_VERSION,
    BOOL_STR (aperture_is_initialized ()),
    BOOL_STR (aperture_is_barcode_detection_enabled ()),
    device_info->str
  );
}


/**
 * ApertureBarcode:
 * @APERTURE_BARCODE_UNKNOWN: A barcode was detected, but Aperture does not recognize its type.
 * @APERTURE_BARCODE_COMPOSITE: The code is a composite of multiple barcode types.
 * @APERTURE_BARCODE_EAN2: <https://en.wikipedia.org/wiki/EAN-2>
 * @APERTURE_BARCODE_EAN5: <https://en.wikipedia.org/wiki/EAN-5>
 * @APERTURE_BARCODE_EAN8: <https://en.wikipedia.org/wiki/EAN-8>
 * @APERTURE_BARCODE_EAN13: <https://en.wikipedia.org/wiki/International_Article_Number>
 * @APERTURE_BARCODE_UPCA: <https://en.wikipedia.org/wiki/Universal_Product_Code>
 * @APERTURE_BARCODE_UPCE: <https://en.wikipedia.org/wiki/Universal_Product_Code#UPC-E>
 * @APERTURE_BARCODE_ISBN10: <https://en.wikipedia.org/wiki/International_Standard_Book_Number>
 * @APERTURE_BARCODE_ISBN13: <https://en.wikipedia.org/wiki/International_Standard_Book_Number>
 * @APERTURE_BARCODE_I25: <https://en.wikipedia.org/wiki/Interleaved_2_of_5>
 * @APERTURE_BARCODE_DATABAR: <https://en.wikipedia.org/wiki/GS1_DataBar>
 * @APERTURE_BARCODE_DATABAR_EXP: <https://en.wikipedia.org/wiki/GS1_DataBar>
 * @APERTURE_BARCODE_CODABAR: <https://en.wikipedia.org/wiki/Codabar>
 * @APERTURE_BARCODE_CODE39: <https://en.wikipedia.org/wiki/Code_39>
 * @APERTURE_BARCODE_CODE93: <https://en.wikipedia.org/wiki/Code_93>
 * @APERTURE_BARCODE_CODE128: <https://en.wikipedia.org/wiki/Code_128>
 * @APERTURE_BARCODE_PDF417: <https://en.wikipedia.org/wiki/PDF417>
 * @APERTURE_BARCODE_QR: <https://en.wikipedia.org/wiki/QR_code>
 *
 * Represents the type of a barcode detected in a video stream.
 *
 * Different barcode types are used for different purposes and different types
 * of data, so it is important to check a barcode's type before attempting to
 * use its data.
 *
 * Since: 0.1
 */


/**
 * PRIVATE:aperture_barcode_type_from_string:
 * @string: a barcode type string from ZBar
 *
 * Takes a string representing a barcode type from ZBar and returns the
 * matching #ApertureBarcode value.
 *
 * Returns: the barcode enum, or %APERTURE_BARCODE_UNKNOWN if the type is
 * not recognized
 */
ApertureBarcode
aperture_barcode_type_from_string (const char *string)
{
  // This list is from https://github.com/ZBar/ZBar/blob/854a5d97059e395807091ac4d80c53f7968abb8f/zbar/symbol.c

  if (g_strcmp0 (string, "COMPOSITE") == 0) {
    return APERTURE_BARCODE_COMPOSITE;
  } else if (g_strcmp0 (string, "EAN-2") == 0) {
    return APERTURE_BARCODE_EAN2;
  } else if (g_strcmp0 (string, "EAN-5") == 0) {
    return APERTURE_BARCODE_EAN5;
  } else if (g_strcmp0 (string, "EAN-8") == 0) {
    return APERTURE_BARCODE_EAN8;
  } else if (g_strcmp0 (string, "EAN-13") == 0) {
    return APERTURE_BARCODE_EAN13;
  } else if (g_strcmp0 (string, "UPC-A") == 0) {
    return APERTURE_BARCODE_UPCA;
  } else if (g_strcmp0 (string, "UPC-E") == 0) {
    return APERTURE_BARCODE_UPCE;
  } else if (g_strcmp0 (string, "ISBN-10") == 0) {
    return APERTURE_BARCODE_ISBN13;
  } else if (g_strcmp0 (string, "ISBN-13") == 0) {
    return APERTURE_BARCODE_ISBN10;
  } else if (g_strcmp0 (string, "I2/5") == 0) {
    return APERTURE_BARCODE_I25;
  } else if (g_strcmp0 (string, "DataBar") == 0) {
    return APERTURE_BARCODE_DATABAR;
  } else if (g_strcmp0 (string, "DataBar-Exp") == 0) {
    return APERTURE_BARCODE_DATABAR_EXP;
  } else if (g_strcmp0 (string, "Codabar") == 0) {
    return APERTURE_BARCODE_CODABAR;
  } else if (g_strcmp0 (string, "CODE-39") == 0) {
    return APERTURE_BARCODE_CODE39;
  } else if (g_strcmp0 (string, "CODE-93") == 0) {
    return APERTURE_BARCODE_CODE93;
  } else if (g_strcmp0 (string, "CODE-128") == 0) {
    return APERTURE_BARCODE_CODE128;
  } else if (g_strcmp0 (string, "PDF417") == 0) {
    return APERTURE_BARCODE_PDF417;
  } else if (g_strcmp0 (string, "QR-Code") == 0) {
    return APERTURE_BARCODE_QR;
  } else {
    return APERTURE_BARCODE_UNKNOWN;
  }
}
