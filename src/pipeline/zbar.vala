/* zbar.vala
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


internal class Aperture.Pipeline.ZBar : Gst.Bin {
    private Gst.Element videoconvert;
    private dynamic Gst.Element zbar;
    private Gst.Element fakesink;


    construct {
        videoconvert = Gst.ElementFactory.make("videoconvert", null);
        zbar = Gst.ElementFactory.make("zbar", null);
        fakesink = Gst.ElementFactory.make("fakesink", null);

        zbar.cache = true;

        add_many(videoconvert, zbar, fakesink);
        videoconvert.link_many(zbar, fakesink);

        var pad = videoconvert.get_static_pad("sink");
        var ghost_pad = new Gst.GhostPad("sink", pad);
        ghost_pad.set_active(true);
        add_pad(ghost_pad);
    }
}
