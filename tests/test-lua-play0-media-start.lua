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
   local sc = tests.scene_new (800, 600, nil, ME)
   local m = assert (media.new (sc, tests.sample ('night')))

   assert (pcall (media.start, nil) == false) -- bad media

   assert (m:start ())          -- start media
   local evt = tests.scene_await (sc, 'start')
   assert (evt.type == 'start' and evt.source == m and evt.resume == false)

   assert (m:start () == false) -- already started
   assert (m:start () == false)

   assert (tests.scene_await (sc, 'tick'))
end
