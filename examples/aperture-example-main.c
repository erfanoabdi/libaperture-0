/*
 * Copyright (C) 2020 James Westman
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      James Westman <james@flyingpimonster.net>
 */

/**
 * A simple example program that prints the amount of available space in the
 * user's Pictures directory, using Aperture's disk_space functions.
 */


#include <stdio.h>
#include <aperture/aperture.h>


void callback (GObject *source, GAsyncResult *res, gpointer user_data);

int
main (void)
{
  GMainLoop *loop;
  const gchar *path;

  loop = g_main_loop_new (NULL, TRUE);

  path = g_get_user_special_dir (G_USER_DIRECTORY_PICTURES);
  printf ("Your Pictures directory is at %s\n", path);

  aperture_disk_space_get_free_space (path, callback, loop);

  g_main_loop_run (loop);
  g_main_loop_unref (loop);

  return 0;
}

void
callback (GObject *_source, GAsyncResult *res, gpointer user_data)
{
  GError *error;
  GMainLoop *loop = user_data;
  guint64 free_space;
  gchar *space_str;

  free_space = aperture_disk_space_get_free_space_finish (res, &error);

  if (error) {
    printf ("%s\n", error->message);
  } else {
    space_str = g_format_size (free_space);
    printf ("%s (%lu bytes) available\n", space_str, free_space);
    g_free (space_str);
  }

  g_main_loop_quit (loop);
}
