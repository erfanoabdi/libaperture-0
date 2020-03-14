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


/**
 * Handles tweening for animations.
 *
 * A tween object keeps track of a widget's animation states, each of which is
 * a double. To start an animation, call start() with the name of the
 * animation, the end value, and the duration. All interpolation is linear.
 *
 * #Tween also uses tick callbacks to make sure the widget is redrawn every
 * tick as long as at least one animation is active.
 *
 * If GTK animations are disabled, then no interpolation will be done and
 * animations will be immediately set to their end value.
 *
 * In Vala, an animation's current value can be retrieved using array access
 * syntax, like this: `tween["animation-name"]`. The same syntax can be used
 * to start an animation. In this case, :duration will be used as the
 * duration.
 */
internal class Aperture.Tween {
    private struct TweenVal {
        int64 start;
        int64 duration;
        double from;
        double to;

        public bool is_active(int64 time) {
            if (duration <= 0) {
                return false;
            }

            return time >= start && time <= start + duration;
        }

        public double get_val(int64 time) {
            if (duration <= 0) {
                return to;
            }

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
    public int64 duration { get; set; default=125; }


    private Gee.HashMap<string, TweenVal?> map;
    private weak Gtk.Widget widget;
    private uint tick_callback_id;
    private bool callback_active;


    /**
     * Creates a new Tween object that manages animations for the widget.
     */
    public Tween(Gtk.Widget widget) {
        this.widget = widget;
        this.map = new Gee.HashMap<string, TweenVal?>();
    }


    /**
     * Starts an animation.
     *
     * Over @duration milliseconds, the value will be interpolated from its
     * current value to @to. If the animation has never been started, if the
     * duration is 0, or if GTK animations are disabled, then the value will be
     * set immediately.
     */
    public void start(string name, double to, int64 duration) {
        if (animations_disabled()) {
            this.map[name] = TweenVal() {
                start = 0,
                duration = 0,
                from = to,
                to = to
            };

            this.widget.queue_draw();

            return;
        }

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

    /**
     * Gets an animation's current value.
     */
    public double get(string name) {
        TweenVal? val = this.map.get(name);
        return val != null ? val.get_val(this.get_time()) : 0;
    }

    /**
     * Starts an animation with :duration as the duration.
     */
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

    private static bool animations_disabled() {
        return !Gtk.Settings.get_default().gtk_enable_animations;
    }
}

