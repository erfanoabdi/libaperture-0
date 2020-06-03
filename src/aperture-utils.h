/* aperture-utils.h
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


#pragma once

#include <glib-object.h>

#if !defined(_LIBAPERTURE_INSIDE) && !defined(_LIBAPERTURE_COMPILATION)
#error "Only <aperture.h> can be included directly."
#endif


G_BEGIN_DECLS


typedef enum {
  APERTURE_BARCODE_UNKNOWN,
  APERTURE_BARCODE_COMPOSITE,
  APERTURE_BARCODE_EAN2,
  APERTURE_BARCODE_EAN5,
  APERTURE_BARCODE_EAN8,
  APERTURE_BARCODE_EAN13,
  APERTURE_BARCODE_UPCA,
  APERTURE_BARCODE_UPCE,
  APERTURE_BARCODE_ISBN10,
  APERTURE_BARCODE_ISBN13,
  APERTURE_BARCODE_I25,
  APERTURE_BARCODE_DATABAR,
  APERTURE_BARCODE_DATABAR_EXP,
  APERTURE_BARCODE_CODABAR,
  APERTURE_BARCODE_CODE39,
  APERTURE_BARCODE_CODE93,
  APERTURE_BARCODE_CODE128,
  APERTURE_BARCODE_PDF417,
  APERTURE_BARCODE_QR,
} ApertureBarcode;


void            aperture_init                         (int *argc,
                                                       char ***argv);
gboolean        aperture_is_initialized               (void);
gboolean        aperture_is_barcode_detection_enabled (void);

ApertureBarcode aperture_barcode_type_from_string     (const char *string);

G_END_DECLS
