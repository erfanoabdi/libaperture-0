/* aperture-device.h
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

#include <glib-object.h>
#include <gio/gio.h>


G_BEGIN_DECLS


#define APERTURE_TYPE_CAMERA (aperture_camera_get_type ())
G_DECLARE_DERIVABLE_TYPE (ApertureCamera, aperture_camera, APERTURE, CAMERA, GObject)


void            aperture_camera_do_flash_async  (ApertureCamera      *self,
                                                 GCancellable        *cancellable,
                                                 GAsyncReadyCallback  callback,
                                                 gpointer             user_data);
gboolean        aperture_camera_do_flash_finish (ApertureCamera *self,
                                                 GAsyncResult *result,
                                                 GError **error);

void            aperture_camera_set_torch    (ApertureCamera *self,
                                              gboolean state);


G_END_DECLS
