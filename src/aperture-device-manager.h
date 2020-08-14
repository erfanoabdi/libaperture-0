/* aperture-device-manager.h
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

#include <glib-object.h>

#include "aperture-camera.h"


G_BEGIN_DECLS


#define APERTURE_TYPE_DEVICE_MANAGER (aperture_device_manager_get_type())
G_DECLARE_FINAL_TYPE (ApertureDeviceManager, aperture_device_manager, APERTURE, DEVICE_MANAGER, GObject)


ApertureDeviceManager *aperture_device_manager_get_instance    (void);

int                    aperture_device_manager_get_num_cameras (ApertureDeviceManager *self);
ApertureCamera        *aperture_device_manager_next_camera     (ApertureDeviceManager *self,
                                                                ApertureCamera        *camera);
ApertureCamera        *aperture_device_manager_get_camera      (ApertureDeviceManager *self,
                                                                int                    idx);


G_END_DECLS
