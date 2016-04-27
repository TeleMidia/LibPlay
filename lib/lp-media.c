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
  GstElement *bin;              /* container */
  GstElement *decoder;          /* content decoder */
  GstClockTime offset;          /* start time offset */
  gint starting;                /* true if media is starting */
  gint stopping;                /* true if media is stopping */
  gint drained;                 /* true if media has drained */
  gint active_pads;             /* number of active ghost pads in bin */
  char *final_uri;              /* final URI */
  struct
  {
    GstElement *volume;         /* audio volume */
    GstElement *convert;        /* audio convert */
    GstElement *resample;       /* audio resample */
    GstElement *filter;         /* audio filter */
    char *mixerpad;             /* name of audio sink pad in mixer */
  } audio;
  struct
  {
    GstElement *filter;         /* video filter */
    char *mixerpad;             /* name of video sink pad in mixer */
  } video;
  struct
  {
    lp_Scene *scene;            /* parent scene */
    char *uri;                  /* content URI */
    int x;                      /* cached x */
    int y;                      /* cached y */
    int z;                      /* cached z */
    int width;                  /* cached width */
    int height;                 /* cached height */
    double alpha;               /* cached alpha */
    gboolean mute;              /* cached mute */
    double volume;              /* cached volume */
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

/* Default values for media properties.  */
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
G_DEFINE_TYPE (lp_Media, lp_media, G_TYPE_OBJECT)


/* callbacks */

/* Called asynchronously whenever media decoder drains its source.  */

static void
lp_media_drained_callback (arg_unused (GstElement *decoder),
                           lp_Media *media)
{
  if (unlikely (_lp_media_has_drained (media)))
    return;                     /* nothing to do */

  g_atomic_int_inc (&media->drained);

  /* FIXME: We shouldn't call lp_media_stop() here, but postponing it to the
     bus callback does not work: the probe callback never gets called.  */
  lp_media_stop (media);
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
  const char *name;

  assert (_lp_media_is_starting (media));
  assert (!_lp_media_is_stopping (media));

  scene = media->prop.scene;
  assert (scene != NULL);

  caps = gst_pad_query_caps (pad, NULL);
  assert (caps != NULL);

  name = gst_structure_get_name (gst_caps_get_structure (caps, 0));
  assert (name != NULL);
  gst_caps_unref (caps);

  /* WARNING: This seems to be the "safest" place to set the offset.
     Previously we were setting it in the bus callback, but that lead to
     errors, e.g., we were unable to re-start drained objects.  */
  gst_pad_set_offset (pad, (gint64) media->offset);

  if (streq (name, "video/x-raw") && _lp_scene_has_video (scene))
    {
      GstElement *mixer;
      GstPad *sinkpad;
      GstPad *ghostpad;

      /* TODO: Handle multiple video streams.  */
      if (unlikely (media->video.mixerpad != NULL))
        {
          _lp_warn ("ignoring extra video stream");
          return;
        }

      _lp_eltmap_alloc_check (media, media_eltmap_video);
      caps = gst_caps_new_simple ("video/x-raw",
                                  "pixel-aspect-ratio",
                                  GST_TYPE_FRACTION, 1, 1, NULL);
      assert (caps != NULL);

      g_object_set (media->video.filter, "caps", caps, NULL);
      gst_caps_unref (caps);

      assert (gst_bin_add (GST_BIN (media->bin), media->video.filter));

      sinkpad = gst_element_get_static_pad (media->video.filter, "sink");
      assert (sinkpad != NULL);
      assert (gst_pad_link (pad, sinkpad) == GST_PAD_LINK_OK);;
      gst_object_unref (sinkpad);

      pad = gst_element_get_static_pad (media->video.filter, "src");
      assert (pad != NULL);
      ghostpad = gst_ghost_pad_new (NULL, pad);
      assert (ghostpad != NULL);
      gst_object_unref (pad);
      assert (gst_pad_set_active (ghostpad, TRUE));
      assert (gst_element_add_pad (media->bin, ghostpad));

      mixer = _lp_scene_get_video_mixer (scene);
      assert (mixer != NULL);

      sinkpad = gst_element_get_request_pad (mixer, "sink_%u");
      assert (sinkpad != NULL);
      media->video.mixerpad = gst_pad_get_name (sinkpad);

      assert (gst_pad_link (ghostpad, sinkpad) == GST_PAD_LINK_OK);
      g_object_set
        (sinkpad,
         "xpos", media->prop.x,
         "ypos", media->prop.y,
         "zorder", media->prop.z,
         "alpha", media->prop.alpha, NULL);

      if (media->prop.width > 0)
        g_object_set (sinkpad, 
            "width", media->prop.width, NULL);
      
      if (media->prop.height > 0)
        g_object_set (sinkpad, 
            "height", media->prop.height, NULL);

      gstx_element_set_state_sync (media->video.filter, GST_STATE_PLAYING);
      gst_object_unref (sinkpad);
      g_atomic_int_inc (&media->active_pads);
    }
  else if (streq (name, "audio/x-raw"))
    {
      GstElement *mixer;
      GstPad *sinkpad;
      GstPad *ghostpad;

      /* TODO: Handle multiple audio streams.  */
      if (unlikely (media->audio.mixerpad != NULL))
        {
          _lp_warn ("ignoring extra audio stream");
          return;
        }

      _lp_eltmap_alloc_check (media, media_eltmap_audio);
      assert (gst_bin_add (GST_BIN (media->bin), media->audio.volume));
      assert (gst_bin_add (GST_BIN (media->bin), media->audio.convert));
      assert (gst_bin_add (GST_BIN (media->bin), media->audio.resample));
      assert (gst_element_link_many
              (media->audio.volume,
               media->audio.convert,
               media->audio.resample, NULL));

      sinkpad = gst_element_get_static_pad (media->audio.volume, "sink");
      assert (sinkpad != NULL);
      assert (gst_pad_link (pad, sinkpad) == GST_PAD_LINK_OK);
      gst_object_unref (sinkpad);

      pad = gst_element_get_static_pad (media->audio.resample, "src");
      assert (pad != NULL);
      ghostpad = gst_ghost_pad_new (NULL, pad);
      assert (ghostpad != NULL);
      gst_object_unref (pad);
      assert (gst_pad_set_active (ghostpad, TRUE));
      assert (gst_element_add_pad (media->bin, ghostpad));

      mixer = _lp_scene_get_audio_mixer (scene);
      assert (mixer != NULL);

      sinkpad = gst_element_get_request_pad (mixer, "sink_%u");
      assert (sinkpad != NULL);
      media->audio.mixerpad = gst_pad_get_name (sinkpad);

      assert (gst_pad_link (ghostpad, sinkpad) == GST_PAD_LINK_OK);
      g_object_set (sinkpad,
                    "mute", media->prop.mute,
                    "volume", media->prop.volume, NULL);

      gstx_element_set_state_sync (media->audio.volume, GST_STATE_PLAYING);
      gstx_element_set_state_sync (media->audio.convert, GST_STATE_PLAYING);
      gstx_element_set_state_sync (media->audio.resample,
                                   GST_STATE_PLAYING);
      gst_object_unref (sinkpad);
      g_atomic_int_inc (&media->active_pads);
    }
  else
    {
      _lp_warn ("unknown stream type: %s", name);
    }
}

/* Called whenever pad state matches BLOCK_DOWNSTREAM.
   This callback is triggered by lp_media_stop().  */

static GstPadProbeReturn
lp_media_pad_probe_callback (GstPad *pad,
                             arg_unused (GstPadProbeInfo *info),
                             lp_Media *media)
{
  GstPad *peer;

  assert (!_lp_media_is_starting (media));
  assert (_lp_media_is_stopping (media));

  peer = gst_pad_get_peer (pad);
  assert (peer != NULL);
  assert (gst_pad_send_event (peer, gst_event_new_eos ()));
  gst_object_unref (peer);
  assert (gst_pad_set_active (pad, FALSE));

  if (g_atomic_int_dec_and_test (&media->active_pads)) /* no more pads */
    {
      if (_lp_media_has_drained (media))
        _lp_scene_dispatch (media->prop.scene, G_OBJECT (media), LP_EOS);
      _lp_scene_dispatch (media->prop.scene, G_OBJECT (media), LP_STOP);
    }

  return GST_PAD_PROBE_REMOVE;
}


/* methods */

static void
lp_media_init (lp_Media *media)
{
  media->offset = GST_CLOCK_TIME_NONE;
  media->starting = 0;
  media->stopping = 0;
  media->drained = 0;
  media->active_pads = 0;
  media->final_uri = NULL;

  media->audio.mixerpad = NULL;
  media->video.mixerpad = NULL;

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
  switch (prop_id)
    {
    case PROP_SCENE:
      g_value_set_pointer (value, media->prop.scene);
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
}

static void
lp_media_set_property (GObject *object, guint prop_id,
                       const GValue *value, GParamSpec *pspec)
{
  lp_Media *media;

  media = LP_MEDIA (object);
  switch (prop_id)
    {
    case PROP_SCENE:
      media->prop.scene = LP_SCENE (g_value_get_pointer (value));
      break;
    case PROP_URI:
      g_free (media->prop.uri);
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

  if (!_lp_media_has_started (media))
    return;

  switch (prop_id)
    {
    case PROP_SCENE:
    case PROP_URI:
      break;
    case PROP_X:
    case PROP_Y:
    case PROP_Z:
    case PROP_ALPHA:
      {
        GstElement *mixer;
        GstPad *sink;

        if (!_lp_scene_has_video (media->prop.scene))
          break;                /* nothing to do */

        mixer = _lp_scene_get_video_mixer (media->prop.scene);
        assert (mixer != NULL);

        sink = gst_element_get_static_pad (mixer, media->video.mixerpad);
        assert (sink != NULL);
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
          case PROP_ALPHA:
            g_object_set (sink, "alpha", media->prop.alpha, NULL);
            break;
          default:
            ASSERT_NOT_REACHED;
          }
        g_object_unref (sink);
        break;
      }
    case PROP_WIDTH:
    case PROP_HEIGHT:
      {
        GstElement *mixer;
        GstPad *sinkpad;
        
        if (!_lp_scene_has_video (media->prop.scene))
          break;                /* nothing to do */
        
        mixer = _lp_scene_get_video_mixer (media->prop.scene);
        assert (mixer != NULL);

        sinkpad = gst_element_get_static_pad (mixer, media->audio.mixerpad);
        assert (sinkpad != NULL);

        g_object_set
          (sinkpad,
           "width", media->prop.width,
           "height", media->prop.height, NULL);

        g_object_unref (sinkpad);
        break;
      }
    case PROP_MUTE:
    case PROP_VOLUME:
      {
        GstElement *mixer;
        GstPad *sink;

        mixer = _lp_scene_get_audio_mixer (media->prop.scene);
        assert (mixer != NULL);

        sink = gst_element_get_static_pad (mixer, media->audio.mixerpad);
        assert (sink != NULL);
        switch (prop_id)
          {
          case PROP_MUTE:
            g_object_set (sink, "mute", media->prop.mute, NULL);
            break;
          case PROP_VOLUME:
            g_object_set (sink, "volume", media->prop.volume, NULL);
            break;
          default:
            ASSERT_NOT_REACHED;
          }
        g_object_unref (sink);
        break;
      }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_media_constructed (GObject *object)
{
  lp_Media *media;

  media = LP_MEDIA (object);

  if (unlikely (media->prop.scene == DEFAULT_SCENE))
    _lp_error ("empty parent scene: %p", media->prop.scene);

  /* FIXME: Users cannot unref media objects directly.  */

  _lp_scene_add_media (media->prop.scene, media);
  g_object_unref (media);
}

static void
lp_media_dispose (GObject *object)
{
  lp_Media *media;

  media = LP_MEDIA (object);
  _lp_debug ("disposing media %p", media);

  if (media->bin != NULL)
    g_object_set_data (G_OBJECT (media->bin), "lp_Media", NULL);

  while (!_lp_media_has_stopped (media))
    {
      lp_media_stop (media);
      _lp_scene_step (media->prop.scene, TRUE);
    }

  G_OBJECT_CLASS (lp_media_parent_class)->dispose (object);
}


static void
lp_media_finalize (GObject *object)
{
  lp_Media *media;

  media = LP_MEDIA (object);

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
    (gobject_class, PROP_SCENE, g_param_spec_pointer
     ("scene", "scene", "parent scene",
      (GParamFlags) (G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_URI, g_param_spec_string
     ("uri", "uri", "content URI",
      DEFAULT_URI,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_X, g_param_spec_int
     ("x", "x", "horizontal position",
      0, G_MAXINT, DEFAULT_X,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_Y, g_param_spec_int
     ("y", "y", "vertical position",
      0, G_MAXINT, DEFAULT_Y,
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

/* Returns true if @media is starting.  */

gboolean
_lp_media_is_starting (lp_Media *media)
{
  return g_atomic_int_get (&media->starting) != 0;
}

/* Returns true if @media has started.  */

gboolean
_lp_media_has_started (lp_Media *media)
{
  if (unlikely (_lp_media_is_starting (media)))
    return FALSE;

  if (unlikely (_lp_media_is_stopping (media)))
    return FALSE;

  return media->bin != NULL;
}

/* Returns true if @media is stopping.  */

gboolean
_lp_media_is_stopping (lp_Media *media)
{
  return g_atomic_int_get (&media->stopping) != 0;
}

/* Returns true if @media has started.  */

gboolean
_lp_media_has_stopped (lp_Media *media)
{
  if (unlikely (_lp_media_is_starting (media)))
    return FALSE;

  if (unlikely (_lp_media_is_stopping (media)))
    return FALSE;

  return media->bin == NULL;
}

/* Returns true if @media has drained.  */

gboolean
_lp_media_has_drained (lp_Media *media)
{
  return g_atomic_int_get (&media->drained) != 0;
}

/* Finishes the start request of @media.  */

void
_lp_media_finish_start (lp_Media *media)
{
  assert (g_atomic_int_dec_and_test (&media->starting));
}

/* Finishes the stop request of @media.  */

void
_lp_media_finish_stop (lp_Media *media)
{
  GstElement *pipeline;

  assert (g_atomic_int_get (&media->active_pads) == 0);

  gstx_element_set_state_sync (media->bin, GST_STATE_NULL);
  pipeline = _lp_scene_get_pipeline (media->prop.scene);
  assert (gst_bin_remove (GST_BIN (pipeline), media->bin));
  gst_object_unref (media->bin);

  media->bin = NULL;            /* indicates has_stopped */
  media->drained = 0;           /* indicates has_drained */
  g_clear_pointer (&media->final_uri, g_free);
  g_clear_pointer (&media->audio.mixerpad, g_free);
  g_clear_pointer (&media->video.mixerpad, g_free);

  assert (g_atomic_int_dec_and_test (&media->stopping));
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
lp_media_new (lp_Scene *scene, const char *uri)
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

  if (unlikely (!_lp_media_has_stopped (media)))
    return FALSE;

  if (gst_uri_is_valid (media->prop.uri))
    {
      media->final_uri = g_strdup (media->prop.uri);
    }
  else
    {
      GError *err = NULL;
      char *final_uri = gst_filename_to_uri (media->prop.uri, &err);
      if (unlikely (final_uri == NULL))
        {
          _lp_warn ("bad URI: %s", media->prop.uri);
          g_error_free (err);
          return FALSE;
        }
      media->final_uri = final_uri;
    }

  _lp_eltmap_alloc_check (media, lp_media_eltmap);
  g_object_set_data (G_OBJECT (media->bin), "lp_Media", media);
  g_object_set (media->decoder, "uri", media->final_uri, NULL);
  gst_bin_add (GST_BIN (media->bin), media->decoder);

  assert (g_signal_connect (media->decoder, "drained",
                            G_CALLBACK (lp_media_drained_callback),
                            media) > 0);
  assert (g_signal_connect (media->decoder, "pad-added",
                            G_CALLBACK (lp_media_pad_added_callback),
                            media) > 0);

  assert (gst_object_ref (media->bin) == media->bin);
  pipeline = _lp_scene_get_pipeline (media->prop.scene);
  assert (gst_bin_add (GST_BIN (pipeline), media->bin));

  g_atomic_int_inc (&media->starting);
  media->offset = gstx_element_get_clock_time (pipeline);

  ret = gst_element_set_state (media->bin, GST_STATE_PLAYING);
  return ret != GST_STATE_CHANGE_FAILURE;
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

  if (unlikely (!_lp_media_has_started (media)))
    return FALSE;

  g_atomic_int_inc (&media->stopping);

  it = gst_element_iterate_src_pads (media->bin);
  assert (it != NULL);

  done = FALSE;
  do
    {
      switch (gst_iterator_next (it, &item))
        {
        case GST_ITERATOR_OK:
        {
          GstPad *pad = GST_PAD (g_value_get_object (&item));
          assert (gst_pad_add_probe
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
        ASSERT_NOT_REACHED;
        break;
      }
    }
  while (!done);

  g_value_unset (&item);
  gst_iterator_free (it);

  return TRUE;
}
