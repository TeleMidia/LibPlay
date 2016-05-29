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
local pcall = pcall

local play = require ('play.play0')
local scene = play.scene
_ENV = nil

do
   local sc = assert (scene.new ())

   assert (pcall (sc.set, nil) == false)
   assert (pcall (sc.set, sc, nil) == false)
   assert (pcall (sc.set, sc, 'unknown') == false)
   assert (pcall (sc.set, sc, 'unknown', 30) == false)

   assert (sc:get ('width') == 0)
   assert (sc:get ('height') == 0)
   assert (pcall (sc.set, sc, 'width', 12) == false)  -- constructor-only
   assert (pcall (sc.set, sc, 'height', 34) == false) -- constructor-only
   assert (sc:get ('width') == 0)
   assert (sc:get ('height') == 0)

   assert (pcall (sc.set, sc, 'width', {}) == false)             -- bad type
   assert (pcall (sc.set, sc, 'height', function()end) == false) -- bad type
   assert (pcall (sc.set, sc, 'pattern', {}) == false)           -- bad type
   assert (pcall (sc.set, sc, 'wave', nil) == false)             -- bad type
   assert (pcall (sc.set, sc, 'interval', {}) == false)          -- bad type
   assert (pcall (sc.set, sc, 'text', function()end) == false) -- bad type
   assert (pcall (sc.set, sc, 'text-color', {}) == false)      -- bad type
   assert (pcall (sc.set, sc, 'text-font', {}) == false)       -- bad type

   assert (sc:get ('pattern') == 2)
   sc:set ('pattern', 3)
   assert (sc:get ('pattern') == 3)

   assert (sc:get ('wave') == 4)
   sc:set ('wave', 0)
   assert (sc:get ('wave') == 0)

   assert (sc:get ('ticks') == 0)
   assert (pcall (sc.set, sc, 'ticks', 33) == false) -- read-only
   assert (sc:get ('ticks') == 0)

   assert (sc:get ('interval') == 1000000000)
   sc:set ('interval', 44)
   assert (sc:get ('interval') == 44)

   assert (sc:get ('time') > 0)
   assert (pcall (sc.set, sc, 'time', 33) == false) -- read-only
   assert (sc:get ('time') > 0)

   assert (sc:get ('lockstep') == false)
   sc:set ('lockstep', true)
   assert (sc:get ('lockstep') == true)

   assert (sc:get ('slave-audio') == false)
   sc:set ('slave-audio', true)
   assert (sc:get ('slave-audio') == true)

   assert (sc:get ('text') == nil)
   sc:set ('text', 'nullius in verba')
   assert (sc:get ('text') == 'nullius in verba')
   sc:set ('text', nil)
   assert (sc:get ('text') == nil)

   assert (sc:get ('text-color') == 0xffffffff)
   sc:set ('text-color', 0)
   assert (sc:get ('text-color') == 0)

   assert (sc:get ('text-font') == nil)
   sc:set ('text-font', 'abusus non tollit usum')
   assert (sc:get ('text-font') == 'abusus non tollit usum')
   sc:set ('text-font', nil)
   assert (sc:get ('text-font') == nil)
end
