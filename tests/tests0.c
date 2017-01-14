/* tests0.c -- Native functions used by Lua tests.lua.
   Copyright (C) 2015-2017 PUC-Rio/Laboratorio TeleMidia

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
#include "luax-macros.h"
#include "test-samples.h"

/* Forward declarations.  */
static int l_samples_dir (lua_State *L);
static int l_sample (lua_State *L);
static int l_sample_random (lua_State *L);
int luaopen_tests0 (lua_State *);

/* Exported functions.  */
static const struct luaL_Reg tests0_funcs[] = {
  {"samples_dir", l_samples_dir},
  {"sample", l_sample},
  {"sample_random", l_sample_random},
  {NULL, NULL},
};


/*-
 * tests0.samples_dir (file:string)
 *       -> path:string
 *
 * Returns the absolute path of @file in samples directory.
 */
static int
l_samples_dir (lua_State *L)
{
  const char *file;

  file = luaL_checkstring (L, 1);
  lua_pushfstring (L, "%s%s", SAMPLES_DIR_PATH, file);

  return 1;
}

/*-
 * tests0.sample (name:string)
 *       ->path:string
 *
 * Returns the absolute path of sample with @name.
 */
static int
l_sample (lua_State *L)
{
  const char *name;

  name = luaL_checkstring (L, 1);
  if (streq (name, "arcade"))
    lua_pushliteral (L, SAMPLE_ARCADE);
  else if (streq (name, "cozy"))
    lua_pushliteral (L, SAMPLE_COZY);
  else if (streq (name, "evileye"))
    lua_pushliteral (L, SAMPLE_EVILEYE);
  else if (streq (name, "earth"))
    lua_pushliteral (L, SAMPLE_EARTH);
  else if (streq (name, "felis"))
    lua_pushliteral (L, SAMPLE_FELIS);
  else if (streq (name, "gnu"))
    lua_pushliteral (L, SAMPLE_GNU);
  else if (streq (name, "clock"))
    lua_pushliteral (L, SAMPLE_CLOCK);
  else if (streq (name, "diode"))
    lua_pushliteral (L, SAMPLE_DIODE);
  else if (streq (name, "lego"))
    lua_pushliteral (L, SAMPLE_LEGO);
  else if (streq (name, "night"))
    lua_pushliteral (L, SAMPLE_NIGHT);
  else if (streq (name, "road"))
    lua_pushliteral (L, SAMPLE_ROAD);
  else if (streq (name, "sync"))
    lua_pushliteral (L, SAMPLE_SYNC);
  else
    {
      lua_pushfstring (L, "unknown sample '%s'", name);
      return lua_error (L);
    }

  return 1;
}

/*-
 * tests0.sample_random ()
 *       ->path:string
 *
 * Returns the absolute path of a random sample.
 */
static int
l_sample_random (lua_State *L)
{
  lua_pushstring (L, random_sample ());
  return 1;
}


int
luaopen_tests0 (lua_State *L)
{
  luaL_newlib (L, tests0_funcs);
  return 1;
}
