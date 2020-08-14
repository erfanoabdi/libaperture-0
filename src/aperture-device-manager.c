/* aperture-device-manager.c
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
 * SECTION:aperture-device-manager
 * @title: ApertureDeviceManager
 * @short_description: Finds and monitors camera devices
 *
 * #ApertureDeviceManager finds and monitors camera devices that can be used
 * in Aperture. It provides functions for listing cameras, and signals for
 * detecting when cameras are plugged in or unplugged.
 */


#include <gst/gst.h>
#include <gio/gio.h>
#include "aperture-device-manager.h"
#include "devices/aperture-device.h"
#include "private/aperture-camera-private.h"


struct _ApertureDeviceManager
{
  GObject parent_instance;

  GstDeviceMonitor *monitor;
  GListStore *device_list;
};

G_DEFINE_TYPE (ApertureDeviceManager, aperture_device_manager, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_NUM_CAMERAS,
  N_PROPS
};
static GParamSpec *props[N_PROPS];

enum {
  SIGNAL_CAMERA_ADDED,
  SIGNAL_CAMERA_REMOVED,
  N_SIGNALS,
};
static guint signals[N_SIGNALS];


/* Finds a matching ApertureCamera in our list, given a GstDevice */
static gboolean
find_device_in_list_model (GListModel *model, GstDevice *gst_device, uint *position)
{
  g_autoptr(ApertureCamera) camera = NULL;
  int i;
  int n = g_list_model_get_n_items (model);

  for (i = 0; i < n; i ++) {
    g_set_object (&camera, g_list_model_get_item (model, i));
    if (aperture_camera_get_gst_device (camera) == gst_device) {
      *position = i;
      return TRUE;
    }
  }

  return FALSE;
}


/* Gets an #ApertureCamera instance from the #ApertureDevice implementation,
 * and adds it to the device manager's list. */
static gboolean
add_camera (ApertureDeviceManager *self, GstDevice *gst_device)
{
  ApertureDevice *device = aperture_device_get_instance ();
  ApertureCamera *camera = aperture_device_get_camera (device, gst_device);

  /* aperture_device_get_camera might return NULL, which means we should
   * ignore this device */
  if (camera != NULL) {
    g_list_store_append (self->device_list, camera);
    return TRUE;
  }

  return FALSE;
}


static gboolean
on_bus_message (GstBus *bus, GstMessage *message, gpointer user_data)
{
  ApertureDeviceManager *self = APERTURE_DEVICE_MANAGER (user_data);
  g_autoptr(GstDevice) device = NULL;
  guint device_index = -1;

  switch (message->type) {
  case GST_MESSAGE_DEVICE_ADDED:
    gst_message_parse_device_added (message, &device);
    g_debug ("New camera detected: %s", gst_device_get_display_name (device));

    if (add_camera (self, device)) {
      device_index = g_list_model_get_n_items (G_LIST_MODEL (self->device_list)) - 1;

      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NUM_CAMERAS]);
      g_signal_emit (self, signals[SIGNAL_CAMERA_ADDED], 0, device_index);
    }

    break;
  case GST_MESSAGE_DEVICE_REMOVED:
    gst_message_parse_device_removed (message, &device);
    g_debug ("Camera removed: %s", gst_device_get_display_name (device));

    if (find_device_in_list_model (G_LIST_MODEL (self->device_list), device, &device_index)) {
      g_list_store_remove (self->device_list, device_index);
    }

    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NUM_CAMERAS]);
    g_signal_emit (self, signals[SIGNAL_CAMERA_REMOVED], 0, device_index);

    break;
  default:
    break;
  }

  return G_SOURCE_CONTINUE;
}


/* VFUNCS */


static void
aperture_device_manager_finalize (GObject *object)
{
  ApertureDeviceManager *self = APERTURE_DEVICE_MANAGER (object);
  g_autoptr(GstBus) bus = gst_device_monitor_get_bus (self->monitor);

  gst_bus_remove_watch (bus);
  gst_device_monitor_stop (self->monitor);
  g_clear_object (&self->monitor);

  g_clear_object (&self->device_list);

  G_OBJECT_CLASS (aperture_device_manager_parent_class)->finalize (object);
}


static void
aperture_device_manager_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  ApertureDeviceManager *self = APERTURE_DEVICE_MANAGER (object);

  switch (prop_id) {
  case PROP_NUM_CAMERAS:
    g_value_set_int (value, aperture_device_manager_get_num_cameras (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


/* INIT */


static void
aperture_device_manager_class_init (ApertureDeviceManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = aperture_device_manager_finalize;
  object_class->get_property = aperture_device_manager_get_property;

  props[PROP_NUM_CAMERAS] =
    g_param_spec_int ("num-cameras",
                      "Number of cameras",
                      "The number of cameras available.",
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, N_PROPS, props);

  /**
   * ApertureDeviceManager::camera-added:
   * @self: the #ApertureDeviceManager
   * @camera_index: the index of the new camera
   *
   * Emitted when a camera is discovered.
   *
   * Since: 0.1
   */
  signals[SIGNAL_CAMERA_ADDED] =
    g_signal_new ("camera-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1, G_TYPE_INT);

  /**
   * ApertureDeviceManager::camera-removed:
   * @self: the #ApertureDeviceManager
   * @camera_index: the index the camera had
   *
   * Emitted when a camera is removed (typically because it has been unplugged).
   *
   * The camera's index is provided, but remember that it is the camera's *old*
   * index. That index is now either invalid or points to a different camera.
   * You can still use it to check whether the camera you were using is the
   * one that was removed.
   *
   * Since: 0.1
   */
  signals[SIGNAL_CAMERA_REMOVED] =
    g_signal_new ("camera-removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1, G_TYPE_INT);
}


static void
aperture_device_manager_init (ApertureDeviceManager *self)
{
  ApertureDevice *device = aperture_device_get_instance ();
  g_autolist(ApertureCamera) cameras = aperture_device_list_cameras (device);
  g_autoptr(GstBus) bus = NULL;
  g_autolist(GstDevice) devices = NULL;
  GList *i;

  self->monitor = gst_device_monitor_new ();
  gst_device_monitor_add_filter (self->monitor, "Source/Video", NULL);
  gst_device_monitor_start (self->monitor);

  self->device_list = g_list_store_new (APERTURE_TYPE_CAMERA);

  /* Add built-in cameras from the device */
  for (i = cameras; i != NULL; i = i->next) {
    g_list_store_append (self->device_list, i->data);
  }

  /* Add devices from GstDeviceMonitor */
  devices = gst_device_monitor_get_devices (self->monitor);
  for (i = devices; i != NULL; i = i->next) {
    add_camera (self, i->data);
  }

  bus = gst_device_monitor_get_bus (self->monitor);
  gst_bus_add_watch (bus, on_bus_message, self);
}


/* PUBLIC */


/**
 * aperture_device_manager_get_instance:
 *
 * Gets an #ApertureDeviceManager.
 *
 * Returns: (transfer full): an #ApertureDeviceManager
 * Since: 0.1
 */
ApertureDeviceManager *
aperture_device_manager_get_instance (void)
{
  static ApertureDeviceManager *instance;

  if (instance == NULL) {
    instance = g_object_new (APERTURE_TYPE_DEVICE_MANAGER, NULL);
    g_object_add_weak_pointer (G_OBJECT (instance), (gpointer *)&instance);
  } else {
    g_object_ref (instance);
  }

  return instance;
}


/**
 * aperture_device_manager_get_num_cameras:
 * @self: an #ApertureDeviceManager
 *
 * Gets the number of available cameras.
 *
 * Returns: the number of available cameras
 * Since: 0.1
 */
int
aperture_device_manager_get_num_cameras (ApertureDeviceManager *self)
{
  g_return_val_if_fail (APERTURE_IS_DEVICE_MANAGER (self), 0);

  return g_list_model_get_n_items (G_LIST_MODEL (self->device_list));
}


/**
 * aperture_device_manager_next_camera:
 * @self: an #ApertureDeviceManager
 * @idx: a camera index
 *
 * Gets the next camera index after @idx, wrapping around if necesary. If there
 * are no cameras available, returns -1.
 *
 * Returns: the index of the next camera, or -1 if there are no cameras
 * Since: 0.1
 */
int
aperture_device_manager_next_camera (ApertureDeviceManager *self, int idx)
{
  int num_cameras;
  g_return_val_if_fail (APERTURE_IS_DEVICE_MANAGER (self), 0);
  g_return_val_if_fail (idx >= 0, 0);

  num_cameras = aperture_device_manager_get_num_cameras (self);

  if (num_cameras == 0) {
    return -1;
  }

  idx ++;

  if (idx >= num_cameras) {
    return 0;
  } else {
    return idx;
  }
}


/**
 * aperture_device_manager_get_camera:
 * @self: an #ApertureDeviceManager
 * @idx: a camera index
 *
 * Gets an #ApertureCamera object for the given camera index.
 *
 * Returns: (transfer full): the #ApertureCamera at @idx
 */
ApertureCamera *
aperture_device_manager_get_camera (ApertureDeviceManager *self, int idx)
{
  g_return_val_if_fail (APERTURE_IS_DEVICE_MANAGER (self), NULL);
  g_return_val_if_fail (idx >= 0, NULL);
  g_return_val_if_fail (idx < g_list_model_get_n_items (G_LIST_MODEL (self->device_list)), NULL);

  return g_list_model_get_item (G_LIST_MODEL (self->device_list), idx);
}
