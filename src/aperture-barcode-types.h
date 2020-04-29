/* aperture-barcode-types.h
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


// This list is from https://github.com/ZBar/ZBar/blob/854a5d97059e395807091ac4d80c53f7968abb8f/zbar/symbol.c

/**
 * APERTURE_BARCODE_COMPOSITE:
 * The code is a composite of multiple barcode types.
 */
#define APERTURE_BARCODE_COMPOSITE "COMPOSITE"

/**
 * APERTURE_BARCODE_EAN2:
 * https://en.wikipedia.org/wiki/EAN-2
 */
#define APERTURE_BARCODE_EAN2 "EAN-2"

/**
 * APERTURE_BARCODE_EAN5:
 * https://en.wikipedia.org/wiki/EAN-5
 */
#define APERTURE_BARCODE_EAN5 "EAN-5"

/**
 * APERTURE_BARCODE_EAN8:
 * https://en.wikipedia.org/wiki/EAN-8
 */
#define APERTURE_BARCODE_EAN8 "EAN-8"

/**
 * APERTURE_BARCODE_EAN13:
 * https://en.wikipedia.org/wiki/International_Article_Number
 */
#define APERTURE_BARCODE_EAN13 "EAN-13"

/**
 * APERTURE_BARCODE_UPCA:
 * https://en.wikipedia.org/wiki/Universal_Product_Code
 */
#define APERTURE_BARCODE_UPCA "UPC-A"

/**
 * APERTURE_BARCODE_UPCE:
 * https://en.wikipedia.org/wiki/Universal_Product_Code#UPC-E
 */
#define APERTURE_BARCODE_UPCE "UPC-E"

/**
 * APERTURE_BARCODE_ISBN10:
 * https://en.wikipedia.org/wiki/International_Standard_Book_Number
 */
#define APERTURE_BARCODE_ISBN10 "ISBN-10"

/**
 * APERTURE_BARCODE_ISBN13:
 * https://en.wikipedia.org/wiki/International_Standard_Book_Number
 */
#define APERTURE_BARCODE_ISBN13 "ISBN-13"

/**
 * APERTURE_BARCODE_I25:
 * https://en.wikipedia.org/wiki/Interleaved_2_of_5
 */
#define APERTURE_BARCODE_I25 "I2/5"

/**
 * APERTURE_BARCODE_DATABAR:
 * https://en.wikipedia.org/wiki/GS1_DataBar
 */
#define APERTURE_BARCODE_DATABAR "DataBar"

/**
 * APERTURE_BARCODE_DATABAR_EXP:
 * https://en.wikipedia.org/wiki/GS1_DataBar
 */
#define APERTURE_BARCODE_DATABAR_EXP "DataBar-Exp"

/**
 * APERTURE_BARCODE_CODABAR:
 * https://en.wikipedia.org/wiki/Codabar
 */
#define APERTURE_BARCODE_CODABAR "Codabar"

/**
 * APERTURE_BARCODE_CODE39:
 * https://en.wikipedia.org/wiki/Code_39
 */
#define APERTURE_BARCODE_CODE39 "CODE-39"

/**
 * APERTURE_BARCODE_CODE93:
 * https://en.wikipedia.org/wiki/Code_93
 */
#define APERTURE_BARCODE_CODE93 "CODE-93"

/**
 * APERTURE_BARCODE_CODE128:
 * https://en.wikipedia.org/wiki/Code_128
 */
#define APERTURE_BARCODE_CODE128 "CODE-128"

/**
 * APERTURE_BARCODE_PDF417:
 * https://en.wikipedia.org/wiki/PDF417
 */
#define APERTURE_BARCODE_PDF417 "PDF417"

/**
 * APERTURE_BARCODE_QR:
 * https://en.wikipedia.org/wiki/QR_code
 */
#define APERTURE_BARCODE_QR "QR-Code"

G_END_DECLS
