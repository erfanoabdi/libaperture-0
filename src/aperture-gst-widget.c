/* aperture-gst-widget.c
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

/* TODO: Use GL sinks where possible. Watch out for bugs. */


#include "aperture-gst-widget.h"


struct _ApertureGstWidget
{
  GtkBin parent_instance;

  GstElement *sink;
};

G_DEFINE_TYPE (ApertureGstWidget, aperture_gst_widget, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_SINK,
  N_PROPS
};
static GParamSpec *props[N_PROPS];


/* VFUNCS */


static void
aperture_gst_widget_finalize (GObject *object)
{
  ApertureGstWidget *self = APERTURE_GST_WIDGET (object);

  g_clear_object (&self->sink);

  G_OBJECT_CLASS (aperture_gst_widget_parent_class)->finalize (object);
}


static void
aperture_gst_widget_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  ApertureGstWidget *self = APERTURE_GST_WIDGET (object);

  switch (prop_id) {
  case PROP_SINK:
    g_value_set_object (value, aperture_gst_widget_get_sink (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


/* INIT */


static void
aperture_gst_widget_class_init (ApertureGstWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = aperture_gst_widget_finalize;
  object_class->get_property = aperture_gst_widget_get_property;

  /**
   * ApertureGstWidget:sink:
   *
   * The widget's GStreamer sink.
   *
   * To display media through the widget, add this sink to a pipeline and link
   * a media source to it.
   *
   * For more about creating media pipelines in GStreamer, see
   * [the GStreamer manual](https://gstreamer.freedesktop.org/documentation/application-development/basics/index.html?gi-language=c).
   *
   * Since: 0.1
   */
  props[PROP_SINK] =
    g_param_spec_object ("sink",
                         "Sink",
                         "The widget's GStreamer sink",
                         GST_TYPE_ELEMENT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, props);
}


static void
aperture_gst_widget_init (ApertureGstWidget *self)
{
  GtkWidget *widget;

  self->sink = gst_element_factory_make ("gtksink", NULL);

  if (self->sink == NULL) {
    g_critical ("Could not create a gtksink for ApertureGstWidget. Are the right gstreamer packages installed?");
    return;
  }

  g_object_get (self->sink, "widget", &widget, NULL);
  gtk_widget_show (widget);
  gtk_container_add (GTK_CONTAINER (self), widget);
}


/* PUBLIC */


/**
 * aperture_gst_widget_new:
 *
 * Creates a new #ApertureGstWidget.
 *
 * Returns: (transfer full): a new #ApertureGstWidget
 * Since: 0.1
 */
ApertureGstWidget *
aperture_gst_widget_new (void)
{
  return g_object_new (APERTURE_TYPE_GST_WIDGET, NULL);
}


/**
 * aperture_gst_widget_get_sink:
 * @self: an #ApertureGstWidget
 *
 * Gets the GStreamer sink associated with this widget.
 *
 * Returns: (transfer none): the sink element associated with this widget
 * Since: 0.1
 */
GstElement *
aperture_gst_widget_get_sink (ApertureGstWidget *self)
{
  g_return_val_if_fail (APERTURE_IS_GST_WIDGET (self), NULL);
  return self->sink;
}
