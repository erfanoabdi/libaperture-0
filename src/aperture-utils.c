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


static gboolean initialized = FALSE;
static gboolean init_warning = FALSE;


/**
 * aperture_init:
 * @argc: pointer to the number of command line arguments, or %NULL
 * @argv: pointer to the program's command line arguments, or %NULL
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
 * aperture_barcode_detection_enabled:
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

