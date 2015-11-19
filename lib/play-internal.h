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

/* lp-properties */
typedef struct _lp_properties_t lp_properties_t;

#define _LP_PROPERTY_DEFAULT_ALPHA 1.0
#define _LP_PROPERTY_DEFAULT_VOLUME 1.0
#define _LP_PROPERTY_DEFAULT_X 0
#define _LP_PROPERTY_DEFAULT_Y 0
#define _LP_PROPERTY_DEFAULT_Z 0

lp_properties_t *
_lp_properties_alloc (void);

void
_lp_properties_free (lp_properties_t *);

lp_properties_t *
_lp_properties_get_metatable (const lp_properties_t *);

lp_properties_t *
_lp_properties_set_metatable (lp_properties_t *, const lp_properties_t *);

unsigned int
_lp_properties_size (const lp_properties_t *);

lp_bool_t
_lp_properties_get (const lp_properties_t *, const char *, GValue *);

lp_bool_t
_lp_properties_set (lp_properties_t *, const char *, const GValue *);

lp_bool_t
_lp_properties_reset (lp_properties_t *, const char *);

void
_lp_properties_reset_all (lp_properties_t *);

/* lp-media */

typedef struct _lp_media_backend_t
{
  lp_media_t *media;
  void *data;
  void (*free) (void *);
  lp_bool_t (*add_child) (lp_media_t *, lp_media_t *);
  lp_bool_t (*remove_child) (lp_media_t *, lp_media_t *);
  lp_bool_t (*post) (lp_media_t *, lp_event_t *);
  lp_bool_t (*get_property) (lp_media_t *, const char *, GValue *);
  lp_bool_t (*set_property) (lp_media_t *, const char *, const GValue *);
} lp_media_backend_t;

void
_lp_media_lock (lp_media_t *);

void
_lp_media_unlock (lp_media_t *);

lp_media_backend_t *
_lp_media_get_backend (lp_media_t *);

lp_media_t *
_lp_media_get_root_ancestor (lp_media_t *);

unsigned int
_lp_media_dispatch (lp_media_t *, lp_event_t *);

/* lp-media-gst */

void
_lp_media_gst_init (lp_media_backend_t *);

/* lp-util */

#define _lp_assert assert
#define _LP_ASSERT_NOT_REACHED ASSERT_NOT_REACHED
#define _LP_STATIC_ASSERT G_STATIC_ASSERT

GValue *
_lp_util_g_value_alloc (void);

void
_lp_util_g_value_free (GValue *);

GValue *
_lp_util_g_value_dup (const GValue *);

GValue *
_lp_util_g_value_init_and_set (GValue *, GType, const void *);

#endif /* PLAY_INTERNAL */
