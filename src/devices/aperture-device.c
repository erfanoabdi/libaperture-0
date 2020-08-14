/* aperture-device.c
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


#include <gst/gst.h>

#include "aperture-device.h"
#include "private/aperture-camera-private.h"


G_DEFINE_TYPE (ApertureDevice, aperture_device, G_TYPE_OBJECT)


/* Autodetects what device this is, and return an instance of the correct
 * #ApertureDevice implementation (or the default, if no supported device is
 * detected) . */
static ApertureDevice *
get_device (void) {
  return g_object_new (APERTURE_TYPE_DEVICE, NULL);
}


/* VFUNCS */


static GList *
aperture_device_list_cameras_impl (ApertureDevice *self)
{
  return NULL;
}


static ApertureCamera *
aperture_device_get_camera_impl (ApertureDevice *self, GstDevice *device)
{
  return aperture_camera_new (device);
}


/* INIT */


static void
aperture_device_class_init (ApertureDeviceClass *klass)
{
  klass->device_class = "unrecognized";

  klass->list_cameras = aperture_device_list_cameras_impl;
  klass->get_camera = aperture_device_get_camera_impl;
}

static void
aperture_device_init (ApertureDevice *self)
{
}


/* INTERNAL */


/**
 * PRIVATE:aperture_device_get_instance:
 *
 * Gets the singleton instance of #ApertureDevice.
 *
 * Returns: (transfer full): the #ApertureDevice
 */
ApertureDevice *
aperture_device_get_instance (void)
{
  static ApertureDevice *device;

  if (G_UNLIKELY (device == NULL)) {
    device = get_device ();
    g_debug ("DEVICE CLASS: %s", APERTURE_DEVICE_GET_CLASS (device)->device_class);
  }

  return device;
}


/**
 * PRIVATE:aperture_device_get_camera:
 * @self: an #ApertureDevice
 * @gst_device: a #GstDevice
 *
 * Creates a new #ApertureCamera for the given #GstDevice detected by a
 * #GstDeviceMonitor.
 *
 * Sometimes, a device that is detected by GStreamer should actually be
 * skipped. In this case, the device implementation will return %NULL, and
 * the #GstDevice should not be used.
 *
 * Implementations should return a custom #ApertureCamera subclass for devices
 * that they know about, and have special functionality for. For unknown
 * devices, implementations must chain the call up so that the default
 * #ApertureCamera is used.
 *
 * Returns: (transfer full)(nullable): an #ApertureCamera, or %NULL if the
 * device should be skipped.
 */
ApertureCamera *
aperture_device_get_camera (ApertureDevice *self, GstDevice *gst_device)
{
  g_return_val_if_fail (APERTURE_IS_DEVICE (self), NULL);
  g_return_val_if_fail (GST_IS_DEVICE (gst_device), NULL);
  return APERTURE_DEVICE_GET_CLASS (self)->get_camera (self, gst_device);
}


/**
 * PRIVATE:aperture_device_list_cameras:
 * @self: an #ApertureDevice
 *
 * Gets a list of built-in cameras. Only cameras that are not detected by a
 * #GstDeviceMonitor should be listed here. For devices that are detected,
 * implementations should use get_camera() to provide an #ApertureCamera.
 *
 * Returns: (transfer full)(element-type ApertureCamera): a list of
 * #ApertureCamera objects
 */
GList *
aperture_device_list_cameras (ApertureDevice *self)
{
  g_return_val_if_fail (APERTURE_IS_DEVICE (self), NULL);
  return APERTURE_DEVICE_GET_CLASS (self)->list_cameras (self);
}
