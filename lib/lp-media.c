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
#include "gstx-macros.h"
  
/* Forward declarations:  */
/* *INDENT-OFF* */
static void __lp_media_constructed (GObject *);
static void __lp_media_get_property (GObject *, guint, GValue *, GParamSpec *);
static void __lp_media_set_property (GObject *, guint, const GValue *, 
    GParamSpec *);
/* *INDENT-ON* */

/* Media object. */
struct _lp_Media
{
  GObject parent;               /* parent object */
  GstElement *decoder;          /* media decoder */
  struct
  {
    GstElement *volume;         /* audio volume  */
    GstElement *convert;        /* audio converter */
    GstElement *resample;       /* audio resampler */
    GstElement *filter;         /* audio filter */
  } audio;
  struct
  {
    GstElement *scale;          /* video scaler */
    GstElement *filter;         /* video filter */
  } video;
  struct
  {
    lp_Scene *scene;            /* parent scene */
    char *uri;                  /* content URI */
    int width;                  /* cached width */
    int height;                 /* cached height */
    int zorder;                 /* cached zorder */
    double alpha;               /* cached alpha */
    double volume;              /* cached volume */
    /* what else? */
  } prop;
};


/* Media properties. */
enum
{
  PROP_SCENE = 1,
  PROP_URI,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_ZORDER,
  PROP_ALPHA,
  PROP_VOLUME,
  N_PROPERTIES
};

G_DEFINE_TYPE (lp_Media, __lp_media, G_TYPE_OBJECT)

static void
__lp_media_get_property (GObject *object, guint prop_id,
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
    case PROP_WIDTH:
      g_value_set_int (value, media->prop.width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, media->prop.height);
      break;
    case PROP_ZORDER:
      g_value_set_int (value, media->prop.zorder);
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
__lp_media_set_property (GObject *object, guint prop_id,
                       const GValue *value, GParamSpec *pspec)
{
  lp_Media *media = LP_MEDIA (object);

  switch (prop_id)
  {
    case PROP_SCENE:
      media->prop.scene = LP_SCENE (g_value_get_pointer (value));
      break;
    case PROP_URI:
      media->prop.uri = g_strdup (g_value_get_string (value));
      break;
    case PROP_WIDTH:
      media->prop.width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      media->prop.height = g_value_get_int (value);
      break;
    case PROP_ZORDER:
      media->prop.zorder = g_value_get_int (value);
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

GType lp_media_get_type ()
{
  return __lp_media_get_type ();
}

/* Default values for scene properties.  */
#define DEFAULT_WIDTH     0     /* pixels */
#define DEFAULT_HEIGHT    0     /* pixels */
#define DEFAULT_ZORDER    0     /* order */
#define DEFAULT_ALPHA    1.0    /* opaque */
#define DEFAULT_VOLUME   1.0    /* highest */

static void
__lp_media_init (lp_Media *media)
{
  media->prop.scene = NULL;
  media->prop.uri = NULL;
  media->prop.width = DEFAULT_WIDTH;
  media->prop.height = DEFAULT_HEIGHT;
  media->prop.zorder = DEFAULT_ZORDER;
  media->prop.alpha = DEFAULT_ALPHA;
  media->prop.volume = DEFAULT_VOLUME;
}

static void
__lp_media_class_init (lp_MediaClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->get_property = __lp_media_get_property;
  gobject_class->set_property = __lp_media_set_property;
  gobject_class->constructed = __lp_media_constructed;
  
  g_object_class_install_property
    (gobject_class, PROP_SCENE, g_param_spec_pointer 
     ("scene", "scene", "media scene parent", 
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  
  g_object_class_install_property
    (gobject_class, PROP_URI, g_param_spec_string 
     ("uri", "URI", "media URI", NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_WIDTH, g_param_spec_int
     ("width", "width", "media width in pixels",
      0, G_MAXINT, DEFAULT_WIDTH, G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_HEIGHT, g_param_spec_int
     ("height", "height", "media height in pixels",
      0, G_MAXINT, DEFAULT_HEIGHT, G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_ZORDER, g_param_spec_int
     ("zorder", "zorder", "media zorder priority",
      0, 10000, DEFAULT_ZORDER, G_PARAM_READWRITE));

  g_object_class_install_property
    (gobject_class, PROP_ALPHA, g_param_spec_double
     ("alpha", "alpha", "media alpha channel",
      0, 1.0, DEFAULT_ALPHA, G_PARAM_READWRITE));
  
  g_object_class_install_property
    (gobject_class, PROP_VOLUME, g_param_spec_double
     ("volume", "volume", "media volume",
      0, 1.0, DEFAULT_VOLUME, G_PARAM_READWRITE));
}

static void
__lp_media_constructed (GObject *object)
{
  lp_Media *media = LP_MEDIA (object);

  if (media->prop.uri != NULL)
  {
    assert ((media->decoder = gst_element_factory_make ("uridecodebin", NULL),
          media->decoder) != NULL);
    g_object_set (G_OBJECT (media->decoder), "uri", media->prop.uri, NULL);
  }
  else /* if there is no URI the media becomes a silent audio */
  {
    assert ((media->decoder = gst_element_factory_make ("audiotestsrc", NULL),
          media->decoder) != NULL);
    g_object_set (G_OBJECT (media->decoder), "wave", 4 /* silence */, NULL);
  }
}
