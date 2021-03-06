--[[ Copyright (C) 2015-2018 PUC-Rio/Laboratorio TeleMidia

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

   assert (pcall (m.get, nil) == false)          -- bad media
   assert (pcall (m.get, m, nil) == false)       -- bad property
   assert (pcall (m.get, m, 'unknown') == false) -- unknown property

   -- assert (m:get ('scene') == sc)             -- FIXME
   assert (m:get ('uri') == tests.sample ('gnu'))
   assert (m:get ('x') == 0)
   assert (m:get ('y') == 0)
   assert (m:get ('z') == 1)
   assert (m:get ('width') == 0)
   assert (m:get ('height') == 0)
   assert (m:get ('alpha') == 1.0)
   assert (m:get ('mute') == false)
   assert (m:get ('volume') == 1.0)
   assert (m:get ('text') == nil)
   assert (m:get ('text-color') == 0xffffffff)
   assert (m:get ('text-font') == nil)
end
