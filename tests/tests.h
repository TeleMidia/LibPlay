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
#include "gx-macros.h"
#include "gstx-macros.h"

GX_INCLUDE_PROLOGUE
#include "play.h"
#include "play-internal.h"
GX_INCLUDE_EPILOGUE

PRAGMA_DIAG_IGNORE (-Wfloat-equal)

/* Sleeps for @n seconds.  */
#define SLEEP(n) g_usleep ((n) * 1000000)

/* Waits for @n ticks in @scene.  */
#define AWAIT(scene, n)                                         \
  STMT_BEGIN                                                    \
  {                                                             \
    int __total = (n);                                          \
    while (__total > 0)                                         \
      {                                                         \
        lp_Event *__evt;                                        \
        __evt = lp_scene_receive ((scene), TRUE);               \
        g_assert_nonnull (__evt);                               \
        if (G_OBJECT_TYPE (__evt) == LP_TYPE_EVENT_TICK)        \
          __total--;                                            \
        g_object_unref (__evt);                                 \
      }                                                         \
  }                                                             \
  STMT_END

#endif /* TESTS_H */
