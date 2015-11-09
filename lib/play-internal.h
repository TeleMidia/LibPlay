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
  
  /* union */
  /* { */
  /*   struct _lp_media_atom_t */
  /*   { */
  /*     GstElement *bin;              /1* element bin *1/ */
  /*     GstElement *decodebin;        /1* decoder *1/ */
     
  /*     /1* image elements *1/ */
  /*     GstElement *imagefreeze;      /1* generates a still frame stream */
  /*                                      from an image *1/ */
      
  /*     /1* video elements *1/ */      
  /*     GstElement *videoscale;       /1* scales video *1/ */
  /*     GstElement *videofilter;      /1* changes video caps  *1/ */
      
  /*     /1* audio elements *1/ */
  /*     GstElement *audiovolume;      /1* changes audio volume *1/ */
  /*     GstElement *audioconvert;     /1* converts audio format  *1/ */
  /*     GstElement *audioresample;    /1* resamples audio  *1/ */
  /*     GstElement *audiofilter;      /1* changes audio caps  *1/ */
  /*   } atom; */
    
  /*   struct _lp_media_scene_t */
  /*   { */
  /*     GstElement *pipeline;         /1* pipeline *1/ */
     
  /*     /1* mixers *1/ */
  /*     GstElement *videomix;         /1* video mixer *1/ */
  /*     GstElement *audiomix;         /1* audio mixer *1/ */
     
  /*     /1* sinks *1/ */
  /*     GstElement *videosink;        /1* video sink *1/ */
  /*     GstElement *audiosink;        /1* audio sink *1/ */
  /*   } scene; */
  /* } elements; */
};

#define _lp_media_lock(m)   g_mutex_lock (&(m)->mutex)
#define _lp_media_unlock(m) g_mutex_unlock (&(m)->mutex)

lp_media_t *
_lp_media_get_default_parent (void);

void
_lp_media_destroy_default_parent (void);

#endif /* PLAY_INTERNAL */
