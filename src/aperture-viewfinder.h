/* aperture-viewfinder.h
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

#if !defined(_LIBAPERTURE_INSIDE) && !defined(_LIBAPERTURE_COMPILATION)
#error "Only <aperture.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "aperture-barcode-result.h"
#include "aperture-enums.h"


G_BEGIN_DECLS


typedef enum {
  APERTURE_VIEWFINDER_STATE_LOADING,
  APERTURE_VIEWFINDER_STATE_READY,
  APERTURE_VIEWFINDER_STATE_RECORDING,
  APERTURE_VIEWFINDER_STATE_TAKING_PICTURE,
  APERTURE_VIEWFINDER_STATE_NO_CAMERAS,
  APERTURE_VIEWFINDER_STATE_ERROR,
} ApertureViewfinderState;

typedef enum {
  APERTURE_VIEWFINDER_ERROR_UNSPECIFIED,
  APERTURE_VIEWFINDER_ERROR_BROKEN_PIPELINE,
  APERTURE_VIEWFINDER_ERROR_PIPELINE_ERROR,
  APERTURE_VIEWFINDER_ERROR_COULD_NOT_TAKE_PICTURE,
} ApertureViewfinderError;


#define APERTURE_TYPE_VIEWFINDER (aperture_viewfinder_get_type())
G_DECLARE_FINAL_TYPE (ApertureViewfinder, aperture_viewfinder, APERTURE, VIEWFINDER, GtkBin)


ApertureViewfinder      *aperture_viewfinder_new                     (void);
void                     aperture_viewfinder_set_camera              (ApertureViewfinder *self,
                                                                      int                 camera);
int                      aperture_viewfinder_get_camera              (ApertureViewfinder *self);
ApertureViewfinderState  aperture_viewfinder_get_state               (ApertureViewfinder *self);
void                     aperture_viewfinder_set_detect_barcodes     (ApertureViewfinder *self,
                                                                      gboolean            detect_barcodes);
gboolean                 aperture_viewfinder_get_detect_barcodes     (ApertureViewfinder *self);
void                     aperture_viewfinder_take_picture_to_file    (ApertureViewfinder *self,
                                                                      const char         *file);
void                     aperture_viewfinder_start_recording_to_file (ApertureViewfinder *self,
                                                                      const char         *file);
void                     aperture_viewfinder_stop_recording          (ApertureViewfinder *self);


G_END_DECLS
