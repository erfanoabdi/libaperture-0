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


/**
 * A widget that displays images and videos in a #HdyPaginator.
 */
public class Aperture.Gallery : Gtk.Bin, Gtk.Buildable {
    public signal void item_added(GalleryPage thumbnail);

    private Hdy.Paginator paginator;

    private List<GalleryPage> pages;

    private Aperture.Widget _widget;
    public Aperture.Widget widget {
        get { return this._widget; }
        set {
            if (value == this.widget) {
                return;
            }

            if (this.widget != null) {
                this.paginator.remove(this.widget);
            }

            this._widget = value;

            if (this.widget != null) {
                this.paginator.prepend(this.widget);
                this.widget.expand = true;
            }

            this.notify_property("is-widget-visible");
        }
    }

    public bool is_widget_visible {
        get {
            return this.widget != null && this.paginator.position < 1;
        }
    }

    public double progress {
        get {
            if (this.widget == null) {
                return 1;
            }

            return this.paginator.position.clamp(0, 1);
        }
    }

    construct {
        this.paginator = new Hdy.Paginator();
        this.paginator.visible = true;
        this.paginator.notify["position"].connect(() => {
            this.notify_property("progress");
            this.notify_property("is-widget-visible");
        });
        this.add(this.paginator);
    }

    public void open() {
        if (this.widget == null) {
            warning("Can't open the gallery without widget.");
            return;
        }

        var items = this.get_items();
        if (items.length() < 1)
            return;

        this.paginator.scroll_to(items.first().data);
    }

    public void close() {
        if (this.widget == null) {
            warning("Can't close the gallery without widget.");
            return;
        }

        this.paginator.scroll_to(widget);
    }

    /**
     * Adds an image to the gallery.
     */
    public void add_image(Gdk.Pixbuf pixbuf) {
        var page = new GalleryPage.for_image(pixbuf);
        page.visible = true;
        page.expand = true;

        this.pages.prepend(page);
        this.paginator.insert(page, this.widget != null ? 1 : 0);
        this.item_added(page);
    }

    /**
     * Gets a list of the pages in the gallery.
     *
     * Do not modify the returned list.
     */
    public List<weak GalleryPage> get_items() {
        return this.pages.copy ();
    }

    public void add_child (Gtk.Builder builder, Object child, string? type) {
        if (type == "widget") {
            assert (child is Aperture.Widget);
            this.widget = child as Aperture.Widget;
            return;
        }

        base.add_child (builder, child, type);
    }
}
