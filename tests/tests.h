/* tests.h -- Common declarations for tests.
   Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia

This file is part of LibPlay.

LibPlay is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

LibPlay is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License
along with LibPlay.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef TESTS_H
#define TESTS_H

#include <config.h>
#include <assert.h>

#include "macros.h"
#include "gstx-macros.h"

#include "play.h"

/* Waits for @n ticks in @scene.  */
#define AWAIT(scene, n)                                 \
  STMT_BEGIN                                            \
  {                                                     \
    int _total = (n);                                   \
    while (_total > 0)                                  \
      {                                                 \
        lp_Event _evt;                                  \
        lp_scene_pop ((scene), TRUE, NULL, &_evt);      \
        if (_evt == LP_TICK)                            \
          _total--;                                     \
      }                                                 \
  }                                                     \
  STMT_END

#endif /* TESTS_H */
