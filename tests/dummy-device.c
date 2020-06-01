/* dummy-device.c
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


#include "dummy-device.h"


struct _DummyDevice
{
  GstDevice parent_instance;
};

G_DEFINE_TYPE (DummyDevice, dummy_device, GST_TYPE_DEVICE)

enum {
  PROP_0,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];


/* VFUNCS */


static void
dummy_device_finalize (GObject *object)
{
  DummyDevice *self = (DummyDevice *)object;

  G_OBJECT_CLASS (dummy_device_parent_class)->finalize (object);
}


static void
dummy_device_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  DummyDevice *self = DUMMY_DEVICE (object);

  switch (prop_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


static void
dummy_device_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  DummyDevice *self = DUMMY_DEVICE (object);

  switch (prop_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


static GstElement *
dummy_device_create_element (GstDevice *device, const char *name)
{
  GstElement *element = gst_element_factory_make ("videotestsrc", NULL);
  g_object_set (element, "pattern", 18, NULL);
  return element;
}


/* INIT */


static void
dummy_device_class_init (DummyDeviceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstDeviceClass *device_class = GST_DEVICE_CLASS (klass);

  object_class->finalize = dummy_device_finalize;
  object_class->get_property = dummy_device_get_property;
  object_class->set_property = dummy_device_set_property;

  device_class->create_element = dummy_device_create_element;
}


static void
dummy_device_init (DummyDevice *self)
{
}


/* PUBLIC */


/**
 * PRIVATE:dummy_device_new:
 *
 * Creates a new #DummyDevice.
 *
 * Returns: (transfer full): a new #DummyDevice
 */
DummyDevice *
dummy_device_new (void)
{
  GstCaps *caps = gst_caps_new_any ();

  return g_object_new (DUMMY_TYPE_DEVICE,
                       "caps", caps,
                       "device-class", "Source/Video",
                       "display-name", "Dummy Camera",
                       NULL);
}

