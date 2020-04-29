/* aperture-pipeline-tee.h
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


#pragma once


#include <gst/gst.h>


G_BEGIN_DECLS


#define APERTURE_TYPE_PIPELINE_TEE (aperture_pipeline_tee_get_type())
G_DECLARE_FINAL_TYPE (AperturePipelineTee, aperture_pipeline_tee, APERTURE, PIPELINE_TEE, GstBin)


AperturePipelineTee *aperture_pipeline_tee_new ();

void aperture_pipeline_tee_add_branch (AperturePipelineTee *self, GstElement *branch);
void aperture_pipeline_tee_remove_branch (AperturePipelineTee *self, GstElement *branch);


G_END_DECLS
