/* lp-version.c -- Library version.
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

/*-
 * lp_version:
 *
 * Returns the version of LibPlay encoded in a single integer.
 * The encoding ensures that later version generate bigger numbers.
 *
 * Return value: the encoded version.
 */
int
lp_version (void)
{
  return LP_VERSION;
}

/*-
 * lp_version_string:
 *
 * Returns the version of LibPlay as a human-readable string.
 *
 * Return value: a string containing the version.
 */
const char *
lp_version_string (void)
{
  return LP_VERSION_STRING;
}
