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


/**
 * A widget for displaying a camera feed and taking pictures and videos from
 * it.
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


    // GTK WIDGETS
    private GstWidget viewfinder;

    // GST ELEMENTS
    private Gst.Pipeline pipeline;
    private Gst.Element source;
    private Gst.Element convert;
    private Gst.Element tee;

    private delegate void BusCallback(Gst.Message msg);
    private BusCallback finish_taking_picture;


    construct {
        // Make sure Aperture is initialized
        init_check();

        // Create the viewfinder
        this.viewfinder = new GstWidget();
        this.viewfinder.expand = true;
        this.viewfinder.visible = true;
        this.attach(this.viewfinder, 0, 0);
        var sink = this.viewfinder.get_sink();
        var viewfinder_bin = this.viewfinder.get_bin();

        // Set up signals
        this.realize.connect(on_realize);
        this.unrealize.connect(on_unrealize);

        // Create pipeline and set up message handlers
        pipeline = new Gst.Pipeline(null);
        pipeline.get_bus().add_watch(Priority.DEFAULT, _on_bus_message_async);

        // Create and connect all the elements
        this.convert = create_element("videoconvert");
        this.tee = create_element("tee");
        Gst.Element q1 = create_element("queue");
        dynamic Gst.Element videoscale = create_element("videoscale");

        videoscale.add_borders = true;

        pipeline.add_many(this.convert, this.tee, q1, videoscale, viewfinder_bin);
        this.convert.link_many(this.tee, q1, videoscale, sink);

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


    /**
     * Takes a picture.
     *
     * This may take a while. The resolution might be changed temporarily,
     * autofocusing might take place, etc. Basically everything you'd expect
     * to happen when you click the photo button on the camera app.
     *
     * When the picture has been taken, the `picture_taken` signal will be
     * emitted with the picture.
     */
    public void take_picture() {
        this.debug_dump();
        this.state = TAKING_PICTURE;

        Gst.Element q = create_element("queue");
        Gst.Element convert = create_element("videoconvert");
        dynamic Gst.Element pixbufsink = create_element("gdkpixbufsink");

        // Not quite sure why this is needed, but if it's not there, you'll
        // get errors when the elements are finalized, saying they're not
        // in the NULL state (even though they were just set to NULL).
        pixbufsink.set_locked_state(true);
        convert.set_locked_state(true);
        q.set_locked_state(true);

        // Connect the elements
        this.pipeline.add_many(q, convert, pixbufsink);
        q.link_many(convert, pixbufsink);

        Gst.Pad tee_src = this.tee.get_request_pad("src_%u");
        Gst.Pad q_sink = q.get_static_pad("sink");
        tee_src.link(q_sink);

        this.finish_taking_picture = (msg) => {
            if (msg.src != pixbufsink) {
                return;
            }

            if (msg.type != ELEMENT) {
                return;
            }

            unowned Gst.Structure structure = msg.get_structure();
            Gdk.Pixbuf pixbuf = (Gdk.Pixbuf) structure.get_value("pixbuf");

            this.state = READY;

            q.set_state(NULL);
            convert.set_state(NULL);
            pixbufsink.set_state(NULL);

            pipeline.remove(q);
            pipeline.remove(convert);
            pipeline.remove(pixbufsink);

            this.tee.release_request_pad(tee_src);

            this.finish_taking_picture = null;

            Idle.add(() => {
                this.picture_taken(pixbuf);
                return Source.REMOVE;
            });
        };

        // Set the elements to playing
        q.sync_state_with_parent();
        convert.sync_state_with_parent();
        pixbufsink.set_state(PLAYING);
    }

    /**
     * Creates a graph of the #ApertureWidget's pipeline, for debugging.
     *
     * The graph can be converted to an image using GraphViz's dot command,
     * like this: `dot -Tpng -o aperture-widget.png aperture-widget.dot`
     *
     * For this to work, the GST_DEBUG_DUMP_DOT_DIR environment variable must
     * be set to a directory path in which to place the file.
     */
    public void debug_dump(string name="aperture-widget") {
        Gst.Debug.bin_to_dot_file(this.pipeline, ALL, name);
    }


    private void _set_camera(Camera new_device) {
        var old_source = this.source;
        var new_source = new_device.create_gstreamer_source();

        if (old_source != null) {
            // Must set a different clock. Otherwise, when the old source
            // is destroyed, its clock will freeze, freezing the pipeline
            // with it
            pipeline.set_clock(Gst.SystemClock.obtain());
            old_source.set_state(NULL);
            pipeline.remove(old_source);
        }

        this.source = new_source;
        pipeline.add(this.source);
        this.source.link(this.convert);
        this.source.sync_state_with_parent();

        if (this.viewfinder.get_realized()) {
            pipeline.set_state(PLAYING);
        }

        this.state = READY;
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
        }

        if (this.finish_taking_picture != null) {
            this.finish_taking_picture(msg);
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
     * The #ApertureWidget is taking a picture.
     */
    TAKING_PICTURE,

    /**
     * The #ApertureWidget could not find any cameras to use.
     */
    NO_CAMERAS,

    /**
     * An error has occurred.
     */
    ERROR,
}
