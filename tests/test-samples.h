/* test-samples.h -- Media samples for tests.
   Copyright (C) 2015-2017 PUC-Rio/Laboratorio TeleMidia

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

#ifndef TEST_SAMPLES_H
#define TEST_SAMPLES_H

#include <config.h>
#include "macros.h"
#include "gx-macros.h"

/* Expands to the path of samples directory.  */
#define SAMPLES_DIR_PATH  TOP_SRCDIR"/tests/samples/"

/* Expands to the path of the given sample file.  */
#define SAMPLES_DIR(file) SAMPLES_DIR_PATH G_STRINGIFY (file)

/* Audio samples. */
#define SAMPLE_ARCADE     SAMPLES_DIR (arcade.mp3)
#define SAMPLE_COZY       SAMPLES_DIR (cozy.oga)
#define SAMPLE_EVILEYE    SAMPLES_DIR (evileye.mp3)

/* Image samples.  */
#define SAMPLE_EARTH      SAMPLES_DIR (earth.gif)
#define SAMPLE_FELIS      SAMPLES_DIR (felis.jpg)
#define SAMPLE_GNU        SAMPLES_DIR (gnu.png)

/* Video samples.  */
#define SAMPLE_CLOCK      SAMPLES_DIR (clock.ogv)
#define SAMPLE_DIODE      SAMPLES_DIR (diode.mp4)
#define SAMPLE_LEGO       SAMPLES_DIR (lego.ogv)
#define SAMPLE_NIGHT      SAMPLES_DIR (night.avi)
#define SAMPLE_ROAD       SAMPLES_DIR (road.ogv)
#define SAMPLE_SYNC       SAMPLES_DIR (sync.m4v)

/* All samples.  */
static ATTR_UNUSED const gchar *samples_all[] =
{
  SAMPLE_ARCADE,
  SAMPLE_CLOCK,
  SAMPLE_COZY,
  SAMPLE_DIODE,
  SAMPLE_EARTH,
  SAMPLE_EVILEYE,
  SAMPLE_FELIS,
  SAMPLE_GNU,
  SAMPLE_LEGO,
  SAMPLE_NIGHT,
  SAMPLE_ROAD,
  SAMPLE_SYNC,
};

/* By format.  */
#define SAMPLE_AVI SAMPLE_NIGHT
#define SAMPLE_GIF SAMPLE_EARTH
#define SAMPLE_JPG SAMPLE_FELIS
#define SAMPLE_M4V SAMPLE_SYNC
#define SAMPLE_MP3 SAMPLE_ARCADE
#define SAMPLE_MP4 SAMPLE_DIODE
#define SAMPLE_OGA SAMPLE_COZY
#define SAMPLE_OGV SAMPLE_LEGO
#define SAMPLE_PNG SAMPLE_GNU

/* Random sample.  */
#define random_sample()\
  samples_all[g_random_int_range (0, nelementsof (samples_all))]

#endif /* TEST_SAMPLES_H */
