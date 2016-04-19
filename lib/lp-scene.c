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
#include "play-internal.h"


#define DEFAULT_WIDTH 0
#define DEFAULT_HEIGHT 0

/* scene properties  */
enum
{
  PROP_WIDTH = 1,
  PROP_HEIGHT,
  N_PROPERTIES
};

/* Forward declarations:  */
/* *INDENT-OFF* */
static void __lp_scene_class_init (lp_scene_tClass *);
static void __lp_scene_constructed (GObject *);
static void __lp_scene_init (lp_scene_t *);
static void __lp_scene_set_property (GObject *, guint, 
    const GValue *, GParamSpec *);
static void __lp_scene_get_property (GObject *, guint, 
    GValue *, GParamSpec *);
/* *INDENT-ON* */

/* Scene object data. */
struct _lp_scene_t
{
  GObject parent_instance;            /* parent struct */
  GList *children;                    /* children list */
  lp_scene_event_func_t handler;      /* event-handler */
  
  int width;                          /* scene width */
  int height;                         /* scene height */

  lp_scene_backend_t *backend;        /* scene backend */
};


G_DEFINE_TYPE (lp_scene_t, __lp_scene, G_TYPE_OBJECT)


static void
__lp_scene_class_init (lp_scene_tClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = __lp_scene_set_property;
  object_class->get_property = __lp_scene_get_property;
  object_class->constructed = __lp_scene_constructed;

  g_object_class_install_property (object_class, PROP_WIDTH,
      g_param_spec_int ("width", "window width", "Defines the width of the "
        "scene window", 0, G_MAXINT, DEFAULT_WIDTH, G_PARAM_CONSTRUCT_ONLY | 
        G_PARAM_READWRITE));
  
  g_object_class_install_property (object_class, PROP_HEIGHT,
      g_param_spec_int ("height", "window height", "Defines the height of the "
        "scene window", 0, G_MAXINT, DEFAULT_HEIGHT, G_PARAM_CONSTRUCT_ONLY | 
        G_PARAM_READWRITE));
}

static void
__lp_scene_init (lp_scene_t *self)
{
  self->width = DEFAULT_WIDTH;
  self->height = DEFAULT_HEIGHT;
}

static void
__lp_scene_constructed (GObject *object)
{
  lp_scene_t *scene = LP_SCENE (object);

  scene->backend = g_new0 (lp_scene_backend_t, 1);
  _lp_assert (scene->backend);

  scene->backend->scene = scene;
  _lp_scene_gst_init (scene->backend);

  if (scene->backend->start_async != NULL)
    scene->backend->start_async(scene);

  G_OBJECT_CLASS (__lp_scene_parent_class)->constructed (object);
}

static void 
__lp_scene_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *spec)
{
  lp_scene_t *scene = LP_SCENE (object);

  switch (prop_id) 
  {
    case PROP_WIDTH:
    {
      scene->width = g_value_get_int (value);
      break;
    }
    case PROP_HEIGHT:
    {
      scene->height = g_value_get_int (value);
      break;
    }
    case N_PROPERTIES:
    default:
      _LP_ASSERT_NOT_REACHED;
  }
}

static void 
__lp_scene_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *spec)
{
  lp_scene_t *scene = LP_SCENE (object);

  switch (prop_id) {
    case PROP_WIDTH:
      g_value_set_int (value, scene->width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, scene->height);
      break;
    case N_PROPERTIES:
    default:
      _LP_ASSERT_NOT_REACHED;
  }
}

GType lp_scene_get_type ()
{
  return __lp_scene_get_type ();
}

lp_scene_backend_t *
_lp_scene_get_backend (lp_scene_t *scene)
{
  _lp_assert (scene != NULL);
  return scene->backend;
}
 
unsigned int
_lp_scene_dispatch (lp_scene_t *scene, lp_event_t *evt)
{
  (void) scene;
  (void) evt;
  /* TODO: all */
  return 0;
}
