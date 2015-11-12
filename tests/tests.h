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

/* *INDENT-OFF* */
PRAGMA_DIAG_IGNORE (-Wfloat-equal)
/* *INDENT-ON* */

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

static ATTR_UNUSED void
assert_media_is_empty (lp_media_t *media, const char *uri)
{
  assert (media != NULL);
  assert (media->status == LP_STATUS_SUCCESS);
  assert (media->ref_count == 1);
  assert (media->parent == NULL);
  assert (g_strcmp0 (media->uri, uri) == 0);
  assert (media->children == NULL);
  assert (media->handlers == NULL);
  assert (media->properties != NULL);
}

#endif /* TESTS_H */
