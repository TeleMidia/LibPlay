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
local tostring = tostring
local tests = require ('tests0')

local play = require ('play.play0')
local scene = play.scene
local media = play.media
_ENV = nil

do
   local sc = assert (scene.new (800, 600))
   local m = assert (media.new (sc, tests.sample ('gnu')))
   sc:set ('text', tostring (m))
   local await = 5
   while await > 0 do
      local t = sc:receive (true)
      assert (t.source == sc)
      if t.type == 'tick' then
         await = await - 1
      end
   end
end
