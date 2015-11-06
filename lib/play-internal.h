/* play-internal.h -- Internal declarations.
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

#ifndef PLAY_INTERNAL
#define PLAY_INTERNAL

#include "macros.h"

/* *INDENT-OFF* */
PRAGMA_DIAG_PUSH ()
PRAGMA_DIAG_IGNORE (-Wvariadic-macros)
#include <glib.h>
#include <glib-object.h>
PRAGMA_DIAG_POP ()
/* *INDENT-ON* */

/* Media object data.  */
struct _lp_media_t
{
  lp_media_t *parent;           /* parent */
  guint refcount;               /* reference counter */
  GMutex mutex;                 /* sync access to object */

  char *uri;                    /* content URI */
  GHashTable *properties;       /* property table */
};

#define _lp_media_lock(m)   g_mutex_lock (&(m)->mutex)
#define _lp_media_unlock(m) g_mutex_unlock (&(m)->mutex)

lp_media_t *
_lp_media_get_default_parent (void);

void
_lp_media_destroy_default_parent (void);

#endif /* PLAY_INTERNAL */
