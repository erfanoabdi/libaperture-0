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
    private bool initialized = false;

    /**
     * Initializes the Aperture library, along with several libraries it
     * depends on. This should be done before any part of the API is used.
     *
     * It is safe to call this multiple times.
     */
    public void init(
            [CCode (array_length_pos=0, array_length_cname="argc", cname="argv")]
            ref unowned string[] args) {

        if (initialized) {
            return;
        }

        Gst.init(ref args);
        GtkClutter.init(ref args);
        ClutterGst.init(ref args);

        initialized = true;
    }

    /**
     * Checks whether aperture_init() has been called.
     */
    public bool is_initialized() {
        return initialized;
    }

    /*
     * Emits an error if Aperture is not initialized.
     */
    private void init_check() {
        if (!initialized) {
            critical("Aperture is not initialized! Please call aperture_init()"
                     + " before using the rest of the library to avoid errors"
                     + " and crashes.");
        }
    }

    /**
     * A debugging tool that prints a #GstStructure to stdout in an easily
     * readable format.
     */
    public void pretty_print_structure(Gst.Structure structure) {
        print("%s\n", structure.get_name());

        for (int i = 0, n = structure.n_fields(); i < n; i ++) {
            string name = structure.nth_field_name(i);
            Value val = structure.get_value(name);
            print("    %s = %s\n", name, val.strdup_contents());
        }
    }


    /**
     * Represents a fraction.
     *
     * This is used for video framerates, among other things.
     */
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

    /**
     * Represents a range of integers.
     */
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


    /*
     * Scales the rectangle (w, h) up or down to fit perfectly in (tw, th).
     */
    private static void scale_to_fit(ref double w, ref double h,
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

    /*
     * Scales the rectangle (w, h) to completely fill (tw, th).
     */
    private static void scale_to_fill(ref double w, ref double h,
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

    /*
     * Finds the amount to translate the first rectangle so that it is centered
     * on the second.
     */
    private static void center(double w, double h,
                               double tw, double th,
                               out double out_x, out double out_y) {
        out_x = tw / 2 - w / 2;
        out_y = th / 2 - h / 2;
    }

    /*
     * Draws a rounded square on a Cairo context.
     *
     * It is up to the caller to call fill() or paint(); this just sets up the
     * path.
     *
     * x and y are the coordinates of the center of the square.
     */
    private void rounded_square(Cairo.Context ctx, double size, double radius, double x, double y) {
        x -= size;
        y -= size;
        size *= 2.0;

        // top right
        ctx.arc(x + size - radius, y + radius, radius, -0.5 * Math.PI, 0);
        // bottom right
        ctx.arc(x + size - radius, y + size - radius, radius, 0, 0.5 * Math.PI);
        // bottom left
        ctx.arc(x + radius, y + size - radius, radius, 0.5 * Math.PI, Math.PI);
        // top left
        ctx.arc(x + radius, y + radius, radius, Math.PI, -0.5 * Math.PI);
        // back to top right
        ctx.line_to(x + size - radius, y);
    }
}
