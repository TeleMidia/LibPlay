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

/* Scene object.  */
struct _lp_Scene
{
  GObject parent;               /* parent object */
  GRecMutex mutex;              /* sync access to scene */
  GstElement *pipeline;         /* scene pipeline */
  gboolean quitting;            /* true if scene is quitting */
  GMainLoop *loop;              /* scene loop */
  GList *events;                /* pending events */
  GList *children;              /* child media objects */
  struct
  {
    GstClockID id;              /* last clock id */
    GstClock *clock;            /* pipeline clock */
    GstClockTime offset;        /* start time offset */
  } clock;
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
    GstElement *text;           /* text overlay */
    GstElement *convert;        /* video convert */
    GstElement *sink;           /* video sink */
  } video;
  struct
  {
    gint mask;                  /* event mask */
    gint width;                 /* cached width */
    gint height;                /* cached height */
    gint pattern;               /* cached pattern */
    gint wave;                  /* cached wave */
    guint64 ticks;              /* total number of ticks */
    guint64 interval;           /* interval between ticks (nanoseconds) */
    guint64 time;               /* running time (nanoseconds) */
    gboolean lockstep;          /* lockstep mode */
    gboolean slave_audio;       /* enslave audio clock */
    gchar *text;                /* cached text */
    guint text_color;           /* cached text color */
    gchar *text_font;           /* cached text font */
  } prop;
};

/* Maps GStreamer elements to offsets in lp_Scene.  */
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
  {"textoverlay",   offsetof (lp_Scene, video.text)},
  {"videoconvert",  offsetof (lp_Scene, video.convert)},
  {"autovideosink", offsetof (lp_Scene, video.sink)},
  {NULL, 0},
};

/* Scene properties.  */
enum
{
  PROP_0,
  PROP_MASK,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_PATTERN,
  PROP_WAVE,
  PROP_TICKS,
  PROP_INTERVAL,
  PROP_TIME,
  PROP_LOCKSTEP,
  PROP_SLAVE_AUDIO,
  PROP_TEXT,
  PROP_TEXT_COLOR,
  PROP_TEXT_FONT,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_MASK         LP_EVENT_MASK_ANY /* any event */
#define DEFAULT_WIDTH        0                 /* no video output */
#define DEFAULT_HEIGHT       0                 /* no video output */
#define DEFAULT_PATTERN      2                 /* black */
#define DEFAULT_WAVE         4                 /* silence */
#define DEFAULT_TICKS        0                 /* no ticks */
#define DEFAULT_INTERVAL     GST_SECOND        /* one tick per second */
#define DEFAULT_TIME         0                 /* zero nanoseconds */
#define DEFAULT_LOCKSTEP     FALSE             /* real-time mode */
#define DEFAULT_SLAVE_AUDIO  FALSE             /* don't enslave audio */
#define DEFAULT_TEXT         NULL              /* not initialized */
#define DEFAULT_TEXT_COLOR   0xffffffff        /* white */
#define DEFAULT_TEXT_FONT    NULL              /* not initialized */

/* Define the lp_Scene type.  */
GX_DEFINE_TYPE (lp_Scene, lp_scene, G_TYPE_OBJECT)

static gint lp_scene_tick_callback (GstClock *, GstClockTime, GstClockID,
                                    lp_Scene *);


/* Locking and unlocking.  */
#define scene_lock(scene)         g_rec_mutex_lock (&(scene)->mutex)
#define scene_unlock(scene)       g_rec_mutex_unlock (&(scene)->mutex)

#define SCENE_LOCKED(scene, stmt)               \
  STMT_BEGIN                                    \
  {                                             \
    scene_lock ((scene));                       \
    stmt;                                       \
    scene_unlock ((scene));                     \
  }                                             \
  STMT_END

#define SCENE_UNLOCKED(scene, stmt)             \
  STMT_BEGIN                                    \
  {                                             \
    scene_unlock ((scene));                     \
    stmt;                                       \
    scene_lock ((scene));                       \
  }                                             \
  STMT_END

/* Other queries.  */
#define scene_is_quitting(scene)  ((scene)->quitting)

/* Enslaves @scene's audio sink clock to @scene clock.  */

static void
scene_enslave_audio_clock (lp_Scene *scene)
{
  GstElement *sink;
  GstClock *clock;

  sink = _lp_scene_get_real_audio_sink (scene);
  g_assert_nonnull (sink);
  g_assert (GST_IS_AUDIO_BASE_SINK (sink));

  GST_OBJECT_FLAG_SET (GST_OBJECT (sink), GST_CLOCK_FLAG_CAN_SET_MASTER);

  clock = GST_AUDIO_BASE_SINK (sink)->provided_clock;
  g_assert_nonnull (clock);

  if (unlikely (!gst_clock_set_master (clock, scene->prop.slave_audio
                                       ? scene->clock.clock : NULL)))
    {
      _lp_warn ("cannot enslave audio clock to scene clock");
    }

  gst_object_unref (sink);
}

/* Returns the real sink (determined by @offset) of @scene.  */

static GstElement *
scene_get_real_sink (lp_Scene *scene, ptrdiff_t offset)
{
  GstElement **elt;
  GObject *obj = NULL;
  GstElement *sink = NULL;

  elt = (GstElement **)(((ptrdiff_t) scene) + offset);
  g_assert_nonnull (*elt);

  if (!GST_IS_CHILD_PROXY (*elt))
    return GST_ELEMENT (gst_object_ref (*elt));

  obj = gst_child_proxy_get_child_by_index (GST_CHILD_PROXY (*elt), 0);
  g_assert_nonnull (obj);

  sink = GST_ELEMENT (obj);
  g_assert_nonnull (sink);
  g_assert (GST_IS_BASE_SINK (sink));

  return sink;
}

/* Runs a single iteration of @scene loop without locking the @scene.
   Call may block if @block is true.  */

static void
scene_step_unlocked (lp_Scene *scene, gboolean block)
{
  GMainContext *ctx;

  g_assert (!scene_is_quitting (scene));
  ctx = g_main_loop_get_context (scene->loop);
  g_assert_nonnull (ctx);
  g_main_context_iteration (ctx, block);
}

/* Updates @scene clock id.  */

static void
scene_update_clock_id (lp_Scene *scene)
{
  GstClock *clock;
  GstClockID id;
  GstClockTime time;
  GstClockCallback cb;

  clock = gst_pipeline_get_clock (GST_PIPELINE (scene->pipeline));
  g_assert_nonnull (clock);
  g_assert (clock == scene->clock.clock);

  time = gst_clock_get_time (clock);
  g_assert (time != GST_CLOCK_TIME_NONE);

  id = gst_clock_new_periodic_id (clock, time, scene->prop.interval);
  g_assert_nonnull (id);
  g_object_unref (clock);

  cb = (GstClockCallback) lp_scene_tick_callback;
  g_assert (gst_clock_id_wait_async (id, cb, scene, NULL) == GST_CLOCK_OK);
  if (scene->clock.id != NULL)
    {
      gst_clock_id_unschedule (scene->clock.id);
      g_clear_pointer (&scene->clock.id, gst_clock_id_unref);
    }
  scene->clock.id = id;
}


/* callbacks */

/* WARNING: In these callbacks, any access to scene *MUST* be locked.  */

/* Signals a pipeline clock tick.  Here we dispatch a tick lp_Event.  */

static gint
lp_scene_tick_callback (arg_unused (GstClock *clock),
                        arg_unused (GstClockTime time),
                        arg_unused (GstClockID id),
                        lp_Scene *scene)
{
  lp_Event *event;
  guint64 ticks;

  SCENE_LOCKED (scene, ticks = scene->prop.ticks);

  event = LP_EVENT (_lp_event_tick_new (scene, ticks));
  g_assert_nonnull (event);
  _lp_scene_dispatch (scene, event);

  return TRUE;
}

/* Signals that scene pipeline has received a message.  */

static gboolean
lp_scene_bus_callback (arg_unused (GstBus *bus),
                       GstMessage *msg,
                       lp_Scene *scene)
{
  g_assert_nonnull (scene);
  g_assert (LP_IS_SCENE (scene));

  switch (GST_MESSAGE_TYPE (msg))
    {
    case GST_MESSAGE_APPLICATION:
      {
        const GstStructure *st;
        lp_Event *event;
        lp_EventMask mask;

        st = gst_message_get_structure (msg);
        g_assert_nonnull (st);

        event = LP_EVENT (gstx_structure_get_pointer (st, "lp_Event"));
        g_assert_nonnull (st);

        mask = lp_event_get_mask (event);
        switch (mask)
          {
          case LP_EVENT_MASK_TICK:
            {
              SCENE_LOCKED (scene, scene->prop.ticks++);
              break;
            }
          case LP_EVENT_MASK_KEY:           /* fall through */
          case LP_EVENT_MASK_POINTER_CLICK: /* fall through */
          case LP_EVENT_MASK_POINTER_MOVE:
            {
              break;            /* nothing to do */
            }
          case LP_EVENT_MASK_ERROR: /* fall through */
          case LP_EVENT_MASK_START: /* fall through */
          case LP_EVENT_MASK_STOP:  /* fall through */
          case LP_EVENT_MASK_SEEK:
            {
              lp_Media *media;

              media = LP_MEDIA (lp_event_get_source (event));
              switch (mask)
                {
                case LP_EVENT_MASK_ERROR:
                  _lp_media_finish_error (media);
                  break;
                case LP_EVENT_MASK_START:
                  _lp_media_finish_start (media);
                  break;
                case LP_EVENT_MASK_STOP:
                  _lp_media_finish_stop (media);
                  break;
                case LP_EVENT_MASK_SEEK:
                  _lp_media_finish_seek (media);
                  break;
                default:
                  g_assert_not_reached ();
                }
              break;
            }
          default:
            g_assert_not_reached ();
          }

        scene_lock (scene);
        scene->events = g_list_append (scene->events, event);
        g_assert_nonnull (scene->events);
        scene_unlock (scene);
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
      {
        GstNavigationEventType type;
        GstEvent *from = NULL;
        lp_Event *to = NULL;

        if (gst_navigation_message_get_type (msg)
            != GST_NAVIGATION_MESSAGE_EVENT)
          break;                /* nothing to do */

        if (unlikely (!gst_navigation_message_parse_event (msg, &from)))
          break;                /* nothing to do */

        g_assert_nonnull (from);
        type = gst_navigation_event_get_type (from);
        switch (type)
          {
          case GST_NAVIGATION_EVENT_KEY_PRESS: /* fall through */
          case GST_NAVIGATION_EVENT_KEY_RELEASE:
            {
              const gchar *key;
              gboolean press;

              g_assert (gst_navigation_event_parse_key_event (from, &key));
              press = type == GST_NAVIGATION_EVENT_KEY_PRESS;
              to = LP_EVENT (_lp_event_key_new (scene, key, press));
              break;
            }
          case GST_NAVIGATION_EVENT_MOUSE_BUTTON_PRESS: /* fall through */
          case GST_NAVIGATION_EVENT_MOUSE_BUTTON_RELEASE:
            {
              gint button;
              gdouble x, y;
              gboolean press;

              g_assert (gst_navigation_event_parse_mouse_button_event
                        (from, &button, &x, &y));
              press = type == GST_NAVIGATION_EVENT_MOUSE_BUTTON_PRESS;
              to = LP_EVENT (_lp_event_pointer_click_new (scene, x, y,
                                                          button, press));
              break;
            }
          case GST_NAVIGATION_EVENT_MOUSE_MOVE:
            {
              gdouble x, y;

              g_assert (gst_navigation_event_parse_mouse_move_event
                        (from, &x, &y));
              to = LP_EVENT (_lp_event_pointer_move_new (scene, x, y));
              break;
            }
          default:
            {
              break;            /* ignore unknown events */
            }
          }

        if (likely (to != NULL))
          _lp_scene_dispatch (scene, LP_EVENT (to));

        gst_event_unref (from);
        break;
      }
    case GST_MESSAGE_EOS:
      break;
    case GST_MESSAGE_ERROR:
    case GST_MESSAGE_WARNING:
      {
        GError *error = NULL;
        gchar *debug = NULL;

        if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR)
          gst_message_parse_error (msg, &error, NULL);
        else
          gst_message_parse_warning (msg, &error, NULL);
        g_assert_nonnull (error);

        debug = gst_error_get_message (error->domain, error->code);
        g_assert_nonnull (debug);

        if (unlikely (error->domain == GST_LIBRARY_ERROR
                      || error->domain == GST_RESOURCE_ERROR
                      || error->domain == GST_CORE_ERROR))
          {
            _lp_critical ("%s: %s", error->message, debug);
          }
        else if (error->domain ==  GST_STREAM_ERROR)
          {
            GstObject *obj;
            lp_Media *media;

            obj = GST_MESSAGE_SRC (msg);
            g_assert_nonnull (obj);

            media = _lp_media_find_media (obj);
            if (media != NULL && _lp_media_get_active_pads (media) == 0)
              {
                GError *errnew;
                lp_Event *event;

                /* FIXME: Currently we only handle async start errors.  */

                errnew = g_error_new_literal (LP_ERROR, LP_ERROR_START,
                                              error->message);
                g_assert_nonnull (errnew);
                event = LP_EVENT (_lp_event_error_new (media, errnew));
                g_error_free (errnew);

                _lp_scene_dispatch (scene, event);
              }
            else
              {
                _lp_error ("STREAM ERROR: %s: %s", error->message, debug);
              }
          }
        else
          {
            g_assert_not_reached (); /* unknown error */
          }

        g_free (debug);
        g_error_free (error);
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
        SCENE_LOCKED (scene, scene_update_clock_id (scene));
        break;
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
    case GST_MESSAGE_UNKNOWN:
    default:
      break;                    /* ignore unknown messages */
    }

  return TRUE;
}


/* methods */

static void
lp_scene_init (lp_Scene *scene)
{
  g_rec_mutex_init (&scene->mutex);
  scene->quitting = FALSE;

  scene->loop = g_main_loop_new (NULL, FALSE);
  g_assert_nonnull (scene->loop);
  scene->events = NULL;
  scene->children = NULL;

  scene->clock.id = NULL;
  scene->clock.clock = NULL;
  scene->clock.offset = GST_CLOCK_TIME_NONE;

  scene->prop.mask = DEFAULT_MASK;
  scene->prop.width = DEFAULT_WIDTH;
  scene->prop.height = DEFAULT_HEIGHT;
  scene->prop.pattern = DEFAULT_PATTERN;
  scene->prop.wave = DEFAULT_WAVE;
  scene->prop.ticks = DEFAULT_TICKS;
  scene->prop.interval= DEFAULT_INTERVAL;
  scene->prop.time = DEFAULT_TIME;
  scene->prop.lockstep = DEFAULT_LOCKSTEP;
  scene->prop.slave_audio = DEFAULT_SLAVE_AUDIO;
  scene->prop.text = DEFAULT_TEXT;
  scene->prop.text_color = DEFAULT_TEXT_COLOR;
  scene->prop.text_font = DEFAULT_TEXT_FONT;
}

static void
lp_scene_get_property (GObject *object, guint prop_id,
                       GValue *value, GParamSpec *pspec)
{
  lp_Scene *scene;

  scene = LP_SCENE (object);
  scene_lock (scene);
  if (unlikely (scene_is_quitting (scene)))
    goto done;

  switch (prop_id)
    {
    case PROP_MASK:
      g_value_set_int (value, scene->prop.mask);
      break;
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
    case PROP_INTERVAL:
      g_value_set_uint64 (value, scene->prop.interval);
      break;
    case PROP_TIME:
      scene->prop.time = _lp_scene_get_running_time (scene);
      g_value_set_uint64 (value, scene->prop.time);
      break;
    case PROP_LOCKSTEP:
      g_value_set_boolean (value, scene->prop.lockstep);
      break;
    case PROP_SLAVE_AUDIO:
      g_value_set_boolean (value, scene->prop.slave_audio);
      break;
    case PROP_TEXT:
      g_value_set_string (value, scene->prop.text);
      break;
    case PROP_TEXT_COLOR:
      g_value_set_uint (value, scene->prop.text_color);
      break;
    case PROP_TEXT_FONT:
      g_value_set_string (value, scene->prop.text_font);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }

 done:
  scene_unlock (scene);
}

static void
lp_scene_set_property (GObject *object, guint prop_id,
                       const GValue *value, GParamSpec *pspec)
{
  lp_Scene *scene;

  scene = LP_SCENE (object);
  scene_lock (scene);
  if (unlikely (scene_is_quitting (scene)))
    goto done;

  switch (prop_id)
    {
    case PROP_MASK:
      scene->prop.mask = g_value_get_int (value);
      break;
    case PROP_WIDTH:
      scene->prop.width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      scene->prop.height = g_value_get_int (value);
      break;
    case PROP_PATTERN:
      scene->prop.pattern = g_value_get_int (value);
      break;
    case PROP_WAVE:
      scene->prop.wave = g_value_get_int (value);
      g_object_set (scene->audio.blank, "wave", scene->prop.wave, NULL);
      break;
    case PROP_TICKS:
      g_assert_not_reached ();  /* read-only */
      break;
    case PROP_INTERVAL:
      scene->prop.interval = g_value_get_uint64 (value);
      scene_update_clock_id (scene);
      break;
    case PROP_TIME:
      g_assert_not_reached ();  /* read-only */
      break;
    case PROP_LOCKSTEP:
      scene->prop.lockstep = g_value_get_boolean (value);
      g_object_set (scene->clock.clock, "lockstep",
                    scene->prop.lockstep, NULL);
      break;
    case PROP_SLAVE_AUDIO:
      scene->prop.slave_audio = g_value_get_boolean (value);
      scene_enslave_audio_clock (scene);
      break;
    case PROP_TEXT:
      g_free (scene->prop.text);
      scene->prop.text = g_value_dup_string (value);
      break;
    case PROP_TEXT_COLOR:
      scene->prop.text_color = g_value_get_uint (value);
      break;
    case PROP_TEXT_FONT:
      g_free (scene->prop.text_font);
      scene->prop.text_font = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }

  switch (prop_id)
    {
    case PROP_PATTERN:          /* fall through */
    case PROP_TEXT:             /* fall through */
    case PROP_TEXT_COLOR:       /* fall through */
    case PROP_TEXT_FONT:
      {
        if (!_lp_scene_has_video (scene))
          break;                /* nothing to do */

        switch (prop_id)
          {
          case PROP_PATTERN:
            g_object_set (scene->video.blank, "pattern",
                          scene->prop.pattern, NULL);
            break;
          case PROP_TEXT:
            g_object_set (scene->video.text, "text",
                          scene->prop.text, NULL);
            break;
          case PROP_TEXT_COLOR:
            g_object_set (scene->video.text,
                          "color", scene->prop.text_color, NULL);
            break;
          case PROP_TEXT_FONT:
            g_object_set (scene->video.text,
                          "font-desc", scene->prop.text_font, NULL);
            break;
          default:
            g_assert_not_reached ();
          }

        break;
      }
      default:
        break;                  /* nothing to do */
    }

 done:
  scene_unlock (scene);
}

static void
lp_scene_constructed (GObject *object)
{
  lp_Scene *scene;
  GstBus *bus;
  gulong ret;

  scene = LP_SCENE (object);
  g_assert (!scene_is_quitting (scene));

  _lp_eltmap_alloc_check (scene, lp_scene_eltmap);

  scene->clock.clock = GST_CLOCK (g_object_new (LP_TYPE_CLOCK, NULL));
  g_assert_nonnull (scene->clock.clock);
  gst_pipeline_use_clock (GST_PIPELINE (scene->pipeline),
                          scene->clock.clock);

  bus = gst_pipeline_get_bus (GST_PIPELINE (scene->pipeline));
  g_assert_nonnull (bus);

  ret = gst_bus_add_watch (bus, (GstBusFunc) lp_scene_bus_callback, scene);
  g_assert (ret > 0);
  gst_object_unref (bus);

  gstx_bin_add (scene->pipeline, scene->audio.blank);
  gstx_bin_add (scene->pipeline, scene->audio.mixer);
  gstx_bin_add (scene->pipeline, scene->audio.sink);
  gstx_element_link (scene->audio.blank, scene->audio.mixer);
  gstx_element_link (scene->audio.mixer, scene->audio.sink);

  if (_lp_scene_has_video (scene))
    {
      GstCaps *caps;

      _lp_eltmap_alloc_check (scene, lp_scene_eltmap_video);
      gstx_bin_add (scene->pipeline, scene->video.blank);
      gstx_bin_add (scene->pipeline, scene->video.filter);
      gstx_bin_add (scene->pipeline, scene->video.mixer);
      gstx_bin_add (scene->pipeline, scene->video.text);
      gstx_bin_add (scene->pipeline, scene->video.convert);
      gstx_bin_add (scene->pipeline, scene->video.sink);
      gstx_element_link (scene->video.blank, scene->video.filter);
      gstx_element_link (scene->video.filter, scene->video.mixer);
      gstx_element_link (scene->video.mixer, scene->video.text);
      gstx_element_link (scene->video.text, scene->video.convert);
      gstx_element_link (scene->video.convert, scene->video.sink);

      caps = gst_caps_new_empty_simple ("video/x-raw");
      g_assert_nonnull (caps);
      gst_caps_set_simple (caps,
                           "width", G_TYPE_INT, scene->prop.width,
                           "height", G_TYPE_INT, scene->prop.height, NULL);
      g_object_set (scene->video.filter, "caps", caps, NULL);
      gst_caps_unref (caps);
    }

  g_object_set (scene,
                "pattern", scene->prop.pattern,
                "wave", scene->prop.wave, NULL);

  gstx_element_set_state_sync (scene->pipeline, GST_STATE_PLAYING);

  scene_lock (scene);
  if (scene->prop.slave_audio)
    {
      scene_enslave_audio_clock (scene);
    }
  scene->clock.offset = gstx_element_get_clock_time (scene->pipeline);
  scene_unlock (scene);
}

static void
lp_scene_dispose (GObject *object)
{
  lp_Scene *scene;
  GList *l;
  GstElement *pipeline;
  GstBus *bus;

  scene = LP_SCENE (object);
  scene_lock (scene);

  if (likely (scene->clock.id != NULL)) /* disable ticks */
    gst_clock_id_unschedule (scene->clock.id);

  while ((l = scene->children) != NULL) /* collect children */
    {
      SCENE_UNLOCKED (scene, g_object_unref (LP_MEDIA (l->data)));
      scene->children = g_list_delete_link (scene->children, l);
    }
  g_assert_null (scene->children);

  scene->quitting = TRUE;       /* disable all scene functions */

  pipeline = scene->pipeline;
  g_assert_nonnull (pipeline);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  g_assert_nonnull (bus);
  g_assert (gst_bus_remove_watch (bus));
  gst_object_unref (bus);

  SCENE_UNLOCKED
    (scene, gstx_element_set_state_sync (pipeline, GST_STATE_NULL));

  gst_object_unref (scene->pipeline);
  g_object_unref (scene->clock.clock);

  if (likely (scene->clock.id != NULL))
    gst_clock_id_unref (scene->clock.id);

  scene_unlock (scene);

  _lp_debug ("disposing scene %p", scene);
  G_OBJECT_CLASS (lp_scene_parent_class)->dispose (object);
}

static void
lp_scene_finalize (GObject *object)
{
  lp_Scene *scene;

  scene = LP_SCENE (object);
  g_rec_mutex_clear (&scene->mutex);
  g_main_loop_unref (scene->loop);

  g_assert_null (scene->children);
  g_list_free_full (scene->events, (GDestroyNotify) g_object_unref);

  g_free (scene->prop.text);
  g_free (scene->prop.text_font);

  _lp_debug ("finalizing scene %p", scene);
  G_OBJECT_CLASS (lp_scene_parent_class)->finalize (object);
}

static void
lp_scene_class_init (lp_SceneClass *cls)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_scene_get_property;
  gobject_class->set_property = lp_scene_set_property;
  gobject_class->constructed = lp_scene_constructed;
  gobject_class->dispose = lp_scene_dispose;
  gobject_class->finalize = lp_scene_finalize;

  g_object_class_install_property
    (gobject_class, PROP_MASK, g_param_spec_int
     ("mask", "mask", "event mask",
      G_MININT, G_MAXINT, DEFAULT_MASK,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_WIDTH, g_param_spec_int
     ("width", "width", "width in pixels",
      0, G_MAXINT, DEFAULT_WIDTH,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_HEIGHT, g_param_spec_int
     ("height", "height", "height in pixels",
      0, G_MAXINT, DEFAULT_HEIGHT,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_PATTERN, g_param_spec_int
     ("pattern", "pattern", "background video pattern",
      0, 24, DEFAULT_PATTERN,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_WAVE, g_param_spec_int
     ("wave", "wave", "background audio wave",
      0, 12, DEFAULT_WAVE,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_TICKS, g_param_spec_uint64
     ("ticks", "ticks", "total number of ticks so far",
      0, G_MAXUINT64, DEFAULT_TICKS,
      (GParamFlags)(G_PARAM_READABLE)));

  g_object_class_install_property
    (gobject_class, PROP_INTERVAL, g_param_spec_uint64
     ("interval", "interval", "interval between ticks (in nanoseconds)",
      0, G_MAXUINT64, DEFAULT_TICKS,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_TIME, g_param_spec_uint64
     ("time", "time", "running time (in nanoseconds)",
      0, G_MAXUINT64, DEFAULT_TIME,
      (GParamFlags)(G_PARAM_READABLE)));

  g_object_class_install_property
    (gobject_class, PROP_LOCKSTEP, g_param_spec_boolean
     ("lockstep", "lock-step mode ", "enable lock-step mode",
      DEFAULT_LOCKSTEP,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_SLAVE_AUDIO, g_param_spec_boolean
     ("slave-audio", "slave-audio ", "enslave audio clock to scene clock",
      DEFAULT_SLAVE_AUDIO,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_TEXT, g_param_spec_string
     ("text", "text", "overlay text",
      DEFAULT_TEXT,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_TEXT_COLOR, g_param_spec_uint
     ("text-color", "text color", "overlay text color",
      0, G_MAXUINT, DEFAULT_TEXT_COLOR,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_TEXT_FONT, g_param_spec_string
     ("text-font", "text font", "overlay text font",
      DEFAULT_TEXT_FONT,
      (GParamFlags)(G_PARAM_READWRITE)));

  if (!gst_is_initialized ())
    {
      GError *error = NULL;
      if (unlikely (!gst_init_check (NULL, NULL, &error)))
        {
          g_assert_nonnull (error);
          _lp_error ("%s", error->message);
          g_error_free (error);
        }
    }
}


/* internal */

/* Adds @media to @scene and takes ownership of @media.  */

void
_lp_scene_add_media (lp_Scene *scene, lp_Media *media)
{
  scene_lock (scene);

  scene->children = g_list_append (scene->children, media);
  g_assert_nonnull (scene->children);

  scene_unlock (scene);
}

/* Returns @scene pipeline.  */

GstElement *
_lp_scene_get_pipeline (lp_Scene *scene)
{
  GstElement *pipeline;

  SCENE_LOCKED (scene, pipeline = scene->pipeline);
  return pipeline;
}

/* Returns the @scene running time (in nanoseconds).  */

GstClockTime
_lp_scene_get_running_time (lp_Scene *scene)
{
  GstClockTime time;

  scene_lock (scene);

  time = gstx_element_get_clock_time (scene->pipeline);
  time = time - scene->clock.offset;

  scene_unlock (scene);
  return time;
}

/* Returns @scene audio mixer.  */

GstElement *
_lp_scene_get_audio_mixer (lp_Scene *scene)
{
  GstElement *mixer;

  SCENE_LOCKED (scene, mixer = scene->audio.mixer);
  return mixer;
}

/* Returns @scene (real) audio sink.
   Transfer-full; unref after usage.  */

GstElement *
_lp_scene_get_real_audio_sink (lp_Scene *scene)
{
  GstElement *sink;

  scene_lock (scene);

  sink = scene_get_real_sink (scene, offsetof (lp_Scene, audio.sink));

  scene_unlock (scene);
  return sink;
}

/* Returns @scene video mixer.  */

GstElement *
_lp_scene_get_video_mixer (lp_Scene *scene)
{
  GstElement *mixer;

  SCENE_LOCKED (scene, mixer = scene->video.mixer);
  return mixer;
}

/* Returns @scene (real) video sink.
   Transfer-full; unref after usage.  */

GstElement *
_lp_scene_get_real_video_sink (lp_Scene *scene)
{
  GstElement *sink;

  scene_lock (scene);

  sink = scene_get_real_sink (scene, offsetof (lp_Scene, video.sink));

  scene_unlock (scene);
  return sink;
}

/* Returns true if @scene has video output.  */

gboolean
_lp_scene_has_video (lp_Scene *scene)
{
  gboolean status;

  scene_lock (scene);

  status = scene->prop.width > 0 && scene->prop.height > 0;

  scene_unlock (scene);
  return status;
}

/* Runs a single iteration of @scene loop.
   Call may block if @block is true.  */

void
_lp_scene_step (lp_Scene *scene, gboolean block)
{
  scene_lock (scene);
  if (unlikely (scene_is_quitting (scene)))
    goto done;

  SCENE_UNLOCKED (scene, scene_step_unlocked (scene, block));

 done:
  scene_unlock (scene);
}

/* Dispatches @event to @scene, i.e., posts an application message
   containing the lp_Event into @scene pipeline bus.  */

void
_lp_scene_dispatch (lp_Scene *scene, lp_Event *event)
{
  GstStructure *st;
  GstMessage *msg;
  GstElement *pipeline;

  st = gst_structure_new ("lp_Event", "lp_Event", G_TYPE_POINTER, event,
                          NULL);
  g_assert_nonnull (st);

  msg = gst_message_new_application (NULL, st);
  g_assert_nonnull (msg);

  pipeline = _lp_scene_get_pipeline (scene); /* locked */
  g_assert_nonnull (pipeline);

  g_assert (gst_element_post_message (pipeline, msg));
}


/* public */

/**
 * lp_scene_new:
 * @width: scene width (in pixels)
 * @height: scene height (in pixels)
 *
 * Creates a new scene with the given dimensions.
 *
 * Returns: (transfer full): a new #lp_Scene
 */
lp_Scene *
lp_scene_new (gint width, gint height)
{
  return LP_SCENE (g_object_new (LP_TYPE_SCENE,
                                 "width", width,
                                 "height", height, NULL));
}

/**
 * lp_scene_to_string:
 * @scene: an #lp_Scene
 *
 * Gets a string representation of @scene.
 *
 * Returns: (transfer full): a string representing the scene
 */
gchar *
lp_scene_to_string (lp_Scene *scene)
{
  gchar *str;
  guint64 time;

  scene_lock (scene);

  g_object_get (scene, "time", &time, NULL);
  str = g_strdup_printf ("\
%s at %p\n\
  mask: 0x%x\n\
  width: %d\n\
  height: %d\n\
  pattern: %d\n\
  wave: %d\n\
  ticks: %" G_GUINT64_FORMAT "\n\
  interval: %" GST_TIME_FORMAT "\n\
  time: %" GST_TIME_FORMAT "\n\
  lockstep: %s\n\
  slave-audio: %s\n\
  text: %s\n\
  text-color: 0x%x\n\
  text-font: %s\n\
",
                         G_OBJECT_TYPE_NAME (scene),
                         scene,
                         (guint) scene->prop.mask,
                         scene->prop.width,
                         scene->prop.height,
                         scene->prop.pattern,
                         scene->prop.wave,
                         scene->prop.ticks,
                         GST_TIME_ARGS (scene->prop.interval),
                         GST_TIME_ARGS (time),
                         strbool (scene->prop.lockstep),
                         strbool (scene->prop.slave_audio),
                         scene->prop.text,
                         scene->prop.text_color,
                         scene->prop.text_font);
  g_assert_nonnull (str);

  scene_unlock (scene);
  return str;
}

/**
 * lp_scene_advance:
 * @scene: an #lp_Scene
 * @time: the amount of time to advance (in nanoseconds)
 *
 * Advances the scene clock by @time nanoseconds.  This function should only
 * be used when @scene is in lock-step mode, i.e., when its "lockstep"
 * property is set to %TRUE.
 *
 * Returns: %TRUE successful, or %FALSE otherwise
 */
gboolean
lp_scene_advance (lp_Scene *scene, guint64 time)
{
  gboolean status = FALSE;

  scene_lock (scene);
  if (unlikely (scene_is_quitting (scene)))
    goto done;

  status = _lp_clock_advance (LP_CLOCK (scene->clock.clock), time);

 done:
  scene_unlock (scene);
  return status;
}

/**
 * lp_scene_receive:
 * @scene: an #lp_Scene
 * @block: whether the call may block
 * @evt: (out) (allow-none): return location for event, or %NULL
 *
 * Receives an event from @scene.
 *
 * Returns: (allow-none) (transfer full): an #lp_Event, or %NULL if there
 * are no pending events
 */
lp_Event *
lp_scene_receive (lp_Scene *scene, gboolean block)
{
  lp_Event *event = NULL;

  scene_lock (scene);
  if (unlikely (scene_is_quitting (scene)))
    goto done;

 retry:
  if (block)
    {
      while (scene->events == NULL)
        SCENE_UNLOCKED (scene, scene_step_unlocked (scene, TRUE));
    }
  else
    {
      SCENE_UNLOCKED (scene, scene_step_unlocked (scene, FALSE));
    }

  if (unlikely (scene->events == NULL))
    {
      event = NULL;
      goto done;                /* nothing to do */
    }

  event = LP_EVENT (scene->events->data);
  g_assert_nonnull (event);
  scene->events = g_list_delete_link (scene->events, scene->events);

  if ((lp_event_get_mask (event) & scene->prop.mask) == 0)
    {
      g_object_unref (event);   /* consume event */
      goto retry;
    }

 done:
  scene_unlock (scene);
  return event;
}
