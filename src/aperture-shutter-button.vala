/* aperture-camera-button.vala
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
 * A shutter button, like the ones seen in most camera apps.
 *
 * The button supports many different modes, such as taking pictures, taking
 * video, active recording, etc. It is up to the application to manage these
 * modes and perform the right actions when the button is clicked.
 * #ApertureShutterButton is little more than a #GtkButton with custom drawing,
 * and it does not operate an #ApertureWidget automatically.
 */
public class Aperture.ShutterButton : Gtk.Button {
    private ShutterButtonMode _mode;
    /**
     * The button's mode, which determines its appearance.
     */
    public ShutterButtonMode mode {
        get {
            return _mode;
        }
        set {
            if (_mode == value) {
                return;
            }

            this._set_mode(value);
            _mode = value;
        }
    }


    private Tween tween;


    construct {
        this.draw.connect(this.on_draw);

        this.tween = new Tween(this);
        this.tween.start("press", 3, 0);
        this.tween.start("mode", 1, 0);
        this.tween.start("record", 0, 0);

        this._mode = PICTURE;

        this.state_flags_changed.connect(this.on_state_flags_changed);
    }


    private void _set_mode(ShutterButtonMode mode) {
        if (mode == PICTURE) {
            this.tween["mode"] = 1;
            this.tween["record"] = 0;
        } else if (mode == VIDEO) {
            this.tween["mode"] = 0;
            this.tween["record"] = 0;
        } else if (mode == RECORDING) {
            this.tween["mode"] = 0;
            this.tween["record"] = 1;
        }
    }

    private void on_state_flags_changed(Gtk.Widget widget, Gtk.StateFlags old) {
        if (Gtk.StateFlags.ACTIVE in this.get_state_flags()) {
            if (!(Gtk.StateFlags.ACTIVE in old)) {
                this.tween.start("press", 6, 125);
            }
        } else if (!(Gtk.StateFlags.ACTIVE in this.get_state_flags())) {
            if (Gtk.StateFlags.ACTIVE in old) {
                this.tween.start("press", 3, 125);
            }
        }
    }

    private bool on_draw(Cairo.Context ctx) {
        int width = this.get_allocated_width();
        int height = this.get_allocated_height();
        int size = int.min(width, height);

        int line = int.min(8, size / 8);

        double color = this.sensitive ? 1 : 0.5;
        double mode_color = this.tween["mode"];

        ctx.set_line_width(line);
        ctx.set_source_rgb(color, color, color);
        ctx.arc(width / 2.0, height / 2.0, (size - line) / 2, 0, 2 * Math.PI);
        ctx.stroke();

        ctx.set_source_rgb(color, mode_color, mode_color);

        double record = this.tween["record"];
        double gap = this.tween["press"];

        if (record == 0) {
            ctx.arc(width / 2.0, height / 2.0, size / 2 - line - gap, 0, 2 * Math.PI);
        } else {
            gap += 3 * record;
            double sq_radius = double.max((1 - record) * size / 2.0 - line - gap, 3);
            double sq_size = Math.sqrt(
                Math.pow((size / 2.0 - line - gap) - sq_radius + Math.sqrt(2 * Math.pow(sq_radius, 2)), 2)
                / 2.0
            );

            rounded_square(ctx, sq_size, sq_radius, width / 2.0, height / 2.0);
        }

        ctx.fill();

        return true;
    }

    /*
     * Gtk.Button.size_allocate sometimes messes with the clip size, which we
     * don't want, so we need to make sure to set it back.
     */
    public override void size_allocate(Gtk.Allocation alloc) {
        base.size_allocate(alloc);
        this.set_clip(alloc);
    }
}

public enum Aperture.ShutterButtonMode {
    /**
     * The button is a white circle with a white border.
     */
    PICTURE,

    /**
     * The button is a red circle with a white border.
     */
    VIDEO,

    /**
     * The inside of the button is a red square.
     */
    RECORDING
}
