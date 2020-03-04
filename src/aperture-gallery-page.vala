/* aperture-gallery-page.vala
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


public class Aperture.GalleryPage : Gtk.EventBox {
    public File file { get; construct; }
    public Gdk.Pixbuf pixbuf { get; construct; }


    private Gtk.DrawingArea da;


    public GalleryPage.for_image(Gdk.Pixbuf pixbuf) {
        Object(pixbuf: pixbuf);
    }

    construct {
        this.da = new Gtk.DrawingArea();
        this.da.visible = true;
        this.da.draw.connect(this.on_draw);
        this.add(this.da);
    }


    private bool on_draw(Cairo.Context ctx) {
        if (this.pixbuf != null) {
            Cairo.Matrix matrix;
            ctx.get_source().get_matrix(out matrix);

            double width = this.get_allocated_width();
            double height = this.get_allocated_height();

            double w = this.pixbuf.get_width();
            double h = this.pixbuf.get_height();
            double old_w = w;
            double old_h = h;
            scale_to_fit(ref w, ref h, width, height);

            double x, y;
            center(w, h, width, height, out x, out y);

            Gdk.cairo_set_source_pixbuf(ctx, this.pixbuf, 0, 0);
            matrix.scale(old_w / w, old_h / h);
            matrix.translate(-x, -y);

            ctx.get_source().set_matrix(matrix);

            ctx.paint();
        }

        return false;
    }
}
