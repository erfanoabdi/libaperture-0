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


#include <gtk/gtk.h>
#include <gst/app/app.h>

#include "dummy-device.h"


struct _DummyDevice
{
  GstDevice parent_instance;

  gchar *image;
};

G_DEFINE_TYPE (DummyDevice, dummy_device, GST_TYPE_DEVICE)

enum {
  PROP_0,
  PROP_IMAGE,
  N_PROPS
};
static GParamSpec *props[N_PROPS];


static GstElement *
create_image_source (const gchar *image)
{
  GstElement *bin = gst_bin_new (NULL);
  g_autoptr(GstPad) pad = NULL;
  GstPad *ghost_pad;

  GBytes *buffer_bytes;
  gsize buf_size;
  GstBuffer *buffer;
  gpointer buf_raw;
  GError *error = NULL;

  GstElement *appsrc;
  GstElement *decodebin;
  GstElement *imagefreeze;

  appsrc = gst_element_factory_make ("appsrc", NULL);
  decodebin = gst_element_factory_make ("pngdec", NULL);
  imagefreeze = gst_element_factory_make ("imagefreeze", NULL);

  buffer_bytes = g_resources_lookup_data (image, G_RESOURCE_LOOKUP_FLAGS_NONE, &error);
  g_assert_no_error (error);

  buf_raw = g_bytes_unref_to_data (buffer_bytes, &buf_size);
  buffer = gst_buffer_new_wrapped (buf_raw, buf_size);

  gst_app_src_push_buffer (GST_APP_SRC (appsrc), buffer);

  gst_bin_add_many (GST_BIN (bin), appsrc, decodebin, imagefreeze, NULL);
  gst_element_link_many (appsrc, decodebin, imagefreeze, NULL);

  pad = gst_element_get_static_pad (imagefreeze, "src");
  ghost_pad = gst_ghost_pad_new ("src", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (bin, ghost_pad);

  return bin;
}


/* VFUNCS */


static void
dummy_device_finalize (GObject *object)
{
  DummyDevice *self = (DummyDevice *)object;

  g_free (self->image);

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
  case PROP_IMAGE:
    g_value_set_string (value, dummy_device_get_image (self));
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
  case PROP_IMAGE:
    dummy_device_set_image (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


static GstElement *
dummy_device_create_element (GstDevice *device, const char *name)
{
  DummyDevice *self;
  GstElement *element = NULL;

  g_return_val_if_fail (DUMMY_IS_DEVICE (device), NULL);
  self = DUMMY_DEVICE (device);

  if (self->image) {
    return create_image_source (self->image);
  } else {
    element = gst_element_factory_make ("videotestsrc", NULL);
    g_object_set (element, "pattern", 18, NULL);
    return element;
  }
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

  props [PROP_IMAGE] =
    g_param_spec_string ("image",
                         "Pixbuf",
                         "Image to use instead of test stream",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);
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


const char *
dummy_device_get_image (DummyDevice *self)
{
  return self->image;
}


void
dummy_device_set_image (DummyDevice *self, const char *image)
{
  g_return_if_fail (DUMMY_IS_DEVICE (self));

  g_clear_pointer (&self->image, g_free);
  self->image = g_strdup (image);
}
