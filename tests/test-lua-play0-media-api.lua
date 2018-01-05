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
local type = type
local assert = assert

local play = require ('play.play0')
local media = play.media
_ENV = nil
do
   assert (type (media.new) == 'function')
   assert (type (media.__gc) == 'function')
   assert (type (media.__tostring) == 'function')
end
