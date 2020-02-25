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
    [CCode (cname="aperture_init")]
    public extern void init();


    [CCode (cname="aperture_get_window_handle")]
    private extern void* get_window_handle(Gdk.Window window);

    [CCode]
    private extern void* get_wayland_display_handle();

    [CCode (cname="aperture_is_wayland_display")]
    private extern bool is_wayland_display();

    [CCode (cname="aperture_create_wayland_context")]
    private extern Gst.Context create_wayland_context();


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
}
