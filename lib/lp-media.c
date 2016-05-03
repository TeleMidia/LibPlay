/* lp-media.c -- Media object.
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

/* Media object.  */
struct _lp_Media
{
  GObject parent;               /* parent object */
  GRecMutex mutex;              /* sync access to media object */
  GstElement *bin;              /* container */
  GstElement *decoder;          /* content decoder */
  GstClockTime offset;          /* start time offset */
  gboolean starting;            /* true if media is starting */
  gboolean stopping;            /* true if media is stopping */
  gboolean drained;             /* true if media has drained */
  guint active_pads;            /* number of active ghost pads in bin */
  gchar *final_uri;             /* final URI */
  struct
  {
    gulong autoplug;            /* auto-plug callback id */
    gulong drain;               /* drain callback id */
    gulong padadded;            /* pad-added callback id */
  } callback;
  struct
  {
    GstElement *volume;         /* audio volume */
    GstElement *convert;        /* audio convert */
    GstElement *resample;       /* audio resample */
    GstElement *filter;         /* audio filter */
    gchar *mixerpad;            /* name of audio sink pad in mixer */
  } audio;
  struct
  {
    GstElement *convert;        /* video convert (optional) */
    GstElement *freeze;         /* image freeze (optional) */
    GstElement *scale;          /* video scale */
    GstElement *filter;         /* video filter */
    gchar *mixerpad;            /* name of video sink pad in mixer */
    gboolean frozen;            /* true if image freeze is present */
  } video;
  struct
  {
    lp_Scene *scene;            /* parent scene */
    gchar *uri;                 /* content URI */
    gint x;                     /* cached x */
    gint y;                     /* cached y */
    gint z;                     /* cached z */
    gint width;                 /* cached width */
    gint height;                /* cached height */
    gdouble alpha;              /* cached alpha */
    gboolean mute;              /* cached mute */
    gdouble volume;             /* cached volume */
  } prop;
};

/* Maps GStreamer elements to lp_Media.  */
static const gstx_eltmap_t lp_media_eltmap[] = {
  {"bin",           offsetof (lp_Media, bin)},
  {"uridecodebin",  offsetof (lp_Media, decoder)},
  {NULL, 0},
};

static const gstx_eltmap_t media_eltmap_audio[] = {
  {"volume",        offsetof (lp_Media, audio.volume)},
  {"audioconvert",  offsetof (lp_Media, audio.convert)},
  {"audioresample", offsetof (lp_Media, audio.resample)},
  {NULL, 0}
};

static const gstx_eltmap_t media_eltmap_video[] = {
  {"videoscale",    offsetof (lp_Media, video.scale)},
  {"capsfilter",    offsetof (lp_Media, video.filter)},
  {NULL, 0},
};

/* Media properties. */
enum
{
  PROP_0,
  PROP_SCENE,
  PROP_URI,
  PROP_X,
  PROP_Y,
  PROP_Z,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_ALPHA,
  PROP_MUTE,
  PROP_VOLUME,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_SCENE   NULL    /* not initialized */
#define DEFAULT_URI     NULL    /* not initialized */
#define DEFAULT_X       0       /* leftmost horizontal position */
#define DEFAULT_Y       0       /* topmost vertical position */
#define DEFAULT_Z       1       /* lowest order */
#define DEFAULT_WIDTH   0       /* natural width */
#define DEFAULT_HEIGHT  0       /* natural height */
#define DEFAULT_ALPHA   1.0     /* natural alpha */
#define DEFAULT_MUTE    FALSE   /* not muted */
#define DEFAULT_VOLUME  1.0     /* natural volume */

/* Define the lp_Media type.  */
GX_DEFINE_TYPE (lp_Media, lp_media, G_TYPE_OBJECT)


#define media_lock(media)    g_rec_mutex_lock (&((media)->mutex))
#define media_unlock(media)  g_rec_mutex_unlock (&((media)->mutex))

#define media_is_starting(media)  ((media)->starting)
#define media_is_stopping(media)  ((media)->stopping)
#define media_has_drained(media)  ((media)->drained)

#define media_has_started(media)                \
  (!media_is_starting ((media))                 \
   && !media_is_stopping ((media))              \
   && (media)->bin != NULL)

#define media_has_stopped(media)                \
  (!media_is_starting ((media))                 \
   && !media_is_stopping ((media))              \
   && (media)->bin == NULL)


/* callbacks */

/* Called asynchronously whenever media decoder finds a new stream.  This is
   triggered immediately before the decoder starts looking for elements to
   handle given the stream.  */

static gboolean
lp_media_autoplug_continue_callback (arg_unused (GstElement *element),
                                          arg_unused (GstPad *pad),
                                          GstCaps *caps,
                                          lp_Media *media)
{
  const gchar *name;

  media_lock (media);

  name = gst_structure_get_name (gst_caps_get_structure (caps, 0));
  if (g_str_has_prefix (name, "image"))
    media->video.frozen = TRUE;

  media_unlock (media);

  return TRUE;
}

/* Called asynchronously whenever media decoder drains its source.  */

static void
lp_media_drained_callback (arg_unused (GstElement *decoder),
                           lp_Media *media)
{
  media_lock (media);

  if (unlikely (media_has_drained (media)))
    goto done;                     /* already drained, nothing to do */

  if (unlikely (media->video.frozen))
    goto done;                     /* static image, nothing to do */

  g_assert_false (media->drained);
  media->drained = TRUE;

  /* FIXME: We shouldn't call lp_media_stop() here, but postponing it to the
     bus callback does not work: the probe callback never gets called.  */

  lp_media_stop (media);

done:
  media_unlock (media);
}

/* Called asynchronously whenever media decoder creates a pad.
   Builds and starts the media processing graph.  */

static void
lp_media_pad_added_callback (arg_unused (GstElement *decoder),
                             GstPad *pad,
                             lp_Media *media)
{
  lp_Scene *scene;
  GstCaps *caps;
  const gchar *name;

  media_lock (media);

  g_assert_true (media_is_starting (media));
  g_assert_false (media_is_stopping (media));

  scene = media->prop.scene;
  g_assert_nonnull (scene);

  caps = gst_pad_query_caps (pad, NULL);
  g_assert_nonnull (caps);

  name = gst_structure_get_name (gst_caps_get_structure (caps, 0));
  g_assert_nonnull (name);
  gst_caps_unref (caps);

  /* WARNING: This seems to be the "safest" place to set the offset.
     Previously, we were setting it in the bus callback but that lead to
     errors, e.g., we were unable to re-start drained objects.  */

  gst_pad_set_offset (pad, (gint64) media->offset);

  if (g_str_equal (name, "video/x-raw") && _lp_scene_has_video (scene))
    {
      GstElement *mixer;
      GstPad *sink;
      GstPad *ghost;

      if (unlikely (media->video.mixerpad != NULL))
        {
          _lp_warn ("ignoring extra video stream");
          goto done;
        }

      _lp_eltmap_alloc_check (media, media_eltmap_video);

      g_object_set (media->video.scale, "add-borders", 0, NULL);
      caps = gst_caps_new_simple ("video/x-raw", "pixel-aspect-ratio",
                                  GST_TYPE_FRACTION, 1, 1, NULL);
      g_assert_nonnull (caps);
      g_object_set (media->video.filter, "caps", caps, NULL);
      gst_caps_unref (caps);

      gstx_bin_add (media->bin, media->video.scale);
      gstx_bin_add (media->bin, media->video.filter);
      gstx_element_link (media->video.scale, media->video.filter);

      if (media->video.frozen)
        {
          media->video.convert = gst_element_factory_make ("videoconvert",
                                                           NULL);
          g_assert_nonnull (media->video.convert);
          media->video.freeze = gst_element_factory_make ("imagefreeze",
                                                          NULL);
          g_assert_nonnull (media->video.freeze);
          gstx_bin_add (media->bin, media->video.convert);
          gstx_bin_add (media->bin, media->video.freeze);
          gstx_element_link (media->video.convert, media->video.freeze);
          gstx_element_link (media->video.freeze, media->video.scale);

          sink = gst_element_get_static_pad (media->video.convert, "sink");
          g_assert_nonnull (sink);
        }
      else
        {
          sink = gst_element_get_static_pad (media->video.scale, "sink");
          g_assert_nonnull (sink);
        }

      g_assert (gst_pad_link (pad, sink) == GST_PAD_LINK_OK);
      gst_object_unref (sink);

      pad = gst_element_get_static_pad (media->video.filter, "src");
      g_assert_nonnull (pad);

      ghost = gst_ghost_pad_new (NULL, pad);
      g_assert_nonnull (ghost);
      gst_object_unref (pad);

      g_assert_true (gst_pad_set_active (ghost, TRUE));
      g_assert_true (gst_element_add_pad (media->bin, ghost));

      mixer = _lp_scene_get_video_mixer (scene);
      g_assert_nonnull (mixer);

      sink = gst_element_get_request_pad (mixer, "sink_%u");
      g_assert_nonnull (sink);

      g_assert_null (media->video.mixerpad);
      media->video.mixerpad = gst_pad_get_name (sink);

      g_assert (gst_pad_link (ghost, sink) == GST_PAD_LINK_OK);
      g_object_set
        (sink,
         "xpos", media->prop.x,
         "ypos", media->prop.y,
         "zorder", media->prop.z,
         "alpha", media->prop.alpha, NULL);

      if (media->prop.width > 0)
        g_object_set (sink, "width", media->prop.width, NULL);

      if (media->prop.height > 0)
        g_object_set (sink, "height", media->prop.height, NULL);
      gst_object_unref (sink);

      if (media->video.frozen)
        {
          gstx_element_set_state_sync (media->video.convert,
                                       GST_STATE_PLAYING);
          gstx_element_set_state_sync (media->video.freeze,
                                       GST_STATE_PLAYING);
        }
      gstx_element_set_state_sync (media->video.scale, GST_STATE_PLAYING);
      gstx_element_set_state_sync (media->video.filter, GST_STATE_PLAYING);
      media->active_pads++;
    }
  else if (g_str_equal (name, "audio/x-raw"))
    {
      GstElement *mixer;
      GstPad *sink;
      GstPad *ghost;

      if (unlikely (media->audio.mixerpad != NULL))
        {
          _lp_warn ("ignoring extra audio stream");
          goto done;
        }

      _lp_eltmap_alloc_check (media, media_eltmap_audio);
      gstx_bin_add (GST_BIN (media->bin), media->audio.volume);
      gstx_bin_add (GST_BIN (media->bin), media->audio.convert);
      gstx_bin_add (GST_BIN (media->bin), media->audio.resample);
      gstx_element_link (media->audio.volume, media->audio.convert);
      gstx_element_link (media->audio.convert, media->audio.resample);

      sink = gst_element_get_static_pad (media->audio.volume, "sink");
      g_assert_nonnull (sink);
      g_assert (gst_pad_link (pad, sink) == GST_PAD_LINK_OK);
      gst_object_unref (sink);

      pad = gst_element_get_static_pad (media->audio.resample, "src");
      g_assert_nonnull (pad);

      ghost = gst_ghost_pad_new (NULL, pad);
      g_assert_nonnull (ghost);
      gst_object_unref (pad);

      g_assert_true (gst_pad_set_active (ghost, TRUE));
      g_assert_true (gst_element_add_pad (media->bin, ghost));

      mixer = _lp_scene_get_audio_mixer (scene);
      g_assert_nonnull (mixer);

      sink = gst_element_get_request_pad (mixer, "sink_%u");
      g_assert_nonnull (sink);

      g_assert_null (media->audio.mixerpad);
      media->audio.mixerpad = gst_pad_get_name (sink);

      g_assert (gst_pad_link (ghost, sink) == GST_PAD_LINK_OK);
      g_object_set (sink,
                    "mute", media->prop.mute,
                    "volume", media->prop.volume, NULL);
      gst_object_unref (sink);

      gstx_element_set_state_sync (media->audio.volume, GST_STATE_PLAYING);
      gstx_element_set_state_sync (media->audio.convert, GST_STATE_PLAYING);
      gstx_element_set_state_sync (media->audio.resample,
                                   GST_STATE_PLAYING);
      media->active_pads++;
    }
  else
    {
      _lp_warn ("unknown stream type: %s", name);
    }

 done:
  media_unlock (media);
}

/* Called whenever pad state matches BLOCK_DOWNSTREAM.
   This callback is triggered by lp_media_stop().  */

static GstPadProbeReturn
lp_media_pad_probe_callback (GstPad *pad,
                             arg_unused (GstPadProbeInfo *info),
                             lp_Media *media)
{
  GstPad *peer;

  media_lock (media);

  g_assert_false (media_is_starting (media));
  g_assert_true (media_is_stopping (media));

  peer = gst_pad_get_peer (pad);
  g_assert_nonnull (peer);
  g_assert (gst_pad_send_event (peer, gst_event_new_eos ()));
  gst_object_unref (peer);

  g_assert_true (gst_pad_set_active (pad, FALSE));
  g_assert (media->active_pads > 0);
  media->active_pads--;

  if (media->active_pads == 0)
    {
      lp_EventStop *event;
      event = lp_event_stop_new (media, media_has_drained (media));
      g_assert_nonnull (event);
      _lp_scene_dispatch (media->prop.scene, LP_EVENT (event));
    }

  media_unlock (media);
  return GST_PAD_PROBE_REMOVE;
}


/* methods */

static void
lp_media_init (lp_Media *media)
{
  g_rec_mutex_init (&media->mutex);
  media->offset = GST_CLOCK_TIME_NONE;
  media->starting = FALSE;
  media->stopping = FALSE;
  media->drained = FALSE;
  media->active_pads = 0;
  media->final_uri = NULL;

  media->callback.autoplug = 0;
  media->callback.drain = 0;
  media->callback.padadded = 0;

  media->audio.mixerpad = NULL;
  media->video.mixerpad = NULL;
  media->video.frozen = FALSE;

  media->prop.scene = DEFAULT_SCENE;
  media->prop.uri = DEFAULT_URI;
  media->prop.x = DEFAULT_X;
  media->prop.y = DEFAULT_Y;
  media->prop.z = DEFAULT_Z;
  media->prop.width = DEFAULT_WIDTH;
  media->prop.height = DEFAULT_HEIGHT;
  media->prop.alpha = DEFAULT_ALPHA;
  media->prop.mute = DEFAULT_MUTE;
  media->prop.volume = DEFAULT_VOLUME;
}

static void
lp_media_get_property (GObject *object, guint prop_id,
                       GValue *value, GParamSpec *pspec)
{
  lp_Media *media;

  media = LP_MEDIA (object);
  media_lock (media);
  switch (prop_id)
    {
    case PROP_SCENE:
      g_value_take_object (value, media->prop.scene);
      break;
    case PROP_URI:
      g_value_set_string (value, media->prop.uri);
      break;
    case PROP_X:
      g_value_set_int (value, media->prop.x);
      break;
    case PROP_Y:
      g_value_set_int (value, media->prop.y);
      break;
    case PROP_Z:
      g_value_set_int (value, media->prop.z);
      break;
    case PROP_WIDTH:
      g_value_set_int (value, media->prop.width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, media->prop.height);
      break;
    case PROP_ALPHA:
      g_value_set_double (value, media->prop.alpha);
      break;
    case PROP_MUTE:
      g_value_set_boolean (value, media->prop.mute);
      break;
    case PROP_VOLUME:
      g_value_set_double (value, media->prop.volume);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
  media_unlock (media);
}

static void
lp_media_set_property (GObject *object, guint prop_id,
                       const GValue *value, GParamSpec *pspec)
{
  lp_Media *media;

  media = LP_MEDIA (object);
  media_lock (media);
  switch (prop_id)
    {
    case PROP_SCENE:
      {                         /* don't take ownership */
        GObject *obj = (GObject *) g_value_get_object (value);
        g_assert (LP_IS_SCENE (obj));
        g_assert (media->prop.scene == DEFAULT_SCENE);
        media->prop.scene = LP_SCENE (obj);
        break;
      }
    case PROP_URI:
      g_assert (media->prop.uri == DEFAULT_URI);
      media->prop.uri = g_value_dup_string (value);
      break;
    case PROP_X:
      media->prop.x = g_value_get_int (value);
      break;
    case PROP_Y:
      media->prop.y = g_value_get_int (value);
      break;
    case PROP_Z:
      media->prop.z = g_value_get_int (value);
      break;
    case PROP_WIDTH:
      media->prop.width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      media->prop.height = g_value_get_int (value);
      break;
    case PROP_ALPHA:
      media->prop.alpha = g_value_get_double (value);
      break;
    case PROP_MUTE:
      media->prop.mute = g_value_get_boolean (value);
      break;
    case PROP_VOLUME:
      media->prop.volume = g_value_get_double (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }

  if (!media_has_started (media))
    goto done;

  switch (prop_id)
    {
    case PROP_SCENE:
    case PROP_URI:
      break;
    case PROP_X:
    case PROP_Y:
    case PROP_Z:
    case PROP_WIDTH:
    case PROP_HEIGHT:
    case PROP_ALPHA:
      {
        GstElement *mixer;
        GstPad *sink;

        if (!_lp_scene_has_video (media->prop.scene))
          break;                /* nothing to do */

        mixer = _lp_scene_get_video_mixer (media->prop.scene);
        g_assert_nonnull (mixer);

        sink = gst_element_get_static_pad (mixer, media->video.mixerpad);
        g_assert_nonnull (sink);

        switch (prop_id)
          {
          case PROP_X:
            g_object_set (sink, "xpos", media->prop.x, NULL);
            break;
          case PROP_Y:
            g_object_set (sink, "ypos", media->prop.y, NULL);
            break;
          case PROP_Z:
            g_object_set (sink, "zorder", media->prop.z, NULL);
            break;
          case PROP_WIDTH:
            g_object_set (sink, "width", media->prop.width, NULL);
            break;
          case PROP_HEIGHT:
            g_object_set (sink, "height", media->prop.height, NULL);
            break;
          case PROP_ALPHA:
            g_object_set (sink, "alpha", media->prop.alpha, NULL);
            break;
          default:
            g_assert_not_reached ();
          }

        g_object_unref (sink);
        break;
      }
    case PROP_MUTE:
    case PROP_VOLUME:
      {
        GstElement *mixer;
        GstPad *sink;

        mixer = _lp_scene_get_audio_mixer (media->prop.scene);
        g_assert_nonnull (mixer);

        sink = gst_element_get_static_pad (mixer, media->audio.mixerpad);
        g_assert_nonnull (sink);

        switch (prop_id)
          {
          case PROP_MUTE:
            g_object_set (sink, "mute", media->prop.mute, NULL);
            break;
          case PROP_VOLUME:
            g_object_set (sink, "volume", media->prop.volume, NULL);
            break;
          default:
            g_assert_not_reached ();
          }

        g_object_unref (sink);
        break;
      }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }

 done:
  media_unlock (media);
}

static void
lp_media_constructed (GObject *object)
{
  lp_Media *media;

  media = LP_MEDIA (object);

  /* FIXME: Users cannot unref media objects directly.  */

  _lp_scene_add_media (media->prop.scene, media);
}

static void
lp_media_dispose (GObject *object)
{
  lp_Media *media;

  media = LP_MEDIA (object);
  media_lock (media);

  if (media->bin != NULL)
    g_object_set_data (G_OBJECT (media->bin), "lp_Media", NULL);

  while (!media_has_stopped (media))
    {
      lp_media_stop (media);

      media_unlock (media);
      _lp_scene_step (media->prop.scene, TRUE);
      media_lock (media);
    }

  media_unlock (media);

  _lp_debug ("disposing media %p", media);
  G_OBJECT_CLASS (lp_media_parent_class)->dispose (object);
}


static void
lp_media_finalize (GObject *object)
{
  lp_Media *media;

  media = LP_MEDIA (object);
  g_rec_mutex_clear (&media->mutex);

  g_free (media->final_uri);
  g_free (media->audio.mixerpad);
  g_free (media->video.mixerpad);
  g_free (media->prop.uri);

  _lp_debug ("finalizing media %p", media);
  G_OBJECT_CLASS (lp_media_parent_class)->finalize (object);
}

static void
lp_media_class_init (lp_MediaClass *cls)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_media_get_property;
  gobject_class->set_property = lp_media_set_property;
  gobject_class->constructed = lp_media_constructed;
  gobject_class->dispose = lp_media_dispose;
  gobject_class->finalize = lp_media_finalize;

  g_object_class_install_property
    (gobject_class, PROP_SCENE, g_param_spec_object
     ("scene", "scene", "parent scene", LP_TYPE_SCENE,
      (GParamFlags) (G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_URI, g_param_spec_string
     ("uri", "uri", "content URI",
      DEFAULT_URI,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_X, g_param_spec_int
     ("x", "x", "horizontal position",
      G_MININT, G_MAXINT, DEFAULT_X,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_Y, g_param_spec_int
     ("y", "y", "vertical position",
      G_MININT, G_MAXINT, DEFAULT_Y,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_Z, g_param_spec_int
     ("z", "z", "z-order",
      0, G_MAXINT, DEFAULT_Z,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_WIDTH, g_param_spec_int
     ("width", "width", "width in pixels",
      0, G_MAXINT, DEFAULT_WIDTH,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_HEIGHT, g_param_spec_int
     ("height", "height", "height in pixels",
      0, G_MAXINT, DEFAULT_HEIGHT,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_ALPHA, g_param_spec_double
     ("alpha", "alpha", "transparency factor",
      0.0, 1.0, DEFAULT_ALPHA,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_MUTE, g_param_spec_boolean
     ("mute", "mute", "mute flag",
      DEFAULT_MUTE,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_VOLUME, g_param_spec_double
     ("volume", "volume", "volume factor",
      0.0, 10.0, DEFAULT_VOLUME,
      G_PARAM_READWRITE));
}


/* internal */

/* Finishes async start.  */

void
_lp_media_finish_start (lp_Media *media)
{
  media_lock (media);

  g_assert_true (media_is_starting (media));
  media->starting = FALSE;
  g_assert_true (media_has_started (media));

  media_unlock (media);
}

/* Releases bin element.  */
static gboolean
_lp_media_release_bin(gpointer data)
{
  GstElement *bin = GST_ELEMENT(data);
  gstx_element_set_state_sync (bin, GST_STATE_NULL);
  gst_object_unref (bin);
  return FALSE;
}

/* Finishes async stop.  */
void
_lp_media_finish_stop (lp_Media *media)
{
  GstElement *pipeline;

  media_lock (media);

  g_assert_true (media_is_stopping (media));
  g_assert (media->active_pads == 0);

  pipeline = _lp_scene_get_pipeline (media->prop.scene);
  g_assert_true (gst_bin_remove (GST_BIN (pipeline), media->bin));
  g_idle_add (_lp_media_release_bin, media->bin);

  g_signal_handler_disconnect (media->decoder, media->callback.autoplug);
  g_signal_handler_disconnect (media->decoder, media->callback.drain);
  g_signal_handler_disconnect (media->decoder, media->callback.padadded);

  media->bin = NULL;            /* indicates has_started and has_stopped */
  media->drained = FALSE;       /* indicates has_drained */
  media->video.frozen = FALSE;  /* indicates static image */

  g_clear_pointer (&media->final_uri, g_free);
  g_clear_pointer (&media->audio.mixerpad, g_free);
  g_clear_pointer (&media->video.mixerpad, g_free);

  media->stopping = FALSE;
  g_assert_true (media_has_stopped (media));

  media_unlock (media);
}


/* public */

/**
 * lp_media_new:
 * @scene: the parent #lp_Scene
 * @uri: content URI
 *
 * Creates a new media object.
 *
 * Returns: (transfer full): a new #lp_Media
 */
lp_Media *
lp_media_new (lp_Scene *scene, const gchar *uri)
{
  return LP_MEDIA (g_object_new (LP_TYPE_MEDIA,
                                 "scene", scene,
                                 "uri", uri, NULL));
}

/**
 * lp_media_start:
 * @media: an #lp_Media
 *
 * Starts @media asynchronously.
 *
 * Returns: %TRUE if successful, or %FALSE otherwise
 */
gboolean
lp_media_start (lp_Media *media)
{
  GstElement *pipeline;
  GstStateChangeReturn ret;

  media_lock (media);

  if (unlikely (!media_has_stopped (media)))
    goto fail;

  if (gst_uri_is_valid (media->prop.uri))
    {
      media->final_uri = g_strdup (media->prop.uri);
    }
  else
    {
      GError *err = NULL;
      gchar *final_uri = gst_filename_to_uri (media->prop.uri, &err);
      if (unlikely (final_uri == NULL))
        {
          _lp_warn ("bad URI: %s", media->prop.uri);
          g_error_free (err);
          goto fail;
        }
      media->final_uri = final_uri;
    }

  _lp_eltmap_alloc_check (media, lp_media_eltmap);

  media->callback.autoplug = g_signal_connect
    (G_OBJECT (media->decoder), "autoplug-continue",
        G_CALLBACK (lp_media_autoplug_continue_callback), media);
  g_assert (media->callback.autoplug > 0);

  media->callback.drain = g_signal_connect
    (media->decoder, "drained",
     G_CALLBACK (lp_media_drained_callback), media);
  g_assert (media->callback.drain > 0);

  media->callback.padadded = g_signal_connect
    (media->decoder, "pad-added",
     G_CALLBACK (lp_media_pad_added_callback), media);
  g_assert (media->callback.padadded > 0);

  g_object_set_data (G_OBJECT (media->bin), "lp_Media", media);
  g_object_set (media->decoder, "uri", media->final_uri, NULL);
  gstx_bin_add (media->bin, media->decoder);

  pipeline = _lp_scene_get_pipeline (media->prop.scene);
  gstx_bin_add (pipeline, media->bin);
  g_assert (gst_object_ref (media->bin) == media->bin);

  ret = gst_element_set_state (media->bin, GST_STATE_PLAYING);
  if (unlikely (ret == GST_STATE_CHANGE_FAILURE))
    goto fail;

  media->offset = _lp_scene_get_clock_time (media->prop.scene);
  media->starting = TRUE;

  media_unlock (media);
  return TRUE;

 fail:
  media_unlock (media);
  return FALSE;
}

/**
 * lp_media_stop:
 * @media: an #lp_Media
 *
 * Stops @media asynchronously.
 *
 * Returns: %TRUE if successful, or %FALSE otherwise
 */
gboolean
lp_media_stop (lp_Media *media)
{
  GstIterator *it;
  gboolean done;
  GValue item = G_VALUE_INIT;

  media_lock (media);

  if (unlikely (!media_has_started (media)))
    goto fail;

  media->stopping = TRUE;

  it = gst_element_iterate_src_pads (media->bin);
  g_assert_nonnull (it);

  done = FALSE;
  do
    {
      switch (gst_iterator_next (it, &item))
        {
        case GST_ITERATOR_OK:
        {
          GstPad *pad = GST_PAD (g_value_get_object (&item));
          g_assert (gst_pad_add_probe
                    (pad,
                     GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                     (GstPadProbeCallback) lp_media_pad_probe_callback,
                     media, NULL) > 0);
          g_value_reset (&item);
          break;
        }
      case GST_ITERATOR_RESYNC:
        gst_iterator_resync (it);
        break;
      case GST_ITERATOR_DONE:
        done = TRUE;
        break;
      case GST_ITERATOR_ERROR:
      default:
        g_assert_not_reached ();
        break;
      }
    }
  while (!done);

  g_value_unset (&item);
  gst_iterator_free (it);

  media_unlock (media);
  return TRUE;

  fail:
  media_unlock (media);
  return FALSE;
}
