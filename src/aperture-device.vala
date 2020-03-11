/* aperture-device.vala
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
 * Represents a device, either a camera or microphone.
 */
public abstract class Aperture.Device : Object {
    /**
     * A user-friendly name for this camera device.
     */
    public string name { get; private set; }

    /**
     * The GStreamer device corresponding to this #ApertureDevice.
     */
    public Gst.Device device { get; construct; }


    construct {
        this.name = device.get_display_name();
    }

    protected Device(Gst.Device device) {
        Object(device: device);
    }


    /**
     * Creates a GStreamer source element that provides input from this device
     * to a pipeline.
     */
    public Gst.Element create_gstreamer_source(string? name=null) {
        return this.device.create_element(name);
    }
}
