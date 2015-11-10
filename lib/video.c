/* play-internal.h -- Internal declarations.
   Copyright (C) 2015 PUC-Rio/Laboratorio TeleMidia

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

#include "play.h"
#include "play-internal.h"

#define VIDEOWIDTH 1080
#define VIDEOHEIGHT 720

static void
__lp_media_get_video_properties (lp_media_t *media, int *x, int *y, 
    int *width, int *height, double *alpha)
{
  _lp_media_lock (media);
  
  if (_lp_media_recursive_get_property_int (media, "x", x) == FALSE)
    *x = 0;
  
  if (_lp_media_recursive_get_property_int (media, "y", y) == FALSE)
    *y = 0;
  
  if (_lp_media_recursive_get_property_int (media, "width", width) == FALSE)
    *width = VIDEOWIDTH;
  
  if (_lp_media_recursive_get_property_int (media, "height", height) == FALSE)
    *height = VIDEOHEIGHT;

  if (_lp_media_recursive_get_property_double (media, "alpha", alpha) == FALSE)
    *alpha = 1.0;

  _lp_media_unlock (media);
}

static void
__lp_scene_setup_video_elements (lp_media_t *scene)
{
  GstElement *videosink = NULL, *videomixer = NULL, *pipeline = NULL;
  GstState state;

  assert (scene->type == LP_MEDIA_SCENE);
  
  pipeline = _lp_media_recursive_get_element (scene, "pipeline");
  assert (pipeline);
  
  videosink = gst_element_factory_make ("xvimagesink", NULL);
  assert (videosink);

  videomixer = gst_element_factory_make ("videomixer", NULL);
  assert (videomixer);
 
  g_hash_table_insert (scene->elements, strdup ("videosink"), videosink);
  g_hash_table_insert (scene->elements, strdup ("videomixer"), videomixer);

  state = GST_STATE (pipeline);

  gst_element_set_state (videosink, state);
  gst_element_set_state (videomixer, state);

  gst_bin_add_many (GST_BIN (pipeline), videomixer, videosink, NULL);
  gst_element_link (videomixer, videosink);
}

int
_lp_media_set_video_bin (lp_media_t *media, GstPad *source_pad)
{
  GstElement *bin = NULL, *videoscale = NULL, *videofilter = NULL, 
             *videomixer = NULL;
  GstCaps *caps = NULL;
  GstPad *sink_pad = NULL, *ghost_pad = NULL, *videomixer_sink_pad = NULL;
  GstPadLinkReturn ret;
  lp_media_t *parent;
  int x = 0, y = 0, z = 0, width = 0, height = 0, return_value = TRUE;
  double alpha;
  
  parent = lp_media_get_parent (media);
  
  videomixer = _lp_media_get_element (parent, "videomixer");
  if (videomixer == NULL)
    __lp_scene_setup_video_elements (parent);

  videomixer = _lp_media_get_element (parent, "videomixer");
  assert (videomixer != NULL);

  bin = _lp_media_get_element (media, "bin");
  assert (bin != NULL);

  videoscale = gst_element_factory_make ("videoscale", NULL);
  assert (videoscale != NULL);

  videofilter = gst_element_factory_make ("capsfilter", NULL);
  assert (videofilter != NULL);
  
  g_hash_table_insert (media->elements, strdup ("videoscale"), videoscale);
  g_hash_table_insert (media->elements, strdup ("videofilter"), videofilter);

  gst_element_set_state (videoscale, GST_STATE_PAUSED);
  gst_element_set_state (videofilter, GST_STATE_PAUSED);

  __lp_media_get_video_properties (media, &x, &y, &width, &height, &alpha);

  caps = gst_caps_new_simple ("video/x-raw",
                              "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, 
                              "width", G_TYPE_INT, width,
                              "height", G_TYPE_INT, height, NULL);

  g_object_set (G_OBJECT (videoscale), "add-borders", 0, NULL);
  g_object_set (G_OBJECT (videofilter), "caps", caps, NULL);
  gst_caps_unref (caps);
  
  gst_bin_add_many (GST_BIN (bin), videoscale, videofilter, NULL);
  if (!gst_element_link (videoscale, videofilter))
    return_value =  FALSE;
  else
  {
    /* TODO: check if the uri is an image */

    sink_pad = gst_element_get_static_pad (videoscale, "sink");
    g_assert (sink_pad);

    ret = gst_pad_link (source_pad, sink_pad);
    if (GST_PAD_LINK_FAILED (ret))
      return_value =  FALSE;
    else
    {
      ghost_pad =
        gst_ghost_pad_new ("v_src",
            gst_element_get_static_pad (videofilter, "src"));

      gst_pad_set_active (ghost_pad, TRUE);
      gst_element_add_pad (bin, ghost_pad);

      videomixer_sink_pad = 
        gst_element_get_request_pad (videomixer, "sink_%u");
      g_assert (videomixer_sink_pad);

      ret = gst_pad_link (ghost_pad, videomixer_sink_pad);
      if (GST_PAD_LINK_FAILED (ret))
        return_value = FALSE;
      else
      {
        g_object_set (videomixer_sink_pad, "xpos", x, NULL);
        g_object_set (videomixer_sink_pad, "ypos", y, NULL);
        g_object_set (videomixer_sink_pad, "zorder", z, NULL);
        g_object_set (videomixer_sink_pad, "alpha", alpha, NULL);

        gst_element_set_state (videoscale, GST_STATE_PLAYING);
        gst_element_set_state (videofilter, GST_STATE_PLAYING);
      }
      
      gst_object_unref (videomixer_sink_pad);
      gst_object_unref (sink_pad);
    }
  }
  return return_value;
}
