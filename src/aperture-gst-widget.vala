/* gst-widget-gl.vala
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


private class Aperture.GstWidget : Gtk.Bin {
    private Gst.Bin bin;
    private dynamic Gst.Element sink;
    private Gst.Element? glupload;


    construct {
        // Do not use gtkglsink on Wayland, it is broken.
        // See https://gitlab.freedesktop.org/gstreamer/gst-plugins-good/issues/671
        // Actually, don't use it at all. Resizing it causes a pipeline error
        // in the camerabin.
        /*if (!is_wayland()) {
            sink = Gst.ElementFactory.make("gtkglsink", null);
        }*/

        if (sink != null) {
            bin = new Gst.Bin(null);

            glupload = Gst.ElementFactory.make("glupload", null);
            bin.add_many(glupload, sink);
            glupload.link(sink);

            var pad = (glupload ?? sink).get_static_pad("sink");
            var ghost_pad = new Gst.GhostPad("sink", pad);
            ghost_pad.set_active(true);
            bin.add_pad(ghost_pad);
        } else {
            sink = Gst.ElementFactory.make("gtksink", null);
        }

        if (this.sink == null) {
            critical("Could not create gtkglsink or gtksink to display camera feed!");
        }

        Gtk.Widget widget = sink.widget;
        widget.visible = true;
        this.add(widget);
    }


    public Gst.Element get_sink() {
        return ((Gst.Element) bin) ?? sink;
    }
}
