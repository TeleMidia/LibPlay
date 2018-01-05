/* test-templates.h -- Templates for tests.
   Copyright (C) 2015-2018 PUC-Rio/Laboratorio TeleMidia

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

#define TEST_TEMPLATE_START_FORMAT(uri)                         \
  STMT_BEGIN                                                    \
  {                                                             \
    lp_Scene *scene;                                            \
    lp_Media *media;                                            \
    lp_Event *event;                                            \
                                                                \
    scene = SCENE_NEW (800, 600, 0);                            \
    media = lp_media_new (scene, (uri));                        \
    g_assert_nonnull (media);                                   \
    g_assert (lp_media_start (media));                          \
    event = await_filtered (scene, 1, LP_EVENT_MASK_START);     \
    g_assert_nonnull (event);                                   \
    g_object_unref (event);                                     \
    await_ticks (scene, 1);                                     \
    g_object_unref (scene);                                     \
  }                                                             \
  STMT_END

#define TEST_TEMPLATE_EVENT_XFAIL_GET(constructor_stmt)         \
  STMT_BEGIN                                                    \
  {                                                             \
    lp_Scene *scene;                                            \
    lp_Media *media;                                            \
    lp_Event *event;                                            \
    int nonexistent;                                            \
                                                                \
    scene = SCENE_NEW (0, 0, 0);                                \
    media = lp_media_new (scene, SAMPLE_GNU);                   \
    g_assert_nonnull (media);                                   \
    event = LP_EVENT (constructor_stmt);                        \
    g_assert_nonnull (event);                                   \
    g_object_get (event, "nonexistent", &nonexistent, NULL);    \
    g_object_unref (event);                                     \
    g_object_unref (scene);                                     \
  }                                                             \
  STMT_END

#define TEST_TEMPLATE_EVENT_XFAIL_SET(constructor_stmt) \
  STMT_BEGIN                                            \
  {                                                     \
    lp_Scene *scene;                                    \
    lp_Media *media;                                    \
    lp_Event *event;                                    \
                                                        \
    scene = SCENE_NEW (0, 0, 0);                        \
    media = lp_media_new (scene, SAMPLE_GNU);           \
    g_assert_nonnull (media);                           \
    event = LP_EVENT (constructor_stmt);                \
    g_assert_nonnull (event);                           \
    g_object_set (event, "nonexistent", 0, NULL);       \
    g_object_unref (event);                             \
    g_object_unref (scene);                             \
  }                                                     \
  STMT_END

#define TEST_TEMPLATE_PAUSE_FORMAT(uri)                         \
  STMT_BEGIN                                                    \
  {                                                             \
    lp_Scene *scene;                                            \
    lp_Media *media;                                            \
    lp_Event *event;                                            \
                                                                \
    scene = SCENE_NEW (800, 600, 0);                            \
    media = lp_media_new (scene, (uri));                        \
    g_assert_nonnull (media);                                   \
    g_assert (lp_media_start (media));                          \
    event = await_filtered (scene, 1, LP_EVENT_MASK_START);     \
    g_assert_nonnull (event);                                   \
    g_object_unref (event);                                     \
    await_ticks (scene, 1);                                     \
    g_assert (lp_media_pause (media));                          \
    event = await_filtered (scene, 1, LP_EVENT_MASK_PAUSE);     \
    g_assert_nonnull (event);                                   \
    g_object_unref (event);                                     \
    g_object_unref (scene);                                     \
  }                                                             \
  STMT_END
#endif /* TEST_TEMPLATES_H */
