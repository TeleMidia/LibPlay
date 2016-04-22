/* lp-scene.c -- Scene object.
   Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia

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

#include "play-internal.h"
#include "play.h"

/* Scene object.  */
struct _lp_Scene
{
  GObject parent;               /* parent object */
  GstElement *pipeline;         /* scene pipeline */
  GstClockID clock_id;          /* last clock id */
  GMainLoop *loop;              /* scene loop */
  GList *messages;              /* pending messages */
  GList *children;              /* child media objects */
  struct
  {
    GstElement *blank;          /* blank audio source */
    GstElement *mixer;          /* audio mixer */
    GstElement *sink;           /* audio sink */
  } audio;
  struct
  {
    GstElement *blank;          /* blank video source */
    GstElement *filter;         /* video filter */
    GstElement *mixer;          /* video mixer */
    GstElement *sink;           /* video sink */
  } video;
  struct
  {
    int width;                  /* cached width */
    int height;                 /* cached height */
    int pattern;                /* cached pattern */
    int wave;                   /* cached wave */
    guint64 ticks;              /* cached ticks */
  } prop;
};

/* Maps GStreamer elements to lp_Scene.  */
static const gstx_eltmap_t lp_scene_eltmap[] = {
  {"pipeline",      offsetof (lp_Scene, pipeline)},
  {"audiotestsrc",  offsetof (lp_Scene, audio.blank)},
  {"audiomixer",    offsetof (lp_Scene, audio.mixer)},
  {"autoaudiosink", offsetof (lp_Scene, audio.sink)},
  {NULL, 0},
};

static const gstx_eltmap_t lp_scene_eltmap_video[] = {
  {"videotestsrc",  offsetof (lp_Scene, video.blank)},
  {"capsfilter",    offsetof (lp_Scene, video.filter)},
  {"compositor",    offsetof (lp_Scene, video.mixer)},
  {"ximagesink",    offsetof (lp_Scene, video.sink)},
  {NULL, 0},
};

/* Scene properties.  */
enum
{
  PROP_0,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_PATTERN,
  PROP_WAVE,
  PROP_TICKS,
  PROP_LAST
};

/* Default values for scene properties.  */
#define DEFAULT_WIDTH    0      /* no video output */
#define DEFAULT_HEIGHT   0      /* no video output */
#define DEFAULT_PATTERN  2      /* black */
#define DEFAULT_WAVE     4      /* silence */
#define DEFAULT_TICKS    0      /* no ticks */

/* Define the lp_Scene type.  */
G_DEFINE_TYPE (lp_Scene, lp_scene, G_TYPE_OBJECT)


/* callbacks */

/* Called asynchronously whenever the scene pipeline clock ticks.
   Sends an "lp_Scene:tick" application message to pipeline.  */

static int
lp_scene_tick_callback (arg_unused (GstClock *clock),
                        GstClockTime time,
                        arg_unused (GstClockID id),
                        lp_Scene *scene)
{
  gstx_element_post_application_message
    (scene->pipeline, scene->pipeline, "lp_Scene:tick",
     "scene", G_TYPE_POINTER, scene,
     "time", GST_TYPE_CLOCK_TIME, time, NULL);
  return TRUE;
}

/* Called asynchronously whenever scene pipeline receives a message.  */

static gboolean
lp_scene_bus_callback (arg_unused (GstBus *bus),
                       GstMessage *msg,
                       gpointer user_data)
{
  lp_Scene *scene;

  scene = LP_SCENE (user_data);
  switch (GST_MESSAGE_TYPE (msg))
    {
    case GST_MESSAGE_APPLICATION:
      {
        const GstStructure *st;

        st = gst_message_get_structure (msg);
        if (gst_structure_has_name (st, "lp_Scene:tick"))
          {
            scene->prop.ticks++;
          }
        else if (gst_structure_has_name (st, "media-stop"))
          {
            lp_Media *media;

            media = (lp_Media *) gstx_structure_get_pointer (st, "media");
            assert (_lp_media_do_stop (media));
          }
        else
          {
            g_critical (G_STRLOC ": unknown application message");
          }
        assert (gst_message_ref (msg) == msg);
        scene->messages = g_list_append (scene->messages, msg);
        break;
      }
    case GST_MESSAGE_ASYNC_DONE:
      break;
    case GST_MESSAGE_ASYNC_START:
      break;
    case GST_MESSAGE_BUFFERING:
      break;
    case GST_MESSAGE_CLOCK_LOST:
      break;
    case GST_MESSAGE_CLOCK_PROVIDE:
      break;
    case GST_MESSAGE_DEVICE_ADDED:
      break;
    case GST_MESSAGE_DEVICE_REMOVED:
      break;
    case GST_MESSAGE_DURATION_CHANGED:
      break;
    case GST_MESSAGE_ELEMENT:
      break;
    case GST_MESSAGE_EOS:
      break;
    case GST_MESSAGE_ERROR:
      {
        GError *err;
        gst_message_parse_error (msg, &err, NULL);
        g_critical (G_STRLOC ": GStreamer error: %s", err->message);
        g_error_free (err);
        break;
      }
    case GST_MESSAGE_EXTENDED:
      break;
    case GST_MESSAGE_HAVE_CONTEXT:
      break;
    case GST_MESSAGE_INFO:
      break;
    case GST_MESSAGE_LATENCY:
      break;
    case GST_MESSAGE_NEED_CONTEXT:
      break;
    case GST_MESSAGE_NEW_CLOCK:
      {
        GstClock *clock;
        GstClockID id;
        GstClockTime time;
        GstClockCallback func;
        GstClockReturn ret;

        clock = gst_pipeline_get_clock (GST_PIPELINE (scene->pipeline));
        assert (clock != NULL);

        time = gst_clock_get_time (clock);
        assert (time != GST_CLOCK_TIME_NONE);

        id = gst_clock_new_periodic_id (clock, time, 1 * GST_SECOND);
        assert (id != NULL);

        func = (GstClockCallback) lp_scene_tick_callback;
        ret = gst_clock_id_wait_async (id, func, scene, NULL);
        assert (ret == GST_CLOCK_OK);
        if (scene->clock_id != NULL)
          {
            gst_clock_id_unschedule (scene->clock_id);
            gst_clock_id_unref (scene->clock_id);
          }
        scene->clock_id = id;
      }
    case GST_MESSAGE_PROGRESS:
      break;
    case GST_MESSAGE_QOS:
      break;
    case GST_MESSAGE_REQUEST_STATE:
      break;
    case GST_MESSAGE_RESET_TIME:
      break;
    case GST_MESSAGE_SEGMENT_DONE:
      break;
    case GST_MESSAGE_SEGMENT_START:
      break;
    case GST_MESSAGE_STATE_CHANGED:
      break;
    case GST_MESSAGE_STATE_DIRTY:
      break;
    case GST_MESSAGE_STEP_DONE:
      break;
    case GST_MESSAGE_STEP_START:
      break;
    case GST_MESSAGE_STREAM_START:
      break;
    case GST_MESSAGE_STREAM_STATUS:
      break;
    case GST_MESSAGE_STRUCTURE_CHANGE:
      break;
    case GST_MESSAGE_TAG:
      break;
    case GST_MESSAGE_TOC:
      break;
    case GST_MESSAGE_WARNING:
      break;
    case GST_MESSAGE_UNKNOWN:
    default:
      break;
    }
  return TRUE;
}


/* private */

static void
lp_scene_init (lp_Scene *scene)
{
  scene->clock_id = NULL;
  scene->loop = g_main_loop_new (NULL, FALSE);
  assert (scene->loop != NULL);
  scene->messages = NULL;
  scene->children = NULL;

  scene->prop.width = DEFAULT_WIDTH;
  scene->prop.height = DEFAULT_HEIGHT;
  scene->prop.pattern = DEFAULT_PATTERN;
  scene->prop.wave = DEFAULT_WAVE;
  scene->prop.ticks = DEFAULT_TICKS;
}

static void
lp_scene_get_property (GObject *object, guint prop_id,
                       GValue *value, GParamSpec *pspec)
{
  lp_Scene *scene;

  scene = LP_SCENE (object);
  switch (prop_id)
    {
    case PROP_WIDTH:
      g_value_set_int (value, scene->prop.width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, scene->prop.height);
      break;
    case PROP_PATTERN:
      g_value_set_int (value, scene->prop.pattern);
      break;
    case PROP_WAVE:
      g_value_set_int (value, scene->prop.wave);
      break;
    case PROP_TICKS:
      g_value_set_uint64 (value, scene->prop.ticks);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_scene_set_property (GObject *object, guint prop_id,
                       const GValue *value, GParamSpec *pspec)
{
  lp_Scene *scene = LP_SCENE (object);

  switch (prop_id)
    {
    case PROP_WIDTH:
      scene->prop.width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      scene->prop.height = g_value_get_int (value);
      break;
    case PROP_PATTERN:
      scene->prop.pattern = g_value_get_int (value);
      if (_lp_scene_has_video (scene))
        {
          g_object_set (scene->video.blank, "pattern",
                        scene->prop.pattern, NULL);
        }
      break;
    case PROP_WAVE:
      scene->prop.wave = g_value_get_int (value);
      g_object_set (scene->audio.blank, "wave", scene->prop.wave, NULL);
      break;
    case PROP_TICKS:
      scene->prop.ticks = g_value_get_uint64 (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_scene_constructed (GObject *object)
{
  lp_Scene *scene;
  GstBus *bus;
  lp_Event evt;

  scene = LP_SCENE (object);

  _lp_eltmap_alloc_check (scene, lp_scene_eltmap);
  bus = gst_pipeline_get_bus (GST_PIPELINE (scene->pipeline));
  assert (bus != NULL);
  assert (gst_bus_add_watch (bus, lp_scene_bus_callback, scene) > 0);
  gst_object_unref (bus);

  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->audio.blank));
  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->audio.mixer));
  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->audio.sink));
  assert (gst_element_link (scene->audio.blank, scene->audio.mixer));
  assert (gst_element_link (scene->audio.mixer, scene->audio.sink));

  if (_lp_scene_has_video (scene))
    {
      GstCaps *caps;

      _lp_eltmap_alloc_check (scene, lp_scene_eltmap_video);
      assert (gst_bin_add (GST_BIN (scene->pipeline), scene->video.blank));
      assert (gst_bin_add (GST_BIN (scene->pipeline), scene->video.filter));
      assert (gst_bin_add (GST_BIN (scene->pipeline), scene->video.mixer));
      assert (gst_bin_add (GST_BIN (scene->pipeline), scene->video.sink));
      assert (gst_element_link (scene->video.blank, scene->video.filter));
      assert (gst_element_link (scene->video.filter, scene->video.mixer));
      assert (gst_element_link (scene->video.mixer, scene->video.sink));

      caps = gst_caps_new_empty_simple ("video/x-raw");
      assert (caps != NULL);
      gst_caps_set_simple (caps,
                           "width", G_TYPE_INT, scene->prop.width,
                           "height", G_TYPE_INT, scene->prop.height, NULL);
      g_object_set (scene->video.filter, "caps", caps, NULL);
      gst_caps_unref (caps);
    }

  g_object_set (scene,
                "pattern", scene->prop.pattern,
                "wave", scene->prop.wave, NULL);

  /* Start pipeline and wait for the first tick.  */
  gstx_element_set_state_sync (scene->pipeline, GST_STATE_PLAYING);
  assert (lp_scene_pop (scene, TRUE, NULL, &evt));
  assert (evt == LP_TICK && scene->prop.ticks == 1);
  scene->prop.ticks = 0;
}

static void
lp_scene_finalize (GObject *object)
{
  lp_Scene *scene;
  GList *l;

  scene = LP_SCENE (object);

  assert (scene->clock_id != NULL);
  gst_clock_id_unschedule (scene->clock_id); /* disable ticks */

  while ((l = scene->children) != NULL) /* collect children */
    {
      lp_Media *media = (lp_Media *)(l->data);
      if (_lp_media_is_stopping (media))
        {
          lp_scene_pop (scene, TRUE, NULL, NULL);
          continue;
        }
      g_object_unref (media);
      scene->children = g_list_delete_link (scene->children, l);
    }
  assert (scene->children == NULL);

  gstx_element_set_state_sync (scene->pipeline, GST_STATE_NULL);
  gst_object_unref (scene->pipeline);

  gst_clock_id_unref (scene->clock_id);
  g_main_loop_unref (scene->loop);
  g_list_free_full (scene->messages, (GDestroyNotify) gst_message_unref);

  G_OBJECT_CLASS (lp_scene_parent_class)->finalize (object);
}

static void
lp_scene_class_init (lp_SceneClass *cls)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (cls);

  gobject_class->get_property = lp_scene_get_property;
  gobject_class->set_property = lp_scene_set_property;
  gobject_class->constructed = lp_scene_constructed;
  gobject_class->finalize = lp_scene_finalize;

  g_object_class_install_property
    (gobject_class, PROP_WIDTH, g_param_spec_int
     ("width", "width", "width in pixels",
      0, G_MAXINT, DEFAULT_WIDTH,
      (GParamFlags) (G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_HEIGHT, g_param_spec_int
     ("height", "height", "height in pixels",
      0, G_MAXINT, DEFAULT_HEIGHT,
      (GParamFlags) (G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_PATTERN, g_param_spec_int
     ("pattern", "pattern", "background video pattern",
      0, 24, DEFAULT_PATTERN,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_WAVE, g_param_spec_int
     ("wave", "wave", "background audio wave",
      0, 12, DEFAULT_WAVE,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_TICKS, g_param_spec_uint64
     ("ticks", "ticks", "total number of ticks so far",
      0, G_MAXUINT64, DEFAULT_TICKS,
      G_PARAM_READABLE));

  if (!gst_is_initialized ())
    {
      GError *err = NULL;
      if (unlikely (!gst_init_check (NULL, NULL, &err)))
        {
          assert (err != NULL);
          g_critical ("cannot initialize GStreamer: %s", err->message);
          g_error_free (err);
        }
    }
}


/* internal */

/* Adds @media to @scene.  */

void
_lp_scene_add_media (lp_Scene *scene, lp_Media *media)
{
  scene->children = g_list_append (scene->children, media);
  assert (g_object_ref (media) == media);
}

/* Returns @scene pipeline.  */

ATTR_PURE GstElement *
_lp_scene_get_pipeline (const lp_Scene *scene)
{
  return scene->pipeline;
}

/* Returns @scene audio mixer.  */

GstElement *
ATTR_PURE _lp_scene_get_audio_mixer (const lp_Scene *scene)
{
  return scene->audio.mixer;
}

/* Returns @scene video mixer.  */

ATTR_PURE GstElement *
_lp_scene_get_video_mixer (const lp_Scene *scene)
{
  return scene->video.mixer;
}

/* Returns true if scene has video output.  */

ATTR_PURE gboolean
_lp_scene_has_video (const lp_Scene *scene)
{
  return scene->prop.width > 0 && scene->prop.height > 0;
}


/* public */

/**
 * lp_scene_new:
 * @width: scene width
 * @height: scene height
 *
 * Creates a new scene with the given dimensions.
 *
 * Returns: (transfer full): a new #lp_Scene
 */
lp_Scene *
lp_scene_new (int width, int height)
{
  return LP_SCENE (g_object_new (LP_TYPE_SCENE,
                                 "width", width,
                                 "height", height, NULL));
}

/**
 * lp_scene_pop:
 * @scene: an #lp_Scene to pop
 * @block: whether the call may block
 * @target: (out) (allow-none) (transfer none): return location for
 *     target, or %NULL.
 * @evt: (out) (allow-none): return location for event, or %NULL.
 *
 * Pops an event from scene.
 *
 * Returns: %TRUE if an event was popped, or %FALSE otherwise
 */
gboolean
lp_scene_pop (lp_Scene *scene, gboolean block,
               GObject **target, lp_Event *evt)
{
  GMainContext *ctx;
  GstMessage *msg;
  const GstStructure *st;

  ctx = g_main_loop_get_context (scene->loop);
  assert (ctx != NULL);

  if (block)
    {
      while (scene->messages == NULL)
        g_main_context_iteration (ctx, TRUE);
    }
  else
    {
      g_main_context_iteration (ctx, FALSE);
    }

  if (scene->messages == NULL)
    return FALSE;               /* nothing to do */

  msg = (GstMessage *) scene->messages->data;
  scene->messages = g_list_delete_link (scene->messages, scene->messages);

  st = gst_message_get_structure (msg);
  if (gst_structure_has_name (st, "lp_Scene:tick"))
    {
      set_if_nonnull (target, G_OBJECT (scene));
      set_if_nonnull (evt, LP_TICK);
    }
  else
    {
      g_critical (G_STRLOC ": unknown application message");
    }
  gst_message_unref (msg);

  return TRUE;
}
