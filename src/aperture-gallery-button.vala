/* aperture-gallery-button.vala
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
 * A button intended to be used to show a Gallery.
 *
 * #ApertureGalleryButton is a specially-styled button that shows a thumbnail
 * of the latest media in a gallery.
 *
 * Actually showing the gallery is left to the application, since there are
 * multiple ways this could be done (using a dialog, or with stacks, etc.)
 */
public class Aperture.GalleryButton : Gtk.Button {
    private Gallery _gallery;
    /**
     * The gallery that the #ApertureGalleryButton is connected to.
     */
    public Gallery gallery {
        get {
            return _gallery;
        }
        set {
            if (_gallery == value) {
                return;
            }

            this.on_gallery_changed(_gallery, value);
            _gallery = value;
        }
    }


    private Tween tween;


    construct {
        this.tween = new Tween(this);
        this.tween.start("size", 0, 0);

        this.draw.connect(this.on_draw);
    }


    private bool on_draw(Cairo.Context ctx) {
        if (this.gallery == null) {
            return true;
        }

        int width = this.get_allocated_width();
        int height = this.get_allocated_height();
        int size = int.min(width, height) - 2;
        double radius = this.tween["size"] * size / 2.0;
        double outer_radius = radius;

        var pages = this.gallery.get_items();
        unowned List<weak GalleryPage> first = pages.first();

        if (first == null) {
            return true;
        }

        if (first.next != null && size != 1) {
            outer_radius = size / 2.0;
            this.draw_image(ctx, width, height, size / 2.0, first.next.data.pixbuf);
        }

        this.draw_image(ctx, width, height, radius, first.data.pixbuf);

        ctx.arc(width / 2.0, height / 2.0, outer_radius, 0, 2 * Math.PI);
        ctx.set_line_width(1.5);
        ctx.set_source_rgb(1, 1, 1);
        ctx.stroke();

        return true;
    }

    private void draw_image(Cairo.Context ctx, int width, int height, double size, Gdk.Pixbuf image) {
        ctx.save();

        Cairo.Matrix matrix;
        ctx.get_source().get_matrix(out matrix);

        double w = image.get_width();
        double h = image.get_height();
        double old_w = w;
        double old_h = h;
        scale_to_fill(ref w, ref h, width, height);

        double x, y;
        center(w, h, width, height, out x, out y);

        Gdk.cairo_set_source_pixbuf(ctx, image, 0, 0);
        matrix.scale(old_w / w, old_h / h);
        matrix.translate(-x, -y);

        ctx.get_source().set_matrix(matrix);

        ctx.arc(width / 2.0, height / 2.0, size, 0, 2 * Math.PI);
        ctx.fill();

        ctx.restore();
    }

    private void on_item_added(Gallery gallery, GalleryPage item) {
        this.tween.start("size", 0, 0);
        this.tween["size"] = 1;
        this.queue_draw();

        update_sensitive();
    }

    private void update_sensitive() {
        this.sensitive = gallery != null && gallery.get_items().length() > 0;
    }

    private void on_gallery_changed(Gallery? old_gallery, Gallery? new_gallery) {
        if (old_gallery != null) {
            old_gallery.item_added.disconnect(this.on_item_added);
        }

        if (new_gallery != null) {
            new_gallery.item_added.connect(this.on_item_added);
        }

        this.queue_draw();
        update_sensitive();
    }

    /*
     * Gtk.Button.size_allocate sometimes messes with the clip size, which we
     * don't want, so we need to make sure to set it back.
     */
    public override void size_allocate(Gtk.Allocation alloc) {
        base.size_allocate(alloc);
        this.set_clip(alloc);
    }
}

