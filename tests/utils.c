/* utils.c
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

/* Various utility functions for running tests. */

/*
 * TestUtilsCallback is a simple way to test asynchronous callbacks.
 *
 * To use it:
 * - Initialize a TestUtilsCallback using testutils_callback_init()
 * - Call testutils_callback_call() with a pointer to the TestUtilsCallback as
 * the first argument (for signals, this means you will need to use
 * g_signal_connect_swapped())
 * - Call testutils_callback_assert_called(). If the callback has been called
 * already, it will return fine. If it has not, a main loop will be run to
 * give it a chance to be called; if the timeout expires and the callback
 * still has not been called, an assertion will be made and the test will fail.
 */


#include <glib-object.h>

#include "utils.h"


static gboolean
timeout_reached (TestUtilsCallback *self)
{
  g_main_loop_quit (self->loop);
  g_assert_not_reached ();
  return G_SOURCE_REMOVE;
}


void
testutils_callback_init (TestUtilsCallback *self)
{
  self->calls = 0;
  self->loop = NULL;
}


void
testutils_callback_assert_called (TestUtilsCallback *self, int timeout)
{
  if (self->calls > 0) {
    self->calls --;
    return;
  }

  self->loop = g_main_loop_new (NULL, FALSE);
  self->timeout_id = g_timeout_add (timeout, G_SOURCE_FUNC (timeout_reached), self);
  g_main_loop_run (self->loop);

  g_source_remove (self->timeout_id);

  g_assert_true (self->calls > 0);
  self->calls --;
  self->loop = NULL;
}


void
testutils_callback_call (TestUtilsCallback *self)
{
  self->calls ++;

  if (self->loop != NULL) {
    g_main_loop_quit (self->loop);
  }
}
