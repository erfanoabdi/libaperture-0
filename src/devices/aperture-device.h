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

#include <gst/gst.h>

#include "aperture-camera.h"


G_BEGIN_DECLS


#define APERTURE_TYPE_DEVICE (aperture_device_get_type())
G_DECLARE_DERIVABLE_TYPE (ApertureDevice, aperture_device, APERTURE, DEVICE, GObject)


struct _ApertureDeviceClass
{
  GObjectClass parent_class;

  const char *device_class;

  GList          * (* list_cameras) (ApertureDevice *device);
  ApertureCamera * (* get_camera)   (ApertureDevice *device,
                                     GstDevice *gst_device);
};


ApertureDevice *aperture_device_get_instance (void);

GList          *aperture_device_list_cameras (ApertureDevice *device);
ApertureCamera *aperture_device_get_camera   (ApertureDevice *device, GstDevice *gst_device);


G_END_DECLS
