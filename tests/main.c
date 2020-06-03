/* test-utils.c
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

#include "private/aperture-private.h"
#include "private/aperture-device-manager-private.h"
#include "dummy-device-provider.h"


void add_barcodes_tests (void);
void add_device_manager_tests (void);
void add_viewfinder_tests (void);


int
main (int argc, char **argv)
{
  aperture_init (&argc, &argv);
  gtk_init (&argc, &argv);
  g_test_init (&argc, &argv, NULL);

  /* Set up the dummy device provider in GStreamer */
  dummy_device_provider_register ();

  add_barcodes_tests ();
  add_device_manager_tests ();
  add_viewfinder_tests ();

  return g_test_run ();
}
