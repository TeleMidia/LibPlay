/* lp-media.c -- Media object.
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

#include <config.h>
#include "play-internal.h"
#include <gst/app/gstappsrc.h>
PRAGMA_DIAG_IGNORE (-Wunused-macros)

/* Media state.  */
typedef enum
{
  STARTED = 0,                  /* media is playing (has started) */
  STARTING,                     /* media is preparing to play (start) */
  STOPPED,                      /* media is stopped */
  STOPPING,                     /* media is preparing to stop */
  SEEKING,                      /* media is seeking */
  PAUSING,                      /* media is pausing */
  PAUSED,                       /* media is paused */
  DISPOSED                      /* media has been disposed */
} lp_MediaState;

/* Media flags.  */
typedef enum
{
  FLAG_NONE     = 0,                 /* no flags set */
  FLAG_DRAINED  = (1 << 0),          /* media has drained */
  FLAG_FROZEN   = (1 << 1),          /* media is a still image */
  FLAG_ALL      = (gint)(0xffffffff) /* all flags set */
} lp_MediaFlag;

/* Media pad flags.  */
typedef enum
{
  PAD_FLAG_NONE     = 0,                 /* no flags set */
  PAD_FLAG_ACTIVE   = (1 << 0),          /* pad is active */
  PAD_FLAG_BLOCKED  = (1 << 1),          /* pad is blocked */
  PAD_FLAG_FLUSHED  = (1 << 2),          /* pad has been flushed */
  PAD_FLAG_ALL      = (gint)(0xffffffff) /* all flags set */
} lp_MediaPadFlag;

/* Media object.  */
struct _lp_Media
{
  GObject parent;               /* parent object */
  GRecMutex mutex;              /* sync access to media */
  GstElement *bin;              /* container */
  GstElement *decoder;          /* content decoder */
  GstClockTime offset;          /* start time offset */
  lp_MediaState state;          /* current state */
  lp_MediaFlag flags;           /* media flags */
  struct
  {                             /* callback handlers: */
    gulong pad_added;           /* pad-added callback id */
    gulong no_more_pads;        /* no-more-pads callback id */
    gulong autoplug_continue;   /* autoplug-continue callback id */
    gulong drained;             /* drained callback id */
  } callback;
  struct
  {                             /* seek offset: */
    gboolean relative;          /* true if last seek was relative */
    gint64 sum;                 /* accumulated offset */
    gint64 last;                /* last offset */
  } seek;
  struct
  {
    GstElement *appsrc;         /* pushes paused video buffer */
    GstElement *freeze;         /* repeats paused video  buffer */
    GstElement *audiosrc;       /* pushes silence */
    GstBuffer *video_buffer;    /* paused video buffer */
    GstClockTime time;          /* pause time */
  } pause;
  struct
  {                             /* audio output: */
    GstElement *convert;        /* audio convert */
    GstElement *resample;       /* audio resample */
    GstElement *filter;         /* audio filter */
    GstPad *pad;                /* audio pad in bin */
    lp_MediaPadFlag flags;      /* audio pad flags */
  } audio;
  struct
  {                             /* video output: */
    GstElement *freeze;         /* image freeze (optional) */
    GstElement *text;           /* text overlay */
    GstPad *pad;                /* video pad in bin */
    lp_MediaPadFlag flags;      /* video pad flags */
  } video;
  struct
  {                             /* properties: */
    lp_Scene *scene;            /* parent scene */
    gchar *uri;                 /* content URI */
    gchar *final_uri;           /* final URI */
    gint x;                     /* cached x */
    gint y;                     /* cached y */
    gint z;                     /* cached z */
    gint width;                 /* cached width */
    gint height;                /* cached height */
    gdouble alpha;              /* cached alpha */
    gboolean mute;              /* cached mute */
    gdouble volume;             /* cached volume */
    gchar *text;                /* cached text */
    guint text_color;           /* cached text color */
    gchar *text_font;           /* cached text font */
  } prop;
};

/* Maps GStreamer elements to offsets in lp_Media.  */
static const gstx_eltmap_t lp_media_eltmap[] = {
  {"bin",           offsetof (lp_Media, bin)},
  {"uridecodebin",  offsetof (lp_Media, decoder)},
  {NULL, 0},
};

static const gstx_eltmap_t media_eltmap_audio[] = {
  {"audioconvert",  offsetof (lp_Media, audio.convert)},
  {"audioresample", offsetof (lp_Media, audio.resample)},
  {NULL, 0}
};

static const gstx_eltmap_t media_eltmap_video[] = {
  {"textoverlay",   offsetof (lp_Media, video.text)},
  {NULL, 0}
};

static const gstx_eltmap_t media_eltmap_video_freeze[] = {
  {"imagefreeze",   offsetof (lp_Media, video.freeze)},
  {NULL, 0}
};

/* Media properties.  */
enum
{
  PROP_0,
  PROP_SCENE,
  PROP_URI,
  PROP_FINAL_URI,
  PROP_X,
  PROP_Y,
  PROP_Z,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_ALPHA,
  PROP_MUTE,
  PROP_VOLUME,
  PROP_TEXT,
  PROP_TEXT_COLOR,
  PROP_TEXT_FONT,
  PROP_LAST
};

/* Property defaults.  */
#define DEFAULT_SCENE       NULL       /* not initialized */
#define DEFAULT_URI         NULL       /* not initialized */
#define DEFAULT_X           0          /* origin */
#define DEFAULT_Y           0          /* origin */
#define DEFAULT_Z           1          /* lowest order */
#define DEFAULT_WIDTH       0          /* natural width */
#define DEFAULT_HEIGHT      0          /* natural height */
#define DEFAULT_ALPHA       1.0        /* natural alpha */
#define DEFAULT_MUTE        FALSE      /* not muted */
#define DEFAULT_VOLUME      1.0        /* natural volume */
#define DEFAULT_TEXT        NULL       /* not initialized */
#define DEFAULT_TEXT_COLOR  0xffffffff /* white */
#define DEFAULT_TEXT_FONT   NULL       /* not initialized */

/* Define the lp_Media type.  */
GX_DEFINE_TYPE (lp_Media, lp_media, G_TYPE_OBJECT)


/* Media locking and unlocking.  */
#define media_lock(m)    g_rec_mutex_lock (&((m)->mutex))
#define media_unlock(m)  g_rec_mutex_unlock (&((m)->mutex))

/* Media state queries.  */
#define media_state_started(m)    ((m)->state == STARTED)
#define media_state_starting(m)   ((m)->state == STARTING)
#define media_state_stopped(m)    ((m)->state == STOPPED)
#define media_state_stopping(m)   ((m)->state == STOPPING)
#define media_state_seeking(m)    ((m)->state == SEEKING)
#define media_state_pausing(m)    ((m)->state == PAUSING)
#define media_state_paused(m)     ((m)->state == PAUSED)
#define media_state_disposed(m)   ((m)->state == DISPOSED)

/* Media flag access.  */
#define media_flags_init(m, f)    ((m)->flags = (lp_MediaFlag)(f))
#define media_flag_set(m, f)      (media_flags_init ((m), (m)->flags | (f)))
#define media_flag_toggle(m, f)   (media_flags_init ((m), (m)->flags ^ (f)))

#define media_has_drained(m)      ((m)->flags & FLAG_DRAINED)
#define media_toggle_drained(m)   (media_flag_toggle (m, FLAG_DRAINED))
#define media_is_frozen(m)        ((m)->flags & FLAG_FROZEN)
#define media_toggle_frozen(m)    (media_flag_toggle (m, FLAG_FROZEN))

/* Media pad queries and pad-flag access.  */
#define MEDIA_PAD_FLAGS_INIT(p, f)   ((p) = (lp_MediaPadFlag)(f))
#define MEIDA_PAD_FLAG_SET(p, f)     (MEDIA_PAD_FLAGS_INIT ((p), (p) | (f)))
#define MEDIA_PAD_FLAG_TOGGLE(p, f)  (MEDIA_PAD_FLAGS_INIT ((p), (p) ^ (f)))

#define media_has_audio(m)           ((m)->audio.pad != NULL)
#define media_has_video(m)           ((m)->video.pad != NULL)
#define media_pad_flag_active(p)     ((p) & PAD_FLAG_ACTIVE)
#define media_pad_flag_blocked(p)    ((p) & PAD_FLAG_BLOCKED)
#define media_pad_flag_flushed(p)    ((p) & PAD_FLAG_FLUSHED)

#define media_is_flag_set_on_all_pads(m, f)                     \
  (!((media_has_audio ((m)) && !((m)->audio.flags & (f)))       \
     || (media_has_video ((m)) && !((m)->video.flags & (f)))))

#define media_is_flag_not_set_on_all_pads(m, f)                 \
  (!((media_has_audio ((m)) && ((m)->audio.flags & (f)))        \
     || (media_has_video ((m)) && ((m)->video.flags & (f)))))

#define media_toggle_flag_on_all_pads(m, f)             \
  STMT_BEGIN                                            \
  {                                                     \
    if (media_has_audio (m))                            \
      MEDIA_PAD_FLAG_TOGGLE ((m)->audio.flags, (f));    \
    if (media_has_video (m))                            \
      MEDIA_PAD_FLAG_TOGGLE ((m)->video.flags, (f));    \
  }                                                     \
  STMT_END

/* Media run-time data.  */
#define media_reset_run_time_data(m)            \
  STMT_BEGIN                                    \
  {                                             \
    (m)->bin = NULL;                            \
    (m)->decoder = NULL;                        \
    (m)->offset = GST_CLOCK_TIME_NONE;          \
    (m)->state = STOPPED;                       \
    (m)->flags = FLAG_NONE;                     \
    (m)->prop.final_uri = NULL;                 \
    (m)->callback.pad_added = 0;                \
    (m)->callback.no_more_pads = 0;             \
    (m)->callback.autoplug_continue = 0;        \
    (m)->callback.drained = 0;                  \
    (m)->seek.relative = FALSE;                 \
    (m)->seek.sum = 0;                          \
    (m)->seek.last = 0;                         \
    (m)->audio.pad = NULL;                      \
    (m)->audio.flags = PAD_FLAG_NONE;           \
    (m)->video.pad = NULL;                      \
    (m)->video.flags = PAD_FLAG_NONE;           \
  }                                             \
  STMT_END

#define media_release_run_time_data(m)                          \
  STMT_BEGIN                                                    \
  {                                                             \
    if ((m)->audio.pad != NULL                                  \
        && GST_OBJECT_REFCOUNT (GST_OBJECT ((m)->audio.pad)))   \
      {                                                         \
        gst_object_unref ((m)->audio.pad);                      \
      }                                                         \
    if ((m)->video.pad != NULL                                  \
        && GST_OBJECT_REFCOUNT (GST_OBJECT ((m)->video.pad)))   \
      {                                                         \
        gst_object_unref ((m)->video.pad);                      \
      }                                                         \
    gst_object_unref ((m)->bin);                                \
    g_free ((m)->prop.final_uri);                               \
    media_reset_run_time_data ((m));                            \
  }                                                             \
  STMT_END

/* Media property cache.  */
#define media_reset_property_cache(m)           \
  STMT_BEGIN                                    \
  {                                             \
    (m)->prop.scene = DEFAULT_SCENE;            \
    (m)->prop.uri = DEFAULT_URI;                \
    (m)->prop.final_uri = DEFAULT_URI;          \
    (m)->prop.x = DEFAULT_X;                    \
    (m)->prop.y = DEFAULT_Y;                    \
    (m)->prop.z = DEFAULT_Z;                    \
    (m)->prop.width = DEFAULT_WIDTH;            \
    (m)->prop.height = DEFAULT_HEIGHT;          \
    (m)->prop.alpha = DEFAULT_ALPHA;            \
    (m)->prop.mute = DEFAULT_MUTE;              \
    (m)->prop.volume = DEFAULT_VOLUME;          \
    (m)->prop.text = DEFAULT_TEXT;              \
    (m)->prop.text_color = DEFAULT_TEXT_COLOR;  \
    (m)->prop.text_font = DEFAULT_TEXT_FONT;    \
  }                                             \
  STMT_END

#define media_release_property_cache(m)         \
  STMT_BEGIN                                    \
  {                                             \
    g_free ((m)->prop.uri);                     \
    g_free ((m)->prop.text);                    \
    g_free ((m)->prop.text_font);               \
  }                                             \
  STMT_END


/* Get the address of the flags associated with @pad in @media.  */

static ATTR_PURE lp_MediaPadFlag *
media_get_pad_flags (lp_Media *media, GstPad *pad)
{
  if (pad == media->audio.pad)
    return &media->audio.flags;

  if (pad == media->video.pad)
    return &media->video.flags;

  g_assert_not_reached ();
  return NULL;
}

/* Installs probe @func with @mask onto @media pads, and stores the probe
   ids into @id_audio and @id_video if these are non-null.  Returns the
   number of installed probes.  */

static guint
media_install_probe (lp_Media *media, GstPadProbeType type,
                     GstPadProbeCallback func,
                     gulong *id_audio, gulong *id_video)
{
  gulong id;
  guint n;

  n = 0;
  if (media_has_audio (media))
    {
      id = gst_pad_add_probe (media->audio.pad, type, func, media, NULL);
      g_assert (id > 0);
      set_if_nonnull (id_audio, id);
      n++;
    }
  if (media_has_video (media))
    {
      id = gst_pad_add_probe (media->video.pad, type, func, media, NULL);
      g_assert (id > 0);
      set_if_nonnull (id_video, id);
      n++;
    }
  return n;
}


/* callbacks */

/* WARNING: In the callbacks, media must be *LOCKED*.  */

/* Signals that a media pad has been blocked, which in this case happens
   immediately after the corresponding pad is added to the media decoder.
   Here we keep the pad blocked until it is explicit unblocked by
   lp_media_no_more_pads_callback().  */

static GstPadProbeReturn
lp_media_pad_added_block_probe_callback (GstPad *pad, GstPadProbeInfo *info,
                                         lp_Media *media)
{
  lp_MediaPadFlag *flags;

  media_lock (media);

  if (unlikely (!media_state_starting (media)))
    goto unblock;               /* drop residual probes */

  flags = media_get_pad_flags (media, pad);
  g_assert_nonnull (flags);
  g_assert (media_pad_flag_active (*flags));

  if (media_pad_flag_blocked (*flags))
    {
      media_unlock (media);
      return GST_PAD_PROBE_OK;  /* block */
    }

  if (media_pad_flag_flushed (*flags))
    goto unblock;               /* drop residual probes */

  MEDIA_PAD_FLAG_TOGGLE (*flags, PAD_FLAG_FLUSHED); /* flush */
  g_assert (media_state_starting (media));
  gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));
  gst_pad_set_offset (pad, (gint64) media->offset);

  if (media_is_flag_set_on_all_pads (media, PAD_FLAG_FLUSHED))
    {
      lp_EventStart *event = _lp_event_start_new (media, FALSE);
      g_assert_nonnull (event);
      _lp_scene_dispatch (media->prop.scene, LP_EVENT (event));
    }

 unblock:
  media_unlock (media);
  return GST_PAD_PROBE_REMOVE;
}

static GstPadProbeReturn
lp_media_pausing_block_probe_callback (GstPad *pad, 
                                       GstPadProbeInfo *info,
                                       arg_unused(lp_Media *media))
{
  GstCaps *caps = NULL; 
  GstPad *peer = NULL;
  const char *name = NULL;

  media_lock (media);

  caps = gst_pad_get_current_caps (pad);
  peer = gst_pad_get_peer (pad);

  /* Do we really need to post flush events? */
  g_assert (gst_pad_send_event (peer, gst_event_new_flush_start ()));
  g_assert (gst_pad_send_event (peer, gst_event_new_flush_stop (FALSE)));
  
  g_assert (gst_pad_unlink (pad, peer));
  
  name = gst_structure_get_name (gst_caps_get_structure (caps, 0));

  if (g_str_equal (name, "video/x-raw"))
  {
    GstPad *appsrc_pad = NULL;

    media->pause.freeze = gst_element_factory_make ("imagefreeze", 
        NULL);
    media->pause.appsrc = gst_element_factory_make ("appsrc", NULL);

    gst_bin_add_many (GST_BIN(media->bin), media->pause.appsrc, 
        media->pause.freeze, NULL);

    gst_app_src_set_caps (GST_APP_SRC(media->pause.appsrc), caps);
    gst_app_src_push_buffer(GST_APP_SRC(media->pause.appsrc), 
        media->pause.video_buffer);

    g_assert (gst_element_link (media->pause.appsrc, media->pause.freeze));

    appsrc_pad = gst_element_get_static_pad (media->pause.freeze, "src");
    g_assert(gst_pad_link (appsrc_pad, peer) == GST_PAD_LINK_OK);

    gst_object_unref (appsrc_pad);

    gst_element_set_state (media->pause.appsrc, GST_STATE_PLAYING);
    gst_element_set_state (media->pause.freeze, GST_STATE_PLAYING);
  }
  else if (g_str_equal (name, "audio/x-raw"))
  {
    GstPad *audiosrc_pad = NULL;

    media->pause.audiosrc = gst_element_factory_make ("audiotestsrc", NULL);
    g_object_set (media->pause.audiosrc, "wave", 4 /*silence*/, NULL);

    gst_bin_add_many (GST_BIN(media->bin), media->pause.audiosrc, NULL);

    audiosrc_pad = gst_element_get_static_pad (media->pause.audiosrc, "src");
    g_assert(gst_pad_link (audiosrc_pad, peer) == GST_PAD_LINK_OK);

    gst_object_unref (audiosrc_pad);

    gst_element_set_state (media->pause.audiosrc, GST_STATE_PLAYING);
  }

  media->state = PAUSED;

  gst_element_set_state (media->decoder, GST_STATE_PAUSED);
  gst_caps_unref(caps);
  gst_object_unref(peer);

  media_unlock (media);
   
  return GST_PAD_PROBE_REMOVE;
}

static GstPadProbeReturn
lp_media_have_data_probe_callback (GstPad *pad, 
                                   GstPadProbeInfo *info,
                                   arg_unused(lp_Media *media))
{
  GstCaps *caps = NULL;
  const char *name = NULL;

  media_lock (media);
  g_assert (media_state_pausing(media));

  caps = gst_pad_query_caps (pad, NULL);
  name = gst_structure_get_name (gst_caps_get_structure (caps, 0));
  gst_caps_unref (caps);

  if (g_str_equal (name, "video/x-raw"))
  {
    media->pause.video_buffer = 
      gst_buffer_copy (GST_PAD_PROBE_INFO_BUFFER (info));
  }

  gst_pad_add_probe (pad, 
      GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, 
      (GstPadProbeCallback) lp_media_pausing_block_probe_callback, 
      media, NULL);
  
  media_unlock (media);
  return GST_PAD_PROBE_REMOVE;
}


/* Signals that a new pad has been added to media decoder.  Here we build,
   link, and pre-roll the necessary audio and video elements.  */

static void
lp_media_pad_added_callback (arg_unused (GstElement *dec),
                             GstPad *pad, lp_Media *media)
{
  lp_Scene *scene;
  GstCaps *caps;
  const gchar *name;

  media_lock (media);

  g_assert (media_state_starting (media));

  scene = media->prop.scene;
  g_assert_nonnull (scene);

  caps = gst_pad_query_caps (pad, NULL);
  g_assert_nonnull (caps);

  name = gst_structure_get_name (gst_caps_get_structure (caps, 0));
  g_assert_nonnull (name);
  gst_caps_unref (caps);

  if (g_str_equal (name, "audio/x-raw"))
    {
      GstElement *mixer;
      GstPad *sink;
      GstPad *ghost;
      gulong id;

      if (unlikely (media_has_audio (media)))
        {
          _lp_warn ("ignoring extra audio stream");
          goto done;
        }

      _lp_eltmap_alloc_check (media, media_eltmap_audio);

      gstx_bin_add (GST_BIN (media->bin), media->audio.convert);
      gstx_bin_add (GST_BIN (media->bin), media->audio.resample);
      gstx_element_link (media->audio.convert, media->audio.resample);

      sink = gst_element_get_static_pad (media->audio.convert, "sink");
      g_assert_nonnull (sink);

      g_assert (gst_pad_link (pad, sink) == GST_PAD_LINK_OK);
      gst_object_unref (sink);

      pad = gst_element_get_static_pad (media->audio.resample, "src");
      g_assert_nonnull (pad);

      ghost = gst_ghost_pad_new (NULL, pad);
      g_assert_nonnull (ghost);
      gst_object_unref (pad);

      g_assert (gst_pad_set_active (ghost, TRUE));
      media->audio.pad = (GstPad *) g_object_ref (ghost); /* ref ghost */
      g_assert (gst_element_add_pad (media->bin, ghost));
      g_assert_nonnull (media->audio.pad);

      mixer = _lp_scene_get_audio_mixer (scene);
      g_assert_nonnull (mixer);

      sink = gst_element_get_request_pad (mixer, "sink_%u");
      g_assert_nonnull (sink);

      id = gst_pad_add_probe
        (ghost, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
         (GstPadProbeCallback) lp_media_pad_added_block_probe_callback,
         media, NULL);
      g_assert (id > 0);

      g_assert (gst_pad_link (ghost, sink) == GST_PAD_LINK_OK);
      g_object_set (sink,
                    "mute", media->prop.mute,
                    "volume", media->prop.volume, NULL);
      gst_object_unref (sink);
      gstx_element_sync_state_with_parent (media->audio.convert);
      gstx_element_sync_state_with_parent (media->audio.resample);
      MEDIA_PAD_FLAGS_INIT (media->audio.flags,
                            PAD_FLAG_ACTIVE | PAD_FLAG_BLOCKED);
    }
  else if (g_str_equal (name, "video/x-raw"))
    {
      GstElement *mixer;
      GstPad *sink;
      GstPad *ghost;
      gulong id;

      if (!_lp_scene_has_video (scene))
        {
          goto done;            /* nothing to do */
        }

      if (unlikely (media_has_video (media)))
        {
          _lp_warn ("ignoring extra video stream");
          goto done;
        }

      if (media_is_frozen (media))
        {
          _lp_eltmap_alloc_check (media, media_eltmap_video_freeze);
          gstx_bin_add (media->bin, media->video.freeze);

          sink = gst_element_get_static_pad (media->video.freeze, "sink");
          g_assert_nonnull (sink);

          g_assert (gst_pad_link (pad, sink) == GST_PAD_LINK_OK);
          gst_object_unref (sink);

          pad = gst_element_get_static_pad (media->video.freeze, "src");
          g_assert_nonnull (pad);
        }

      _lp_eltmap_alloc_check (media, media_eltmap_video);
      gstx_bin_add (media->bin, media->video.text);

      sink = gst_element_get_static_pad (media->video.text, "video_sink");
      g_assert_nonnull (sink);

      g_assert (gst_pad_link (pad, sink) == GST_PAD_LINK_OK);
      gst_object_unref (sink);

      if (media_is_frozen (media))
        gst_object_unref (pad);

      pad = gst_element_get_static_pad (media->video.text, "src");
      g_assert_nonnull (pad);

      ghost = gst_ghost_pad_new (NULL, pad);
      g_assert_nonnull (ghost);
      gst_object_unref (pad);

      g_assert (gst_pad_set_active (ghost, TRUE));
      media->video.pad = (GstPad *) g_object_ref (ghost); /* ref ghost */
      g_assert (gst_element_add_pad (media->bin, ghost));
      g_assert_nonnull (media->video.pad);

      mixer = _lp_scene_get_video_mixer (scene);
      g_assert_nonnull (mixer);

      sink = gst_element_get_request_pad (mixer, "sink_%u");
      g_assert_nonnull (sink);

      id = gst_pad_add_probe
        (ghost, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
         (GstPadProbeCallback) lp_media_pad_added_block_probe_callback,
         media, NULL);
      g_assert (id > 0);

      g_assert (gst_pad_link (ghost, sink) == GST_PAD_LINK_OK);
      g_object_set
        (sink,
         "xpos", media->prop.x,
         "ypos", media->prop.y,
         "zorder", media->prop.z,
         "alpha", media->prop.alpha, NULL);

      if (media->prop.width > 0)
        g_object_set (sink, "width", media->prop.width, NULL);

      if (media->prop.height > 0)
        g_object_set (sink, "height", media->prop.height, NULL);
      gst_object_unref (sink);

      if (media->prop.text != NULL)
        {
          g_object_set (media->video.text,
                        "text", media->prop.text,
                        "color", media->prop.text_color, NULL);
        }

      if (media->prop.text_font != NULL)
        {
          g_object_set (media->video.text, "font-desc",
                        media->prop.text_font, NULL);
        }

      if (media_is_frozen (media))
        gstx_element_sync_state_with_parent (media->video.freeze);
      gstx_element_sync_state_with_parent (media->video.text);
      MEDIA_PAD_FLAGS_INIT (media->video.flags,
                            PAD_FLAG_ACTIVE | PAD_FLAG_BLOCKED);
    }
  else
    {
      _lp_warn ("unknown stream type: %s", name);
    }

 done:
  media_unlock (media);
}

/* Signals that no more pads will be added to media decoder.  Here we
   dispatch a start lp_Event that will eventually trigger
   _lp_media_finish_start() and put media in state "started".  */

static void
lp_media_no_more_pads_callback (GstElement *dec, lp_Media *media)
{
  media_lock (media);

  g_assert (media_state_starting (media));

  if (unlikely (!media_has_audio (media) && !media_has_video (media)))
    {
      lp_EventError *event = _lp_event_error_new_start_no_pads (media);
      g_assert_nonnull (event);
      _lp_scene_dispatch (media->prop.scene, LP_EVENT (event));
      goto done;
    }

  g_assert (media_is_flag_set_on_all_pads (media, PAD_FLAG_ACTIVE));
  g_assert (media_is_flag_set_on_all_pads (media, PAD_FLAG_BLOCKED));
  media_toggle_flag_on_all_pads (media, PAD_FLAG_BLOCKED); /* unblock */

 done:
  g_signal_handler_disconnect (dec, media->callback.pad_added);
  g_signal_handler_disconnect (dec, media->callback.no_more_pads);
  g_signal_handler_disconnect (dec, media->callback.autoplug_continue);

  media_unlock (media);
}

/* Signals that media decoder has found a new stream, which in this case
   happens immediately before the decoder starts looking for elements to
   decode the stream.  Here we check if the stream corresponds to a still
   image (PNG, JPG, etc.) and, if so, we mark it as frozen.  */

static gboolean
lp_media_autoplug_continue_callback (arg_unused (GstElement *elt),
                                     arg_unused (GstPad *pad),
                                     GstCaps *caps, lp_Media *media)
{
  const gchar *name;

  name = gst_structure_get_name (gst_caps_get_structure (caps, 0));
  g_assert_nonnull (name);

  if (!g_str_has_prefix (name, "image"))
    return TRUE;                /* not an image, nothing to do */

  media_lock (media);
  media_flag_set (media, FLAG_FROZEN); /* freeze */
  media_unlock (media);
  return TRUE;
}

/* Signals that media decoder has drained its source.  Here we stop the
   media object (if it is not frozen).  */

static void
lp_media_drained_callback (GstElement *dec, lp_Media *media)
{
  media_lock (media);

  g_signal_handler_disconnect (dec, media->callback.drained);

  if (media_is_frozen (media))
    goto done;                  /* still image, nothing to do */

  g_assert (!media_has_drained (media));
  media_toggle_drained (media); /* drain */
  lp_media_stop (media);

 done:
  media_unlock (media);
}

/* Signals that a media pad has been blocked, which in this case happens
   immediately after lp_media_stop() is called.  Here we wait until all
   media pads have been blocked, then we dispatch a stop lp_Event that will
   eventually trigger _lp_media_finish_stop() and put media in state
   "stopped".  */

static GstPadProbeReturn
lp_media_stop_block_probe_callback (GstPad *pad,
                                    arg_unused (GstPadProbeInfo *info),
                                    lp_Media *media)
{
  GstPad *peer;
  lp_MediaPadFlag *flags;

  media_lock (media);

  if (unlikely (!media_state_stopping (media)))
    goto done;                  /* drop residual probes */

  flags = media_get_pad_flags (media, pad);
  g_assert_nonnull (flags);
  g_assert (*flags == PAD_FLAG_ACTIVE);

  peer = gst_pad_get_peer (pad);
  g_assert_nonnull (peer);
  g_assert (gst_pad_send_event (peer, gst_event_new_eos ()));
  gst_object_unref (peer);

  g_assert (gst_pad_set_active (pad, FALSE));
  MEDIA_PAD_FLAG_TOGGLE (*flags, PAD_FLAG_ACTIVE); /* deactivate */

  if (media_is_flag_not_set_on_all_pads (media, PAD_FLAG_ACTIVE))
    {
      lp_EventStop *event;
      event = _lp_event_stop_new (media, media_has_drained (media));
      g_assert_nonnull (event);
      _lp_scene_dispatch (media->prop.scene, LP_EVENT (event));
    }

 done:
  media_unlock (media);

  return GST_PAD_PROBE_OK;
}

/* Signals that a media pad has received a flush event, which in this case
   happens after the pad has been sought.  Here we dispatch a seek lp_Event
   that will eventually trigger _lp_media_finish_seek() and put media back
   in state "started".  */

static GstPadProbeReturn
lp_media_seek_flush_probe_callback (GstPad *pad, GstPadProbeInfo *info,
                                    lp_Media *media)
{
  lp_MediaPadFlag *flags;
  GstEvent *evt;

  media_unlock (media);

  g_assert (media_state_seeking (media));

  flags = media_get_pad_flags (media, pad);
  g_assert_nonnull (flags);
  g_assert (media_pad_flag_active (*flags));

  evt = GST_PAD_PROBE_INFO_EVENT (info);
  g_assert_nonnull (evt);

  if (GST_EVENT_TYPE (evt) != GST_EVENT_FLUSH_STOP)
    goto pass;                  /* nothing to do  */

  if (media_pad_flag_flushed (*flags))
    goto remove;                /* drop residual probes */

  MEDIA_PAD_FLAG_TOGGLE (*flags, PAD_FLAG_FLUSHED); /* flush */
  g_assert (media_state_seeking (media));
  gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

  if (media_is_flag_set_on_all_pads (media, PAD_FLAG_FLUSHED))
    {
      lp_EventSeek *event = _lp_event_seek_new (media, media->seek.relative,
                                                media->seek.last);
      g_assert_nonnull (event);
      _lp_scene_dispatch (media->prop.scene, LP_EVENT (event));
    }

 remove:
  media_unlock (media);
  return GST_PAD_PROBE_REMOVE;

 pass:
  media_unlock (media);
  return GST_PAD_PROBE_PASS;
}


/* methods */

static void
lp_media_init (lp_Media *media)
{
  g_rec_mutex_init (&media->mutex);
  media_reset_run_time_data (media);
  media_reset_property_cache (media);
}

static void
lp_media_get_property (GObject *object, guint prop_id,
                       GValue *value, GParamSpec *pspec)
{
  lp_Media *media;

  media = LP_MEDIA (object);
  media_lock (media);

  switch (prop_id)
    {
    case PROP_SCENE:
      g_value_take_object (value, media->prop.scene);
      break;
    case PROP_URI:
      g_value_set_string (value, media->prop.uri);
      break;
    case PROP_FINAL_URI:
      g_value_set_string (value, media->prop.final_uri);
      break;
    case PROP_X:
      g_value_set_int (value, media->prop.x);
      break;
    case PROP_Y:
      g_value_set_int (value, media->prop.y);
      break;
    case PROP_Z:
      g_value_set_int (value, media->prop.z);
      break;
    case PROP_WIDTH:
      g_value_set_int (value, media->prop.width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, media->prop.height);
      break;
    case PROP_ALPHA:
      g_value_set_double (value, media->prop.alpha);
      break;
    case PROP_MUTE:
      g_value_set_boolean (value, media->prop.mute);
      break;
    case PROP_VOLUME:
      g_value_set_double (value, media->prop.volume);
      break;
    case PROP_TEXT:
      g_value_set_string (value, media->prop.text);
      break;
    case PROP_TEXT_COLOR:
      g_value_set_uint (value, media->prop.text_color);
      break;
    case PROP_TEXT_FONT:
      g_value_set_string (value, media->prop.text_font);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }

  media_unlock (media);
}

static void
lp_media_set_property (GObject *object, guint prop_id,
                       const GValue *value, GParamSpec *pspec)
{
  lp_Media *media;

  media = LP_MEDIA (object);
  media_lock (media);

  switch (prop_id)
    {
    case PROP_SCENE:            /* don't take ownership */
      g_assert (media->prop.scene == DEFAULT_SCENE);
      media->prop.scene = (lp_Scene *) g_value_get_object (value);
      g_assert (LP_IS_SCENE (media->prop.scene));
      break;
    case PROP_URI:
      g_assert (media->prop.uri == DEFAULT_URI);
      media->prop.uri = g_value_dup_string (value);
      g_assert_nonnull (media->prop.uri);
      break;
    case PROP_X:
      media->prop.x = g_value_get_int (value);
      break;
    case PROP_Y:
      media->prop.y = g_value_get_int (value);
      break;
    case PROP_Z:
      media->prop.z = g_value_get_int (value);
      break;
    case PROP_WIDTH:
      media->prop.width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      media->prop.height = g_value_get_int (value);
      break;
    case PROP_ALPHA:
      media->prop.alpha = g_value_get_double (value);
      break;
    case PROP_MUTE:
      media->prop.mute = g_value_get_boolean (value);
      break;
    case PROP_VOLUME:
      media->prop.volume = g_value_get_double (value);
      break;
    case PROP_TEXT:
      g_free (media->prop.text);
      media->prop.text = g_value_dup_string (value);
      break;
    case PROP_TEXT_COLOR:
      media->prop.text_color = g_value_get_uint (value);
      break;
    case PROP_TEXT_FONT:
      g_free (media->prop.text_font);
      media->prop.text_font = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }

  if (!media_state_started (media))
    goto done;                  /* nothing to do */

  switch (prop_id)
    {
    case PROP_SCENE:            /* fall through */
    case PROP_URI:
      {
        break;                  /* nothing to do */
      }
    case PROP_X:                /* fall through */
    case PROP_Y:                /* fall through */
    case PROP_Z:                /* fall through */
    case PROP_WIDTH:            /* fall through */
    case PROP_HEIGHT:           /* fall through */
    case PROP_ALPHA:
      {
        GstPad *sink;

        if (!_lp_scene_has_video (media->prop.scene))
          break;                /* nothing to do */

        if (!media_has_video (media))
          break;                /* nothing to do */

        sink = gst_pad_get_peer (media->video.pad);
        g_assert_nonnull (sink);

        switch (prop_id)
          {
          case PROP_X:
            g_object_set (sink, "xpos", media->prop.x, NULL);
            break;
          case PROP_Y:
            g_object_set (sink, "ypos", media->prop.y, NULL);
            break;
          case PROP_Z:
            g_object_set (sink, "zorder", media->prop.z, NULL);
            break;
          case PROP_WIDTH:
            g_object_set (sink, "width", media->prop.width, NULL);
            break;
          case PROP_HEIGHT:
            g_object_set (sink, "height", media->prop.height, NULL);
            break;
          case PROP_ALPHA:
            g_object_set (sink, "alpha", media->prop.alpha, NULL);
            break;
          default:
            g_assert_not_reached ();
          }

        g_object_unref (sink);
        break;
      }
    case PROP_TEXT:             /* fall through */
    case PROP_TEXT_COLOR:       /* fall through */
    case PROP_TEXT_FONT:
      {
        GstElement *text;

        if (!_lp_scene_has_video (media->prop.scene))
          break;                /* nothing to do */

        if (!media_has_video (media))
          break;                /* nothing to do */

        text = media->video.text;
        g_assert_nonnull (text);

        switch (prop_id)
          {
          case PROP_TEXT:
            {
              g_object_set (text, "text", media->prop.text, NULL);
              break;
            }
          case PROP_TEXT_COLOR:
            {
              g_object_set (text, "color", media->prop.text_color, NULL);
              break;
            }
          case PROP_TEXT_FONT:
            {
              char *font = media->prop.text_font;
              g_object_set (text, "font-desc", (font) ? font : "", NULL);
              break;
            }
          default:
            g_assert_not_reached ();
          }
        break;
      }
    case PROP_MUTE:             /* fall through */
    case PROP_VOLUME:
      {
        GstPad *sink;

        if (!media_has_audio (media))
          break;                /* nothing to do */

        sink = gst_pad_get_peer (media->audio.pad);
        g_assert_nonnull (sink);

        switch (prop_id)
          {
          case PROP_MUTE:
            g_object_set (sink, "mute", media->prop.mute, NULL);
            break;
          case PROP_VOLUME:
            g_object_set (sink, "volume", media->prop.volume, NULL);
            break;
          default:
            g_assert_not_reached ();
          }
        g_object_unref (sink);
        break;
      }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }

 done:
  media_unlock (media);
}

static void
lp_media_constructed (GObject *object)
{
  lp_Scene *scene;
  lp_Media *media;

  media = LP_MEDIA (object);

  media_lock (media);
  g_assert (media_state_stopped (media));
  scene = media->prop.scene;
  media_unlock (media);

  g_assert (LP_IS_SCENE (scene));
  _lp_scene_add_media (scene, media);
}

static void
lp_media_dispose (GObject *object)
{
  lp_Scene *scene;
  lp_Media *media;

  media = LP_MEDIA (object);
  media_lock (media);

  if (media_state_disposed (media))
    {
      media_unlock (media);
      return;                   /* drop residual calls */
    }

  scene = media->prop.scene;
  while (!media_state_stopped (media))
    {
      media_unlock (media);
      lp_media_stop (media);
      _lp_scene_step (scene, TRUE);
      media_lock (media);
    }

  g_assert (media_state_stopped (media));
  media_release_property_cache (media);
  media->state = DISPOSED;
  media_unlock (media);

  G_OBJECT_CLASS (lp_media_parent_class)->dispose (object);
}

static void
lp_media_finalize (GObject *object)
{
  lp_Media *media;

  media = LP_MEDIA (object);
  g_assert (media_state_disposed (media));
  g_rec_mutex_clear (&media->mutex);

  G_OBJECT_CLASS (lp_media_parent_class)->finalize (object);
}

static void
lp_media_class_init (lp_MediaClass *cls)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cls);
  gobject_class->get_property = lp_media_get_property;
  gobject_class->set_property = lp_media_set_property;
  gobject_class->constructed = lp_media_constructed;
  gobject_class->dispose = lp_media_dispose;
  gobject_class->finalize = lp_media_finalize;

  g_object_class_install_property
    (gobject_class, PROP_SCENE, g_param_spec_object
     ("scene", "scene", "parent scene", LP_TYPE_SCENE,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_URI, g_param_spec_string
     ("uri", "uri", "content URI",
      DEFAULT_URI,
      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_FINAL_URI, g_param_spec_string
     ("final-uri", "final-uri", "final URI after resolving relative path",
      DEFAULT_URI,
      (GParamFlags)(G_PARAM_READABLE)));

  g_object_class_install_property
    (gobject_class, PROP_X, g_param_spec_int
     ("x", "x", "horizontal position",
      G_MININT, G_MAXINT, DEFAULT_X,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_Y, g_param_spec_int
     ("y", "y", "vertical position",
      G_MININT, G_MAXINT, DEFAULT_Y,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_Z, g_param_spec_int
     ("z", "z", "z-order",
      0, G_MAXINT, DEFAULT_Z,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_WIDTH, g_param_spec_int
     ("width", "width", "width in pixels",
      0, G_MAXINT, DEFAULT_WIDTH,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_HEIGHT, g_param_spec_int
     ("height", "height", "height in pixels",
      0, G_MAXINT, DEFAULT_HEIGHT,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_ALPHA, g_param_spec_double
     ("alpha", "alpha", "transparency factor",
      0.0, 1.0, DEFAULT_ALPHA,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_MUTE, g_param_spec_boolean
     ("mute", "mute", "mute flag",
      DEFAULT_MUTE,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_VOLUME, g_param_spec_double
     ("volume", "volume", "volume factor",
      0.0, 10.0, DEFAULT_VOLUME,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_TEXT, g_param_spec_string
     ("text", "text", "overlay text",
      DEFAULT_TEXT,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_TEXT_COLOR, g_param_spec_uint
     ("text-color", "text color", "overlay text color",
      0, G_MAXUINT, DEFAULT_TEXT_COLOR,
      (GParamFlags)(G_PARAM_READWRITE)));

  g_object_class_install_property
    (gobject_class, PROP_TEXT_FONT, g_param_spec_string
     ("text-font", "text font", "overlay text font",
      DEFAULT_TEXT_FONT,
      (GParamFlags)(G_PARAM_READWRITE)));
}


/* internal */

/* Searches up in the hierarchy of @obj for a media bin.
   Returns the associated media if successful, or NULL otherwise.  */

lp_Media *
_lp_media_find_media (GstObject *obj)
{
  GstObject *parent;
  lp_Media *media;
  gpointer data;

  if (unlikely (obj == NULL))
    return NULL;

  if (GST_IS_BIN (obj) &&
      (data = g_object_get_data (G_OBJECT (obj), "lp_Media")) != NULL)
    {
      return LP_MEDIA (data);
    }

  parent = gst_object_get_parent (obj);
  if (unlikely (parent == NULL))
    return NULL;

  media = _lp_media_find_media (parent);
  gst_object_unref (parent);

  return media;
}

/* Finishes async error in @media.  */

void
_lp_media_finish_error (lp_Media *media)
{
  media_lock (media);

  media->state = STOPPING;
  _lp_media_finish_stop (media);

  media_unlock (media);
}

/* Finishes async start in @media.  */

void
_lp_media_finish_start (lp_Media *media)
{
  media_lock (media);

  g_assert (media_state_starting (media));
  gstx_element_sync_state_with_parent (media->bin);

  g_assert (media_is_flag_set_on_all_pads (media, PAD_FLAG_ACTIVE));
  g_assert (media_is_flag_not_set_on_all_pads (media, PAD_FLAG_BLOCKED));
  g_assert (media_is_flag_set_on_all_pads (media, PAD_FLAG_FLUSHED));
  media_toggle_flag_on_all_pads (media, PAD_FLAG_FLUSHED); /* un-flush */
  media->state = STARTED;

  media_unlock (media);
}

/* Finishes async stop in @media.  */

void
_lp_media_finish_stop (lp_Media *media)
{
  GstElement *pipeline;

  media_lock (media);

  g_assert (media_state_stopping (media));
  g_assert (media->audio.flags == PAD_FLAG_NONE);
  g_assert (media->video.flags == PAD_FLAG_NONE);

  if (media_has_audio (media))
    g_clear_pointer (&media->audio.pad, gst_object_unref);

  if (media_has_video (media))
    g_clear_pointer (&media->video.pad, gst_object_unref);

  pipeline = _lp_scene_get_pipeline (media->prop.scene);
  g_assert_nonnull (pipeline);
  g_assert (gst_bin_remove (GST_BIN (pipeline), media->bin));
  gstx_element_set_state_sync (media->bin, GST_STATE_NULL);
  media_release_run_time_data (media);

  media_unlock (media);
}

/* Finishes async seek in @media.  */

void
_lp_media_finish_seek (lp_Media *media)
{
  media_lock (media);

  g_assert (media_state_seeking (media));
  g_assert (media_is_flag_set_on_all_pads (media, PAD_FLAG_ACTIVE));
  g_assert (media_is_flag_not_set_on_all_pads (media, PAD_FLAG_BLOCKED));
  g_assert (media_is_flag_set_on_all_pads (media, PAD_FLAG_FLUSHED));
  media_toggle_flag_on_all_pads (media, PAD_FLAG_FLUSHED); /* un-flush */
  media->state = STARTED;

  media_unlock (media);
}


/* public */

/**
 * lp_media_new:
 * @scene: the parent #lp_Scene
 * @uri: content URI
 *
 * Creates a new media object.
 *
 * Returns: (transfer full): a new #lp_Media
 */
lp_Media *
lp_media_new (lp_Scene *scene, const gchar *uri)
{
  return LP_MEDIA (g_object_new (LP_TYPE_MEDIA,
                                 "scene", scene,
                                 "uri", uri, NULL));
}

/**
 * lp_media_to_string:
 * @media: an #lp_Media
 *
 * Gets a string representation of @media.
 *
 * Returns: (transfer full): a string representing the media
 */
gchar *
lp_media_to_string (lp_Media *media)
{
  gchar *str;

  media_lock (media);

  str = g_strdup_printf ("\
%s at %p\n\
  scene: %p\n\
  uri: %s\n\
  x: %d\n\
  y: %d\n\
  z: %d\n\
  width: %d\n\
  height: %d\n\
  alpha: %.2lg\n\
  mute: %s\n\
  volume: %.2lg\n\
  text: %s\n\
  text-color: 0x%x\n\
  text-font: %s\n\
",
                         G_OBJECT_TYPE_NAME (media),
                         media,
                         media->prop.scene,
                         media->prop.uri,
                         media->prop.x,
                         media->prop.y,
                         media->prop.z,
                         media->prop.width,
                         media->prop.height,
                         media->prop.alpha,
                         strbool (media->prop.mute),
                         media->prop.volume,
                         media->prop.text,
                         media->prop.text_color,
                         media->prop.text_font);
  g_assert_nonnull (str);

  media_unlock (media);
  return str;
}

/**
 * lp_media_start:
 * @media: an #lp_Media
 *
 * Starts @media asynchronously.
 *
 * Returns: %TRUE if successful, or %FALSE otherwise
 */
gboolean
lp_media_start (lp_Media *media)
{
  GstElement *pipeline;
  GstStateChangeReturn ret;

  media_lock (media);

  if (unlikely (!media_state_stopped (media)))
    goto fail;                  /* nothing to do */

  if (gst_uri_is_valid (media->prop.uri))
    {
      media->prop.final_uri = g_strdup (media->prop.uri);
    }
  else
    {
      GError *error = NULL;
      gchar *final_uri = gst_filename_to_uri (media->prop.uri, &error);
      if (unlikely (final_uri == NULL))
        {
          _lp_warn ("bad URI: %s", media->prop.uri);
          g_error_free (error);
          goto fail;
        }
      media->prop.final_uri = final_uri;
    }

  _lp_eltmap_alloc_check (media, lp_media_eltmap);

  media->callback.pad_added = g_signal_connect
    (media->decoder, "pad-added", /* GstElement */
     G_CALLBACK (lp_media_pad_added_callback), media);
  g_assert (media->callback.pad_added > 0);

  media->callback.no_more_pads = g_signal_connect
    (media->decoder, "no-more-pads", /* GstElement */
     G_CALLBACK (lp_media_no_more_pads_callback), media);
  g_assert (media->callback.no_more_pads);

  media->callback.autoplug_continue = g_signal_connect
    (media->decoder, "autoplug-continue", /* GstURIDecodeBin */
     G_CALLBACK (lp_media_autoplug_continue_callback), media);
  g_assert (media->callback.autoplug_continue > 0);

  media->callback.drained = g_signal_connect
    (media->decoder, "drained", /* GstURIDecodeBin */
     G_CALLBACK (lp_media_drained_callback), media);
  g_assert (media->callback.drained > 0);

  g_object_set_data (G_OBJECT (media->bin), "lp_Media", media);
  g_object_set (media->decoder, "uri", media->prop.final_uri, NULL);
  gstx_bin_add (media->bin, media->decoder);

  pipeline = _lp_scene_get_pipeline (media->prop.scene);
  g_assert_nonnull (pipeline);
  gstx_bin_add (pipeline, media->bin);
  g_assert (gst_object_ref (media->bin) == media->bin);

  media->state = STARTING;
  ret = gst_element_set_state (media->bin, GST_STATE_PAUSED);
  if (unlikely (ret == GST_STATE_CHANGE_FAILURE))
    {
      gstx_element_set_state_sync (media->bin, GST_STATE_NULL);
      gstx_bin_remove (pipeline, media->bin);
      media_release_run_time_data (media);
      goto fail;
    }

  /* To properly synchronize multiple media objects we need to save the 
   * offset at this point */

  media->offset = _lp_scene_get_running_time (media->prop.scene);
  g_assert (GST_CLOCK_TIME_IS_VALID (media->offset));


  media_unlock (media);
  return TRUE;

 fail:
  media_unlock (media);
  return FALSE;
}

/**
 * lp_media_stop:
 * @media: an #lp_Media
 *
 * Stops @media asynchronously.
 *
 * Returns: %TRUE if successful, or %FALSE otherwise
 */
gboolean
lp_media_stop (lp_Media *media)
{
  media_lock (media);

  if (unlikely (!media_state_started (media)))
    goto fail;                  /* nothing to do */

  media->state = STOPPING;
  media_install_probe
    (media, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
     (GstPadProbeCallback) lp_media_stop_block_probe_callback, NULL, NULL);

  media_unlock (media);
  return TRUE;

 fail:
  media_unlock (media);
  return FALSE;
}

/**
 * lp_media_seek:
 * @media: an #lp_Media
 * @relative: %TRUE if offset is relative to current playback time
 * @offset: a relative offset time (in nanoseconds)
 *
 * Seeks in @media asynchronously by @offset.
 *
 * If @relative is %TRUE, @offset is assumed to be relative to the current
 * playback time of @media.  Positive @offset values seek forward from the
 * current time, and negative values seek backward.
 *
 * If @relative is %FALSE, @offset is assumed to be an absolute offset in
 * @media time.  Positive @offset values indicate an offset from @media
 * beginning and negative values indicate an offset from its end.
 *
 * Returns: %TRUE if successful, or %FALSE otherwise
 */
gboolean
lp_media_seek (lp_Media *media, gboolean relative, gint64 offset)
{
  GstQuery *query;
  gboolean seekable;
  gulong id_audio;
  gulong id_video;
  gint64 duration;
  guint64 now;
  gint64 run;
  gint64 sum;
  gint64 abs;
  gint flags;
  gboolean status;

  media_lock (media);

  if (unlikely (!media_state_started (media)))
    goto fail;                  /* nothing to do */

  query = gst_query_new_seeking (GST_FORMAT_TIME);
  g_assert_nonnull (query);
  if (unlikely (!gst_element_query (media->decoder, query)))
    {
      gst_query_unref (query);
      goto fail;                /* not seekable */
    }

  seekable = FALSE;
  gst_query_parse_seeking (query, NULL, &seekable, NULL, NULL);
  gst_query_unref (query);

  if (unlikely (!seekable))
    goto fail;                  /* not seekable */

  duration = GST_CLOCK_TIME_NONE;
  if (!relative && offset < 0.0) /* resolve duration */
    {
      query = gst_query_new_duration (GST_FORMAT_TIME);
      g_assert_nonnull (query);
      if (unlikely (!gst_element_query (media->decoder, query)))
        {
          gst_query_unref (query);
          goto fail;
        }
      gst_query_parse_duration (query, NULL, &duration);
      gst_query_unref (query);
    }

  media->state = SEEKING;
  media_install_probe
    (media, GST_PAD_PROBE_TYPE_EVENT_FLUSH,
     (GstPadProbeCallback) lp_media_seek_flush_probe_callback,
     &id_audio, &id_video);

  now = _lp_scene_get_running_time (media->prop.scene);
  g_assert (GST_CLOCK_TIME_IS_VALID (now));
  run = now - media->offset;
  g_assert (run > 0);
  sum = media->seek.sum;

  if (relative)                 /* offset is relative */
    {
      abs = run + sum + offset;
      if (abs > 0)
        sum += offset;
      else
        abs = 0;
    }
  else                          /* offset is absolute */
    {
      abs = clamp (run + sum, 0, G_MAXINT64);
      if (offset < 0)
        {
          g_assert (GST_CLOCK_STIME_IS_VALID (duration));
          offset = duration + offset;
        }
      sum = sum + offset - abs;
      abs = offset;
    }

  _lp_debug ("\n\
seek (%s) %p\n\
  now: %" GST_TIME_FORMAT "\n\
  run: %" GST_TIME_FORMAT "\n\
  sum: %" GST_STIME_FORMAT "\n\
  abs: %" GST_TIME_FORMAT "\n",
             relative ? "relative" : "absolute",
             media,
             GST_TIME_ARGS (now),
             GST_TIME_ARGS (run),
             GST_STIME_ARGS (sum),
             GST_TIME_ARGS (abs));

  media->seek.relative = relative; /* last seek type */
  media->seek.last = offset;       /* last seek offset */

  flags = 0
    | GST_SEEK_FLAG_FLUSH       /* flush bin */
    | GST_SEEK_FLAG_ACCURATE    /* be accurate */
    | GST_SEEK_FLAG_TRICKMODE;  /* decode only key frames */

  status = gst_element_seek_simple (media->bin, GST_FORMAT_TIME,
                                    (GstSeekFlags) flags, abs);
  if (unlikely (!status))
    {
      if (media_has_audio (media))
        gst_pad_remove_probe (media->audio.pad, id_audio);
      if (media_has_video (media))
        gst_pad_remove_probe (media->video.pad, id_video);
      goto fail;
    }

  /* Update the accumulate offset only in case of success.  */
  media->seek.sum = sum;

  if (media_has_audio (media))
    gst_pad_set_offset (media->audio.pad, now);
  if (media_has_video (media))
    gst_pad_set_offset (media->video.pad, now);

  media_unlock (media);
  return TRUE;

 fail:
  media_unlock (media);
  return FALSE;
}

/**
 * lp_media_get_runnint_time:
 * @media: an #lp_Media
 *
 * Returns the running time of @media.
 *
 * Returns: @media running time
 */
guint64
lp_media_get_running_time (lp_Media *media)
{
  GstClockTime time;

  media_lock (media);

  if (unlikely (!media_state_started (media)))
    goto fail;                  /* nothing to do */

  time = gstx_element_get_clock_time (
      _lp_scene_get_pipeline(media->prop.scene));
  time = time - media->offset;

  media_unlock (media);
  return time;

 fail:
  media_unlock (media);
  return GST_CLOCK_TIME_NONE;
}

gboolean
lp_media_pause (lp_Media *media)
{
  GstIterator *it = NULL;
  GValue value = G_VALUE_INIT;
  gboolean ret = TRUE;

  g_assert (media);

  media_lock(media);
  if (!media_state_started (media))
  {
    ret = FALSE;
    goto finish;
  }

  media->state = PAUSING;
  media->pause.time = _lp_scene_get_running_time (media->prop.scene);
  
  it = gst_element_iterate_src_pads (media->decoder);
  while (gst_iterator_next (it, &value) == GST_ITERATOR_OK)
  {
    GstPad *p = NULL; 

    p = GST_PAD (g_value_get_object (&value));
    gst_pad_add_probe (p, GST_PAD_PROBE_TYPE_BUFFER, 
        (GstPadProbeCallback) lp_media_have_data_probe_callback, 
        media, NULL);
  }
  gst_iterator_free (it);

finish:
  media_unlock(media);

  return ret;
}
