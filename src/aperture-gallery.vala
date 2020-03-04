/* aperture-gallery.vala
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


public class Aperture.Gallery : Gtk.Grid {
    public signal void item_added(GalleryPage thumbnail);


    private Hdy.Paginator paginator;


    construct {
        this.paginator = new Hdy.Paginator();
        this.paginator.visible = true;
        this.paginator.indicator_style = DOTS;
        this.add(this.paginator);
    }


    public void add_image(Gdk.Pixbuf pixbuf) {
        var page = new GalleryPage.for_image(pixbuf);
        page.visible = true;
        page.expand = true;

        this.paginator.insert(page, -1);
        this.item_added(page);
    }

    public List<weak GalleryPage> get_items() {
        return (List<weak GalleryPage>) this.paginator.get_children();
    }
}
