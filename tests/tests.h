/* tests.h -- Common declarations for tests.
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

#ifndef TESTS_H
#define TESTS_H

#include <config.h>
#include <stdio.h>

#include "play.h"
#include "play-internal.h"

#define ASSERT(cond)                                            \
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

static ATTR_UNUSED void
ASSERT_MEDIA_IS_EMPTY (lp_media_t *media, const char *uri)
{
  ASSERT (media != NULL);
  ASSERT (media->status == LP_STATUS_SUCCESS);
  ASSERT (media->ref_count == 1);
  ASSERT (media->parent == NULL);
  ASSERT (g_strcmp0 (media->uri, uri) == 0);
  ASSERT (media->children == NULL);
  ASSERT (media->properties != NULL);
  ASSERT (g_hash_table_size (media->properties) == 0);
}

#endif /* TESTS_H */
