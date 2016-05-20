/* test-templates.h -- Templates for tests.
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

#ifndef TEST_TEMPLATES_H
#define TEST_TEMPLATES_H

#include <config.h>
#include "tests.h"

#define TEST_TEMPLATE_START_FORMAT(uri)         \
  STMT_BEGIN                                    \
  {                                             \
    lp_Scene *scene;                            \
    lp_Media *media;                            \
                                                \
    scene = SCENE_NEW (800, 600, 0);            \
    media = lp_media_new (scene, (uri));        \
    g_assert_nonnull (media);                   \
    g_assert (lp_media_start (media));          \
                                                \
    await_ticks (scene, 3);                     \
    g_object_unref (scene);                     \
  }                                             \
  STMT_END

#endif /* TEST_TEMPLATES_H */
