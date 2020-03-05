/* aperture-camera-switcher-button.vala
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


public class Aperture.CameraSwitcherButton : Gtk.Button {
    public signal void camera_changed(Camera new_camera);


    private int _camera_index;
    public int camera_index {
        get {
            return _camera_index;
        }
        set {
            if (value >= _devices.cameras.length()) {
                value = 0;
            }

            if (value != _camera_index) {
                _camera_index = value;
                this.camera_changed(_devices.cameras.nth_data(_camera_index));
            }
        }
    }

    public Camera camera {
        get {
            return _devices.cameras.nth_data(this.camera_index);
        }
        set {
            for (int i = 0, n = (int) _devices.cameras.length(); i < n; i ++) {
                if (_devices.cameras.nth_data(i) == value) {
                    this.camera_index = i;
                    return;
                }
            }
        }
    }


    private DeviceManager _devices;


    construct {
        var icon = new Gtk.Image.from_icon_name("camera-switch", BUTTON);
        icon.visible = true;
        this.add(icon);

        this.no_show_all = true;

        _devices = DeviceManager.get_instance();
        _devices.camera_added.connect(_on_camera_list_changed);
        _devices.camera_removed.connect(_on_camera_list_changed);
        _devices.start();

        // make sure visibility is correct initially
        this._on_camera_list_changed(null);

        this.clicked.connect(_on_clicked);
    }


    private void _on_camera_list_changed(Camera? camera) {
        this.visible = _devices.cameras.length() > 1;
    }

    private void _on_clicked() {
        int index = this.camera_index + 1;

        this.camera_index = index;
    }
}
