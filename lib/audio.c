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

static const char *default_audio_caps = "audio/x-raw,rate=48000"; 
            /* Do we need other caps? */

static void
__lp_media_get_audio_properties (lp_media_t *media, double *volume)
{
  _lp_media_lock (media);
  
  if (_lp_media_recursive_get_property_double (media, "volume", volume)
      == FALSE)
    *volume = 1.0;
  
  _lp_media_unlock (media);
}

static void
__lp_media_setup_audio_elements (lp_media_t *media)
{
  GstElement *audiosink = NULL, *audiomixer = NULL, *pipeline = NULL;
  GstState state;
  lp_media_t *parent;
 
  parent = lp_media_get_parent (media);

  pipeline = _lp_media_recursive_get_element (media, "pipeline");
 
  if (parent == NULL)
    audiosink = gst_element_factory_make ("autoaudiosink", NULL);
  else
  {
    /* TODO: get the audio mixer of the parent. If the parent doesn't have
     * an audiomixer, it should be created and recursively linked with the
     * audiomixer of its parent until the audiosink is reached.*/
  }

  audiomixer = gst_element_factory_make ("adder", NULL);
  assert (audiomixer);
 
  g_hash_table_insert (media->elements, strdup ("audiosink"), audiosink);
  g_hash_table_insert (media->elements, strdup ("audiomixer"), audiomixer);

  state = GST_STATE (pipeline);

  gst_element_set_state (audiosink, state);
  gst_element_set_state (audiomixer, state);

  gst_bin_add_many (GST_BIN (pipeline), audiomixer, audiosink, NULL);
  gst_element_link (audiomixer, audiosink);
}

int
_lp_media_set_audio_bin (lp_media_t *media, GstPad *source_pad)
{
  GstElement *bin = NULL, *audiovolume = NULL, *audioconvert = NULL, 
             *audiomixer = NULL, *audioresample = NULL, 
             *audiofilter = NULL;
  GstPad *sink_pad = NULL, *ghost_pad = NULL, *output_sink_pad = NULL;
  GstCaps *caps = NULL;
  GstPadLinkReturn ret;
  lp_media_t *parent;
  int code = TRUE;

  bin = _lp_media_get_element (media, "bin");
  parent = lp_media_get_parent (media);
  
  audiomixer = _lp_media_get_element (parent, "audiomixer");
  if (audiomixer == NULL)
    __lp_media_setup_audio_elements (parent);
  
  audiomixer = _lp_media_get_element (parent, "audiomixer");
  assert (audiomixer);

  audiovolume = gst_element_factory_make ("volume", NULL);
  assert (audiovolume);

  audioconvert = gst_element_factory_make ("audioconvert", NULL);
  assert (audioconvert);

  audioresample = gst_element_factory_make ("audioresample", NULL);
  assert (audioresample);

  audiofilter = gst_element_factory_make ("capsfilter", NULL);
  assert (audiofilter);

  _lp_media_add_element (media, strdup ("audiovolume"), audiovolume);
  _lp_media_add_element (media, strdup ("audioconvert"), audioconvert);
  _lp_media_add_element (media, strdup ("audioresample"), audioresample);
  _lp_media_add_element (media, strdup ("audiofilter"), audiofilter);
  
  gst_element_set_state (audiovolume, GST_STATE_PAUSED);
  gst_element_set_state (audioconvert, GST_STATE_PAUSED);
  gst_element_set_state (audioresample, GST_STATE_PAUSED);
  gst_element_set_state (audiofilter, GST_STATE_PAUSED);

  caps = gst_caps_from_string (default_audio_caps);
  g_assert (caps);

  g_object_set (audiofilter, "caps", caps, NULL);
  gst_caps_unref (caps);

  gst_bin_add_many (GST_BIN (bin), audiovolume, audioconvert, audioresample,
                    audiofilter, NULL);

  if (!gst_element_link_many (audiovolume, audioconvert,
                              audioresample, audiofilter,
                              NULL))
  {
    g_debug ("Could not link audioconvert and audiovolume together\n.");
    code = FALSE;
  }
  else
  {
    sink_pad = gst_element_get_static_pad (audiovolume, "sink");
    g_assert (sink_pad);

    ret = gst_pad_link (source_pad, sink_pad);
    if (GST_PAD_LINK_FAILED (ret))
    {
      code = FALSE;
    }
    else
    {
      double volume;

      __lp_media_get_audio_properties (media, &volume);

      g_object_set (G_OBJECT (audiovolume), "volume", volume, NULL);

      gst_element_set_state (audiovolume, GST_STATE_PLAYING);
      gst_element_set_state (audioconvert, GST_STATE_PLAYING);
      gst_element_set_state (audioresample, GST_STATE_PLAYING);
      gst_element_set_state (audiofilter, GST_STATE_PLAYING);

      ghost_pad =
        gst_ghost_pad_new ("a_src",
                           gst_element_get_static_pad
                           (audiofilter, "src"));
      gst_pad_set_active (ghost_pad, TRUE);
      gst_element_add_pad (bin, ghost_pad);

      output_sink_pad =
        gst_element_get_request_pad (audiomixer, "sink_%u");
      g_assert (output_sink_pad != NULL);

      ret = gst_pad_link (ghost_pad, output_sink_pad);
      if (GST_PAD_LINK_FAILED (ret))
        code = FALSE;

      gst_object_unref (output_sink_pad);
      gst_object_unref (sink_pad);
    }
  }
  return code;
}
