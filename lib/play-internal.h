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
PRAGMA_DIAG_IGNORE (-Wcast-qual)
PRAGMA_DIAG_IGNORE (-Wconversion)
PRAGMA_DIAG_IGNORE (-Wpedantic)
PRAGMA_DIAG_IGNORE (-Wvariadic-macros)
#include <glib.h>
#include <glib-object.h>
#include <gst/gst.h>
PRAGMA_DIAG_POP ()
/* *INDENT-ON* */

/* lp-properties */

#define LP_PROPERTY_DEFAULT_WIDTH  800 /* pixels */
#define LP_PROPERTY_DEFAULT_HEIGHT 600 /* pixels */

typedef struct _lp_properties_t
{
  GHashTable *hash;             /* hash table */
} lp_properties_t;

lp_properties_t *
_lp_properties_alloc (void);

void
_lp_properties_free (lp_properties_t *);

void
_lp_properties_reset_all (lp_properties_t *);

int
_lp_properties_get (lp_properties_t *, const char *, GValue *);

int
_lp_properties_set (lp_properties_t *, const char *, GValue *);

/* lp-media */

struct _lp_media_t
{
  lp_status_t status;            /* error status */
  gint ref_count;                /* reference counter */
  lp_media_t *parent;            /* parent */
  char *uri;                     /* content URI */
  GList *children;               /* children list */
  lp_properties_t *properties;   /* property table */
};

#define _lp_media_is_valid(m) ((m) != NULL && !(m)->status)

/* lp-util */

GValue *
_lp_util_g_value_alloc (GType);

void
_lp_util_g_value_free (GValue *);

GValue *
_lp_util_g_value_dup (GValue *);

#endif /* PLAY_INTERNAL */
