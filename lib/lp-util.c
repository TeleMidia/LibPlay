/* lp-util.c -- Utility functions.
   Copyright (C) 2015 PUC-Rio/Laboratorio TeleMidia

This file is part of LibPlay.

LibPlay is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

LibPlay is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License
along with LibPlay.  If not, see <http://www.gnu.org/licenses/>.  */

#include <config.h>

#include "play.h"
#include "play-internal.h"

#include "macros.h"

/* Allocates a new #GValue of the given #GType.
   This function always returns a valid pointer.  */

ATTR_USE_RESULT GValue *
_lp_util_g_value_alloc (GType type)
{
  GValue *value = g_slice_new0 (GValue);
  assert (value != NULL);
  g_value_init (value, type);
  return value;
}

/* Frees a #GValue allocated with _lp_util_g_value_alloc().  */

void
_lp_util_g_value_free (GValue *value)
{
  g_value_unset (value);
  g_slice_free (GValue, value);
}
