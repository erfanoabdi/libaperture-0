/* aperture-demo.c
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


#include <stdio.h>
#include <aperture.h>

#include "aperture-demo-window.h"


static void
on_activate (GtkApplication *app)
{
  ApertureDemoWindow *window = aperture_demo_window_new (app);
  gtk_widget_show (GTK_WIDGET (window));
}


int
main (int argc, char **argv)
{
  g_autoptr(GtkApplication) app;

  aperture_init (&argc, &argv);

  app = gtk_application_new ("io.gnome.Aperture.Demo", G_APPLICATION_FLAGS_NONE);

  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}

