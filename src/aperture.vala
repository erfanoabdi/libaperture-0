/* aperture.vala
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

namespace Aperture {
    [CCode]
    private extern void copy_buffer_to_surface(Gst.Buffer buffer, Cairo.ImageSurface surf);


    /**
     * Emits an error if GStreamer is not initialized.
     */
    public void init_check() {
        if (!Gst.is_initialized()) {
            critical("GStreamer is not initialized! Please call gst_init() before using libaperture.");
        }
    }

    public void pretty_print_structure(Gst.Structure structure) {
        print("%s\n", structure.get_name());

        for (int i = 0, n = structure.n_fields(); i < n; i ++) {
            string name = structure.nth_field_name(i);
            Value val = structure.get_value(name);
            print("    %s = %s\n", name, val.strdup_contents());
        }
    }


    [Compact][Immutable]
    public struct Fraction {
        int num;
        int den;

        public Fraction(int num, int den) {
            this.num = num;
            this.den = den;
        }


        public double as_double() {
            return (double) num / (double) den;
        }

        public string to_string() {
            return "%d/%d".printf(num, den);
        }
    }

    [Compact][Immutable]
    public struct Range {
        int min;
        int max;
        int step;

        public Range(int min, int max, int step=1) {
            if (min > max) {
                this.min = min;
                this.max = max;
            } else {
                this.min = max;
                this.max = min;
            }

            this.step = step;
        }

        public Range.from_gst_range(Value range) {
            this.min = Gst.Value.get_int_range_min(range);
            this.max = Gst.Value.get_int_range_max(range);
            this.step = Gst.Value.get_int_range_step(range);
        }


        public string to_string() {
            if (step == 1) {
                return "%d-%d".printf(min, max);
            } else {
                return "%d-%d %% %d".printf(min, max, step);
            }
        }
    }


    /**
     * Scales the rectangle (w, h) up or down to fit perfectly in (tw, th).
     */
    public void scale_to_fit(ref double w, ref double h,
                             double tw, double th) {
        double ratio = w / h;
        double t_ratio = tw / th;
        if (t_ratio < ratio) {
            w = tw;
            h = tw * (1 / ratio);
        } else {
            w = th * ratio;
            h = th;
        }
    }

    /**
     * Scales the rectangle (w, h) to completely fill (tw, th).
     */
    public void scale_to_fill(ref double w, ref double h,
                              double tw, double th) {
        double ratio = w / h;
        double t_ratio = tw / th;
        if (t_ratio > ratio) {
            w = tw;
            h = tw * (1 / ratio);
        } else {
            w = th * ratio;
            h = th;
        }
    }

    /**
     * Same as scale_to_fit(), but does not make the rectangle bigger; if the
     * first rectangle is smaller than the second, no change will be made.
     */
    public void shrink_to_fit(ref double w, ref double h,
                              double tw, double th) {
        double nw = w, nh = h;
        scale_to_fit(ref nw, ref nh, tw, th);
        if (nw < w) {
            w = nw;
            h = nh;
        }
    }

    /**
     * Finds the amount to translate the first rectangle so that it is centered
     * on the second.
     */
    public void center(double w, double h,
                       double tw, double th,
                       out double out_x, out double out_y) {
        out_x = tw / 2 - w / 2;
        out_y = th / 2 - h / 2;
    }
}
