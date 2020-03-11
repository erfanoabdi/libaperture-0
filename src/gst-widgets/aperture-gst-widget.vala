/* aperture-gst-widget.vala
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

public interface Aperture.GstWidget : Gtk.Widget {
    /**
     * Creates an ApertureGstWidget instance.
     */
    public static GstWidget create() {
        return new GstWidgetClutter();
    }

    /**
     * Gets the sink element that should be linked to the pipeline.
     */
    public abstract Gst.Element get_sink();
}
