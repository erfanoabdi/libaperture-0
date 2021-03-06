/* utils.h
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


#include <aperture.h>
#include <glib-object.h>


G_BEGIN_DECLS


typedef struct {
  gboolean loop_running;
  int calls;
  uint timeout_id;
} TestUtilsCallback;


void testutils_callback_init                  (TestUtilsCallback     *self);
void testutils_callback_assert_called         (TestUtilsCallback     *self,
                                               int                    timeout);
void testutils_callback_assert_already_called (TestUtilsCallback *self);
void testutils_callback_call                  (TestUtilsCallback     *self);

void testutils_wait_for_device_change         (ApertureDeviceManager *manager);

void testutils_assert_quadrants_pixbuf        (GdkPixbuf *pixbuf);


G_END_DECLS
