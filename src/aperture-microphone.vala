/* aperture-microphone.vala
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

public class Aperture.Microphone : Device {
    internal Microphone(Gst.Device device) {
        Object(device: device);
    }


    public List<AudioProfile?> get_available_profiles() {
        var caps = device.get_caps();
        uint size = caps.get_size();
        var profiles = new List<AudioProfile?>();

        for (uint i = 0; i < size; i ++) {
            unowned Gst.Structure structure = caps.get_structure(i);

            string[] formats = {};
            Range channels;
            Range bitrate;

            channels = Range.from_gst_range(structure.get_value("channels"));
            bitrate = Range.from_gst_range(structure.get_value("rate"));

            ValueArray format_array;
            string? format = structure.get_string("format");
            if (format != null) {
                formats += format;
            } else if (structure.get_list("format", out format_array)) {
                foreach (Value format_item in format_array) {
                    formats += format_item.get_string();
                }
            } else {
                formats += structure.get_name();
            }

            foreach (var format_item in formats) {
                profiles.append(AudioProfile() {
                    format = format_item,
                    channels = channels,
                    bitrate = bitrate
                });
            }
        }

        return profiles;
    }
}
