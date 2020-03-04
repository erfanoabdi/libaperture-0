/* aperture-tween.vala
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

internal class Aperture.Tween {
    private struct TweenVal {
        int64 start;
        int64 duration;
        double from;
        double to;

        public bool is_active(int64 time) {
            return time >= start && time <= start + duration;
        }

        public double get_val(int64 time) {
            double percent = (time - start) / (double) duration;
            percent = percent.clamp(0, 1);
            double diff = to - from;
            return percent * diff + from;
        }

        public int64 time_left(int64 time) {
            return int64.max(start + duration - time, 0);
        }
    }


    /**
     * Default duration, in milliseconds.
     */
    public int64 duration;


    private Gee.HashMap<string, TweenVal?> map;
    private weak Gtk.Widget widget;
    private uint tick_callback_id;
    private bool callback_active;


    public Tween(Gtk.Widget widget) {
        this.widget = widget;
        this.map = new Gee.HashMap<string, TweenVal?>();
        this.duration = 125;
    }


    public void start(string name, double to, int64 duration) {
        int64 time = this.get_time();

        // milliseconds to microseconds
        duration *= 1000;

        TweenVal? old = this.map.get(name);
        TweenVal val;
        if (old != null) {
            val = TweenVal() {
                start = time,
                duration = duration - old.time_left(time),
                from = old.get_val(time),
                to = to
            };
        } else {
            val = TweenVal() {
                start = time,
                duration = duration,
                from = to,
                to = to
            };
        }


        this.map[name] = val;

        if (this.is_active() && !this.callback_active) {
            this.tick_callback_id = this.widget.add_tick_callback(this.tick_callback);
            this.callback_active = true;
        }
    }

    public double get(string name) {
        TweenVal? val = this.map.get(name);
        return val != null ? val.get_val(this.get_time()) : 0;
    }

    public void set(string name, double val) {
        this.start(name, val, this.duration);
    }

    /**
     * Determines whether any tween is currently active.
     */
    public bool is_active() {
        foreach (TweenVal val in this.map.values) {
            if (val.is_active(this.get_time())) {
                return true;
            }
        }

        return false;
    }


    private bool tick_callback(Gtk.Widget widget, Gdk.FrameClock clock) {
        this.widget.queue_draw();

        if (!is_active()) {
            this.callback_active = false;
            return false;
        } else {
            return true;
        }
    }

    private int64 get_time() {
        var clock = this.widget.get_frame_clock();
        return clock != null ? clock.get_frame_time() : 0;
    }
}

