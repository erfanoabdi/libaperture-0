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
#include "aperture-enums.h"


G_BEGIN_DECLS


typedef enum {
  APERTURE_VIEWFINDER_STATE_LOADING,
  APERTURE_VIEWFINDER_STATE_READY,
  APERTURE_VIEWFINDER_STATE_NO_CAMERAS,
  APERTURE_VIEWFINDER_STATE_ERROR,
} ApertureViewfinderState;

typedef enum {
  APERTURE_MEDIA_CAPTURE_ERROR_OPERATION_IN_PROGRESS,
  APERTURE_MEDIA_CAPTURE_ERROR_NO_RECORDING_TO_STOP,
  APERTURE_MEDIA_CAPTURE_ERROR_CAMERA_DISCONNECTED,
  APERTURE_MEDIA_CAPTURE_ERROR_INTERRUPTED,
  APERTURE_MEDIA_CAPTURE_ERROR_NOT_READY,
} ApertureMediaCaptureError;


#define APERTURE_TYPE_VIEWFINDER (aperture_viewfinder_get_type())
G_DECLARE_FINAL_TYPE (ApertureViewfinder, aperture_viewfinder, APERTURE, VIEWFINDER, GtkBin)


ApertureViewfinder      *aperture_viewfinder_new                     (void);
void                     aperture_viewfinder_set_camera              (ApertureViewfinder *self,
                                                                      int                 camera,
                                                                      GError            **err);
int                      aperture_viewfinder_get_camera              (ApertureViewfinder *self);
ApertureViewfinderState  aperture_viewfinder_get_state               (ApertureViewfinder *self);
void                     aperture_viewfinder_set_detect_barcodes     (ApertureViewfinder *self,
                                                                      gboolean            detect_barcodes);
gboolean                 aperture_viewfinder_get_detect_barcodes     (ApertureViewfinder *self);

void                     aperture_viewfinder_take_picture_async          (ApertureViewfinder *self,
                                                                          GCancellable *cancellable,
                                                                          GAsyncReadyCallback callback,
                                                                          gpointer user_data);
GdkPixbuf               *aperture_viewfinder_take_picture_finish         (ApertureViewfinder *self,
                                                                          GAsyncResult *result,
                                                                          GError **error);

void                     aperture_viewfinder_start_recording_to_file     (ApertureViewfinder *self,
                                                                          const char *file,
                                                                          GError **err);
void                     aperture_viewfinder_stop_recording_async        (ApertureViewfinder *self,
                                                                          GCancellable *cancellable,
                                                                          GAsyncReadyCallback callback,
                                                                          gpointer user_data);
gboolean                 aperture_viewfinder_stop_recording_finish       (ApertureViewfinder *self,
                                                                          GAsyncResult *result,
                                                                          GError **error);

#define APERTURE_MEDIA_CAPTURE_ERROR (aperture_media_capture_error_quark())
GQuark aperture_media_capture_error_quark (void);


G_END_DECLS
