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

/**
 * SECTION:aperture-camera
 * @title: ApertureCamera
 * @short_description: A camera device
 *
 * #ApertureCamera represents a camera plugged into the device. It is used to
 * query information about the camera or change its parameters.
 *
 * Since: 0.1
 */


#include <gst/gst.h>

#include "aperture-camera.h"
#include "private/aperture-camera-private.h"


typedef struct {
  int idx;
} ApertureCameraPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (ApertureCamera, aperture_camera, G_TYPE_OBJECT)


/* VFUNCS */


static void
aperture_camera_finalize (GObject *object)
{
  G_OBJECT_CLASS (aperture_camera_parent_class)->finalize (object);
}


static void
aperture_camera_do_flash_async_impl (ApertureCamera *self,
                                     GCancellable *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer user_data)
{
  GTask *task;

  g_return_if_fail (APERTURE_IS_CAMERA (self));
  g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, aperture_camera_do_flash_async_impl);

  g_task_return_boolean (task, FALSE);
  g_object_unref (task);
}


static gboolean
aperture_camera_do_flash_finish_impl (ApertureCamera *self, GAsyncResult *result, GError **error)
{
  g_return_val_if_fail (APERTURE_IS_CAMERA (self), FALSE);
  g_return_val_if_fail (G_IS_TASK (result), FALSE);

  return g_task_propagate_boolean (G_TASK (result), error);
}


static int
aperture_camera_get_source_element_impl (ApertureCamera *self)
{
  ApertureCameraPrivate *priv;

  g_return_val_if_fail (APERTURE_IS_CAMERA (self), 0);

  priv = aperture_camera_get_instance_private (self);

  return priv->idx;
}


static void
aperture_camera_set_torch_impl (ApertureCamera *self, gboolean state)
{
  /* noop */
}


/* INIT */


static void
aperture_camera_class_init (ApertureCameraClass *klass)
{

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = aperture_camera_finalize;

  klass->do_flash_async = aperture_camera_do_flash_async_impl;
  klass->do_flash_finish = aperture_camera_do_flash_finish_impl;
  klass->get_source_element = aperture_camera_get_source_element_impl;
  klass->set_torch = aperture_camera_set_torch_impl;
}


static void
aperture_camera_init (ApertureCamera *self)
{
}


/* PUBLIC */


/**
 * aperture_camera_do_flash_async:
 * @self: an #ApertureCamera
 * @cancellable: (nullable): a #GCancellable
 * @callback: a #GAsyncReadyCallback to execute upon completion
 * @user_data: closure data for @callback
 *
 * Activates the flash associated with this camera. When this is done,
 * @callback will be called.
 *
 * The flash will be turned off automatically, usually after a few hundred
 * milliseconds (depending on the model of the flash device). The callback is
 * called while the flash is still on.
 *
 * Since: 0.1
 */
void
aperture_camera_do_flash_async (ApertureCamera *self, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
  APERTURE_CAMERA_GET_CLASS (self)->do_flash_async (self, cancellable, callback, user_data);
}


/**
 * aperture_camera_do_flash_finish:
 * @self: an #ApertureCamera
 * @result: a #GAsyncResult provided to callback
 * @error: a location for a #GError, or %NULL
 *
 * Gets the result of an operation started by aperture_camera_do_flash_async().
 * You should call this in your callback if you want to know whether the
 * operation succeeded; if you don't care, there is no need to call this
 * function.
 *
 * Returns: %TRUE if the flash was successfully enabled, otherwise %FALSE
 *
 * Since: 0.1
 */
gboolean
aperture_camera_do_flash_finish (ApertureCamera *self, GAsyncResult *result, GError **error)
{
  return APERTURE_CAMERA_GET_CLASS (self)->do_flash_finish (self, result, error);
}


/**
 * aperture_camera_set_torch:
 * @self: an #ApertureCamera
 * @state: %TRUE to turn the torch on, or %FALSE to turn it off
 *
 * Turns the torch associated with this camera on or off.
 *
 * Typically, flash bulbs have two modes: flash and torch. Flash is brighter,
 * but only lasts for a few hundred milliseconds. Torch is not as bright but
 * can be left on indefinitely.
 *
 * Since: 0.1
 */
void
aperture_camera_set_torch (ApertureCamera *self, gboolean state)
{
  APERTURE_CAMERA_GET_CLASS (self)->set_torch (self, state);
}


/* INTERNAL */


/**
 * PRIVATE:aperture_camera_new:
 * @idx: (transfer full): The GStreamer device for the camera
 *
 * Create a new #ApertureCamera.
 *
 * Returns: (transfer full): a newly created #ApertureCamera
 */
ApertureCamera *
aperture_camera_new (int idx)
{
  ApertureCamera *camera;
  ApertureCameraPrivate *priv;

  camera = g_object_new (APERTURE_TYPE_CAMERA, NULL);
  priv = aperture_camera_get_instance_private (camera);
  priv->idx = idx;

  return camera;
}


/**
 * PRIVATE:aperture_camera_get_source_element:
 * @self: an #ApertureCamera
 *
 * Gets a GStreamer source element that provides this camera's video feed.
 *
 * Returns: (transfer full): a newly created source #GstElement, or %NULL
 * if @previous was reconfigured instead
 */
int
aperture_camera_get_source_element (ApertureCamera *self)
{
  g_return_val_if_fail (APERTURE_IS_CAMERA (self), 0);

  return APERTURE_CAMERA_GET_CLASS (self)->get_source_element (self);
}
