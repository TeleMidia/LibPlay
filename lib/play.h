/* play.h -- Simple multimedia library.
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

#ifndef PLAY_H
#define PLAY_H

#include <glib-object.h>

#ifdef  __cplusplus
# define LP_BEGIN_DECLS extern "C" { /* } */
# define LP_END_DECLS           /* { */ }
#else
# define LP_BEGIN_DECLS
# define LP_END_DECLS
#endif

#if defined LP_BUILDING && defined LP_HAVE_VISIBILITY
# define LP_API __attribute__((__visibility__("default")))
#elif defined LP_BUILDING && defined _MSC_VER && !defined LP_STATIC
# define LP_API __declspec(dllexport)
#elif defined _MSC_VER && !defined LP_STATIC
# define LP_API __declspec(dllimport)
#else
# define LP_API
#endif

LP_BEGIN_DECLS

#include <playconf.h>

/* version */

#define LP_VERSION_ENCODE(major, minor, micro)\
  (((major) * 10000) + ((minor) * 100) + ((micro) * 1))

#define LP_VERSION\
  LP_VERSION_ENCODE (LP_VERSION_MAJOR, LP_VERSION_MINOR, LP_VERSION_MICRO)

#define LP_VERSION_TOSTRING_(major, minor, micro)\
  #major"."#minor"."#micro

#define LP_VERSION_TOSTRING(major, minor, micro)\
  LP_VERSION_TOSTRING_ (major, minor, micro)

#define LP_VERSION_STRING\
  LP_VERSION_TOSTRING (LP_VERSION_MAJOR, LP_VERSION_MINOR, LP_VERSION_MICRO)

LP_API gint
lp_version (void);

LP_API const gchar *
lp_version_string (void);

#define LP_CLOCK_TIME_INVALID ((guint64) - 1)

/* types */

#define LP_TYPE_EVENT\
  (lp_event_get_type ())

#define LP_EVENT(obj)\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), LP_TYPE_EVENT, lp_Event))

#define LP_EVENT_CLASS(cls)\
  (G_TYPE_CHECK_CLASS_CAST((cls), LP_TYPE_EVENT, lp_EventClass))

#define LP_IS_EVENT(obj)\
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LP_TYPE_EVENT))

#define LP_IS_EVENT_CLASS(cls)\
  (G_TYPE_CHECK_CLASS_TYPE ((cls), LP_TYPE_EVENT))

#define LP_EVENT_GET_CLASS(obj)\
  (G_TYPE_INSTANCE_GET_CLASS ((obj), LP_TYPE_EVENT, lp_EventClass))

typedef struct _lp_Event lp_Event;
typedef struct _lp_EventClass lp_EventClass;
typedef struct _lp_EventPrivate lp_EventPrivate;

struct _lp_EventClass
{
  GObjectClass parent_class;
  gchar *(*to_string) (lp_Event *);
};

struct _lp_Event
{
  GObject parent_instance;
  lp_EventPrivate *priv;
};

LP_API GType
lp_event_get_type (void) G_GNUC_CONST;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (lp_Event, g_object_unref)

typedef enum
{
  LP_EVENT_MASK_NONE          = 0,
  LP_EVENT_MASK_QUIT          = (1 << 0),
  LP_EVENT_MASK_TICK          = (1 << 1),
  LP_EVENT_MASK_KEY           = (1 << 2),
  LP_EVENT_MASK_POINTER_CLICK = (1 << 3),
  LP_EVENT_MASK_POINTER_MOVE  = (1 << 4),
  LP_EVENT_MASK_ERROR         = (1 << 5),
  LP_EVENT_MASK_START         = (1 << 6),
  LP_EVENT_MASK_STOP          = (1 << 7),
  LP_EVENT_MASK_SEEK          = (1 << 8),
  LP_EVENT_MASK_PAUSE         = (1 << 9),
  LP_EVENT_MASK_ANY           = (gint)(0xfffffffff)
} lp_EventMask;

#define LP_TYPE_EVENT_QUIT (lp_event_quit_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventQuit, lp_event_quit,
                             LP, EVENT_QUIT, lp_Event)

#define LP_TYPE_EVENT_TICK (lp_event_tick_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventTick, lp_event_tick,
                             LP, EVENT_TICK, lp_Event)

#define LP_TYPE_EVENT_KEY (lp_event_key_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventKey, lp_event_key,
                             LP, EVENT_KEY, lp_Event)

#define LP_TYPE_EVENT_POINTER_CLICK (lp_event_pointer_click_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventPointerClick, lp_event_pointer_click,
                             LP, EVENT_POINTER_CLICK, lp_Event)

#define LP_TYPE_EVENT_POINTER_MOVE (lp_event_pointer_move_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventPointerMove, lp_event_pointer_move,
                             LP, EVENT_POINTER_MOVE, lp_Event)

#define LP_TYPE_EVENT_ERROR (lp_event_error_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventError, lp_event_error,
                             LP, EVENT_ERROR, lp_Event)

#define LP_TYPE_EVENT_START (lp_event_start_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventStart, lp_event_start,
                             LP, EVENT_START, lp_Event)

#define LP_TYPE_EVENT_STOP (lp_event_stop_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventStop, lp_event_stop,
                             LP, EVENT_STOP, lp_Event)

#define LP_TYPE_EVENT_SEEK (lp_event_seek_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventSeek, lp_event_seek,
                             LP, EVENT_SEEK, lp_Event)

#define LP_TYPE_EVENT_PAUSE (lp_event_pause_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventPause, lp_event_pause,
                             LP, EVENT_PAUSE, lp_Event)

/**
 * lp_Error:
 * @LP_ERROR_START: Error while starting.
 * @LP_ERROR_STOP: Error while stopping.
 * @LP_ERROR_SEEK: Error while seeking.
 * @LP_ERROR_PAUSE: Error while pausing.
 * @LP_ERROR_LAST: Total number of #lp_Error types.
 *
 * Types of asynchronous errors.
 */
typedef enum
{
  LP_ERROR_START = 0,           /* error while starting */
  LP_ERROR_STOP,                /* error while stopping */
  LP_ERROR_SEEK,                /* error while seeking */
  LP_ERROR_PAUSE,               /* error while pausing */
  LP_ERROR_LAST,                /* total number of error codes */
} lp_Error;

#define LP_ERROR lp_error_quark ()
LP_API
GQuark lp_error_quark (void);

#define LP_TYPE_MEDIA (lp_media_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_Media, lp_media, LP, MEDIA, GObject)

#define LP_TYPE_SCENE (lp_scene_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_Scene, lp_scene, LP, SCENE, GObject)

/* event */

LP_API lp_EventMask
lp_event_get_mask (lp_Event *);

LP_API GObject *
lp_event_get_source (lp_Event *);

LP_API gchar *
lp_event_to_string (lp_Event *);

/* media */

LP_API lp_Media *
lp_media_new (lp_Scene *, const gchar *);

LP_API gchar *
lp_media_to_string (lp_Media *);

LP_API gboolean
lp_media_start (lp_Media *);

LP_API gboolean
lp_media_stop (lp_Media *);

LP_API gboolean
lp_media_seek (lp_Media *, gboolean, gint64);

LP_API guint64
lp_media_get_running_time (lp_Media *);

LP_API guint64
lp_media_get_start_time (lp_Media *);

LP_API gboolean
lp_media_pause (lp_Media *);

/* scene */

LP_API lp_Scene *
lp_scene_new (gint, gint);

LP_API gchar *
lp_scene_to_string (lp_Scene *);

LP_API gboolean
lp_scene_advance (lp_Scene *, guint64);

LP_API lp_Event *
lp_scene_receive (lp_Scene *, gboolean);

LP_API void
lp_scene_quit (lp_Scene *);

LP_API gboolean
lp_scene_pause (lp_Scene *);

LP_API gboolean
lp_scene_resume (lp_Scene *);

LP_END_DECLS

#endif /* PLAY_H */
