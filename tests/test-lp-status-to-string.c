/* Copyright (C) 2015 PUC-Rio/Laboratorio TeleMidia

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

#include "tests.h"

#define CHECK_STATUS_STRING(s, str)\
  ASSERT (streq (lp_status_to_string (s), str))

int
main (void)
{
  size_t i;
  for (i = 0; i <= LP_STATUS_LAST_STATUS; i++)
    {
      lp_status_t s = (lp_status_t) i;
      printf ("%d\n", (int) i);
      switch (i)
        {
        case LP_STATUS_SUCCESS:
          CHECK_STATUS_STRING (s, "no error has occurred");
          break;
        case LP_STATUS_NULL_POINTER:
          CHECK_STATUS_STRING (s, "NULL pointer");
          break;
        case LP_STATUS_READ_ERROR:
          CHECK_STATUS_STRING (s, "error while reading from input stream");
          break;
        case LP_STATUS_WRITE_ERROR:
          CHECK_STATUS_STRING (s, "error while writing to output stream");
          break;
        case LP_STATUS_FILE_NOT_FOUND:
          CHECK_STATUS_STRING (s, "file not found");
          break;
        case LP_STATUS_NEGATIVE_COUNT:
          CHECK_STATUS_STRING (s, "negative number used where it is not allowed");
          break;
        case LP_STATUS_INVALID_PARENT:
          CHECK_STATUS_STRING(s, "invalid parent");
          break;
        case LP_STATUS_LAST_STATUS:
          CHECK_STATUS_STRING (s, "<unknown error status>");
          break;
        default:
          ASSERT (0);
        }
    }

  exit (EXIT_SUCCESS);
}
