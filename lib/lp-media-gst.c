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
static lp_media_gst_t *__lp_media_gst_check (lp_media_t *);
static GstPipeline *__lp_media_gst_get_pipeline (lp_media_t *);
static gboolean __lp_gst_media_pipeline_bus_async_callback (GstBus *, GstMessage *, gpointer);
static GstBusSyncReply __lp_gst_media_pipeline_bus_sync_callback (arg_unused (GstBus *), GstMessage *, arg_unused (gpointer));
/* *INDENT-ON* */

/* Checks and returns the back-end data associated with @media.  */

static lp_media_gst_t *
__lp_media_gst_check (lp_media_t *media)
{
  lp_media_backend_t *backend;

  _lp_assert (media != NULL);
  backend = _lp_media_get_backend (media);

  _lp_assert (backend != NULL);
  return (lp_media_gst_t *) backend->data;
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
  gst_bus_set_sync_handler (bus, __lp_gst_media_pipeline_bus_sync_callback,
                            NULL, NULL);
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

/* Handles #GstPipeline clock ticks.  */

static gboolean
__lp_gst_media_pipeline_clock_callback (arg_unused (GstClock *clock),
                                        GstClockTime time,
                                        arg_unused (GstClockID id),
                                        arg_unused (gpointer data))
{
  _lp_assert (data != NULL);
  time = time - gst_element_get_base_time (GST_ELEMENT (data));
  g_print ("tick (thread %p): %"GST_TIME_FORMAT"\n",
           (void *) g_thread_self (), GST_TIME_ARGS (time));
  return TRUE;
}

/* Handles #GstBus asynchronous messages.  */

static gboolean
__lp_gst_media_pipeline_bus_async_callback (arg_unused (GstBus *bus),
                                            GstMessage *message,
                                            arg_unused (gpointer data))
{
  GstObject *obj;
  lp_media_t *media;
  lp_media_gst_t *gst;

  gstx_dump_message ("async", message);

  obj = GST_MESSAGE_SRC (message);
  _lp_assert (obj != NULL);

  media = (lp_media_t *) g_object_get_data (G_OBJECT (obj), "lp_media");
  if (media != NULL)
    gst = __lp_media_gst_check (media);

  switch (GST_MESSAGE_TYPE (message))
    {
    case GST_MESSAGE_STATE_CHANGED:
      {
        GstState new_state;
        lp_event_t event;

        if (media == NULL)
          break;                /* nothing to do */

        gst_message_parse_state_changed (message, NULL, &new_state, NULL);
        if (new_state != GST_STATE_PLAYING)
          break;

        lp_event_init_start (&event);
        _lp_media_dispatch (media, &event);
        break;
      }
    case GST_MESSAGE_NEW_CLOCK:
      {
        GstClock *clock;
        GstClockID id;
        GstClockTime time;
        GstClockReturn ret;

        _lp_assert (GST_IS_PIPELINE (obj));
        _lp_assert (media != NULL);
        _lp_assert (gst != NULL);

        clock = gst_pipeline_get_clock (GST_PIPELINE (obj));
        _lp_assert (clock != NULL);

        time = gst_clock_get_time (clock);
        _lp_assert (time != GST_CLOCK_TIME_NONE);

        id = gst_clock_new_periodic_id (clock, time, 1 * GST_SECOND);
        _lp_assert (id != NULL);
        g_object_unref (clock);

        ret = gst_clock_id_wait_async
          (id, __lp_gst_media_pipeline_clock_callback, obj, NULL);
        _lp_assert (ret == GST_CLOCK_OK);
        gst_clock_id_unref (id);
        break;
      }
    default:
      break;
    }

  return TRUE;
}

/* Handles #GstBus synchronous messages.  */

static GstBusSyncReply
__lp_gst_media_pipeline_bus_sync_callback (arg_unused (GstBus *bus),
                                           GstMessage *message,
                                           arg_unused (gpointer data))
{
  gstx_dump_message ("sync", message);
  return GST_BUS_PASS;
}

/*************************** Back-end callbacks ***************************/

/* Frees @media's back-end data.  */

static void
__lp_media_gst_free_func (void *data)
{
  lp_media_gst_t *gst = (lp_media_gst_t *) data;
  if (gst->pipeline != NULL)
    gst_object_unref (gst->pipeline);
  g_free (gst);
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

/* Allocates initializes #lp_media_backend_t.  */

void
_lp_media_gst_init (lp_media_backend_t *backend)
{
  lp_media_gst_t *gst;

  if(!gst_is_initialized ())
    gst_init (NULL, NULL);

  _lp_assert (backend != NULL);
  _lp_assert (backend->media != NULL);
  _lp_assert (backend->data == NULL);
  _lp_assert (backend->free == NULL);
  _lp_assert (backend->add_child == NULL);
  _lp_assert (backend->remove_child == NULL);
  _lp_assert (backend->post == NULL);
  _lp_assert (backend->get_property == NULL);
  _lp_assert (backend->set_property == NULL);

  gst = (lp_media_gst_t *) g_malloc (sizeof (*gst));
  _lp_assert (gst != NULL);
  memset (gst, 0, sizeof (*gst));

  backend->data = gst;
  backend->free = __lp_media_gst_free_func;
  backend->post = __lp_media_gst_post_func;
}
