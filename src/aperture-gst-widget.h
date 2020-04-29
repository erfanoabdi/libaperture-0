/* aperture-gst-widget.h
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

#include <gtk/gtk.h>
#include <gst/gst.h>


G_BEGIN_DECLS


#define APERTURE_TYPE_GST_WIDGET (aperture_gst_widget_get_type())
G_DECLARE_FINAL_TYPE (ApertureGstWidget, aperture_gst_widget, APERTURE, GST_WIDGET, GtkBin)


ApertureGstWidget *aperture_gst_widget_new      (void);
GstElement        *aperture_gst_widget_get_sink (ApertureGstWidget *self);


G_END_DECLS
