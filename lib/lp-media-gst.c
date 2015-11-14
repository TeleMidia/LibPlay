/* lp-media-gst.h -- GStreamer back-end.
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

#include <config.h>

#include "play.h"
#include "play-internal.h"

/* *INDENT-OFF* */
PRAGMA_DIAG_PUSH ()
PRAGMA_DIAG_IGNORE (-Wcast-align)
PRAGMA_DIAG_IGNORE (-Wcast-qual)
PRAGMA_DIAG_IGNORE (-Wconversion)
PRAGMA_DIAG_IGNORE (-Wpedantic)
PRAGMA_DIAG_IGNORE (-Wvariadic-macros)
#include <gst/gst.h>
PRAGMA_DIAG_POP ()
/* *INDENT-ON* */

/* GStreamer back-end data.  */
typedef struct _lp_media_gst_t
{
  GstPipeline *pipeline;
} lp_media_gst_t;

/* Forward declarations: */
/* *INDENT-OFF* */
static lp_media_gst_t *__lp_media_gst_check (const lp_media_t *);
static GstPipeline *__lp_media_gst_get_pipeline (lp_media_t *);
/* *INDENT-ON* */

/* Checks and returns the back-end data associated with @media.  */

static lp_media_gst_t *
__lp_media_gst_check (const lp_media_t *media)
{
  _lp_assert (media != NULL);
  _lp_assert (media->backend.data != NULL);
  return (lp_media_gst_t *) media->backend.data;
}

/* Gets #GstPipeline associated with @media.  Returns a #GstPipeline if
   successful, or %NULL if @media has no associated #GstPipeline.  */

static ATTR_UNUSED GstPipeline *
__lp_media_gst_get_pipeline (lp_media_t *media)
{
  lp_media_gst_t *gst;

  gst = __lp_media_gst_check (media);
  if (gst->pipeline != NULL)
    return gst->pipeline;

  if (media->parent != NULL)
    return __lp_media_gst_get_pipeline (media->parent);

  return NULL;
}

/*************************** Back-end callbacks ***************************/

/* Frees @media's back-end data.  */

static void
__lp_media_gst_free_func (void *data)
{
  g_free (data);
}

/* Posts @event to @media.  */

static lp_bool_t
__lp_media_gst_post_func (lp_media_t *media, lp_event_t *event)
{
  _lp_assert (media != NULL);
  _lp_assert (event != NULL);

  switch (event->type)
    {
    case LP_EVENT_USER:
      _lp_media_dispatch (media, event);
      break;
    case LP_EVENT_START:
    case LP_EVENT_STOP:
    default:
      _LP_ASSERT_NOT_REACHED;
    }

  return TRUE;
}

/*************************** Internal functions ***************************/

/* Allocates and initializes @media's back-end data.  */

void
_lp_media_gst_init (lp_media_t *media)
{
  lp_media_gst_t *gst;

  _lp_assert (media != NULL);
  _lp_assert (media->backend.data == NULL);
  _lp_assert (media->backend.free == NULL);
  _lp_assert (media->backend.add_child == NULL);
  _lp_assert (media->backend.remove_child == NULL);
  _lp_assert (media->backend.post == NULL);
  _lp_assert (media->backend.get_property == NULL);
  _lp_assert (media->backend.set_property == NULL);

  gst = (lp_media_gst_t *) g_malloc (sizeof (*gst));
  _lp_assert (gst != NULL);
  memset (gst, 0, sizeof (*gst));

  media->backend.data = gst;
  media->backend.free = __lp_media_gst_free_func;
  media->backend.post = __lp_media_gst_post_func;
}
