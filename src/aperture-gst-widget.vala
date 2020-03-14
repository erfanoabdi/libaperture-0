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
        bin = new Gst.Bin(null);

        sink = Gst.ElementFactory.make("gtkglsink", null);
        if (sink != null) {
            glupload = Gst.ElementFactory.make("glupload", null);
            bin.add_many(glupload, sink);
            glupload.link(sink);
        } else {
            sink = Gst.ElementFactory.make("gtksink", null);
            bin.add(sink);
        }

        if (this.sink == null) {
            critical("Could not create gtkglsink or gtksink to display camera feed!");
        }

        Gtk.Widget widget = sink.widget;
        this.add(widget);
    }


    public Gst.Element get_sink() {
        return glupload ?? sink;
    }

    public Gst.Bin get_bin() {
        return bin;
    }
}
