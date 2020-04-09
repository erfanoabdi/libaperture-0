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
 * A widget that displays images and videos in a #HdyCarousel.
 */
public class Aperture.Gallery : Gtk.Bin, Gtk.Buildable {
    public signal void item_added(GalleryPage thumbnail);

    private Hdy.Carousel carousel;

    private List<GalleryPage> pages;

    private Aperture.Viewfinder _viewfinder;
    public Aperture.Viewfinder viewfinder {
        get { return this._viewfinder; }
        set {
            if (value == this.viewfinder) {
                return;
            }

            if (this.viewfinder != null) {
                this.carousel.remove(this.viewfinder);
            }

            this._viewfinder = value;

            if (this.viewfinder != null) {
                this.carousel.prepend(this.viewfinder);
                this.viewfinder.expand = true;
            }

            this.notify_property("is-viewfinder-visible");
        }
    }

    public bool is_viewfinder_visible {
        get {
            return this.viewfinder != null && this.carousel.position < 1;
        }
    }

    public double progress {
        get {
            if (this.viewfinder == null) {
                return 1;
            }

            return this.carousel.position.clamp(0, 1);
        }
    }

    construct {
        this.carousel = new Hdy.Carousel();
        this.carousel.visible = true;
        this.carousel.notify["position"].connect(() => {
            this.notify_property("progress");
            this.notify_property("is-viewfinder-visible");
        });
        this.add(this.carousel);
    }

    public void open() {
        if (this.viewfinder == null) {
            warning("Can't open the gallery without viewfinder.");
            return;
        }

        var items = this.get_items();
        if (items.length() < 1)
            return;

        this.carousel.scroll_to(items.first().data);
    }

    public void close() {
        if (this.viewfinder == null) {
            warning("Can't close the gallery without viewfinder.");
            return;
        }

        this.carousel.scroll_to(viewfinder);
    }

    /**
     * Scrolls the carousel forward (toward the viewfinder and newer pages).
     */
    public bool scroll_forward() {
        int position = (int) this.carousel.position;
        if (position <= 0) {
            return false;
        }

        var children = this.carousel.get_children();
        this.carousel.scroll_to(children.nth_data(position - 1));
        return true;
    }

    /**
     * Scrolls the carousel backward (away from the viewfinder, toward older
     * pages).
     */
    public bool scroll_backward() {
        int position = (int) this.carousel.position;
        var children = this.carousel.get_children();

        if (position >= children.length() - 1) {
            return false;
        }

        this.carousel.scroll_to(children.nth_data(position + 1));
        return true;
    }

    /**
     * Adds an image to the gallery.
     */
    public void add_image(Gdk.Pixbuf pixbuf) {
        var page = new GalleryPage.for_image(pixbuf);
        page.visible = true;
        page.expand = true;

        this.pages.prepend(page);
        this.carousel.insert(page, this.viewfinder != null ? 1 : 0);
        this.item_added(page);
    }

    /**
     * Gets a list of the pages in the gallery.
     *
     * Do not modify the returned list.
     */
    public List<weak GalleryPage> get_items() {
        return this.pages.copy();
    }

    public void add_child(Gtk.Builder builder, Object child, string? type) {
        if (type == "viewfinder") {
            assert(child is Aperture.Viewfinder);
            this.viewfinder = child as Aperture.Viewfinder;
            return;
        }

        base.add_child (builder, child, type);
    }
}
