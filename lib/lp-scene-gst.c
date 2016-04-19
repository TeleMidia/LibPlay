/* lp-scene-gst.c -- GStreamer back-end.
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


const char *default_audio_caps = "audio/x-raw,rate=48000";
            /* Do we need other caps? */

/* GStreamer back-end data.  */
typedef struct _lp_scene_gst_t
{
  GstElement *pipeline;
  GstClock *clock;
  GstClockID clock_id;
  GstClockTime start_offset;

  /* mixers */
  GstElement *videomixer;
  GstElement *audiomixer;

  /* sinks */
  GstElement *videosink;
  GstElement *audiosink;

  GMutex mutex;

} lp_scene_gst_t;

/* Forward declarations: */
/* *INDENT-OFF* */
static lp_scene_gst_t *__lp_scene_gst_check (lp_scene_t *);
static lp_bool_t __lp_scene_gst_install_pipeline (lp_scene_t *);
static lp_bool_t __lp_scene_gst_set_audio_background (lp_scene_gst_t *);
static lp_bool_t __lp_scene_gst_set_video_background (lp_scene_gst_t *, int, 
    int);
static void __lp_scene_gst_start_async (lp_scene_t *);
static GstElement *__lp_scene_gst_get_pipeline (lp_scene_t *);
static gboolean __lp_scene_gst_pipeline_bus_async_callback (GstBus *, 
    GstMessage *, gpointer);
static GstBusSyncReply __lp_scene_gst_pipeline_bus_sync_callback (GstBus *, 
    GstMessage *, gpointer);
static lp_bool_t __lp_scene_gst_alloc_and_link_mixer (const char *, 
    GstElement **, const char *sink_element, GstElement **, GstElement*);
static void __lp_scene_gst_lock (lp_scene_gst_t *);
static void __lp_scene_gst_unlock (lp_scene_gst_t *);
/* *INDENT-ON* */

/* Checks and returns the back-end data associated with @media.  */

static lp_scene_gst_t *
__lp_scene_gst_check (lp_scene_t *scene)
{
  lp_scene_backend_t *backend;

  _lp_assert (scene != NULL);
  backend = _lp_scene_get_backend (scene);

  _lp_assert (backend != NULL);
  return (lp_scene_gst_t *) backend->data;
}

/* Allocates and installs a new #GstPipeline and some helper #GstElements to 
 * @scene.  Returns %TRUE if successful, or %FALSE if @scene already has an 
 * associated #GstPipeline (or some weird error has occured).  */

static lp_bool_t
__lp_scene_gst_install_pipeline (lp_scene_t *scene)
{
  lp_scene_gst_t *gst = NULL;
  lp_bool_t status = TRUE;
  GstBus *bus;

  gst = __lp_scene_gst_check (scene);
  if (gst->pipeline == NULL)
  {
    GstBus *bus = NULL;
    GstStateChangeReturn ret;
    int width, height;
    
    __lp_scene_gst_lock (gst);

    gst->pipeline = gst_pipeline_new ("pipeline");
    _lp_assert (gst->pipeline != NULL);

    bus = gst_pipeline_get_bus (GST_PIPELINE(gst->pipeline));
    _lp_assert (bus != NULL);
    /* gst_bus_set_sync_handler (bus, __lp_scene_gst_pipeline_bus_sync_callback, */
    /*     NULL, NULL); */
    gst_bus_add_watch (bus, __lp_scene_gst_pipeline_bus_async_callback, NULL);
    g_object_unref (bus);

    g_object_set_data (G_OBJECT (gst->pipeline), "lp_scene", scene);

    if (gst->clock == NULL)
    {
      gst->clock = gst_system_clock_obtain ();
      _lp_assert (gst->clock != NULL);
    }

    gst_pipeline_use_clock (GST_PIPELINE(gst->pipeline), gst->clock);

    /* setting the silence background audio */
    _lp_assert (__lp_scene_gst_set_audio_background (gst) == TRUE);

    /* setting the black background video, if needed */
    g_object_get (scene, "width", &width, "height", &height, NULL);
    if (width > 0 && height > 0)
      _lp_assert (__lp_scene_gst_set_video_background (gst, width,
            height) == TRUE);

    gst->start_offset = gst_clock_get_time (gst->clock);
    gst_element_set_base_time (gst->pipeline, gst->start_offset);
    
    if (unlikely (gst_element_set_state (GST_ELEMENT (gst->pipeline), 
            GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE))
      status = FALSE;

    __lp_scene_gst_unlock (gst);
  }
  else
    status = FALSE;

  return status;
}

static lp_bool_t 
__lp_scene_gst_set_audio_background (lp_scene_gst_t *gst)
{
  GstElement *silence = NULL;
  GstElement *capsfilter = NULL;
  GstCaps *caps = NULL;
  GstPad *src = NULL;
  GstPad *sink = NULL;

  if (__lp_scene_gst_alloc_and_link_mixer ("audiomixer", &gst->audiomixer, 
        "autoaudiosink", &gst->audiosink, gst->pipeline) == FALSE)
    return FALSE;

  silence = gst_element_factory_make ("audiotestsrc", NULL);
  capsfilter = gst_element_factory_make ("capsfilter", NULL);

  _lp_assert (silence != NULL);
  _lp_assert (capsfilter != NULL);

  caps = gst_caps_from_string (default_audio_caps);
  _lp_assert (caps != NULL);

  g_object_set (G_OBJECT (silence), "wave", /* silence */ 4, NULL);
  g_object_set (G_OBJECT (silence), "volume", 0, NULL);
  g_object_set (capsfilter, "caps", caps, NULL);

  gst_caps_unref (caps);

  gst_bin_add_many (GST_BIN (gst->pipeline), silence, capsfilter, NULL);

  _lp_assert (gst_element_link_many (silence, capsfilter, NULL));
  src = gst_element_get_static_pad (capsfilter, "src");
  sink = gst_element_get_request_pad (gst->audiomixer, "sink_%u");

  _lp_assert (src != NULL);
  _lp_assert (sink != NULL);

  _lp_assert (gst_pad_link (src, sink) == GST_PAD_LINK_OK);

  gst_object_unref (src);
  gst_object_unref (sink);

  return TRUE;
}

static lp_bool_t 
__lp_scene_gst_set_video_background (lp_scene_gst_t *gst, int w, int h)
{
  GstElement *background = NULL;
  GstElement *videoscale = NULL;
  GstElement *capsfilter = NULL;
  GstCaps *caps = NULL;
  GstPad *src = NULL;
  GstPad *sink = NULL;

  if (__lp_scene_gst_alloc_and_link_mixer ("videomixer", &gst->videomixer, 
        "xvimagesink", &gst->videosink, gst->pipeline) == FALSE)
    return FALSE;

  background = gst_element_factory_make ("videotestsrc", NULL);
  videoscale = gst_element_factory_make ("videoscale", NULL);
  capsfilter = gst_element_factory_make ("capsfilter", NULL);

  _lp_assert (background != NULL);
  _lp_assert (videoscale != NULL);
  _lp_assert (capsfilter != NULL);

  caps = gst_caps_new_simple ("video/x-raw",
      "framerate", GST_TYPE_FRACTION, 25, 1,
      "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, 
      "width", G_TYPE_INT, w, "height",G_TYPE_INT, h,
      NULL);

  _lp_assert (caps != NULL);

  g_object_set (G_OBJECT (background), "pattern", /*black*/ 2, NULL);
  g_object_set (G_OBJECT (videoscale), "add-borders", 0, NULL);
  g_object_set (G_OBJECT (capsfilter), "caps", caps, NULL);

  gst_caps_unref (caps);

  gst_bin_add_many (GST_BIN (gst->pipeline), background, videoscale, 
      capsfilter, NULL);

  _lp_assert (gst_element_link_many (background, videoscale, capsfilter, 
        NULL));

  src = gst_element_get_static_pad (capsfilter, "src");
  sink = gst_element_get_request_pad (gst->videomixer, "sink_%u");

  _lp_assert (src != NULL);
  _lp_assert (sink != NULL);

  _lp_assert (gst_pad_link (src, sink) == GST_PAD_LINK_OK);

  gst_element_set_state (background, GST_STATE_PLAYING);
  gst_element_set_state (videoscale, GST_STATE_PLAYING);
  gst_element_set_state (capsfilter, GST_STATE_PLAYING);

  gst_object_unref (src);
  gst_object_unref (sink);

  return TRUE;
}

static lp_bool_t
__lp_scene_gst_alloc_and_link_mixer (const char *mixer_element,
                                     GstElement ** mixer,
                                     const char *sink_element,
                                     GstElement ** sink, GstElement * bin)
{
  /* TODO: Gets parent's mixer and link with mixer when parent != NULL */
  if (mixer_element == NULL || bin == NULL)
    return FALSE;

  *mixer = gst_element_factory_make (mixer_element, NULL);
  _lp_assert (*mixer);

  gst_element_set_state (*mixer, GST_STATE_PAUSED);

  if (sink_element != NULL)     /* Sink needs to be created  */
  {
    *sink = gst_element_factory_make (sink_element, NULL);
    _lp_assert (*sink != NULL);

    gst_bin_add (GST_BIN (bin), *sink);
    gst_element_set_state (*sink, GST_STATE_PAUSED);
  }

  gst_bin_add (GST_BIN (bin), *mixer);
  return gst_element_link (*mixer, *sink);
}


/* Requests an asynchronous start of @scene.
   Returns %TRUE if successful, or %FALSE if @scene cannot start.  */

void
__lp_scene_gst_start_async (lp_scene_t *scene)
{
  lp_bool_t status;

  _lp_assert ((status = __lp_scene_gst_install_pipeline (scene), status) 
      == TRUE);
}

/************************** GStreamer callbacks ***************************/

/* Handles #GstElement clock ticks.  */

static ATTR_UNUSED gboolean
__lp_scene_gst_pipeline_clock_callback (arg_unused (GstClock *clock),
                                        GstClockTime time,
                                        arg_unused (GstClockID id),
                                        gpointer data)
{
  lp_scene_t *media;
  lp_event_t tick;

  _lp_assert (data != NULL);
  time = time - gst_element_get_base_time (GST_ELEMENT (data));
  /* g_print ("tick (thread %p): %" GST_TIME_FORMAT "\n", */
  /*          (void *) g_thread_self (), GST_TIME_ARGS (time)); */

  media = (lp_scene_t *) g_object_get_data (G_OBJECT (data), "lp_media");
  _lp_assert (media != NULL);

  lp_event_init_tick (&tick);
  _lp_scene_dispatch (media, &tick);
  return TRUE;
}

/* Handles #GstBus asynchronous messages.  */

static gboolean
__lp_scene_gst_pipeline_bus_async_callback (arg_unused (GstBus *bus),
                                            GstMessage *message,
                                            arg_unused (gpointer data))
{
  GstObject *obj;
  lp_scene_t *scene;
  lp_scene_gst_t *gst;

  /* gstx_dump_message ("async", message); */

  obj = GST_MESSAGE_SRC (message);
  _lp_assert (obj != NULL);

  scene = (lp_scene_t *) g_object_get_data (G_OBJECT (obj), "lp_scene");
  if (scene != NULL)
    gst = __lp_scene_gst_check (scene);

  switch (GST_MESSAGE_TYPE (message))
  {
    case GST_MESSAGE_STATE_CHANGED:
    {
      GstState new_state;
      GstState old_state;
      lp_event_t event;

      if (scene == NULL)
        break;                  /* nothing to do */

      gst_message_parse_state_changed (message, &old_state,
                                       &new_state, NULL);

      if (new_state == GST_STATE_PLAYING)
      {
        lp_event_init_start (&event);
      }
      else if (old_state == GST_STATE_PAUSED
               && new_state == GST_STATE_READY)
      {
        GstStateChangeReturn status;
        status = gst_element_set_state (GST_ELEMENT (obj), GST_STATE_NULL);
        if (unlikely (status == GST_STATE_CHANGE_FAILURE))
          return FALSE;

        lp_event_init_stop (&event);
      }
      else
      {
        break;
      }

      _lp_scene_dispatch (scene, &event);

      if (event.type == LP_EVENT_STOP)
      {
        /* gst_object_unref (obj); */
        /* lp_scene_destroy (media); */
      }
      break;
    }
    case GST_MESSAGE_NEW_CLOCK:
    {
      GstClock *clock;
      GstClockID id;
      GstClockTime time;
      GstClockReturn ret;

      _lp_assert (GST_IS_PIPELINE (obj));
      _lp_assert (scene != NULL);
      _lp_assert (gst != NULL);

      clock = gst_pipeline_get_clock (GST_PIPELINE (obj));
      _lp_assert (clock != NULL);

      time = gst_clock_get_time (clock);
      _lp_assert (time != GST_CLOCK_TIME_NONE);

      id = gst_clock_new_periodic_id (clock, time, 1 * GST_SECOND);
      _lp_assert (id != NULL);
      g_object_unref (clock);

      ret = gst_clock_id_wait_async
        (id, __lp_scene_gst_pipeline_clock_callback, obj, NULL);
      _lp_assert (ret == GST_CLOCK_OK);
      if (gst->clock_id != NULL)
      {
        gst_clock_id_unschedule (gst->clock_id);
        gst_clock_id_unref (gst->clock_id);
      }
      gst->clock_id = id;
      break;
    }
    case GST_MESSAGE_APPLICATION:
    {
      if (strcmp (gst_structure_get_name (gst_message_get_structure (message)),
            "media-stop") == 0)
      {
        /* GstIterator *bin_it = NULL; */
        /* GValue data = G_VALUE_INIT; */
        /* gboolean done = FALSE; */

        /* lp_scene_reference (media); */
        /* gst_object_ref (gst->bin); */
        /* gst_element_set_state (gst->bin, GST_STATE_NULL); */
        /* gst_element_get_state (gst->bin, NULL, NULL, GST_CLOCK_TIME_NONE); */

        /* gst_bin_remove (GST_BIN (__lp_scene_gst_get_pipeline(media)), */ 
        /*     gst->bin); */

        /* bin_it = gst_bin_iterate_elements (GST_BIN (gst->bin)); */
        /* while (!done) */
        /* { */
        /*   GstElement *element; */
        /*   switch (gst_iterator_next (bin_it, &data)) */
        /*   { */
        /*     case GST_ITERATOR_OK: */
        /*       { */
        /*         element = GST_ELEMENT (g_value_get_object (&data)); */
        /*         _lp_assert (element != NULL); */

        /*         gst_object_ref (element); */
        /*         gst_bin_remove (GST_BIN (gst->bin), element); */
        /*         gst_element_set_state (element, GST_STATE_NULL); */
        /*         break; */
        /*       } */
        /*     case GST_ITERATOR_RESYNC: */
        /*       { */
        /*         gst_iterator_resync (bin_it); */
        /*         break; */
        /*       } */
        /*     case GST_ITERATOR_ERROR: */
        /*     case GST_ITERATOR_DONE: */
        /*       { */
        /*         done = TRUE; */
        /*         break; */
        /*       } */
        /*     default: */
        /*       break; */
        /*   } */
        /* } */
        /* gst_iterator_free (bin_it); */
        /* lp_scene_destroy (media); */
      }
      break;
    }
    default:
      break;
  }

  return TRUE;
}

/* Handles #GstBus synchronous messages.  */

static GstBusSyncReply
__lp_scene_gst_pipeline_bus_sync_callback (arg_unused (GstBus *bus),
                                           arg_unused (GstMessage *message),
                                           arg_unused (gpointer data))
{
  /* gstx_dump_message ("sync", message); */
  return GST_BUS_PASS;
}

/*************************** Back-end callbacks ***************************/

/* Frees @media's back-end data.  */

static void
__lp_scene_gst_free_func (void *data)
{
  lp_scene_gst_t *gst = (lp_scene_gst_t *) data;
  if (gst->clock_id)
  {
    /* TODO: Release thread.  */
    gst_clock_id_unschedule (gst->clock_id);
    gst_clock_id_unref (gst->clock_id);
  }

  if (gst->clock != NULL)
    gst_object_unref (gst->clock);

  if (gst->pipeline != NULL)
    gst_object_unref (gst->pipeline);

  g_mutex_clear (&gst->mutex);

  g_free (gst);
}

/* Gets the current time of @media*/
static uint64_t
__lp_scene_gst_get_time_func (const lp_scene_t *media)
{
  lp_scene_gst_t *gst;
  _lp_assert (media != NULL);

  gst = __lp_scene_gst_check (deconst(lp_scene_t *, media));
  if (gst->pipeline == NULL || gst->clock == NULL)
    return 0;

  return gst_clock_get_time (gst->clock) - gst->start_offset;
}


/*************************** Internal functions ***************************/

/* Allocates initializes #lp_scene_backend_t.  */

void
_lp_scene_gst_init (lp_scene_backend_t *backend)
{
  lp_scene_gst_t *gst;

  if (!gst_is_initialized ())
    gst_init (NULL, NULL);

  _lp_assert (backend != NULL);
  _lp_assert (backend->scene != NULL);
  _lp_assert (backend->data == NULL);
  _lp_assert (backend->free == NULL);
  _lp_assert (backend->get_property == NULL);
  _lp_assert (backend->set_property == NULL);
  _lp_assert (backend->get_time == NULL);

  gst = g_new0 (lp_scene_gst_t, 1);
  _lp_assert (gst != NULL);

  g_mutex_init (&gst->mutex);

  backend->data = gst;
  backend->free = __lp_scene_gst_free_func;
  backend->start_async = __lp_scene_gst_start_async;
  backend->get_time = __lp_scene_gst_get_time_func;
}

static void 
__lp_scene_gst_lock (lp_scene_gst_t *gst)
{
  _lp_assert (gst != NULL);
  g_mutex_lock (&gst->mutex);
}

static void 
__lp_scene_gst_unlock (lp_scene_gst_t *gst)
{
  _lp_assert (gst != NULL);
  g_mutex_unlock (&gst->mutex);
}

#if HAVE_SYNCCLOCK
void
_lp_scene_gst_set_sync_clock (lp_scene_backend_t *backend, 
    lp_sync_clock_t *clock)
{
  GstClock *syncclock;
  GstElement *pipeline;
  lp_scene_t *root;
  lp_scene_gst_t *gst;

  syncclock = (GstClock *) _lp_sync_clock_get_internal_clock (clock);
  
  root = _lp_scene_get_root_ancestor (backend->media);
  _lp_scene_lock (root);

  pipeline = __lp_scene_gst_get_pipeline (root);
  if (pipeline != NULL)
    gst_pipeline_use_clock (GST_PIPELINE(gst->pipeline), syncclock);

  gst = __lp_scene_gst_check (root);
  gst->clock = syncclock;

  _lp_scene_unlock (root);
}
#endif
