/* aperture.c
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

#include <glib-2.0/glib.h>
#include <gst/gst.h>

#include <gdk/gdk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#endif
#if defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#endif
#if defined (GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#endif
#if defined (GDK_WINDOWING_WAYLAND)
#include <gdk/gdkwayland.h>
#endif

static gboolean initted = FALSE;

void
aperture_init ()
{
  if (initted)
    return;

  gst_init (0, NULL);

  initted = TRUE;
}


guintptr
aperture_get_window_handle (GdkWindow *window)
{
  #if defined (GDK_WINDOWING_WIN32)
    if (GDK_IS_WIN32_WINDOW (window)) {
      return (guintptr) GDK_WINDOW_HWND (window);
    }
  #endif
  #if defined (GDK_WINDOWING_QUARTZ)
    if (GDK_IS_QUARTZ_WINDOW (window)) {
      return (guintptr) gdk_quartz_window_get_nsview (window);
    }
  #endif
  #if defined (GDK_WINDOWING_X11)
    if (GDK_IS_X11_WINDOW (window)) {
      return (guintptr) GDK_WINDOW_XID (window);
    }
  #endif
  #if defined (GDK_WINDOWING_WAYLAND)
    if (GDK_IS_WAYLAND_WINDOW (window)) {
      return (guintptr) gdk_wayland_window_get_wl_surface (window);
    }
  #endif

  return (guintptr) NULL;
}

gboolean
aperture_is_wayland_display ()
{
  #if defined (GDK_WINDOWING_WAYLAND)
    GdkDisplay *display;

    display = gdk_display_get_default ();
    return GDK_IS_WAYLAND_DISPLAY (display);
  #else
    return FALSE;
  #endif
}

gpointer
aperture_get_wayland_display_handle ()
{
  #if defined (GDK_WINDOWING_WAYLAND)
    GdkDisplay *display;

    display = gdk_display_get_default ();
    if (GDK_IS_WAYLAND_DISPLAY (display)) {
      return (gpointer) gdk_wayland_display_get_wl_display (display);
    } else {
      g_critical ("Cannot get wayland display handle because Wayland support is not enabled!");
    }
  #else
    g_critical ("Cannot get wayland display handle because Wayland support is not enabled!");
  #endif

  return NULL;
}

#define GST_WAYLAND_DISPLAY_HANDLE_CONTEXT_TYPE "GstWaylandDisplayHandleContextType"
GstContext *
gst_wayland_display_handle_context_new (struct wl_display * display)
{
  GstContext *context =
      gst_context_new (GST_WAYLAND_DISPLAY_HANDLE_CONTEXT_TYPE, TRUE);
  gst_structure_set (gst_context_writable_structure (context),
      "handle", G_TYPE_POINTER, display, NULL);
  return context;
}

/**
 * Creates a GstContext with the current Wayland display handle.
 */
GstContext *
aperture_create_wayland_context ()
{
  GstContext *result;
  GstStructure *structure;
  gpointer handle;

  /*result = gst_context_new ("GstWaylandDisplayHandleContextType", TRUE);
  structure = gst_context_writable_structure (result);
  handle = aperture_get_wayland_display_handle ();
  gst_structure_set (structure, "handle", G_TYPE_POINTER, handle, NULL);*/
  return gst_wayland_display_handle_context_new (aperture_get_wayland_display_handle ());

  return result;
}

