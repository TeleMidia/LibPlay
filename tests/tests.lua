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

local assert = assert
local pairs = pairs

local play = require ('play.play0')
local scene = play.scene
local media = play.media

local tests0 = assert (require ('tests0'))
local tests = {}
_ENV = nil

-- Install tests0 aliases.

do
   for k,v in pairs (tests0) do
      tests[k] = v
   end
end

-- Creates a new test scene.

function tests.scene_new (width, height, pattern, me)
   local sc = assert (scene.new (width, height))
   if pattern ~= nil then
      sc:set ('pattern', pattern)
   end
   if me ~= nil then
      sc:set ('text', me)
      sc:set ('text-color', 0xffffff00)
      sc:set ('text-font', 'sans italic bold 16')
   end
   return sc
end

-- Awaits for the given number of events.

function tests.scene_await (sc, tp, n)
   local tp = tp or 'tick'
   local n = n or 1
   local evt = nil
   while n > 0 do
      evt = assert (sc:receive (true))
      if assert (evt.type) == (tp or 'tick') then
         n = n - 1
      end
   end
   return assert (evt)
end

return tests
