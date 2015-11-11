/* lp-status.c -- Error status.
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


/* Exported functions.  */

/*-
 * lp_status_to_string:
 * @status: a #lp_status_t
 *
 * Returns a human-readable description of a #cairo_status_t.
 *
 * Returns: a string representation of the status.
 */
const char *
lp_status_to_string (lp_status_t status)
{
  switch (status)
    {
    case LP_STATUS_SUCCESS:
      return "no error has occurred";
    case LP_STATUS_NULL_POINTER:
      return "NULL pointer";
    case LP_STATUS_READ_ERROR:
      return "error while reading from input stream";
    case LP_STATUS_WRITE_ERROR:
      return "error while writing to output stream";
    case LP_STATUS_FILE_NOT_FOUND:
      return "file not found";
    case LP_STATUS_NEGATIVE_COUNT:
      return "negative number used where it is not allowed";
    case LP_STATUS_INVALID_PARENT:
      return "invalid parent";
    case LP_STATUS_LAST_STATUS:
    default:
      return "<unknown error status>";
    }
}
