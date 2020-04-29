/* aperture-barcode-result.c
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

/**
 * ApertureBarcodeResult:
 *
 * #ApertureBarcodeResult represents a barcode that has been scanned from
 * a camera feed.
 *
 * The code types are taken directly from the ZBar library, which is used
 * internally for scanning barcodes. See the APERTURE_BARCODE_TYPE_* macros
 * for a list of potential values. Always make sure the barcode is of the type
 * your application expects!
 *
 * The data is generally in text format--a UPC code, for example, will be
 * represented as ASCII numerals, not a binary representation.
 */


#include "aperture-barcode-result.h"

G_DEFINE_BOXED_TYPE (ApertureBarcodeResult, aperture_barcode_result, aperture_barcode_result_copy, aperture_barcode_result_free)


/**
 * aperture_barcode_result_new:
 * @code_type: The type of code
 * @data: The data contained in the barcode
 *
 * Creates a new #ApertureBarcodeResult.
 *
 * Returns: (transfer full): A newly created #ApertureBarcodeResult
 */
ApertureBarcodeResult *
aperture_barcode_result_new (const char *code_type, const char *data)
{
  ApertureBarcodeResult *self;

  g_return_val_if_fail (code_type != NULL, NULL);
  g_return_val_if_fail (data != NULL, NULL);

  self = g_slice_new0 (ApertureBarcodeResult);
  self->code_type = g_strdup (code_type);
  self->data = g_strdup (data);
  self->priv = NULL;

  return self;
}


/**
 * aperture_barcode_result_copy:
 * @self: an #ApertureBarcodeResult
 *
 * Makes a deep copy of an #ApertureBarcodeResult.
 *
 * Returns: (transfer full): A newly created #ApertureBarcodeResult with the same
 *   contents as @self
 */
ApertureBarcodeResult *
aperture_barcode_result_copy (ApertureBarcodeResult *self)
{
  ApertureBarcodeResult *copy;

  g_return_val_if_fail (self, NULL);

  copy = aperture_barcode_result_new (self->code_type, self->data);

  return copy;
}


/**
 * aperture_barcode_result_free:
 * @self: an #ApertureBarcodeResult
 *
 * Frees an #ApertureBarcodeResult allocated using aperture_barcode_result_new()
 * or aperture_barcode_result_copy().
 */
void
aperture_barcode_result_free (ApertureBarcodeResult *self)
{
  g_return_if_fail (self);

  g_free (self->code_type);
  g_free (self->data);

  g_slice_free (ApertureBarcodeResult, self);
}

