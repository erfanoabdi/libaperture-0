/* aperture-camera-private.h
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
#include "aperture-camera.h"


G_BEGIN_DECLS


struct _ApertureCameraClass
{
  GObjectClass parent_class;

  void     (* do_flash_async)  (ApertureCamera *device,
                                GCancellable *cancellable,
                                GAsyncReadyCallback callback,
                                gpointer user_data);
  gboolean (* do_flash_finish) (ApertureCamera *device,
                                GAsyncResult *result,
                                GError **error);

  void (* set_torch)    (ApertureCamera *device,
                         gboolean flash);

  int (* get_source_element) (ApertureCamera *self);
};


ApertureCamera *aperture_camera_new (int idx);

int aperture_camera_get_source_element (ApertureCamera *camera);


G_END_DECLS
