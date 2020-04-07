/* aperture-viewfinder.vala
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
 * A widget for displaying a camera feed and taking pictures and videos from
 * it.
 */
public class Aperture.Viewfinder : Gtk.Grid {
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
     * The loading state of the #ApertureViewfinder.
     */
    public State state { get; set; default=State.LOADING; }


    /**
     * Emitted when a picture is done being taken.
     */
    public signal void picture_taken(Gdk.Pixbuf pixbuf);


    /**
     * Emitted when a video is done being taken.
     */
    public signal void video_taken();


    // GTK WIDGETS
    private GstWidget gst_widget;

    // GST ELEMENTS
    private dynamic Gst.Element camerabin;
    private Gst.Pipeline pipeline;

    private delegate void BusCallback(Gst.Message msg);
    private BusCallback finish_taking_picture;


    construct {
        // Make sure Aperture is initialized
        init_check();

        // Create the GstWidget for displaying the viewfinder feed
        this.gst_widget = new GstWidget();
        this.gst_widget.expand = true;
        this.gst_widget.visible = true;
        this.attach(this.gst_widget, 0, 0);

        // Set up signals
        this.realize.connect(on_realize);
        this.unrealize.connect(on_unrealize);

        // Create pipeline and set up message handlers
        pipeline = new Gst.Pipeline(null);
        pipeline.get_bus().add_watch(Priority.DEFAULT, _on_bus_message_async);

        // Create the camerabin
        camerabin = create_element("camerabin");
        camerabin.viewfinder_sink = this.gst_widget.get_sink();
        pipeline.add_many(camerabin);

        // Pick a camera
        var devices = DeviceManager.get_instance();
        devices.start();

        devices.camera_removed.connect(_on_camera_removed);

        if (devices.cameras.length() > 0) {
            state = READY;
            this.camera = devices.cameras.first().data;
        } else {
            state = NO_CAMERAS;
        }
    }


    /**
     * Takes a picture and saves it to a file.
     *
     * This may take a while. The resolution might be changed temporarily,
     * autofocusing might take place, etc. Basically everything you'd expect
     * to happen when you click the photo button on the camera app.
     *
     * When the picture has been taken and saved, ::picture_taken will be
     * emitted.
     */
    public void take_picture(string file)
            requires (state == READY) {

        state = TAKING_PICTURE;

        camerabin.mode = 1; // Image mode
        camerabin.location = file;
        Signal.emit_by_name(camerabin, "start-capture");
    }

    /**
     * Starts recording a video. The video will be saved to @file.
     */
    public void start_recording(string file)
            requires (state == READY) {

        state = RECORDING;

        camerabin.mode = 2; // Video mode
        camerabin.location = file;
        Signal.emit_by_name(camerabin, "start-capture");
    }

    /**
     * Stop recording video. ::video_taken will be emitted when this is done.
     */
    public void stop_recording()
            requires (state == RECORDING) {

        Signal.emit_by_name(camerabin, "stop-capture");
    }

    /**
     * Creates a graph of the #ApertureViewfinder's pipeline, for debugging.
     *
     * The graph can be converted to an image using GraphViz's dot command,
     * like this: `dot -Tpng -o aperture-viewfinder.png aperture-viewfinder.dot`
     *
     * For this to work, the GST_DEBUG_DUMP_DOT_DIR environment variable must
     * be set to a directory path in which to place the file.
     */
    public void debug_dump(string name="aperture-viewfinder") {
        Gst.Debug.bin_to_dot_file(this.pipeline, ALL, name);
    }


    private void _set_camera(Camera new_device)
            // Cannot change camera while recording or taking picture
            requires (state == READY) {

        dynamic Gst.Element wrapper = create_element("wrappercamerabinsrc");
        wrapper.video_source = new_device.create_gstreamer_source();
        camerabin.camera_source = wrapper;

        // Must change camerabin to NULL and back to PLAYING for the change
        // to take effect
        camerabin.set_state(NULL);
        if (gst_widget.get_realized()) {
            camerabin.set_state(PLAYING);
        }
    }


    private bool _on_bus_message_async(Gst.Bus bus, Gst.Message msg) {
        switch (msg.type) {
        case ERROR:
            GLib.Error err;
            string debug_info;

            msg.parse_error(out err, out debug_info);
            stderr.printf("Error received from element %s: %s\n", msg.src.name, err.message);
            stderr.printf("Debugging information: %s\n", (debug_info != null) ? debug_info : "none");

            debug_dump("libaperture-error");

            return true;
        case ELEMENT:
            if (msg.has_name("image-done")) {
                on_finish_taking_picture.begin();
                return true;
            } else if (msg.has_name("video-done")) {
                state = READY;
                video_taken();
                return true;
            }

            break;
        }

        if (this.finish_taking_picture != null) {
            this.finish_taking_picture(msg);
        }

        return true;
    }

    private async void on_finish_taking_picture()
            requires (state == TAKING_PICTURE) {
        try {
            var file = File.new_for_path(camerabin.location);
            var stream = yield file.read_async();
            var pixbuf = yield new Gdk.Pixbuf.from_stream_async(stream);

            state = READY;
            picture_taken(pixbuf);
        } catch (Error e) {
            _set_error("Could not take picture: " + e.message);
        }
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


    private void _set_error(string message) {
        critical(message);
        this.state = ERROR;
    }

    private Gst.Element? create_element(string factory) {
        var element = Gst.ElementFactory.make(factory, null);
        if (element == null) {
            _set_error("Failed to create element %s".printf(factory));
        }
        return element;
    }

    private void on_realize() {
        this.pipeline.set_state(PLAYING);
    }

    private void on_unrealize() {
        this.pipeline.set_state(NULL);
    }
}


/**
 * The state of an #ApertureViewfinder.
 */
public enum Aperture.State {
    /**
     * The #ApertureViewfinder is still loading.
     */
    LOADING,

    /**
     * The #ApertureViewfinder is ready to be used.
     */
    READY,

    /**
     * The #ApertureViewfinder is recording a video.
     */
    RECORDING,

    /**
     * The #ApertureViewfinder is taking a picture.
     */
    TAKING_PICTURE,

    /**
     * The #ApertureViewfinder could not find any cameras to use.
     */
    NO_CAMERAS,

    /**
     * An error has occurred.
     */
    ERROR,
}
