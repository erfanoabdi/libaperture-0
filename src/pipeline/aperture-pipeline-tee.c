/* aperture-pipeline-tee.c
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


#include "aperture-pipeline-tee.h"


struct _AperturePipelineTee
{
  GstBin parent_instance;

  GstElement *tee;
  GHashTable *queues;
};

G_DEFINE_TYPE (AperturePipelineTee, aperture_pipeline_tee, GST_TYPE_BIN)


/* VFUNCS */


static void
aperture_pipeline_tee_finalize (GObject *object)
{
  AperturePipelineTee *self = APERTURE_PIPELINE_TEE (object);

  g_hash_table_unref (self->queues);

  G_OBJECT_CLASS (aperture_pipeline_tee_parent_class)->finalize (object);
}


/* INIT */


static void
aperture_pipeline_tee_class_init (AperturePipelineTeeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = aperture_pipeline_tee_finalize;
}


static void
aperture_pipeline_tee_init (AperturePipelineTee *self)
{
  g_autoptr(GstPad) pad = NULL;
  GstPad *ghost_pad;

  self->queues = g_hash_table_new (NULL, NULL);

  self->tee = gst_element_factory_make ("tee", NULL);
  gst_bin_add (GST_BIN (self), self->tee);

  pad = gst_element_get_static_pad (self->tee, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (GST_ELEMENT (self), ghost_pad);
}


/* PUBLIC */


/**
 * PRIVATE:aperture_pipeline_tee_new:
 *
 * Creates a new #AperturePipelineTee.
 *
 * Returns: (transfer full): a new #AperturePipelineTee
 */
AperturePipelineTee *
aperture_pipeline_tee_new (void)
{
  return g_object_new (APERTURE_TYPE_PIPELINE_TEE, NULL);
}


/**
 * PRIVATE:aperture_pipeline_tee_add_branch:
 * @self: an #AperturePipelineTee
 * @branch: (transfer full): an element to add to the tee
 *
 * Adds an element to the tee.
 *
 * A queue will be inserted between the tee and the element, and element states
 * are synced automatically.
 */
void
aperture_pipeline_tee_add_branch (AperturePipelineTee *self, GstElement *branch)
{
  GstElement *queue;
  g_autoptr(GstPad) tee_pad = NULL;
  g_autoptr(GstPad) queue_pad = NULL;

  g_return_if_fail (APERTURE_IS_PIPELINE_TEE (self));
  g_return_if_fail (GST_IS_ELEMENT (branch));

  queue = gst_element_factory_make ("queue", NULL);
  g_hash_table_insert (self->queues, branch, queue);

  gst_bin_add_many (GST_BIN (self), queue, branch, NULL);
  gst_element_link (queue, branch);

  tee_pad = gst_element_get_request_pad (self->tee, "src_%u");
  queue_pad = gst_element_get_static_pad (queue, "sink");
  gst_pad_link (tee_pad, queue_pad);

  gst_element_sync_state_with_parent (queue);
  gst_element_sync_state_with_parent (branch);
}


/**
 * PRIVATE:aperture_pipeline_tee_remove_branch:
 * @self: an #AperturePipelineTee
 * @branch: the element to remove
 *
 * Removes an element from the tee.
 */
void
aperture_pipeline_tee_remove_branch (AperturePipelineTee *self, GstElement *branch)
{
  GstElement *queue;
  g_autoptr(GstPad) tee_pad = NULL;
  g_autoptr(GstPad) queue_pad = NULL;

  g_return_if_fail (APERTURE_IS_PIPELINE_TEE (self));
  g_return_if_fail (GST_IS_ELEMENT (branch));
  g_return_if_fail (g_hash_table_contains (self->queues, branch));

  queue = g_hash_table_lookup (self->queues, branch);

  queue_pad = gst_element_get_static_pad (queue, "sink");
  tee_pad = gst_pad_get_peer (queue_pad);
  gst_element_release_request_pad (self->tee, tee_pad);

  gst_element_set_state (queue, GST_STATE_NULL);
  gst_element_set_state (branch, GST_STATE_NULL);

  gst_element_unlink (self->tee, queue);

  gst_bin_remove (GST_BIN (self), queue);
  gst_bin_remove (GST_BIN (self), branch);

  g_hash_table_remove (self->queues, branch);
}
