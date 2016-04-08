/* lp-media-gst.h -- GStreamer back-end.
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

#include <config.h>

#include "gstx-macros.h"
#include "play.h"
#include "play-internal.h"

/* GStreamer back-end data.  */
typedef struct _lp_media_gst_t
{
  GstPipeline *pipeline;
  GstClock *clock;
  GstClockID clock_id;
  GstClockTime start_offset;
  uint pads;

  /* GstElement */
  GstElement *bin;
  GstElement *decoder;

  /* audio */
  GstElement *audiovolume;
  GstElement *audioconvert;
  GstElement *audioresample;
  GstElement *audiofilter;

  /* video */
  GstElement *videoscale;
  GstElement *videofilter;

  /* image */
  GstElement *imagefreeze;

  /* mixers */
  GstElement *videomixer;
  GstElement *audiomixer;

  /* sinks */
  GstElement *videosink;
  GstElement *audiosink;

  GMutex mutex;

} lp_media_gst_t;

static const char *default_audio_caps = "audio/x-raw,rate=48000";
            /* Do we need other caps? */

/* Forward declarations: */
/* *INDENT-OFF* */
static lp_media_gst_t *__lp_media_gst_check (lp_media_t *);
static GstPipeline *__lp_media_gst_get_pipeline (lp_media_t *);
static gboolean __lp_media_gst_pipeline_bus_async_callback (GstBus *, 
    GstMessage *, gpointer);
static GstBusSyncReply __lp_media_gst_pipeline_bus_sync_callback (GstBus *, 
    GstMessage *, gpointer);
static void __lp_media_gst_pad_added_callback (GstElement *, GstPad *, 
    gpointer);
static lp_bool_t __lp_media_gst_set_audio_bin (lp_media_t *, GstPad *);
static lp_bool_t __lp_media_gst_set_video_bin (lp_media_t *, GstPad *);
static lp_bool_t __lp_media_gst_alloc_and_link_mixer (const char *, 
    GstElement **, const char *sink_element, GstElement **, GstElement*);
static GstPadProbeReturn __lp_media_gst_eos_event_callback (GstPad *, 
    GstPadProbeInfo *, gpointer);
/* *INDENT-ON* */

/* Checks and returns the back-end data associated with @media.  */

static lp_media_gst_t *
__lp_media_gst_check (lp_media_t *media)
{
  lp_media_backend_t *backend;

  _lp_assert (media != NULL);
  backend = _lp_media_get_backend (media);

  _lp_assert (backend != NULL);
  return (lp_media_gst_t *) backend->data;
}

/* Gets #GstPipeline associated with @media.  Returns a #GstPipeline if
   successful, or %NULL if @media has no associated #GstPipeline.  */

static GstPipeline *
__lp_media_gst_get_pipeline (lp_media_t *media)
{
  lp_media_t *root = _lp_media_get_root_ancestor (media);
  lp_media_gst_t *gst = __lp_media_gst_check (root);
  return gst->pipeline;
}

/* Allocates and installs a new #GstPipeline to @media.  Returns %TRUE if
   successful, or %FALSE if @media already has an associated
#GstPipeline.  */

static lp_bool_t
__lp_media_gst_install_pipeline (lp_media_t *media)
{
  lp_media_t *root;
  lp_media_gst_t *gst;
  lp_bool_t success;

  GstPipeline *pipeline;
  GstBus *bus;

  root = _lp_media_get_root_ancestor (media);
  _lp_media_lock (root);

  gst = __lp_media_gst_check (root);
  if (gst->pipeline == NULL)
  {
    GstElement *silence, *capsfilter;
    GstCaps *caps;
    GstPad *src, *sink;
    
    pipeline = GST_PIPELINE (gst_pipeline_new ("pipeline"));
    _lp_assert (pipeline != NULL);

    bus = gst_pipeline_get_bus (pipeline);
    _lp_assert (bus != NULL);
    gst_bus_set_sync_handler (bus, __lp_media_gst_pipeline_bus_sync_callback,
        NULL, NULL);
    gst_bus_add_watch (bus, __lp_media_gst_pipeline_bus_async_callback, NULL);
    g_object_unref (bus);

    g_object_set_data (G_OBJECT (pipeline), "lp_media", root);
    gst->pipeline = pipeline;

    gst->clock = gst_system_clock_obtain ();
    _lp_assert (gst->clock != NULL);


    gst_pipeline_use_clock (GST_PIPELINE(gst->pipeline), gst->clock);

    _lp_assert (__lp_media_gst_alloc_and_link_mixer ("adder",
                                                    &gst->audiomixer,
                                                    "autoaudiosink",
                                                    &gst->audiosink,
                                                    GST_ELEMENT
                                                    (pipeline)) == TRUE);


    silence = gst_element_factory_make ("audiotestsrc", NULL);
    capsfilter = gst_element_factory_make ("capsfilter", NULL);

    _lp_assert (silence != NULL);
    _lp_assert (capsfilter != NULL);

    caps = gst_caps_from_string (default_audio_caps);
    _lp_assert (caps != NULL);

    g_object_set (G_OBJECT (silence), "wave", /* silence */ 4, NULL);
    g_object_set (G_OBJECT (silence), "volume", 0, NULL);
    g_object_set (capsfilter, "caps", caps, NULL);

    gst_caps_unref (caps);

    gst_bin_add_many (GST_BIN (pipeline), silence, capsfilter, NULL);

    _lp_assert (gst_element_link_many (silence, capsfilter, NULL));
    src = gst_element_get_static_pad (capsfilter, "src");
    sink = gst_element_get_request_pad (gst->audiomixer, "sink_%u");

    _lp_assert (src != NULL);
    _lp_assert (sink != NULL);

    _lp_assert (gst_pad_link (src, sink) == GST_PAD_LINK_OK);

    gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
    gst_element_set_base_time (GST_ELEMENT (pipeline), 
        gst_clock_get_time (gst->clock));

    gst_object_unref (src);
    gst_object_unref (sink);
    success = TRUE;
  }
  else
    success = FALSE;

  _lp_media_unlock (root);
  return success;
}

static lp_bool_t
__lp_media_gst_alloc_and_link_mixer (const char *mixer_element,
                                     GstElement ** mixer,
                                     const char *sink_element,
                                     GstElement ** sink, GstElement * bin)
{
  /* TODO: Gets parent's mixer and link with mixer when parent != NULL */
  if (mixer_element == NULL || bin == NULL)
    return FALSE;

  *mixer = gst_element_factory_make (mixer_element, NULL);
  _lp_assert (*mixer);

  gst_element_set_state (*mixer, GST_STATE_PAUSED);

  if (sink_element != NULL)     /* Sink needs to be created  */
  {
    *sink = gst_element_factory_make (sink_element, NULL);
    _lp_assert (*sink != NULL);

    gst_bin_add (GST_BIN (bin), *sink);
    gst_element_set_state (*sink, GST_STATE_PAUSED);
  }

  gst_bin_add (GST_BIN (bin), *mixer);
  return gst_element_link (*mixer, *sink);
}

/* Returns the audio or video mixer (depending of the @mixer_type) of the 
   @media. If the @media doesn't have the mixer, the function creates it. 
   Returns a pointer (GstElement *) to the mixer*/

static GstElement *
__lp_media_gst_get_mixer (lp_media_t *media, const char *mixer_type)
{
  GstElement *mixer = NULL;
  GstPipeline *pipeline = NULL;
  lp_media_t *parent;
  lp_media_t *root;
  lp_media_gst_t *gst;

  root = _lp_media_get_root_ancestor (media);
  gst = __lp_media_gst_check (root);
  pipeline = gst->pipeline;
  if (pipeline == NULL)
    return NULL;

  parent = lp_media_get_parent (media);
  gst = __lp_media_gst_check (media);
  if (strcmp (mixer_type, "video") == 0)
  {
    if (gst->videomixer != NULL)
      mixer = gst->videomixer;
    else
    {
      if (parent == NULL)
      {
        GstElement *background, *videoscale, *capsfilter;
        GstCaps *caps;
        GstPad *src, *sink;
        lp_bool_t status;
        int w, h;

        _lp_assert (gst->videosink == NULL);

        status = __lp_media_gst_alloc_and_link_mixer ("videomixer",
                                                      &gst->videomixer,
                                                      "xvimagesink",
                                                      &gst->videosink,
                                                      GST_ELEMENT
                                                      (pipeline));
        
        _lp_assert (status == TRUE);

        background = gst_element_factory_make ("videotestsrc", NULL);
        videoscale = gst_element_factory_make ("videoscale", NULL);
        capsfilter = gst_element_factory_make ("capsfilter", NULL);

        _lp_assert (background != NULL);
        _lp_assert (videoscale != NULL);
        _lp_assert (capsfilter != NULL);

        lp_media_get_property_int (media, "width", &w);
        lp_media_get_property_int (media, "height", &h);

        _lp_assert (w != 0);
        _lp_assert (h != 0);

        caps = gst_caps_new_simple ("video/x-raw",
                              "framerate", GST_TYPE_FRACTION, 25, 1,
                              "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, 
                              "width", G_TYPE_INT, w, "height",G_TYPE_INT, h,
                              NULL);

        _lp_assert (caps != NULL);

        g_object_set (G_OBJECT (background), "pattern", /*black*/ 2, NULL);
        g_object_set (G_OBJECT (videoscale), "add-borders", 0, NULL);
        g_object_set (G_OBJECT (capsfilter), "caps", caps, NULL);
        
        gst_caps_unref (caps);
        
        gst_bin_add_many (GST_BIN (pipeline), background, videoscale, 
            capsfilter, NULL);

        _lp_assert (gst_element_link_many (background, videoscale, capsfilter, 
              NULL));

        src = gst_element_get_static_pad (capsfilter, "src");
        sink = gst_element_get_request_pad (gst->videomixer, "sink_%u");

        _lp_assert (src != NULL);
        _lp_assert (sink != NULL);

        _lp_assert (gst_pad_link (src, sink) == GST_PAD_LINK_OK);

        gst_element_set_state (background, GST_STATE_PLAYING);
        gst_element_set_state (videoscale, GST_STATE_PLAYING);
        gst_element_set_state (capsfilter, GST_STATE_PLAYING);

        gst_object_unref (src);
        gst_object_unref (sink);
      }
      else
      {
        /* TODO */
      }
      mixer = gst->videomixer;
    }
  }
  else if (strcmp (mixer_type, "audio") == 0)
  {
    if (gst->audiomixer != NULL)
      mixer = gst->audiomixer;
    else
    {
      if (parent == NULL)
      {
        lp_bool_t status;
        _lp_assert (gst->audiosink == NULL);

        status = __lp_media_gst_alloc_and_link_mixer ("adder",
                                                      &gst->audiomixer,
                                                      "autoaudiosink",
                                                      &gst->audiosink,
                                                      GST_ELEMENT
                                                      (pipeline));

        _lp_assert (status == TRUE);
      }
      else
      {
        /* TODO */
      }
      mixer = gst->audiomixer;
    }
  }

  return mixer;
}

/* Allocates and links appropriate elements to handle video streams 
   Returns %TRUE if successful, or %FALSE otherwise */
static lp_bool_t
__lp_media_gst_set_video_bin (lp_media_t *media, GstPad *source_pad)
{
  GstElement *videomixer = NULL;
  GstCaps *caps = NULL;
  GstPad *sink_pad = NULL, *ghost_pad = NULL, *videomixer_sink_pad = NULL;
  GstPadLinkReturn ret;
  lp_media_t *parent;
  lp_media_gst_t *gst;
  int x = 0, y = 0, z = 0, width = 0, height = 0;
  lp_bool_t status = TRUE;
  double alpha;

  gst = __lp_media_gst_check (media);
  parent = lp_media_get_parent (media);

  videomixer = __lp_media_gst_get_mixer (parent, "video");
  _lp_assert (videomixer != NULL);

  gst->videoscale = gst_element_factory_make ("videoscale", NULL);
  assert (gst->videoscale != NULL);

  gst->videofilter = gst_element_factory_make ("capsfilter", NULL);
  assert (gst->videofilter != NULL);

  gst_element_set_state (gst->videoscale, GST_STATE_PAUSED);
  gst_element_set_state (gst->videofilter, GST_STATE_PAUSED);

  lp_media_get_property_int (media, "x", &x);
  lp_media_get_property_int (media, "y", &y);
  lp_media_get_property_int (media, "z", &z);
  lp_media_get_property_int (media, "width", &width);
  lp_media_get_property_int (media, "height", &height);
  lp_media_get_property_double (media, "alpha", &alpha);

  caps = gst_caps_new_simple ("video/x-raw",
                              "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                              "width", G_TYPE_INT, width,
                              "height", G_TYPE_INT, height, NULL);

  g_object_set (G_OBJECT (gst->videoscale), "add-borders", 0, NULL);
  g_object_set (G_OBJECT (gst->videofilter), "caps", caps, NULL);
  gst_caps_unref (caps);

  gst_bin_add_many (GST_BIN (gst->bin), gst->videoscale,
                    gst->videofilter, NULL);
  if (!gst_element_link (gst->videoscale, gst->videofilter))
    status = FALSE;
  else
  {
    /* TODO: check if the uri is an image */
    sink_pad = gst_element_get_static_pad (gst->videoscale, "sink");
    _lp_assert (sink_pad != NULL);

    ret = gst_pad_link (source_pad, sink_pad);
    if (GST_PAD_LINK_FAILED (ret))
      status = FALSE;
    else
    {
      ghost_pad =
        gst_ghost_pad_new ("v_src",
                           gst_element_get_static_pad (gst->videofilter,
                                                       "src"));

      gst_pad_set_active (ghost_pad, TRUE);
      gst_element_add_pad (gst->bin, ghost_pad);

      videomixer_sink_pad =
        gst_element_get_request_pad (videomixer, "sink_%u");
      _lp_assert (videomixer_sink_pad != NULL);

      ret = gst_pad_link (ghost_pad, videomixer_sink_pad);
      if (GST_PAD_LINK_FAILED (ret))
        status = FALSE;
      else
      {
        g_object_set (videomixer_sink_pad, "xpos", x, NULL);
        g_object_set (videomixer_sink_pad, "ypos", y, NULL);
        g_object_set (videomixer_sink_pad, "zorder", z, NULL);
        g_object_set (videomixer_sink_pad, "alpha", alpha, NULL);

        gst_element_set_state (gst->videoscale, GST_STATE_PLAYING);
        gst_element_set_state (gst->videofilter, GST_STATE_PLAYING);
      }

      gst_object_unref (videomixer_sink_pad);
      gst_object_unref (sink_pad);
    }
  }

  return status;
}

/* Allocates and links appropriate elements to handle audio streams 
   Returns %TRUE if successful, or %FALSE otherwise */
static lp_bool_t
__lp_media_gst_set_audio_bin (lp_media_t *media, GstPad *source_pad)
{
  GstElement *audiomixer = NULL;
  GstCaps *caps = NULL;
  GstPad *sink_pad = NULL, *ghost_pad = NULL, *audiomixer_sink_pad = NULL;
  GstPadLinkReturn ret;
  lp_media_t *parent;
  lp_media_gst_t *gst;
  lp_bool_t status = TRUE;

  gst = __lp_media_gst_check (media);
  parent = lp_media_get_parent (media);

  audiomixer = __lp_media_gst_get_mixer (parent, "audio");
  _lp_assert (audiomixer != NULL);

  gst->audiovolume = gst_element_factory_make ("volume", NULL);
  _lp_assert (gst->audiovolume != NULL);

  gst->audioconvert = gst_element_factory_make ("audioconvert", NULL);
  _lp_assert (gst->audioconvert != NULL);

  gst->audioresample = gst_element_factory_make ("audioresample", NULL);
  _lp_assert (gst->audioresample != NULL);

  gst->audiofilter = gst_element_factory_make ("capsfilter", NULL);
  _lp_assert (gst->audiofilter != NULL);

  gst_element_set_state (gst->audiovolume, GST_STATE_PAUSED);
  gst_element_set_state (gst->audioconvert, GST_STATE_PAUSED);
  gst_element_set_state (gst->audioresample, GST_STATE_PAUSED);
  gst_element_set_state (gst->audiofilter, GST_STATE_PAUSED);

  caps = gst_caps_from_string (default_audio_caps);
  _lp_assert (caps != NULL);

  g_object_set (gst->audiofilter, "caps", caps, NULL);
  gst_caps_unref (caps);

  gst_bin_add_many (GST_BIN (gst->bin), gst->audiovolume, gst->audioconvert,
                    gst->audioresample, gst->audiofilter, NULL);

  if (!gst_element_link_many (gst->audiovolume, gst->audioconvert,
                              gst->audioresample, gst->audiofilter, NULL))
    status = FALSE;
  else
  {
    sink_pad = gst_element_get_static_pad (gst->audiovolume, "sink");
    _lp_assert (sink_pad != NULL);

    ret = gst_pad_link (source_pad, sink_pad);
    if (GST_PAD_LINK_FAILED (ret))
      status = FALSE;
    else
    {
      double volume;
      lp_media_get_property_double (media, "volume", &volume);

      g_object_set (G_OBJECT (gst->audiovolume), "volume", volume, NULL);

      ghost_pad =
        gst_ghost_pad_new ("a_src",
                       gst_element_get_static_pad(gst->audiofilter, "src"));

      gst_pad_set_active (ghost_pad, TRUE);
      gst_element_add_pad (gst->bin, ghost_pad);

      audiomixer_sink_pad =
        gst_element_get_request_pad (audiomixer, "sink_%u");
      _lp_assert (audiomixer_sink_pad != NULL);

      ret = gst_pad_link (ghost_pad, audiomixer_sink_pad);
      if (GST_PAD_LINK_FAILED (ret))
        status = FALSE;
      else
      {
        gst_element_set_state (gst->audiovolume, GST_STATE_PLAYING);
        gst_element_set_state (gst->audioconvert, GST_STATE_PLAYING);
        gst_element_set_state (gst->audioresample, GST_STATE_PLAYING);
        gst_element_set_state (gst->audiofilter, GST_STATE_PLAYING);
      }

      gst_object_unref (audiomixer_sink_pad);
      gst_object_unref (sink_pad);
    }
  }

  return status;
}

static GstPadProbeReturn
__lp_media_gst_eos_event_callback (GstPad *pad, GstPadProbeInfo * info,
                                   gpointer data)
{
  (void) pad;
  (void) info;
  (void) data;
  return GST_PAD_PROBE_OK;
}


/* Requests an asynchronous stop of @media.
   Returns %TRUE if successful, or %FALSE if @media cannot stop.  */

static lp_bool_t
__lp_media_gst_stop_async (lp_media_t *media)
{
  GstStateChangeReturn status;
  GstPipeline *pipeline;

  pipeline = __lp_media_gst_get_pipeline (media);
  if (pipeline == NULL)
    return FALSE;

  if (GST_STATE (pipeline) != GST_STATE_PLAYING
      && GST_STATE (pipeline) != GST_STATE_PAUSED)
    return FALSE;

  status = gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_READY);
  if (unlikely (status == GST_STATE_CHANGE_FAILURE))
    return FALSE;

  return TRUE;
}

/* Requests an asynchronous start of @media.
   Returns %TRUE if successful, or %FALSE if @media cannot start.  */

static lp_bool_t
__lp_media_gst_start_async (lp_media_t *media)
{
  GstStateChangeReturn status;
  GstPipeline *pipeline;
  lp_media_t *parent;

  parent = lp_media_get_parent (media);

  pipeline = __lp_media_gst_get_pipeline (media);
  if (pipeline == NULL)
  {
    lp_media_t *root;
    root = _lp_media_get_root_ancestor (media);

    _lp_assert (__lp_media_gst_install_pipeline (root));
    pipeline = __lp_media_gst_get_pipeline (root);
    _lp_assert (pipeline != NULL);

    status = gst_element_set_state (GST_ELEMENT (pipeline),
                                    GST_STATE_PLAYING);
    if (unlikely (status == GST_STATE_CHANGE_FAILURE))
      return FALSE;
  }

  if (parent != NULL)
  {
    lp_media_t *root;
    lp_media_gst_t *gst;
    lp_media_gst_t *root_gst;

    const char *uri;

    gst = __lp_media_gst_check (media);

    gst->bin = gst_bin_new (NULL);
    gst->decoder = gst_element_factory_make ("uridecodebin", NULL);

    _lp_assert (gst->bin);
    _lp_assert (gst->decoder);

    gst_bin_add (GST_BIN (gst->bin), gst->decoder);
    gst_bin_add (GST_BIN (pipeline), gst->bin);

    uri = lp_media_get_content_uri (media);

    g_object_set_data (G_OBJECT (gst->bin), "lp_media", media);
    g_object_set (G_OBJECT (gst->decoder), "uri", uri, NULL);
    g_signal_connect (G_OBJECT (gst->decoder), "pad-added",
                      G_CALLBACK (__lp_media_gst_pad_added_callback),
                      media);

    root = _lp_media_get_root_ancestor (parent);
    _lp_assert (root != NULL);
    
    root_gst = __lp_media_gst_check (root);
    _lp_assert (root_gst);

    gst->start_offset = gst_clock_get_time (root_gst->clock) - 
     gst_element_get_base_time (GST_ELEMENT (pipeline));

    if (unlikely (gst->start_offset == GST_CLOCK_TIME_NONE))
      gst->start_offset = 0;

    status = gst_element_set_state (GST_ELEMENT (gst->bin),
                                    GST_STATE_PLAYING);
    if (unlikely (status == GST_STATE_CHANGE_FAILURE))
      return FALSE;

  }
  return TRUE;
}

/************************** GStreamer callbacks ***************************/

/* Links #GstPad created dynamically */
static void
__lp_media_gst_pad_added_callback (arg_unused(GstElement *src),
                                   GstPad *pad, gpointer data)
{
  GstCaps *pad_caps = NULL;
  GstStructure *pad_struct = NULL;
  GstPad *peer = NULL;
  const gchar *pad_type = NULL;
  lp_bool_t status;
  lp_media_t *root = NULL;
  lp_media_t *media = (lp_media_t *) data;
  lp_media_gst_t *gst = NULL;


  _lp_assert (media != NULL);
  gst = __lp_media_gst_check (media);

  root = _lp_media_get_root_ancestor (media); 


  pad_caps = gst_pad_query_caps (pad, NULL);
  pad_struct = gst_caps_get_structure (pad_caps, 0);
  pad_type = gst_structure_get_name (pad_struct);

  _lp_media_lock (root);
  
  
  if (g_str_has_prefix (pad_type, "video"))
    status = __lp_media_gst_set_video_bin (media, pad);
  else if (g_str_has_prefix (pad_type, "audio"))
    status = __lp_media_gst_set_audio_bin (media, pad);
  
  _lp_media_unlock (root);

  _lp_assert (status);

  if (gst->start_offset > 0)
    gst_pad_set_offset (pad, (gint64) gst->start_offset);

  peer = gst_pad_get_peer (pad);

  gst_pad_add_probe (peer,
                     GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
                     __lp_media_gst_eos_event_callback, media, NULL);

}

/* Handles #GstPipeline clock ticks.  */

static ATTR_UNUSED gboolean
__lp_media_gst_pipeline_clock_callback (arg_unused (GstClock *clock),
                                        GstClockTime time,
                                        arg_unused (GstClockID id),
                                        gpointer data)
{
  lp_media_t *media;
  lp_event_t tick;

  _lp_assert (data != NULL);
  time = time - gst_element_get_base_time (GST_ELEMENT (data));
  g_print ("tick (thread %p): %" GST_TIME_FORMAT "\n",
           (void *) g_thread_self (), GST_TIME_ARGS (time));

  media = (lp_media_t *) g_object_get_data (G_OBJECT (data), "lp_media");
  _lp_assert (media != NULL);

  lp_event_init_tick (&tick);
  _lp_media_dispatch (media, &tick);
  return TRUE;
}

/* Handles #GstBus asynchronous messages.  */

static gboolean
__lp_media_gst_pipeline_bus_async_callback (arg_unused (GstBus *bus),
                                            GstMessage *message,
                                            arg_unused (gpointer data))
{
  GstObject *obj;
  lp_media_t *media;
  lp_media_gst_t *gst;

  (void) gst;

  /* gstx_dump_message ("async", message); */

  obj = GST_MESSAGE_SRC (message);
  _lp_assert (obj != NULL);

  media = (lp_media_t *) g_object_get_data (G_OBJECT (obj), "lp_media");
  if (media != NULL)
    gst = __lp_media_gst_check (media);

  switch (GST_MESSAGE_TYPE (message))
  {
    case GST_MESSAGE_STATE_CHANGED:
    {
      GstState new_state;
      GstState old_state;
      lp_event_t event;

      if (media == NULL)
        break;                  /* nothing to do */

      gst_message_parse_state_changed (message, &old_state,
                                       &new_state, NULL);

      if (new_state == GST_STATE_PLAYING)
      {
        lp_event_init_start (&event);
      }
      else if (old_state == GST_STATE_PAUSED
               && new_state == GST_STATE_READY)
      {
        GstStateChangeReturn status;
        status = gst_element_set_state (GST_ELEMENT (obj), GST_STATE_NULL);
        if (unlikely (status == GST_STATE_CHANGE_FAILURE))
          return FALSE;

        lp_event_init_stop (&event);
      }
      else
      {
        break;
      }

      _lp_media_dispatch (media, &event);
      break;
    }
    case GST_MESSAGE_NEW_CLOCK:
    {
      GstClock *clock;
      GstClockID id;
      GstClockTime time;
      GstClockReturn ret;

      _lp_assert (GST_IS_PIPELINE (obj));
      _lp_assert (media != NULL);
      _lp_assert (gst != NULL);

      clock = gst_pipeline_get_clock (GST_PIPELINE (obj));
      _lp_assert (clock != NULL);

      time = gst_clock_get_time (clock);
      _lp_assert (time != GST_CLOCK_TIME_NONE);

      id = gst_clock_new_periodic_id (clock, time, 1 * GST_SECOND);
      _lp_assert (id != NULL);
      g_object_unref (clock);

      ret = gst_clock_id_wait_async
        (id, __lp_media_gst_pipeline_clock_callback, obj, NULL);
      _lp_assert (ret == GST_CLOCK_OK);
      if (gst->clock_id != NULL)
      {
        gst_clock_id_unschedule (gst->clock_id);
        gst_clock_id_unref (gst->clock_id);
      }
      gst->clock_id = id;
      break;
    }
    default:
      break;
  }

  return TRUE;
}

/* Handles #GstBus synchronous messages.  */

static GstBusSyncReply
__lp_media_gst_pipeline_bus_sync_callback (arg_unused (GstBus *bus),
                                           arg_unused (GstMessage *message),
                                           arg_unused (gpointer data))
{
  /* gstx_dump_message ("sync", message); */
  return GST_BUS_PASS;
}

/*************************** Back-end callbacks ***************************/

/* Frees @media's back-end data.  */

static void
__lp_media_gst_free_func (void *data)
{
  lp_media_gst_t *gst = (lp_media_gst_t *) data;
  if (gst->clock_id)
  {
    /* TODO: Release thread.  */
    gst_clock_id_unschedule (gst->clock_id);
    gst_clock_id_unref (gst->clock_id);
  }

  if (gst->decoder)
    gst_object_unref (gst->decoder);

  if (gst->audiovolume)
    gst_object_unref (gst->audiovolume);

  if (gst->audioconvert)
    gst_object_unref (gst->audioconvert);

  if (gst->audioresample)
    gst_object_unref (gst->audioresample);

  if (gst->audiofilter)
    gst_object_unref (gst->audiofilter);

  if (gst->videoscale)
    gst_object_unref (gst->videoscale);

  if (gst->videofilter)
    gst_object_unref (gst->videofilter);

  if (gst->imagefreeze)
    gst_object_unref (gst->imagefreeze);

  if (gst->bin)
    gst_object_unref (gst->bin);

  if (gst->clock != NULL)
    gst_object_unref (gst->clock);

  if (gst->pipeline != NULL)
    gst_object_unref (gst->pipeline);

  g_mutex_clear (&gst->mutex);

  g_free (gst);
}

/* Posts @event to @media.  */

static lp_bool_t
__lp_media_gst_post_func (lp_media_t *media, lp_event_t *event)
{
  _lp_assert (media != NULL);
  _lp_assert (event != NULL);

  switch (event->type)
  {
    case LP_EVENT_START:
      __lp_media_gst_start_async (media);
      break;
    case LP_EVENT_STOP:
      __lp_media_gst_stop_async (media);
      break;
    case LP_EVENT_USER:
      _lp_media_dispatch (media, event);
      break;
    default:
      _LP_ASSERT_NOT_REACHED;
  }

  return TRUE;
}

/*************************** Internal functions ***************************/

/* Allocates initializes #lp_media_backend_t.  */

void
_lp_media_gst_init (lp_media_backend_t *backend)
{
  lp_media_gst_t *gst;

  if (!gst_is_initialized ())
    gst_init (NULL, NULL);

  _lp_assert (backend != NULL);
  _lp_assert (backend->media != NULL);
  _lp_assert (backend->data == NULL);
  _lp_assert (backend->free == NULL);
  _lp_assert (backend->add_child == NULL);
  _lp_assert (backend->remove_child == NULL);
  _lp_assert (backend->post == NULL);
  _lp_assert (backend->get_property == NULL);
  _lp_assert (backend->set_property == NULL);

  gst = (lp_media_gst_t *) g_malloc (sizeof (*gst));
  _lp_assert (gst != NULL);
  memset (gst, 0, sizeof (*gst));

  g_mutex_init (&gst->mutex);

  backend->data = gst;
  backend->free = __lp_media_gst_free_func;
  backend->post = __lp_media_gst_post_func;
}
