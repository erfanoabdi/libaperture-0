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


/**
 * A button for switching between available cameras.
 *
 * Clicking the button cycles between cameras. If zero or one cameras are
 * present, then the button will become invisible. Thus, applications should
 * not set the widget's visibility themselves, but use a wrapper container if
 * necessary.
 */
public class Aperture.CameraSwitcherButton : Gtk.Button {
    public signal void camera_changed(Camera new_camera);


    private int _camera_index;
    public int camera_index {
        get {
            return _camera_index;
        }
        set {
            if (value >= this.devices.cameras.length()) {
                value = 0;
            }

            if (value != _camera_index) {
                _camera_index = value;
                this.camera_changed(this.devices.cameras.nth_data(_camera_index));
            }
        }
    }

    public Camera camera {
        get {
            return this.devices.cameras.nth_data(this.camera_index);
        }
        set {
            for (int i = 0, n = (int) this.devices.cameras.length(); i < n; i ++) {
                if (this.devices.cameras.nth_data(i) == value) {
                    this.camera_index = i;
                    return;
                }
            }
        }
    }


    private DeviceManager devices;


    construct {
        var icon = new Gtk.Image.from_icon_name("camera-switch", BUTTON);
        icon.visible = true;
        this.add(icon);

        this.no_show_all = true;

        this.devices = DeviceManager.get_instance();
        this.devices.camera_added.connect(on_camera_list_changed);
        this.devices.camera_removed.connect(on_camera_list_changed);
        this.devices.start();

        // make sure visibility is correct initially
        this.on_camera_list_changed(null);

        this.clicked.connect(on_clicked);
    }


    private void on_camera_list_changed(Camera? camera) {
        this.visible = this.devices.cameras.length() > 1;
    }

    private void on_clicked() {
        int index = this.camera_index + 1;

        this.camera_index = index;
    }
}
