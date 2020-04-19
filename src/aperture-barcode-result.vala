/* aperture-barcode-result.vala
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
 * Represents the result of scanning a barcode.
 */
[Compact][Immutable]
public struct Aperture.BarcodeResult {
    /**
     * The type of barcode that was scanned.
     *
     * This is taken from ZBar's zbar_get_symbol_name(). See the
     * APERTURE_BARCODE_RESULT_TYPE_* constants for valid types.
     */
    public string type;

    /**
     * The data contained in the barcode.
     */
    public string data;


    // This list is from https://github.com/ZBar/ZBar/blob/854a5d97059e395807091ac4d80c53f7968abb8f/zbar/symbol.c

    /**
     * The code is a composite of multiple barcode types.
     */
    public const string TYPE_COMPOSITE = "COMPOSITE";

    /**
     * https://en.wikipedia.org/wiki/EAN-2
     */
    public const string TYPE_EAN2 = "EAN-2";

    /**
     * https://en.wikipedia.org/wiki/EAN-5
     */
    public const string TYPE_EAN5 = "EAN-5";

    /**
     * https://en.wikipedia.org/wiki/EAN-8
     */
    public const string TYPE_EAN8 = "EAN-8";

    /**
     * https://en.wikipedia.org/wiki/International_Article_Number
     */
    public const string TYPE_EAN13 = "EAN-13";

    /**
     * https://en.wikipedia.org/wiki/Universal_Product_Code
     */
    public const string TYPE_UPCA = "UPC-A";

    /**
     * https://en.wikipedia.org/wiki/Universal_Product_Code#UPC-E
     */
    public const string TYPE_UPCE = "UPC-E";

    /**
     * https://en.wikipedia.org/wiki/International_Standard_Book_Number
     */
    public const string TYPE_ISBN10 = "ISBN-10";

    /**
     * https://en.wikipedia.org/wiki/International_Standard_Book_Number
     */
    public const string TYPE_ISBN13 = "ISBN-13";

    /**
     * https://en.wikipedia.org/wiki/Interleaved_2_of_5
     */
    public const string TYPE_I25 = "I2/5";

    /**
     * https://en.wikipedia.org/wiki/GS1_DataBar
     */
    public const string TYPE_DATABAR = "DataBar";

    /**
     * https://en.wikipedia.org/wiki/GS1_DataBar
     */
    public const string TYPE_DATABAR_EXP = "DataBar-Exp";

    /**
     * https://en.wikipedia.org/wiki/Codabar
     */
    public const string TYPE_CODABAR = "Codabar";

    /**
     * https://en.wikipedia.org/wiki/Code_39
     */
    public const string TYPE_CODE39 = "CODE-39";

    /**
     * https://en.wikipedia.org/wiki/Code_93
     */
    public const string TYPE_CODE93 = "CODE-93";

    /**
     * https://en.wikipedia.org/wiki/Code_128
     */
    public const string TYPE_CODE128 = "CODE-128";

    /**
     * https://en.wikipedia.org/wiki/PDF417
     */
    public const string TYPE_PDF417 = "PDF417";

    /**
     * https://en.wikipedia.org/wiki/QR_code
     */
    public const string TYPE_QR = "QR-Code";
}
