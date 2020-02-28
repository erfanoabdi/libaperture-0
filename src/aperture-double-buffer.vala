/* aperture-double-buffer.vala
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
 * A simple, thread-safe double buffer implementation that uses
 * Cairo.ImageSurface.
 */
public class Aperture.DoubleBuffer : Object {
    public delegate void BufferFunc(Cairo.ImageSurface buffer);


    /**
     * Emitted when the buffer is swapped, on the thread that called swap().
     */
    public signal void swapped();


    /**
     * The surface that is being displayed.
     */
    private Cairo.ImageSurface front { get; set; }

    /**
     * The surface that is being rendered to.
     */
    private Cairo.ImageSurface back { get; set; }

    /*
     * The width and height that the buffers should be.
     */
    public int width { get; private set; }
    public int height { get; private set; }


    /**
     * Creates a new DoubleBuffer with the given width and height. The
     * surfaces will use the ARGB32 format.
     */
    public DoubleBuffer(int width, int height) {
        this.width = width;
        this.height = height;

        this.front = new Cairo.ImageSurface(Cairo.Format.ARGB32, this.width, this.height);
        this.back = new Cairo.ImageSurface(Cairo.Format.ARGB32, this.width, this.height);
    }


    /**
     * Calls the given function with the front buffer. Thread-safe.
     */
    public void with_front(BufferFunc func) {
        lock (this.front) {
            func(this.front);
        }
    }


    /**
     * Calls the given function with the back buffer. Thread-safe.
     */
    public void with_back(BufferFunc func) {
        lock (this.back) {
            func(this.back);
        }
    }


    /**
     * Swaps the front and back buffers.
     */
    public void swap() {
        lock (this.front) {
            lock (this.back) {
                Cairo.ImageSurface old_front = this.front;

                this.front = this.back;

                if (old_front.get_width() == this.width &&
                    old_front.get_height() == this.height) {

                    this.back = old_front;
                } else {
                    this.back = new Cairo.ImageSurface(Cairo.Format.ARGB32, this.width, this.height);
                }

                this.swapped();
            }
        }
    }


    /**
     * Changes the size of the double buffer.
     *
     * The back buffer will be resized immediately, but the front buffer will
     * not be resized until it is swapped to the back.
     */
    public void resize(int new_width, int new_height) {
        this.width = new_width;
        this.height = new_height;

        lock (this.back) {
            if (this.back.get_width() != this.width ||
                this.back.get_height() != this.height) {

                this.back = new Cairo.ImageSurface(Cairo.Format.ARGB32, this.width, this.height);
            }
        }
    }


    /**
     * Makes the back buffer solid transparent, then swaps the buffers.
     */
    public void clear() {
        this.with_back((back) => {
            Cairo.Context cr = new Cairo.Context(back);
            cr.set_operator(Cairo.Operator.CLEAR);
            cr.paint();
        });

        this.swap();
    }
}

