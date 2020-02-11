/* aperture-camera.vala
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

public class Aperture.Camera : Device {
    internal Camera(Gst.Device device) {
        Object(device: device);
    }


    public List<VideoProfile?> get_available_profiles() {
        var caps = device.get_caps();
        uint size = caps.get_size();
        var profiles = new List<VideoProfile?>();

        for (uint i = 0; i < size; i ++) {
            unowned Gst.Structure structure = caps.get_structure(i);

            int width, height;
            Fraction[] framerates = {};
            string format;

            structure.get_int("width", out width);
            structure.get_int("height", out height);
            format = structure.get_string("format");

            // Framerate might or might not be a list
            int framerate_num, framerate_den;
            bool single_fraction = structure.get_fraction(
                "framerate", out framerate_num, out framerate_den
            );
            if (single_fraction) {
                framerates += Fraction(framerate_num, framerate_den);
            } else {
                ValueArray array;
                structure.get_list("framerate", out array);
                foreach (Value val in array) {
                    framerate_num = Gst.Value.get_fraction_numerator(val);
                    framerate_den = Gst.Value.get_fraction_denominator(val);
                    framerates += Fraction(framerate_num, framerate_den);
                }
            }

            if (format == null) {
                format = structure.get_name();
            }

            foreach (var framerate in framerates) {
                profiles.append(VideoProfile() {
                    width = width,
                    height = height,
                    framerate = framerate,
                    format = format
                });
            }
        }

        return profiles;
    }
}
