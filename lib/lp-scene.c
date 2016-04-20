/* lp-scene.c -- Scene object.
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
#include "gstx-macros.h"

/* Scene object. */
struct _lp_Scene
{
  GObject parent;               /* parent object */
  GstElement *pipeline;         /* scene pipeline */
  GstClockID clock_id;          /* last clock id */
  gint quitted;                 /* true if scene has quitted */
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
  } prop;
};

/* Maps GStreamer elements to for lp_Scene.  */
static const gstx_eltmap_t lp_scene_eltmap[] = {
  {"pipeline",      offsetof (lp_Scene, pipeline)},
  {"audiotestsrc",  offsetof (lp_Scene, audio.blank)},
  {"audiomixer",    offsetof (lp_Scene, audio.mixer)},
  {"autoaudiosink", offsetof (lp_Scene, audio.sink)},
  {"videotestsrc",  offsetof (lp_Scene, video.blank)},
  {"capsfilter",    offsetof (lp_Scene, video.filter)},
  {"compositor",    offsetof (lp_Scene, video.mixer)},
  {"glimagesink",   offsetof (lp_Scene, video.sink)},
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
  PROP_LAST
};

/* Default values for scene properties.  */
#define DEFAULT_WIDTH    0      /* pixels */
#define DEFAULT_HEIGHT   0      /* pixels */
#define DEFAULT_PATTERN  2      /* black */
#define DEFAULT_WAVE     4      /* silence */

G_DEFINE_TYPE (lp_Scene, lp_scene, G_TYPE_OBJECT)

static void
lp_scene_init (lp_Scene *scene)
{
  scene->clock_id = NULL;
  scene->quitted = 0;
  scene->prop.width = DEFAULT_WIDTH;
  scene->prop.height = DEFAULT_HEIGHT;
  scene->prop.pattern = DEFAULT_PATTERN;
  scene->prop.wave = DEFAULT_WAVE;

  assert (gstx_eltmap_alloc (scene, lp_scene_eltmap, NULL));
  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->audio.blank));
  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->audio.mixer));
  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->audio.sink));
  assert (gst_element_link (scene->audio.blank, scene->audio.mixer));
  assert (gst_element_link (scene->audio.mixer, scene->audio.sink));
  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->video.blank));
  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->video.filter));
  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->video.mixer));
  assert (gst_bin_add (GST_BIN (scene->pipeline), scene->video.sink));
  assert (gst_element_link (scene->video.blank, scene->video.filter));
  assert (gst_element_link (scene->video.filter, scene->video.mixer));
  assert (gst_element_link (scene->video.mixer, scene->video.sink));
  gstx_element_set_state_sync (scene->pipeline, GST_STATE_PLAYING);
}

static void
lp_scene_get_property (GObject *object, guint prop_id,
                       GValue *value, GParamSpec *pspec)
{
  lp_Scene *scene = LP_SCENE (object);

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
      break;
    case PROP_WAVE:
      scene->prop.wave = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
lp_scene_class_init (lp_SceneClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->get_property = lp_scene_get_property;
  gobject_class->set_property = lp_scene_set_property;

  g_object_class_install_property
    (gobject_class, PROP_WIDTH, g_param_spec_int
     ("width", "width", "scene width in pixels",
      0, G_MAXINT, DEFAULT_WIDTH,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_HEIGHT, g_param_spec_int
     ("height", "height", "scene height in pixels",
      0, G_MAXINT, DEFAULT_HEIGHT,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_PATTERN, g_param_spec_int
     ("pattern", "pattern", "scene background pattern",
      0, 24, DEFAULT_PATTERN,
      G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_WAVE, g_param_spec_int
     ("wave", "wave", "scene wave pattern",
      0, 12, DEFAULT_WAVE,
      G_PARAM_READWRITE));

  if (!gst_is_initialized ())
    assert (gst_init_check (NULL, NULL, NULL));
}
