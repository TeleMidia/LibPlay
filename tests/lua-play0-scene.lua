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

local type = type
local assert = assert
local print = print

local scene = require ('play.play0')
_ENV = nil

-- check API
do
   assert (type (scene.new) == 'function')
   assert (type (scene.__gc) == 'function')
end

-- check constructor
do
   local sc = scene.new ()
   print (sc)
   assert (sc:get ('width') == 0)
   assert (sc:get ('height') == 0)
   assert (sc:get ('pattern') == 2)
   assert (sc:get ('wave') == 4)
   assert (sc:get ('ticks') == 0);
end
