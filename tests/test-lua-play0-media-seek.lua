--[[ Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia

This file is part of LibPLay.

LibPLay is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

LibPLay is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License
along with LibPLay.  If not, see <http://www.gnu.org/licenses/>.  ]]--

local ME = assert (arg[1]:sub (3))
local assert = assert
local pcall = pcall
local tests = require ('tests')

local play = require ('play.play0')
local scene = play.scene
local media = play.media
_ENV = nil

do
   local w, h = 800, 600
   local sc = tests.scene_new (w, h, nil, ME)
   local interval = sc:get ('interval')
   local m = {
      assert (media.new (sc, tests.sample ('clock'))),
      assert (media.new (sc, tests.sample ('clock'))),
      assert (media.new (sc, tests.sample ('clock'))),
      assert (media.new (sc, tests.sample ('clock')))
   }

   for i=1,#m do
      m[i]:set ('width', w/2)
      m[i]:set ('height', h/2)
   end
   m[2]:set ('y', h/2)
   m[3]:set ('x', w/2)
   m[4]:set ('x', w/2)
   m[4]:set ('y', h/2)

   assert (pcall (media.seek, nil) == false) -- bad media
   assert (m[1]:seek () == false)            -- not started
   assert (m[2]:seek () == false)            -- not started
   assert (m[3]:seek () == false)            -- not started
   assert (m[4]:seek () == false)            -- not started

   for i=1,#m do
      assert (m[i]:start ())    -- start i-th media
   end
   assert (tests.scene_await (sc, 'start', 4))
   assert (tests.scene_await (sc, 'tick', 2))

   for i=1,#m do
      assert (m[i]:seek (i * interval)) -- seek i-th media by i seconds
   end
   assert (tests.scene_await (sc, 'seek', 4))
   assert (tests.scene_await (sc, 'tick', 2))

   for i=1,#m do
      assert (m[i]:seek (-i * interval)) -- seek i-th media by -i seconds
   end
   assert (tests.scene_await (sc, 'seek', 4))
   assert (tests.scene_await (sc, 'tick', 2))

   for i=1,#m do
      assert (m[i]:seek (false, 0)) -- seek all media back to beginning
   end
   assert (tests.scene_await (sc, 'seek', 4))
   assert (tests.scene_await (sc, 'tick', 2))

   for i=1,#m do
      assert (m[i]:seek (false, -i * interval)) -- seek all media to end
   end
   assert (tests.scene_await (sc, 'seek', 4))
   assert (tests.scene_await (sc, 'stop', 4))
end
