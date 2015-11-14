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

#include "gstx-macros.h"
#include "play.h"
#include "play-internal.h"

/* GStreamer back-end data.  */
typedef struct _lp_media_gst_t
{
  GstPipeline *pipeline;
} lp_media_gst_t;

/* Forward declarations: */
/* *INDENT-OFF* */
static lp_media_gst_t *__lp_media_gst_check (const lp_media_t *);
static GstPipeline *__lp_media_gst_get_pipeline (lp_media_t *);
static gboolean __lp_gst_media_pipeline_bus_async_callback (GstBus *, GstMessage *, gpointer);
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

static GstPipeline *
__lp_media_gst_get_pipeline (lp_media_t *media)
{
  lp_media_t *root = _lp_media_get_root_ancestor (media);
  lp_media_gst_t *gst = __lp_media_gst_check (root);
  return gst->pipeline;
}

/* Allocates and installs a new #GstPipeline to @media.  Returns %TRUE if
   successful, or %FALSE if @media already has an associated
   #GstPipeline.  */

static lp_bool_t
__lp_media_gst_install_pipeline (lp_media_t *media)
{
  lp_media_t *root;
  lp_media_gst_t *gst;

  GstPipeline *pipeline;
  GstBus *bus;

  root = _lp_media_get_root_ancestor (media);
  gst = __lp_media_gst_check (root);
  if (gst->pipeline != NULL)
    return FALSE;

  pipeline = GST_PIPELINE (gst_pipeline_new ("pipeline"));
  _lp_assert (pipeline != NULL);

  bus = gst_pipeline_get_bus (pipeline);
  _lp_assert (bus != NULL);
  gst_bus_add_watch (bus, __lp_gst_media_pipeline_bus_async_callback, NULL);
  g_object_unref (bus);

  g_object_set_data (G_OBJECT (pipeline), "lp_media", root);
  gst->pipeline = pipeline;
  return TRUE;
}

/* Requests an asynchronous start of @media.
   Returns %TRUE if successful, or %FALSE if @media cannot start.  */

static lp_bool_t
__lp_media_gst_start_async (lp_media_t *media)
{
  GstStateChangeReturn status;
  GstPipeline *pipeline;

  pipeline = __lp_media_gst_get_pipeline (media);
  if (pipeline == NULL)
    {
      _lp_assert (__lp_media_gst_install_pipeline (media));
      pipeline = __lp_media_gst_get_pipeline (media);
      _lp_assert (pipeline != NULL);
    }

  status = gst_element_set_state (GST_ELEMENT (pipeline),
                                  GST_STATE_PLAYING);
  if (unlikely (status == GST_STATE_CHANGE_FAILURE))
    return FALSE;

  return TRUE;
}

/************************** GStreamer callbacks ***************************/

/* Handles #GstBus asynchronous messages.  */

static gboolean
__lp_gst_media_pipeline_bus_async_callback (arg_unused (GstBus *bus),
                                            GstMessage *message,
                                            arg_unused (gpointer data))
{
  GstObject *obj;

  gstx_dump_message (message);

  obj = GST_MESSAGE_SRC (message);
  _lp_assert (obj != NULL);

  switch (GST_MESSAGE_TYPE (message))
    {
    case GST_MESSAGE_STATE_CHANGED:
      {
        GstState new_state;
        lp_media_t *media;
        lp_event_t event;

        media = (lp_media_t *) g_object_get_data (G_OBJECT (obj),
                                                  "lp_media");
        if (media == NULL)
          break;

        gst_message_parse_state_changed (message, NULL, &new_state, NULL);
        if (new_state != GST_STATE_PLAYING)
          break;

        lp_event_init_start (&event);
        _lp_media_dispatch (media, &event);
      }
    default:
      break;
    }

  return TRUE;
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
    case LP_EVENT_START:
      __lp_media_gst_start_async (media);
      break;
    case LP_EVENT_USER:
      _lp_media_dispatch (media, event);
      break;
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

  if (!gst_is_initialized ())
    gst_init (NULL, NULL);

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
