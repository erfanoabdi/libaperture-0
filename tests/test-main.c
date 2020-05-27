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


static void
test_init ()
{
  g_assert_false (aperture_is_initialized ());
  aperture_init (NULL, NULL);
  g_assert_true (aperture_is_initialized ());
}


static void
test_barcodes_enum ()
{
  g_assert_cmpint (aperture_barcode_type_from_string ("COMPOSITE"), ==, APERTURE_BARCODE_COMPOSITE);
  g_assert_cmpint (aperture_barcode_type_from_string ("DataBar"), ==, APERTURE_BARCODE_DATABAR);
  g_assert_cmpint (aperture_barcode_type_from_string ("QR-Code"), ==, APERTURE_BARCODE_QR);
  g_assert_cmpint (aperture_barcode_type_from_string ("I2/5"), ==, APERTURE_BARCODE_I25);
  g_assert_cmpint (aperture_barcode_type_from_string ("three zebras walking into a bar"), ==, APERTURE_BARCODE_UNKNOWN);
}


int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/init", test_init);

  g_test_add_func ("/barcodes/enum", test_barcodes_enum);

  return g_test_run ();
}
