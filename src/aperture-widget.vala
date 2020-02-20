/* aperture-widget.vala
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


public class Aperture.Widget : Gtk.Grid {
    private Camera _camera;
    /**
     * The camera device that is currently being used.
     */
    public Camera camera {
        get {
            return _camera;
        }
        set {
            if (value != _camera) {
                _set_camera(value);
                _camera = value;
            }
        }
    }

    /**
     * The loading state of the #ApertureWidget.
     */
    public State state { get; set; default=State.LOADING; }


    /**
     * Emitted when a picture is taken by the user or by calling
     * take_picture().
     */
    public signal void picture_taken(Gdk.Pixbuf pixbuf);

    /**
     * Emitted when a snapshot is taken by the user or by calling
     * take_snapshot().
     */
    public signal void snapshot_taken(Gdk.Pixbuf pixbuf);


    // GTK WIDGETS
    private Gtk.DrawingArea _viewfinder;

    // GST ELEMENTS
    private Gst.Pipeline _pipeline;
    private Gst.Element _source;
    private Gst.Element _sink;
    private Gst.Element _convert1;


    construct {
        // Make sure Aperture is initialized
        init();

        // Build the widget
        _viewfinder = new Gtk.DrawingArea();
        _viewfinder.realize.connect(this._on_realize);
        _viewfinder.draw.connect(this._on_draw);
        _viewfinder.expand = true;
        this.attach(this._viewfinder, 0, 0);

        // Create pipeline and set up message handlers
        _pipeline = new Gst.Pipeline(null);
        _pipeline.get_bus().set_sync_handler(_on_bus_message_sync);
        _pipeline.get_bus().add_watch(Priority.DEFAULT, _on_bus_message_async);

        // Create and connect all the elements
        _convert1 = _create_element("videoconvert");

        if (is_wayland_display()) {
            _sink = _create_element("waylandsink");
        } else {
            _sink = _create_element("xvimagesink");
        }

        _pipeline.add_many(_convert1, _sink);
        _convert1.link_many(_sink);

        // Pick a camera
        var devices = DeviceManager.get_instance();
        devices.start();

        devices.camera_removed.connect(_on_camera_removed);

        if (devices.cameras.length() > 0) {
            this.camera = devices.cameras.first().data;
        } else {
            state = NO_CAMERAS;
        }
    }

    ~Widget() {
        // Make sure the pipeline is in NULL state before it is finalized!
        if (_source != null) {
            _pipeline.set_state(NULL);
        }
    }


    private void _on_realize() {
        _pipeline.set_state(PLAYING);
    }

    private void _set_camera(Camera new_device) {
        var old_source = _source;
        var new_source = new_device.create_gstreamer_source();

        if (old_source != null) {
            // Must set a different clock. Otherwise, when the old source
            // is destroyed, its clock will freeze, freezing the pipeline
            // with it
            _pipeline.set_clock(Gst.SystemClock.obtain());
            old_source.set_state(NULL);
            _pipeline.remove(old_source);
        }

        _source = new_source;
        _pipeline.add(_source);
        _source.link(_convert1);
        _source.sync_state_with_parent();

        if (_viewfinder.get_realized()) {
            _pipeline.set_state(PLAYING);
        }

        this.state = READY;
    }

    private Gst.BusSyncReply _on_bus_message_sync(Gst.Bus bus, Gst.Message msg) {
        if (Gst.Video.is_video_overlay_prepare_window_handle_message(msg)) {
            var window = _viewfinder.get_window();
            var handle = get_window_handle(window);

            if (is_wayland_display()) {
                _sink.set_context(create_wayland_context());
            }

            ((Gst.Video.Overlay) _sink).set_window_handle(handle);

            msg.unref();
            return DROP;
        }

        return PASS;
    }

    private bool _on_bus_message_async(Gst.Bus bus, Gst.Message msg) {
        switch (msg.type) {
        case ERROR:
            GLib.Error err;
            string debug_info;

            msg.parse_error(out err, out debug_info);
            stderr.printf("Error received from element %s: %s\n", msg.src.name, err.message);
            stderr.printf("Debugging information: %s\n", (debug_info != null) ? debug_info : "none");
            break;
        }

        return true;
    }

    private void _on_camera_removed(Camera camera) {
        if (camera == this.camera) {
            // the current camera was removed
            var devices = DeviceManager.get_instance();
            if (devices.cameras.length() > 0) {
                this.camera = devices.cameras.first().data;
            } else {
                this.camera = null;
                this.state = NO_CAMERAS;
            }
        }
    }

    private bool _on_draw(Cairo.Context ctx) {
        if (_pipeline.current_state == PAUSED || _pipeline.current_state == PLAYING) {
            return false;
        }

        Gtk.Allocation alloc;
        _viewfinder.get_allocation(out alloc);
        ctx.set_source_rgb(0, 0, 0);
        ctx.rectangle(0, 0, alloc.width, alloc.height);
        ctx.fill();

        return false;
    }

    private void _set_error(string message) {
        critical(message);
        this.state = ERROR;
    }

    private Gst.Element? _create_element(string factory) {
        var element = Gst.ElementFactory.make(factory, null);
        if (element == null) {
            _set_error("Failed to create element %s".printf(factory));
        }
        return element;
    }
}


/**
 * The state of an #ApertureWidget.
 */
public enum Aperture.State {
    /**
     * The #ApertureWidget is still loading.
     */
    LOADING,

    /**
     * The #ApertureWidget is ready to be used.
     */
    READY,

    /**
     * The #ApertureWidget is recording a video.
     */
    RECORDING,

    /**
     * The #ApertureWidget could not find any cameras to use.
     */
    NO_CAMERAS,

    /**
     * An error has occurred.
     */
    ERROR,
}
