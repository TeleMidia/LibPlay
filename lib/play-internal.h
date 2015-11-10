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
PRAGMA_DIAG_IGNORE (-Wcast-qual)
PRAGMA_DIAG_IGNORE (-Wconversion)
PRAGMA_DIAG_IGNORE (-Wpedantic)
#include <glib.h>
#include <glib-object.h>
#include <gst/gst.h>
PRAGMA_DIAG_POP ()
/* *INDENT-ON* */

/* Media object data.  */
struct _lp_media_t
{
  lp_media_t *parent;               /* parent */
  guint refcount;                   /* reference counter */
  GMutex mutex;                     /* sync access to object */
  
  char *uri;                        /* content URI */
  GHashTable *properties;           /* property table */
  GArray *handlers;                 /* handler array */
  lp_media_type_t type;             /* LP_MEDIA_ATOM or LP_MEDIA_SCENE */     
  
  GHashTable *elements;             /* GStreamer elements table  */
  GstClockTime start_offset;        /* set the start offset */
};

#define _lp_media_lock(m)   g_mutex_lock (&(m)->mutex)
#define _lp_media_unlock(m) g_mutex_unlock (&(m)->mutex)

#define _lp_media_get_element(m,e) \
  (GstElement *) g_hash_table_lookup(m->elements, e)

#define _lp_media_add_element(m,k,v) \
  g_hash_table_insert (m->elements,k,v)

lp_media_t *
_lp_media_get_default_parent (void);

void
_lp_media_destroy_default_parent (void);

void
_lp_media_atom_start (lp_media_t *);

void
_lp_media_scene_start (lp_media_t *);

void
_lp_media_atom_stop (lp_media_t *);

void
_lp_media_scene_stop (lp_media_t *);

int
_lp_media_set_video_bin (lp_media_t *, GstPad *);

int
_lp_media_set_audio_bin (lp_media_t *, GstPad *);

int
_lp_media_recursive_get_property_int (lp_media_t *, const char*, int *);

int
_lp_media_recursive_get_property_double(lp_media_t *, const char*, double *);

int
_lp_media_recursive_get_property_string (lp_media_t *, const char*, char **);

int
_lp_media_recursive_get_property_pointer (lp_media_t *, const char*, void **);

GstElement *
_lp_media_recursive_get_element (lp_media_t *, const char *);

#endif /* PLAY_INTERNAL */
