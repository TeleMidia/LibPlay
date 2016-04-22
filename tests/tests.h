/* tests.h -- Common declarations for tests.
   Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia

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

#ifndef TESTS_H
#define TESTS_H

#include <config.h>
#include <stdio.h>

#include <glib.h>

#include "play.h"
#include "macros.h"

#undef assert
#define assert(cond)                                            \
  STMT_BEGIN                                                    \
  {                                                             \
    if (unlikely (!(cond)))                                     \
      {                                                         \
        fprintf (stderr, "%s:%d: ASSERTION FAILED!\n--> %s\n",  \
                 __FILE__, __LINE__, STRINGIFY (cond));         \
        abort ();                                               \
      }                                                         \
  }                                                             \
  STMT_END

#define SLEEP(s) g_usleep ((s) * 1000000)

#endif /* TESTS_H */
