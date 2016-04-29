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
#include "play.h"

#include "macros.h"
#include "gx-macros.h"
#include "luax-macros.h"


/* Property descriptor.  */
typedef struct Property
{
  const char *name;
  GType type;
} Property;


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
int luaopen_play_play0 (lua_State *);

/* Scene object metamethods.  */
static const struct luaL_Reg scene_funcs[] = {
  {"new", l_scene_new},
  {"__gc", __l_scene_gc},
  {"__tostring", __l_scene_tostring},
  {"get", l_scene_get},
  {NULL, NULL},
};


PRAGMA_DIAG_IGNORE (-Wunused-macros)
#if defined DEBUG && DEBUG
# define debug(fmt, ...)                                \
  g_print (G_STRLOC " (thread %p): " fmt "\n",          \
           (void *) g_thread_self (), ## __VA_ARGS__)
#else
# define debug(fmt, ...)        /* nothing */
#endif


/* Checks if the object at INDEX is a scene.
   Returns the scene object if successful, otherwise throws an error.  */

static Scene *
scene_check (lua_State *L, int index)
{
  return (Scene *) luaL_checkudata (L, index, SCENE);
}


/*-
 * scene.new ([width:int, height:int])
 * scene:new ()
 *      -> scene:Scene; or
 *      -> nil, errmsg:string
 *
 * Creates a new scene with the given dimensions.
 *
 * Returns the new scene if successful, otherwise returns nil plus error
 * message.
 */
static int
l_scene_new (lua_State *L)
{
  Scene *scene;
  int width;
  int height;

  luax_optudata (L, 1, SCENE);
  width = (int) clamp (luaL_optinteger (L, 2, 0), G_MININT, G_MAXINT);
  height = (int) clamp (luaL_optinteger (L, 3, 0), G_MININT, G_MAXINT);

  scene = (Scene *) lua_newuserdata (L, sizeof (*scene));
  assert (scene != NULL);

  scene->scene = lp_scene_new (width, height);
  assert (scene->scene != NULL);

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
  Scene *scene;

  scene = scene_check (L, 1);
  g_object_unref (scene->scene);

  return 0;
}

/*-
 * scene:__tostring ()
 * Dumps scene to a string and returns it.
 */
static int
__l_scene_tostring (lua_State *L)
{
  Scene *scene;
  int width;
  int height;
  int pattern;
  int wave;
  guint64 ticks;

  scene = scene_check (L, 1);
  g_object_get (scene->scene,
                "width", &width,
                "height", &height,
                "pattern", &pattern,
                "wave", &wave,
                "ticks", &ticks, NULL);

  lua_pushfstring (L,"\
scene (%p)\n\
  width:   %d\n\
  height:  %d\n\
  pattern: %d\n\
  wave:    %d\n\
  ticks:   %d\n\
",
                   scene, width, height, pattern, wave,
                   (int) clamp (ticks, 0, G_MAXINT));
  return 1;
}

/*-
 * scene:get (name:string)
 *      -> value:any
 *
 * Returns the value of property NAME.
 */
static int
l_scene_get (lua_State *L)
{
  Scene *scene;
  const char *name;

  GType type;
  GValue value = G_VALUE_INIT;

  scene = scene_check (L, 1);
  name = luaL_checkstring (L, 2);

  type = gx_object_find_property_type (G_OBJECT (scene->scene), name);
  if (unlikely (type == G_TYPE_INVALID))
    {
      lua_pushfstring (L, "unknown property '%s'", name);
      return lua_error (L);
    }

  g_value_init (&value, type);
  g_object_get_property (G_OBJECT (scene->scene), name, &value);
  switch (G_VALUE_TYPE (&value))
    {
    case G_TYPE_INT:
      lua_pushinteger (L, g_value_get_int (&value));
      break;
    case G_TYPE_UINT64:
      lua_pushinteger (L, (int) clamp (g_value_get_uint64 (&value),
                                       0, G_MAXINT));
      break;
    default:
      ASSERT_NOT_REACHED;
    }
  g_value_unset (&value);

  return 1;
}


int
luaopen_play_play0 (lua_State *L)
{
  luax_newmetatable (L, SCENE);
  luaL_setfuncs (L, scene_funcs, 0);
  return 1;
}
