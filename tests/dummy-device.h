/* dummy-device.h
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


#define DUMMY_TYPE_DEVICE (dummy_device_get_type())

G_DECLARE_FINAL_TYPE (DummyDevice, dummy_device, DUMMY, DEVICE, GstDevice)


DummyDevice *dummy_device_new       (void);

const char  *dummy_device_get_image (DummyDevice *self);
void         dummy_device_set_image (DummyDevice *self,
                                     const char  *image);

G_END_DECLS
