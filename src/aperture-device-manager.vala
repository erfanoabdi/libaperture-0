/* aperture-device-manager.vala
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

public class Aperture.DeviceManager : Object {
    /**
     * Emitted when a new camera is discovered.
     */
    public signal void camera_added(Camera camera);

    /**
     * Emitted when a camera is removed (unplugged, turned off, etc.)
     */
    public signal void camera_removed(Camera camera);

    /**
     * Emitted when a new camera is discovered.
     */
    public signal void microphone_added(Microphone microphone);

    /**
     * Emitted when a camera is removed (unplugged, turned off, etc.)
     */
    public signal void microphone_removed(Microphone microphone);


    private Gst.DeviceMonitor _monitor;

    public List<Microphone> microphones = new List<Microphone>();

    public List<Camera> cameras = new List<Camera>();


    construct {
        init();

        this._monitor = new Gst.DeviceMonitor();
        this._monitor.get_bus().add_watch(Priority.DEFAULT, this._on_bus_message);
    }


    public void start() {
        this._monitor.start();

        foreach (var device in this._monitor.get_devices()) {
            this._add_device(device);
        }
    }


    private void _add_device(Gst.Device device) {
        if (!device.has_classes("Source")) {
            return;
        }

        if (!_is_valid_device(device)) {
            return;
        }

        if (device.has_classes("Audio")) {
            var microphone = new Microphone(device);
            this.microphones.append(microphone);
            this.microphone_added(microphone);
        } else if (device.has_classes("Video")) {
            var camera = new Camera(device);
            this.cameras.append(camera);
            this.camera_added(camera);
        }
    }

    private bool _is_valid_device(Gst.Device device) {
        var props = device.get_properties();
        bool is_default = true;
        if (props.get_boolean("is-default", out is_default) && !is_default) {
            return false;
        }

        return true;
    }

    // for your own safety, do not destroy vital testing apparatus
    private void _remove_device(Gst.Device device) {
        if (!device.has_classes("Source")) {
            return;
        }

        if (device.has_classes("Audio")) {
            var microphone = (Microphone) find_by_gst_device(microphones, device);
            if (microphone != null) {
                this.microphones.remove(microphone);
                this.microphone_removed(microphone);
            }
        } else if (device.has_classes("Video")) {
            var camera = (Camera) find_by_gst_device(cameras, device);
            if (camera != null) {
                this.cameras.remove(camera);
                this.camera_removed(camera);
            }
        }
    }

    private Device? find_by_gst_device(List<Device> list, Gst.Device gst_device) {
        Device? result = null;
        foreach (Device device in list) {
            if (device.device == gst_device) {
                result = device;
                break;
            }
        }
        return result;
    }

    private bool _on_bus_message(Gst.Bus bus, Gst.Message message) {
        Gst.Device device;

        switch (message.type) {
        case DEVICE_ADDED:
            message.parse_device_added(out device);
            this._add_device(device);
            break;
        case DEVICE_REMOVED:
            message.parse_device_removed(out device);
            this._remove_device(device);
            break;
        }

        return Source.CONTINUE;
    }
}
