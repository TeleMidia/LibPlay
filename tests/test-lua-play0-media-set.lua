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
   local sc = assert (scene.new ())
   local m = assert (media.new (sc, tests.sample ('gnu')))

   assert (pcall (m.set, nil) == false)              -- bad media
   assert (pcall (m.set, m, nil) == false)           -- bad property
   assert (pcall (m.set, m, 'unknown') == false)     -- unknown property
   assert (pcall (m.set, m, 'unknown', 30) == false) -- unknown property

   assert (pcall (m.set, m, 'scene', nil) == false)       -- read-only
   assert (pcall (m.set, m, 'uri', '') == false)          -- read-only
   assert (pcall (m.set, m, 'x', function()end) == false) -- bad type
   assert (pcall (m.set, m, 'y', function()end) == false) -- bad type
   assert (pcall (m.set, m, 'z', function()end) == false) -- bad type
   assert (pcall (m.set, m, 'width', {}) == false)        -- bad type
   assert (pcall (m.set, m, 'height', {}) == false)       -- bad type
   assert (pcall (m.set, m, 'alpha', {}) == false)        -- bad type
   assert (pcall (m.set, m, 'volume', {}) == false)       -- bad type
   assert (pcall (m.set, m, 'text', {}) == false)         -- bad type
   assert (pcall (m.set, m, 'text-color', {}) == false)   -- bad type
   assert (pcall (m.set, m, 'text-font', {}) == false)    -- bad type

   assert (m:get ('x') == 0)
   m:set ('x', 2)
   assert (m:get ('x') == 2)

   assert (m:get ('y') == 0)
   m:set ('y', 2)
   assert (m:get ('y') == 2)

   assert (m:get ('z') == 1)
   m:set ('z', 30)
   assert (m:get ('z') == 30)

   assert (m:get ('width') == 0)
   m:set ('width', 100)
   assert (m:get ('width') == 100)

   assert (m:get ('height') == 0)
   m:set ('height', 100)
   assert (m:get ('height') == 100)

   assert (m:get ('alpha') == 1.0)
   m:set ('alpha', 0)
   assert (m:get ('alpha') == 0.0)

   assert (m:get ('mute') == false)
   m:set ('mute', true)
   assert (m:get ('mute') == true)

   assert (m:get ('volume') == 1.0)
   m:set ('volume', 0)
   assert (m:get ('volume') == 0)

   assert (m:get ('text') == nil)
   m:set ('text', 'nullius in verba')
   assert (m:get ('text') == 'nullius in verba')

   assert (m:get ('text-color') == 0xffffffff)
   m:set ('text-color', 0)
   assert (m:get ('text-color') == 0)

   assert (m:get ('text-font') == nil)
   m:set ('text-font', 'abusus non tollit usum')
   assert (m:get ('text-font') == 'abusus non tollit usum')
   m:set ('text-font', nil)
   assert (m:get ('text-font') == nil)
end
