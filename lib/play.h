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
typedef enum lp_Event_Mouse_Button_Type lp_Event_Mouse_Button_Type;
typedef enum lp_Event_Key_Type lp_Event_Key_Type;

struct _lp_EventClass
{
  GObjectClass parent_class;
};

struct _lp_Event
{
  GObject parent_instance;
  lp_EventPrivate *priv;
};

typedef enum
{
  LP_EVENT_KEY_PRESS = 0,
  LP_EVENT_KEY_RELEASE,
} lp_EventKeyType;

LP_API GType
lp_event_get_type (void) G_GNUC_CONST;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (lp_Event, g_object_unref)

#define LP_TYPE_EVENT_TICK (lp_event_tick_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventTick, lp_event_tick,
                             LP, EVENT_TICK, lp_Event)

#define LP_TYPE_EVENT_ERROR (lp_event_error_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventError, lp_event_error,
                             LP, EVENT_ERROR, lp_Event)

#define LP_TYPE_EVENT_START (lp_event_start_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventStart, lp_event_start,
                             LP, EVENT_START, lp_Event)

#define LP_TYPE_EVENT_STOP (lp_event_stop_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventStop, lp_event_stop,
                             LP, EVENT_STOP, lp_Event)

#define LP_TYPE_EVENT_MOUSE_BUTTON (lp_event_mouse_button_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventMouseButton, lp_event_mouse_button,
                             LP, EVENT_MOUSE_BUTTON, lp_Event)

#define LP_TYPE_EVENT_KEY (lp_event_key_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_EventKey, lp_event_key,
                             LP, EVENT_KEY, lp_Event)

#define LP_TYPE_SCENE (lp_scene_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_Scene, lp_scene, LP, SCENE, GObject)

#define LP_TYPE_MEDIA (lp_media_get_type ())
LP_API G_DECLARE_FINAL_TYPE (lp_Media, lp_media, LP, MEDIA, GObject)

/* event */

LP_API GObject *
lp_event_get_source (lp_Event *);

lp_EventTick *
lp_event_tick_new (lp_Scene *, guint64);

lp_EventError *
lp_event_error_new (lp_Media *, GError *);

LP_API lp_EventStart *
lp_event_start_new (lp_Media *, gboolean);

LP_API lp_EventStop *
lp_event_stop_new (lp_Media *, gboolean);

LP_API lp_EventMouseButton *
lp_event_mouse_button_new (lp_Scene *, double, double, int,
                           gboolean);

LP_API lp_EventKey *
lp_event_key_new (lp_Scene *, const char *key, lp_EventKeyType);

guint64
lp_event_tick_get_serial (lp_EventTick *);

GError *
lp_event_error_get_error (lp_EventError *);

gboolean
lp_event_start_is_resume (lp_EventStart *);

gboolean
lp_event_stop_is_eos (lp_EventStop *);

/* scene */

LP_API lp_Scene *
lp_scene_new (gint, gint);

LP_API gboolean
lp_scene_advance (lp_Scene *, guint64);

LP_API lp_Event *
lp_scene_receive (lp_Scene *, gboolean);

/* media */

LP_API lp_Media *
lp_media_new (lp_Scene *, const gchar *);

LP_API gboolean
lp_media_start (lp_Media *);

LP_API gboolean
lp_media_stop (lp_Media *);

LP_END_DECLS

#endif /* PLAY_H */
