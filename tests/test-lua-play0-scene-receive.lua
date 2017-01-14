--[[ Copyright (C) 2015-2017 PUC-Rio/Laboratorio TeleMidia

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
local error = error
local pairs = pairs
local pcall = pcall
local print = print
local type = type
local tests = require ('tests')

local play = require ('play.play0')
local scene = play.scene
_ENV = nil

local function dump (t)
   local s = '{'
   for k,v in pairs (t) do
      if k ~= 'source' then
         s = s..('%s=%s,'):format (k,v)
      end
   end
   return s:sub (1,-2)..'}'
end

do
   local sc = tests.scene_new (800, 600, nil, ME)
   sc:set ('background', 0)
   sc:set ('wave', 0)

   assert (pcall (sc.receive, nil) == false) -- bad scene

   local await = 5
   while await > 0 do
      local t = sc:receive (true)
      assert (t.source == sc)
      if t.type == 'tick' then
         assert (type (t.serial) == 'number')
         sc:set ('background', (sc:get ('background') + 1) % 4)
         sc:set ('wave', sc:get ('wave') + 1)
         await = await - 1
      elseif t.type == 'key' then
         assert (type (t.key) == 'string')
         assert (type (t.press) == 'boolean')
      elseif t.type == 'pointer-click' then
         assert (type (t.x) == 'number')
         assert (type (t.y) == 'number')
         assert (type (t.button) == 'number')
         assert (type (t.press) == 'boolean')
      elseif t.type == 'pointer-move' then
         assert (type (t.x) == 'number')
         assert (type (t.y) == 'number')
      else
         error ('unexpected event')
      end
      local s = dump (t)
      sc:set ('text', ME..'\n'..s)
      print (s)
   end
end
