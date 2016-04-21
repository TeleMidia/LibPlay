/* lp-media.c -- Media object.
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

/* Media object. */
struct _lp_Media
{
  GObject parent;               /* parent object */
  GstElement *bin;              /* container */
  GstElement *decoder;          /* content decoder */
  GstClockTime offset;          /* start time offset */
  gint playing;                 /* true if media is playing */
  gint stopping;                /* true if media is stopping */
  gint active_pads;             /* number of active ghost pads in bin */
  char *abs_uri;                /* absolute URI */
  struct
  {
    GstElement *volume;         /* audio volume */
    GstElement *convert;        /* audio convert */
    GstElement *resample;       /* audio resample */
    GstElement *filter;         /* audio filter */
    char *mixerpad;             /* name of sink pad in mixer */
  } audio;
  struct
  {
    GstElement *scale;          /* video scale */
    GstElement *filter;         /* video filter */
    char *mixerpad;             /* name of sink pad in mixer */
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
  PROP_VOLUME,
  PROP_LAST
};

/* Default values for media properties.  */
#define DEFAULT_X         0     /* leftmost horizontal position */
#define DEFAULT_Y         0     /* topmost vertical position */
#define DEFAULT_Z         1     /* lowest order */
#define DEFAULT_WIDTH     0     /* natural width */
#define DEFAULT_HEIGHT    0     /* natural height */
#define DEFAULT_ALPHA   1.0     /* natural alpha */
#define DEFAULT_VOLUME  1.0     /* natural volume */

/* Define the lp_Media type.  */
G_DEFINE_TYPE (lp_Media, lp_media, G_TYPE_OBJECT)


/* callbacks */

/* Called asynchronously whenever a media decoder creates a pad.
   Builds and starts the media processing graph.  */

static void
lp_media_pad_added_callback (GstElement *decoder,
                          GstPad *pad,
                          lp_Media *media)
{
  lp_Scene *scene;
  GstCaps *caps;
  const char *name;

  GstElement *mixer;
  GstPad *sinkpad;
  GstPad *ghostpad;

  assert (g_atomic_int_get (&media->playing));
  assert (!g_atomic_int_get (&media->stopping));

  scene = media->prop.scene;
  _lp_scene_check (scene);

  caps = gst_pad_query_caps (pad, NULL);
  assert (caps != NULL);

  name = gst_structure_get_name (gst_caps_get_structure (caps, 0));
  assert (name != NULL);
  gst_caps_unref (caps);

  if (g_atomic_int_get (&media->active_pads) == 0)
    {
      /* TODO: Is this the right way to get the offset? */
      GstElement *pipeline = _lp_scene_get_pipeline (scene);
      media->offset = gstx_element_get_clock_time (pipeline);
    }
  gst_pad_set_offset (pad, media->offset);

  if (streq (name, "video/x-raw") && _lp_scene_has_video (scene))
    {
      mixer = _lp_scene_get_video_mixer (scene);

      /* FIXME: Handle errors and multiple video streams.  */
      assert (media->video.mixerpad == NULL);
      _lp_eltmap_alloc_check (media, media_eltmap_video);

      caps = gst_caps_new_simple
        ("video/x-raw",
         "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, NULL);
      assert (caps != NULL);

      if (media->prop.width > 0)
        gst_caps_set_simple (caps, "width", G_TYPE_INT,
                             media->prop.width, NULL);

      if (media->prop.height > 0)
        gst_caps_set_simple (caps, "height", G_TYPE_INT,
                             media->prop.height, NULL);

      g_object_set (media->video.scale, "add-borders", 0, NULL);
      g_object_set (media->video.filter, "caps", caps, NULL);
      gst_caps_unref (caps);

      assert (gst_bin_add (GST_BIN (media->bin), media->video.scale));
      assert (gst_bin_add (GST_BIN (media->bin), media->video.filter));
      assert (gst_element_link (media->video.scale, media->video.filter));

      sinkpad = gst_element_get_static_pad (media->video.scale, "sink");
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

      sinkpad = gst_element_get_request_pad (mixer, "sink_%u");
      assert (sinkpad != NULL);
      media->video.mixerpad = gst_pad_get_name (sinkpad);

      assert (gst_pad_link (ghostpad, sinkpad) == GST_PAD_LINK_OK);
      g_object_set
        (sinkpad,
         "xpos", media->prop.x,
         "ypos", media->prop.y,
         "zorder", media->prop.z,
         "alpha", media->prop.alpha,
         NULL);
      gstx_element_set_state_sync (media->video.filter, GST_STATE_PLAYING);
      gstx_element_set_state_sync (media->video.scale, GST_STATE_PLAYING);
      gst_object_unref (sinkpad);
      g_atomic_int_inc (&media->active_pads);
    }
  else if (streq (name, "audio/x-raw"))
    {
      mixer = _lp_scene_get_audio_mixer (scene);

      /* FIXME: Handle errors and multiple audio streams.  */
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

      sinkpad = gst_element_get_request_pad (mixer, "sink_%u");
      assert (sinkpad != NULL);
      media->audio.mixerpad = gst_pad_get_name (sinkpad);

      assert (gst_pad_link (ghostpad, sinkpad) == GST_PAD_LINK_OK);
      gstx_element_set_state_sync (media->audio.volume, GST_STATE_PLAYING);
      gstx_element_set_state_sync (media->audio.convert, GST_STATE_PLAYING);
      gstx_element_set_state_sync (media->audio.resample,
                                   GST_STATE_PLAYING);
      gst_object_unref (sinkpad);
      g_atomic_int_inc (&media->active_pads);
    }
  else
    {
      return;                   /* nothing to do */
    }
}


/* private */

static void
lp_media_init (lp_Media *media)
{
  media->offset = GST_CLOCK_TIME_NONE;
  media->playing = 0;
  media->stopping = 0;
  media->active_pads = 0;
  media->abs_uri = NULL;

  media->audio.mixerpad = NULL;
  media->video.mixerpad = NULL;

  media->prop.scene = NULL;
  media->prop.uri = NULL;
  media->prop.x = DEFAULT_X;
  media->prop.y = DEFAULT_Y;
  media->prop.z = DEFAULT_Z;
  media->prop.width = DEFAULT_WIDTH;
  media->prop.height = DEFAULT_HEIGHT;
  media->prop.alpha = DEFAULT_ALPHA;
  media->prop.volume = DEFAULT_VOLUME;
}

static void
lp_media_get_property (GObject *object, guint prop_id,
                       GValue *value, GParamSpec *pspec)
{
  lp_Media *media = LP_MEDIA (object);

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
  lp_Media *media = LP_MEDIA (object);

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
      media->prop.y = g_value_get_int (value);
      break;
    case PROP_Y:
      media->prop.y = g_value_get_int (value);
      break;
    case PROP_Z:
      media->prop.x = g_value_get_int (value);
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
    case PROP_VOLUME:
      media->prop.volume = g_value_get_double (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_media_constructed (GObject *object)
{
  lp_Media *media = LP_MEDIA (object);

  _lp_scene_check (media->prop.scene);
  _lp_scene_add (media->prop.scene, media);

  /* FIXME: Users must not unref medias directly
     (i.e., without ref them first).  */

  g_object_unref (media);
}

static void
lp_media_finalize (GObject *object)
{
  lp_Media *media = LP_MEDIA (object);

  _lp_debug ("finalizing media %p", media);

  G_OBJECT_CLASS (lp_media_parent_class)->finalize (object);
}

static void
lp_media_class_init (lp_MediaClass *cls)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (cls);

  gobject_class->get_property = lp_media_get_property;
  gobject_class->set_property = lp_media_set_property;
  gobject_class->constructed = lp_media_constructed;
  gobject_class->finalize = lp_media_finalize;

  g_object_class_install_property
    (gobject_class, PROP_SCENE, g_param_spec_pointer
     ("scene", "scene", "parent scene",
      G_PARAM_CONSTRUCT_ONLY
      | G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_URI, g_param_spec_string
     ("uri", "uri", "content URI",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

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
    (gobject_class, PROP_VOLUME, g_param_spec_double
     ("volume", "volume", "volume factor",
      0, 1.0, DEFAULT_VOLUME,
      G_PARAM_READWRITE));
}


/* public */

/**
 * lp_scene_new:
 * @width: scene width
 * @height: scene height
 *
 * Allocates and returns a new #lp_Scene with the given dimensions.
 */
lp_Media *
lp_media_new (lp_Scene *scene, const char *uri)
{
  return g_object_new (LP_TYPE_MEDIA,
                       "scene", scene,
                       "uri", uri, NULL);
}

/**
 * lp_media_start:
 * @media: an #lp_Media
 *
 * Starts media.
 *
 * Returns: true if successful, or false otherwise.
 */
gboolean
lp_media_start (lp_Media *media)
{
  _lp_scene_check (media->prop.scene);

  if (unlikely (g_atomic_int_get (&media->playing)
                || g_atomic_int_get (&media->stopping)))
    {
      return FALSE;             /* nothing to do */
    }

  if (gst_uri_is_valid (media->prop.uri))
    {
      media->abs_uri = g_strdup (media->prop.uri);
    }
  else
    {
      GError *err = NULL;
      char *abs_uri = gst_filename_to_uri (media->prop.uri, &err);
      if (unlikely (abs_uri == NULL))
        {
          g_warning (G_STRLOC "bad uri: %s", media->prop.uri);
          g_error_free (err);
          return FALSE;
        }
      g_free (media->abs_uri);
      media->abs_uri = abs_uri;
    }

  _lp_eltmap_alloc_check (media, lp_media_eltmap);

  g_object_set (media->decoder, "uri", media->abs_uri, NULL);
  gst_bin_add (GST_BIN (media->bin), media->decoder);
  assert (g_signal_connect
          (media->decoder, "pad-added",
           G_CALLBACK (lp_media_pad_added_callback), media) > 0);

  assert (gst_object_ref (media->bin) == media->bin);
  assert (gst_bin_add (GST_BIN (_lp_scene_get_pipeline (media->prop.scene)),
                       media->bin));

  g_atomic_int_inc (&media->playing);
  gstx_element_set_state_sync (media->decoder, GST_STATE_PLAYING);

  return TRUE;
}

/**
 * lp_media_stop:
 * @media: an #lp_Media
 *
 * Stops media.
 *
 * Returns: true if successful, or false otherwise.
 */
gboolean
lp_media_stop (lp_Media *media)
{
  return FALSE;
}
