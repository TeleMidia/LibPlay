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

local play = require ('play.play0')
local scene = play.scene
_ENV = nil

do
   local sc = assert (scene.new ())

   assert (pcall (sc.get, nil) == false)           -- bad scene
   assert (pcall (sc.get, sc, nil) == false)       -- bad property
   assert (pcall (sc.get, sc, 'unknown') == false) -- unknown property

   assert (sc:get ('width') == 0)
   assert (sc:get ('height') == 0)
   assert (sc:get ('background') == 0)
   assert (sc:get ('wave') == 4)
   assert (sc:get ('ticks') == 0)
   assert (sc:get ('interval') == 1000000000)
   assert (sc:get ('time') > 0)
   assert (sc:get ('lockstep') == false)
   assert (sc:get ('slave-audio') == false)
   assert (sc:get ('text') == nil)
   assert (sc:get ('text-color') == 0xffffffff)
   assert (sc:get ('text-font') == nil)

   local sc = assert (scene.new (800, 600))
   assert (sc:get ('width') == 800)
   assert (sc:get ('height') == 600)
end
