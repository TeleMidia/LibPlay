/* play0.c -- Lua bindings for LibPlay.
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

#include <lua.h>
#include <lauxlib.h>

#include "macros.h"
#include "gx-macros.h"
#include "luax-macros.h"

GX_INCLUDE_PROLOGUE
#include "play.h"
GX_INCLUDE_EPILOGUE

/* Scene object.  */
typedef struct Scene
{
  lp_Scene *scene;
} Scene;

/* Registry key for scene metatable.  */
#define SCENE "play.scene"

/* Forward declarations.  */
static int l_scene_new (lua_State *);
static int __l_scene_gc (lua_State *);
static int __l_scene_tostring (lua_State *);
static int l_scene_get (lua_State *);
static int l_scene_set (lua_State *);
static int l_scene_receive (lua_State *);
int luaopen_play_play0 (lua_State *);

/* Scene object metamethods.  */
static const struct luaL_Reg scene_funcs[] = {
  {"new", l_scene_new},
  {"__gc", __l_scene_gc},
  {"__tostring", __l_scene_tostring},
  {"get", l_scene_get},
  {"set", l_scene_set},
  {"receive", l_scene_receive},
  {NULL, NULL},
};


PRAGMA_DIAG_IGNORE (-Wunused-macros)
#if defined DEBUG && DEBUG
# define debug(fmt, ...)                                \
  g_print (G_STRLOC " [thread %p] " fmt "\n",           \
           (void *) g_thread_self (), ## __VA_ARGS__)
#else
# define debug(fmt, ...)        /* nothing */
#endif

/* Throws a run-time error.  */
#define error_throw(L, msg)\
  (lua_pushstring ((L), (msg)), lua_error ((L)))

/* Throws a run-time error (formatted string version).  */
#define error_throw_format(L, fmt, ...)\
  (lua_pushfstring ((L), fmt, ## __VA_ARGS__), lua_error ((L)))

/* Throws an "unknown property" error.  */
#define error_throw_property_unknown(L, obj, prop)      \
  error_throw_format ((L), "unknown %s property '%s'",  \
                      G_OBJECT_TYPE_NAME (obj), prop)

/* Throws a "non-writable property" error.  */
#define error_throw_property_not_writable(L, obj, prop)         \
  error_throw_format ((L), "non-writable %s property '%s'",     \
                      G_OBJECT_TYPE_NAME (obj), prop)


/* Pushes a table corresponding to @event onto @L.
   This function pushes nil if @event is NULL.  */

static void
play_event_push (lua_State *L, lp_Event *event)
{
  GObject *src;
  lp_EventMask mask;

  if (event == NULL)            /* no event, nothing to do */
    {
      lua_pushnil (L);
      return;
    }

  lua_newtable (L);
  src = lp_event_get_source (event);
  g_assert_nonnull (src);

  mask = lp_event_get_mask (event);
  switch (mask)
    {
    case LP_EVENT_MASK_TICK:          /* fall through */
    case LP_EVENT_MASK_KEY:           /* fall through */
    case LP_EVENT_MASK_POINTER_CLICK: /* fall through */
    case LP_EVENT_MASK_POINTER_MOVE:
      {
        lp_Scene *scene;
        Scene *sw;

        sw = (Scene *) g_object_get_data (src, SCENE);
        g_assert_nonnull (sw);

        scene = LP_SCENE (src);
        g_assert (sw->scene == scene);

        /* TODO: Set "source" field.  */

        switch (mask)
          {
          case LP_EVENT_MASK_TICK:
            {
              guint64 serial;

              luax_setstringfield (L, -1, "type", "tick");
              g_object_get (event, "serial", &serial, NULL);
              luax_setintegerfield (L, -1, "serial", serial);
              break;
            }
          case LP_EVENT_MASK_KEY:
            {
              gchar *key;
              gboolean press;

              luax_setstringfield (L, -1, "type", "key");
              g_object_get (event, "key", &key, "press", &press, NULL);
              luax_setstringfield (L, -1, "key", key);
              luax_setbooleanfield (L, -1, "press", press);
              g_free (key);
              break;
            }
          case LP_EVENT_MASK_POINTER_CLICK:
            {
              gdouble x;
              gdouble y;
              gint button;
              gboolean press;

              luax_setstringfield (L, -1, "type", "pointer-click");
              g_object_get (event, "x", &x, "y", &y,
                            "button", &button, "press", &press, NULL);
              luax_setnumberfield (L, -1, "x", x);
              luax_setnumberfield (L, -1, "y", y);
              luax_setintegerfield (L, -1, "button", button);
              luax_setbooleanfield (L, -1, "press", press);
              break;
            }
          case LP_EVENT_MASK_POINTER_MOVE:
            {
              gdouble x;
              gdouble y;

              luax_setstringfield (L, -1, "type", "pointer-move");
              g_object_get (event, "x", &x, "y", &y, NULL);
              luax_setnumberfield (L, -1, "x", x);
              luax_setnumberfield (L, -1, "y", y);
              break;
            }
          default:
            g_assert_not_reached ();
          }

        break;
      }
    case LP_EVENT_MASK_ERROR:   /* fall through */
    case LP_EVENT_MASK_START:   /* fall through */
    case LP_EVENT_MASK_STOP:    /* fall through */
    case LP_EVENT_MASK_SEEK:
      {
        error_throw (L, "not implemented");
        break;
      }
    default:
      g_assert_not_reached ();
    }
}

/* Checks if the object at @index is a scene. If successful, returns the
   scene and stores the wrapped lp_Scene into @scene.  Otherwise, throws an
   error.  */

static Scene *
scene_check (lua_State *L, int index, lp_Scene **scene)
{
  Scene *sw;

  sw = (Scene *) luaL_checkudata (L, index, SCENE);
  g_assert_nonnull (sw);
  g_assert_nonnull (sw->scene);
  set_if_nonnull (scene, sw->scene);

  return sw;
}


/*-
 * scene.new ([width:number, height:number])
 * scene:new ()
 *      -> scene:Scene; or
 *      -> nil, errmsg:string
 *
 * Creates a new scene with the given dimensions.
 *
 * Returns the new scene if successful, or nil plus error message.
 */
static int
l_scene_new (lua_State *L)
{
  Scene *sw;
  int width;
  int height;

  luax_optudata (L, 1, SCENE);
  width = (int) clamp (luaL_optinteger (L, 2, 0), G_MININT, G_MAXINT);
  height = (int) clamp (luaL_optinteger (L, 3, 0), G_MININT, G_MAXINT);

  sw = (Scene *) lua_newuserdata (L, sizeof (*sw));
  g_assert_nonnull (sw);

  sw->scene = lp_scene_new (width, height);
  g_assert_nonnull (sw->scene);

  g_object_set_data (G_OBJECT (sw->scene), SCENE, sw);
  luaL_setmetatable (L, SCENE);

  return 1;
}

/*-
 * scene:__gc ()
 *
 * Destroys scene.
 */
static int
__l_scene_gc (lua_State *L)
{
  lp_Scene *scene;

  scene_check (L, 1, &scene);
  g_object_unref (scene);

  return 0;
}

/*-
 * scene:__tostring ()
 *      -> s:string
 *
 * Dumps scene to a string and returns it.
 */
static int
__l_scene_tostring (lua_State *L)
{
  lp_Scene *scene;
  char *str;

  scene_check (L, 1, &scene);
  str = lp_scene_to_string (scene);
  g_assert_nonnull (str);

  lua_pushstring (L, str);
  g_free (str);

  return 1;
}

/*-
 * scene:get (name:string)
 *      -> value:any
 *
 * Gets scene property value.
 */
static int
l_scene_get (lua_State *L)
{
  lp_Scene *scene;
  const gchar *name;

  GParamSpec *pspec;
  GValue value = G_VALUE_INIT;

  scene_check (L, 1, &scene);
  name = luaL_checkstring (L, 2);

  pspec = gx_object_find_property (scene, name);
  if (unlikely (pspec == NULL))
    return error_throw_property_unknown (L, scene, name);

  g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));
  g_object_get_property (G_OBJECT (scene), name, &value);
  switch (G_VALUE_TYPE (&value))
    {
    case G_TYPE_BOOLEAN:
      lua_pushboolean (L, g_value_get_boolean (&value));
      break;
    case G_TYPE_INT:
      lua_pushinteger (L, g_value_get_int (&value));
      break;
    case G_TYPE_UINT:
      lua_pushinteger (L, g_value_get_uint (&value));
      break;
    case G_TYPE_UINT64:
      lua_pushinteger (L, (lua_Integer) clamp (g_value_get_uint64 (&value),
                                               0, G_MAXINT));
      break;
    case G_TYPE_STRING:
      {
        const char *str = g_value_get_string (&value);
        if (str == NULL)
          lua_pushnil (L);
        else
          lua_pushstring (L, str);
        break;
      }
    default:
     g_assert_not_reached ();
    }
  g_value_unset (&value);

  return 1;
}

/*-
 * scene:set (name:string, value:any)
 *
 * Sets scene property name to value.
 */
static int
l_scene_set (lua_State *L)
{
  lp_Scene *scene;
  const gchar *name;

  GParamSpec *pspec;

  scene_check (L, 1, &scene);
  name = luaL_checkstring (L, 2);

  pspec = gx_object_find_property (scene, name);
  if (unlikely (pspec == NULL))
    {
      return error_throw_property_unknown (L, scene, name);
    }

  if (unlikely ((!(pspec->flags & G_PARAM_WRITABLE))
                || (pspec->flags & G_PARAM_CONSTRUCT_ONLY)))
    {
      return error_throw_property_not_writable (L, scene, name);
    }

  switch (G_PARAM_SPEC_VALUE_TYPE (pspec))
    {
    case G_TYPE_BOOLEAN:
      g_object_set (scene, name, lua_toboolean (L, 3), NULL);
      break;
    case G_TYPE_INT:            /* fall through */
    case G_TYPE_UINT:
      g_object_set (scene, name, luaL_checkinteger (L, 3), NULL);
      break;
    case G_TYPE_UINT64:
      g_object_set (scene, name, luaL_checkinteger (L, 3), NULL);
      break;
    case G_TYPE_STRING:
      g_object_set (scene, name, luaL_optstring (L, 3, NULL), NULL);
      break;
    default:
      g_assert_not_reached ();
    }

  return 0;
}

/*-
 * scene:receive (block:boolean)
 *      -> event:table; or
 *      -> nil
 *
 * Receives an event from scene.
 * This call may block if block parameter is given.
 *
 * Returns a table representing the event or nil (no event).
 */
static int
l_scene_receive (lua_State *L)
{
  lp_Scene *scene;
  int block;

  lp_Event *event;

  scene_check (L, 1, &scene);
  block = lua_toboolean (L, 2);

  event = lp_scene_receive (scene, block);
  play_event_push (L, event);
  if (event != NULL)
    g_object_unref (event);

  return 1;
}


int
luaopen_play_play0 (lua_State *L)
{
  lua_newtable (L);
  luax_newmetatable (L, SCENE);
  luaL_setfuncs (L, scene_funcs, 0);
  lua_setfield (L, -2, "scene");

  return 1;
}
