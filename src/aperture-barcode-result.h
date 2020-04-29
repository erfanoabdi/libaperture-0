/* aperture-barcode-result.h
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

#if !defined(_LIBAPERTURE_INSIDE) && !defined(_LIBAPERTURE_COMPILATION)
#error "Only <aperture.h> can be included directly."
#endif

#include <glib-object.h>


G_BEGIN_DECLS


#define APERTURE_TYPE_BARCODE_RESULT (aperture_barcode_result_get_type ())

typedef struct _ApertureBarcodeResult ApertureBarcodeResult;

struct _ApertureBarcodeResult
{
  char *code_type;
  char *data;

  /*< private >*/
  gpointer priv;
};

GType                      aperture_barcode_result_get_type (void) G_GNUC_CONST;
ApertureBarcodeResult     *aperture_barcode_result_new      (const char            *code_type,
                                                             const char            *data);
ApertureBarcodeResult     *aperture_barcode_result_copy     (ApertureBarcodeResult *self);
void                       aperture_barcode_result_free     (ApertureBarcodeResult *self);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (ApertureBarcodeResult, aperture_barcode_result_free)


G_END_DECLS
