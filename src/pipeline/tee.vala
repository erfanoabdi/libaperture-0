/* tee.vala
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
 * A GstBin that manages a tee element.
 *
 * Elements can be added and removed from the tee, and #AperturePipelineTee
 * handles all the details (setting pipeline states, adding a queue on each
 * branch, etc). Each branch needs to be a single element. If multiple elements
 * are needed, use a #GstBin.
 */
internal class Aperture.Pipeline.Tee : Gst.Bin {
    private Gst.Element tee;
    private Gee.HashMap<Gst.Element, Gst.Element> queues;


    construct {
        queues = new Gee.HashMap<Gst.Element, Gst.Element>();

        tee = Gst.ElementFactory.make("tee", null);
        add(tee);

        var pad = tee.get_static_pad("sink");
        var ghost_pad = new Gst.GhostPad("sink", pad);
        ghost_pad.set_active(true);
        add_pad(ghost_pad);
    }


    /**
     * Adds an element as a branch of the tee.
     *
     * The branch should not be added to the pipeline. It will be added as a
     * child of the #AperturePipelineTee automatically.
     */
    public void add_branch(Gst.Element branch) {
        var queue = Gst.ElementFactory.make("queue", null);
        queues[branch] = queue;

        add(queue);
        add(branch);
        queue.link(branch);

        var tee_pad = tee.get_request_pad("src_%u");
        var queue_pad = queue.get_static_pad("sink");
        tee_pad.link(queue_pad);

        queue.sync_state_with_parent();
        branch.sync_state_with_parent();
    }


    /**
     * Removes a branch from the tee.
     */
    public void remove_branch(Gst.Element branch)
            requires (queues.has_key(branch)) {

        var queue = queues[branch];
        queues.unset(branch);

        var tee_pad = queue.get_static_pad("sink").get_peer();
        tee.release_request_pad(tee_pad);

        queue.set_state(NULL);
        branch.set_state(NULL);

        tee.unlink(queue);

        remove(queue);
        remove(branch);
    }
}
