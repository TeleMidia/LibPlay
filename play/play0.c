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
PRAGMA_DIAG_IGNORE (-Wunused-macros)

/* Scene object.  */
typedef struct Scene
{
  lp_Scene *scene;              /* wrapped scene */
  gboolean quitted;             /* true if scene has quitted */
} Scene;

/* Media object.  */
typedef struct Media
{
  Scene *sw;                    /* parent wrapped scene */
  lp_Media *media;              /* wrapped media */
} Media;

/* Registry keys for metatables.  */
#define SCENE "play.scene"
#define MEDIA "play.media"

/* Local registry for this module.  */
static guint play_registry_size = 0;
#define PLAY_REGISTRY_INDEX (deconst (void *, &play_registry_size))

#define play_registry_create(L)\
  luax_mregistry_create (L, PLAY_REGISTRY_INDEX)

#define play_registry_destroy(L)\
  luax_mregistry_destroy (L, PLAY_REGISTRY_INDEX)

#define play_registry_get(L)\
  luax_mregistry_get (L, PLAY_REGISTRY_INDEX)

#define play_registry_dump(L)                   \
  STMT_BEGIN                                    \
  {                                             \
    play_registry_get ((L));                    \
    if (!lua_isnil ((L), -1))                   \
      luax_dump_table ((L), -1);                \
    lua_pop ((L), 1);                           \
  }                                             \
  STMT_END

#define play_registry_add_scene(L, sw)          \
  STMT_BEGIN                                    \
  {                                             \
    if (play_registry_size == 0)                \
      {                                         \
        lua_newtable ((L));                     \
        play_registry_create (L);               \
      }                                         \
    play_registry_get ((L));                    \
    lua_newtable ((L));                         \
    lua_pushvalue ((L), -3);                    \
    lua_rawseti ((L), -2, 0);                   \
    lua_rawsetp ((L), -2, (sw));                \
    lua_pop ((L), 1);                           \
    play_registry_size++;                       \
    g_assert (play_registry_size > 0);          \
  }                                             \
  STMT_END

#define play_registry_remove_scene(L, sw)       \
  STMT_BEGIN                                    \
  {                                             \
    g_assert (play_registry_size > 0);          \
    play_registry_get ((L));                    \
    lua_pushnil ((L));                          \
    lua_rawsetp ((L), -2, (sw));                \
    lua_pop ((L), 1);                           \
    if (--play_registry_size == 0)              \
      play_registry_destroy ((L));              \
  }                                             \
  STMT_END

#define play_registry_get_scene_table(L, sw)    \
  STMT_BEGIN                                    \
  {                                             \
    g_assert (play_registry_size > 0);          \
    play_registry_get ((L));                    \
    lua_rawgetp ((L), -1, (sw));                \
    g_assert (lua_istable ((L), -1));           \
    lua_replace ((L), -2);                      \
  }                                             \
  STMT_END

#define play_registry_get_scene(L, sw)          \
  STMT_BEGIN                                    \
  {                                             \
    play_registry_get_scene_table ((L), (sw));  \
    lua_rawgeti ((L), -1, 0);                   \
    g_assert (lua_isuserdata ((L), -1));        \
    lua_replace ((L), -2);                      \
  }                                             \
  STMT_END

#define play_registry_add_media(L, mw)                  \
  STMT_BEGIN                                            \
  {                                                     \
    play_registry_get_scene_table ((L), (mw)->sw);      \
    lua_pushvalue ((L), -2);                            \
    lua_rawsetp ((L), -2, (mw));                        \
    lua_pop ((L), 1);                                   \
  }                                                     \
  STMT_END

#define play_registry_remove_media(L, mw)               \
  STMT_BEGIN                                            \
  {                                                     \
    play_registry_get_scene_table ((L), (mw)->sw);      \
    lua_pushnil ((L));                                  \
    lua_rawsetp ((L), -2, (mw));                        \
    lua_pop ((L), 1);                                   \
  }                                                     \
  STMT_END

#define play_registry_get_media(L, mw)                  \
  STMT_BEGIN                                            \
  {                                                     \
    play_registry_get_scene_table ((L), (mw)->sw);      \
    lua_rawgetp ((L), -1, (mw));                        \
    assert (lua_isuserdata ((L), -1));                  \
    lua_replace ((L), -2);                              \
  }                                                     \
  STMT_END

/* Forward declarations.  */
static int l_scene_new (lua_State *);
static int __l_scene_gc (lua_State *);
static int __l_scene_tostring (lua_State *);
static int l_scene_get (lua_State *);
static int l_scene_set (lua_State *);
static int l_scene_receive (lua_State *);
static int l_scene_quit (lua_State *);
static int l_media_new (lua_State *);
static int __l_media_gc (lua_State *);
static int __l_media_tostring (lua_State *);
static int l_media_start (lua_State *);
static int l_media_stop (lua_State *);
static int l_media_seek (lua_State *);
int luaopen_play_play0 (lua_State *);

/* Scene object metamethods.  */
static const struct luaL_Reg scene_funcs[] = {
  {"new", l_scene_new},
  {"__gc", __l_scene_gc},
  {"__tostring", __l_scene_tostring},
  {"get", l_scene_get},
  {"set", l_scene_set},
  {"receive", l_scene_receive},
  {"quit", l_scene_quit},
  {NULL, NULL},
};

/* Media object metamethods.  */
static const struct luaL_Reg media_funcs[] = {
  {"new", l_media_new},
  {"__gc", __l_media_gc},
  {"__tostring", __l_media_tostring},
  {"start", l_media_start},
  {"stop", l_media_stop},
  {"seek", l_media_seek},
  {NULL, NULL},
};


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
    case LP_EVENT_MASK_QUIT:          /* fall through */
    case LP_EVENT_MASK_TICK:          /* fall through */
    case LP_EVENT_MASK_KEY:           /* fall through */
    case LP_EVENT_MASK_POINTER_CLICK: /* fall through */
    case LP_EVENT_MASK_POINTER_MOVE:
      {
        Scene *sw;
        lp_Scene *scene;

        sw = (Scene *) g_object_get_data (src, SCENE);
        g_assert_nonnull (sw);

        scene = LP_SCENE (src);
        g_assert (sw->scene == scene);

        play_registry_get_scene (L, sw);
        lua_setfield (L, -2, "source");

        switch (mask)
          {
          case LP_EVENT_MASK_QUIT:
            {
              break;
            }
          case LP_EVENT_MASK_TICK:
            {
              guint64 serial;

              g_object_get (event, "serial", &serial, NULL);

              luax_setstringfield (L, -1, "type", "tick");
              luax_setintegerfield (L, -1, "serial", serial);
              break;
            }
          case LP_EVENT_MASK_KEY:
            {
              gchar *key;
              gboolean press;

              g_object_get (event,
                            "key", &key,
                            "press", &press, NULL);

              luax_setstringfield (L, -1, "type", "key");
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

              g_object_get (event,
                            "x", &x,
                            "y", &y,
                            "button", &button,
                            "press", &press, NULL);

              luax_setstringfield (L, -1, "type", "pointer-click");
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

              g_object_get (event, "x", &x, "y", &y, NULL);

              luax_setstringfield (L, -1, "type", "pointer-move");
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
        Media *mw;
        lp_Media *media;

        mw = (Media *) g_object_get_data (src, MEDIA);
        g_assert_nonnull (mw);

        media = LP_MEDIA (src);
        g_assert (mw->media == media);

        play_registry_get_media (L, mw);
        lua_setfield (L, -2, "source");

        switch (mask)
          {
          case LP_EVENT_MASK_ERROR:
            {
              const char *map[] = {"start", "stop", "seek"};
              GError *error = NULL;

              g_object_get (event, "error", &error, NULL);
              g_assert_nonnull (error);
              g_assert (nelementsof (map) == LP_ERROR_LAST);
              g_assert (error->code >= 0 && error->code < LP_ERROR_LAST);

              luax_setstringfield (L, -1, "type", "error");
              luax_setstringfield (L, -1, "code", map[error->code]);
              luax_setstringfield (L, -1, "message", error->message);

              g_error_free (error);
              break;
            }
          case LP_EVENT_MASK_START:
            {
              gboolean resume;

              g_object_get (event, "resume", &resume, NULL);

              luax_setstringfield (L, -1, "type", "start");
              luax_setbooleanfield (L, -1, "resume", resume);
              break;
            }
          case LP_EVENT_MASK_STOP:
            {
              gboolean eos;

              g_object_get (event, "eos", &eos, NULL);

              luax_setstringfield (L, -1, "type", "stop");
              luax_setbooleanfield (L, -1, "eos", eos);
              break;
            }
          case LP_EVENT_MASK_SEEK:
            {
              gboolean relative;
              gint64 offset;

              g_object_get (event,
                            "relative", &relative,
                            "offset", &offset, NULL);

              luax_setstringfield (L, -1, "type", "seek");
              luax_setbooleanfield (L, -1, "relative", relative);
              luax_setintegerfield (L, -1, "offset", offset);
              break;
            }
          default:
            g_assert_not_reached ();
          }
        break;
      }
    default:
      g_assert_not_reached ();
    }
}

/* Checks if the object at @index is a scene.  If successful, returns the
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

/* Checks if the object at @index is a media.  If successful, returns the
   media and stores the wrapped lp_Media into @media.  Otherwise, throws an
   error.  */

static Media *
media_check (lua_State *L, int index, lp_Media **media)
{
  Media *mw;

  mw = (Media *) luaL_checkudata (L, index, MEDIA);
  g_assert_nonnull (mw);
  g_assert_nonnull (mw->media);
  set_if_nonnull (media, mw->media);

  return mw;
}


/*-
 * scene.new ([width:number, height:number])
 * scene:new ([width:number, height:number])
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
  int width;
  int height;

  Scene *sw;

  luax_optudata (L, 1, SCENE);
  width = (int) clamp (luaL_optinteger (L, 2, 0), G_MININT, G_MAXINT);
  height = (int) clamp (luaL_optinteger (L, 3, 0), G_MININT, G_MAXINT);

  sw = (Scene *) lua_newuserdata (L, sizeof (*sw));
  g_assert_nonnull (sw);

  sw->scene = lp_scene_new (width, height);
  g_assert_nonnull (sw->scene);
  sw->quitted = FALSE;

  g_object_set_data (G_OBJECT (sw->scene), SCENE, sw);
  luaL_setmetatable (L, SCENE);

  play_registry_add_scene (L, sw);
  debug ("creating scene %p", sw);
  play_registry_dump (L);

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
  Scene *sw;
  lp_Scene *scene;

  sw = scene_check (L, 1, &scene);
  if (!sw->quitted)
    {
      lua_pushcfunction (L, l_scene_quit);
      lua_pushvalue (L, 1);
      lua_call (L, 1, 1);
      g_assert (lua_toboolean (L, -1));
    }

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

/*-
 * scene:quit ()
 *      -> b:boolean
 *
 * Quits scene if it has not quit yet.
 *
 * Returns true if successful, or false otherwise.
 */
static int
l_scene_quit (lua_State *L)
{
  Scene *sw;
  lp_Scene *scene;

  sw = scene_check (L, 1, &scene);
  if (unlikely (sw->quitted))
    {
      lua_pushboolean (L, FALSE);
      return 1;
    }

  lp_scene_quit (scene);
  sw->quitted = TRUE;
  lua_pushboolean (L, TRUE);

  play_registry_remove_scene (L, sw);
  debug ("quitting scene %p", sw);
  play_registry_dump (L);

  return 1;
}


/*-
 * media.new (scene:Scene, uri:string)
 * media:new (scene:Scene, uri:string)
 *      -> media:Media; or
 *      -> nil, errmsg:string
 *
 * Creates a new media with the given parent scene and content URI.
 *
 * Returns the new media if successful, or nil plus error message.
 */
static int
l_media_new (lua_State *L)
{
  Scene *sw;
  lp_Scene *scene;
  const char *uri;

  Media *mw;

  luax_optudata (L, 1, MEDIA);
  sw = scene_check (L, 2, &scene);
  uri = luaL_checkstring (L, 3);

  if (unlikely (sw->quitted))
    {
      lua_pushnil (L);
      lua_pushliteral (L, "scene has quitted");
      return 2;
    }

  mw = (Media *) lua_newuserdata (L, sizeof (*mw));
  g_assert_nonnull (mw);

  mw->sw = sw;
  mw->media = lp_media_new (mw->sw->scene, uri);
  g_assert_nonnull (mw->media); /* cannot fail */

  g_object_set_data (G_OBJECT (mw->media), MEDIA, mw);
  luaL_setmetatable (L, MEDIA);

  play_registry_add_media (L, mw);
  debug ("creating media %p in %p", mw, mw->sw);
  play_registry_dump (L);

  return 1;
}

/*-
 * media:__gc ()
 *
 * Destroys media.
 */
static int
__l_media_gc (lua_State *L)
{
  lp_Media *media;

  media_check (L, 1, &media);

  /* FIXME: We should keep a reference to media.  */
  /* g_object_unref (media); */

  return 0;
}

/*-
 * media:__tostring ()
 *      -> s:string
 *
 * Dumps media to a string and returns it.
 */
static int
__l_media_tostring (lua_State *L)
{
  lp_Media *media;
  char *str;

  media_check (L, 1, &media);
  str = lp_media_to_string (media);
  g_assert_nonnull (str);

  lua_pushstring (L, str);
  g_free (str);

  return 1;
}

/*-
 * media:start ()
 *      -> b:boolean
 *
 * Starts media asynchronously.
 *
 * Returns true if successful, or false otherwise.
 */
static int
l_media_start (lua_State *L)
{
  lp_Media *media;

  media_check (L, 1, &media);
  lua_pushboolean (L, lp_media_start (media));

  return 1;
}

/*-
 * media:stop ()
 *      -> b:boolean
 *
 * Stops media asynchronously.
 *
 * Returns true if successful, or false otherwise.
 */
static int
l_media_stop (lua_State *L)
{
  lp_Media *media;

  media_check (L, 1, &media);
  lua_pushboolean (L, lp_media_stop (media));

  return 1;
}

/*-
 * media:seek (relative:boolean, offset:number)
 *      -> b:boolean
 *
 * Seeks in media asynchronously by offset nanoseconds.
 *
 * Returns true if successful, or false otherwise.
 */
static int
l_media_seek (lua_State *L)
{
  lp_Media *media;
  gboolean relative;
  lua_Integer offset;

  media_check (L, 1, &media);
  relative = lua_toboolean (L, 2);
  offset = luaL_checkinteger (L, 3);
  lua_pushboolean (L, lp_media_seek (media, relative, offset));

  return 1;
}


int
luaopen_play_play0 (lua_State *L)
{
  lua_newtable (L);

  luax_newmetatable (L, SCENE);
  luaL_setfuncs (L, scene_funcs, 0);
  lua_setfield (L, -2, "scene");

  luax_newmetatable (L, MEDIA);
  luaL_setfuncs (L, media_funcs, 0);
  lua_setfield (L, -2, "media");

  return 1;
}
