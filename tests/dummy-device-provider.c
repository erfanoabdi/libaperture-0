/* dummy-device-provider.c
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


#include "dummy-device-provider.h"
#include "dummy-device.h"


struct _DummyDeviceProvider
{
  GstDeviceProvider parent_instance;

  GList *devices;
};

G_DEFINE_TYPE (DummyDeviceProvider, dummy_device_provider, GST_TYPE_DEVICE_PROVIDER)

enum {
  PROP_0,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];


void dummy_device_provider_add (DummyDeviceProvider *self);
void dummy_device_provider_remove (DummyDeviceProvider *self);


/* VFUNCS */


static void
dummy_device_provider_finalize (GObject *object)
{
  DummyDeviceProvider *self = (DummyDeviceProvider *)object;

  g_list_free_full (self->devices, g_object_unref);

  G_OBJECT_CLASS (dummy_device_provider_parent_class)->finalize (object);
}


static GList *
dummy_device_provider_probe (GstDeviceProvider *self)
{
  return g_list_copy_deep (self->devices, (GCopyFunc) g_object_ref, NULL);
}


static gboolean
dummy_device_provider_start (GstDeviceProvider *provider)
{
  return TRUE;
}


static void
dummy_device_provider_stop (GstDeviceProvider *provider)
{
  DummyDeviceProvider *self = DUMMY_DEVICE_PROVIDER (provider);

  while (self->devices) {
    dummy_device_provider_remove (self);
  }
}


/* INIT */


static void
dummy_device_provider_class_init (DummyDeviceProviderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstDeviceProviderClass *device_provider_class = GST_DEVICE_PROVIDER_CLASS (klass);

  object_class->finalize = dummy_device_provider_finalize;

  device_provider_class->probe = dummy_device_provider_probe;
  device_provider_class->start = dummy_device_provider_start;
  device_provider_class->stop = dummy_device_provider_stop;

  gst_device_provider_class_set_static_metadata (device_provider_class,
                                                 "Dummy device provider",
                                                 "Source/Video",
                                                 "Dummy device provider for unit tests",
                                                 "James Westman <james@flyingpimonster.net>");
}


static void
dummy_device_provider_init (DummyDeviceProvider *self)
{
}


/* PUBLIC */


/**
 * PRIVATE:dummy_device_provider_new:
 *
 * Creates a new #DummyDeviceProvider.
 *
 * Returns: (transfer full): a new #DummyDeviceProvider
 */
DummyDeviceProvider *
dummy_device_provider_new (void)
{
  return g_object_new (DUMMY_TYPE_DEVICE_PROVIDER, NULL);
}


/**
 * PRIVATE:dummy_device_provider_add:
 * @self: a #DummyDeviceProvider
 *
 * Adds a new device to the dummy provider, causing a device added message to
 * be posted on the device monitor's bus.
 *
 * Note that the device manager starts with one device by default, so this is
 * unnecessary unless you're testing adding and removing devices, or testing
 * multiple devices.
 */
void
dummy_device_provider_add (DummyDeviceProvider *self)
{
  DummyDevice *device;

  g_return_if_fail (DUMMY_IS_DEVICE_PROVIDER (self));

  device = dummy_device_new ();
  gst_device_provider_device_add (GST_DEVICE_PROVIDER (self), GST_DEVICE (device));
  self->devices = g_list_prepend (self->devices, device);
}


/**
 * PRIVATE:dummy_device_provider_remove:
 * @self: a #DummyDeviceProvider
 *
 * Removes a device from the device provider, causing a device removed message
 * to be posted on the device monitor's bus. Devices are removed opposite the
 * order they were added (in a stack structure).
 */
void
dummy_device_provider_remove (DummyDeviceProvider *self)
{
  GList *first;

  g_return_if_fail (DUMMY_IS_DEVICE_PROVIDER (self));

  if (!self->devices) {
    return;
  }

  first = g_list_first (self->devices);
  gst_device_provider_device_remove (GST_DEVICE_PROVIDER (self), GST_DEVICE (first->data));
  self->devices = g_list_delete_link (first, first);
}


/**
 * PRIVATE:dummy_device_provider_register:
 *
 * Registers the dummy device provider.
 *
 * This should only be called once.
 */
void
dummy_device_provider_register (void)
{
  g_autoptr(GstDeviceProvider) dummy = NULL;
  g_autolist(GstDeviceProviderFactory) providers = NULL;
  GList *i;

  /* Register the dummy provider */
  gst_device_provider_register (NULL, "dummy-device-provider", G_MAXINT, DUMMY_TYPE_DEVICE_PROVIDER);

  /* Hide all the other providers so we're only working with our dummy devices.
   * This allows us to test things like no-cameras state, even on machines
   * that have real cameras attached. */
  dummy = gst_device_provider_factory_get_by_name ("dummy-device-provider");
  providers = gst_device_provider_factory_list_get_device_providers (GST_RANK_NONE);

  for (i = providers; i != NULL; i = i->next) {
    g_autofree char *name = gst_object_get_name (GST_OBJECT (i->data));
    if (g_strcmp0 (name, "dummy-device-provider") != 0) {
      g_print ("%s\n", name);
      gst_device_provider_hide_provider (dummy, name);
    }
  }
}
