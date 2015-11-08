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
  union
  {
    struct _lp_media_atom_t
    {
      GstElement *bin;              /* element bin */
      GstElement *decodebin;        /* decoder */
     
      /* image elements */
      GstElement *imagefreeze;      /* generates a still frame stream
                                       from an image */
      
      /* video elements */      
      GstElement *videoscale;       /* scales video */
      GstElement *videofilter;      /* changes video caps  */
      
      /* audio elements */
      GstElement *audiovolume;      /* changes audio volume */
      GstElement *audioconvert;     /* converts audio format  */
      GstElement *audioresample;    /* resamples audio  */
      GstElement *audiofilter;      /* changes audio caps  */
    } atom;
    
    struct _lp_media_scene_t
    {
      GstElement *pipeline;         /* pipeline */
     
      /* mixers */
      GstElement *videomix;         /* video mixer */
      GstElement *audiomix;         /* audio mixer */
     
      /* sinks */
      GstElement *videosink;        /* video sink */
      GstElement *audiosink;        /* audio sink */
    } scene;
  } elements;
};

#define _lp_media_lock(m)   g_mutex_lock (&(m)->mutex)
#define _lp_media_unlock(m) g_mutex_unlock (&(m)->mutex)

lp_media_t *
_lp_media_get_default_parent (void);

void
_lp_media_destroy_default_parent (void);

#endif /* PLAY_INTERNAL */
