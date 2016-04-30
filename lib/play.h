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
  LP_VERSION_TOSTRING_(major, minor, micro)

#define LP_VERSION_STRING\
  LP_VERSION_TOSTRING (LP_VERSION_MAJOR, LP_VERSION_MINOR, LP_VERSION_MICRO)

LP_API int
lp_version (void);

LP_API const gchar *
lp_version_string (void);

/* event */

#define LP_TYPE_EVENT\
  (lp_event_get_type ())

#define LP_EVENT(obj)\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), LP_TYPE_EVENT, lp_Event))

#define LP_EVENT_CLASS(cls)\
  (G_TYPE_CHECK_CLASS_CAST ((cls), LP_TYPE_EVENT, lp_EventClass))

#define LP_IS_EVENT(obj)\
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LP_TYPE_EVENT))

#define LP_IS_EVENT_CLASS(cls)\
  (G_TYPE_CHECK_CLASS_TYPE ((cls), LP_TYPE_EVENT))

#define LP_EVENT_GET_CLASS(obj)\
  (G_TYPE_INSTANCE_GET_CLASS ((obj), LP_TYPE_EVENT, lp_EventClass))

typedef struct _lp_Event lp_Event;
typedef struct _lp_EventPrivate lp_EventPrivate;
typedef struct _lp_EventClass lp_EventClass;

struct _lp_Event
{
  GObject parent_instance;
  lp_EventPrivate *priv;
};

struct _lp_EventClass
{
  GObjectClass parent_class;
};

LP_API GType
lp_event_get_type (void);

/* LEGACY */
typedef enum
{
  LP_EERROR,
  LP_ETICK,
  LP_ESTART,
  LP_ESTOP,
  LP_EEOS,
} lp_EEvent;

/* scene */

LP_API G_DECLARE_FINAL_TYPE (lp_Scene, lp_scene, LP, SCENE, GObject)
#define LP_TYPE_SCENE (lp_scene_get_type ())

LP_API lp_Scene *
lp_scene_new (int, int);

LP_API gboolean
lp_scene_pop (lp_Scene *, gboolean, GObject **, lp_EEvent *);

LP_API gboolean
lp_scene_advance (lp_Scene *, guint64);

/* media */

LP_API G_DECLARE_FINAL_TYPE (lp_Media, lp_media, LP, MEDIA, GObject)
#define LP_TYPE_MEDIA (lp_media_get_type ())

LP_API lp_Media *
lp_media_new (lp_Scene *, const gchar *);

LP_API gboolean
lp_media_start (lp_Media *);

LP_API gboolean
lp_media_stop (lp_Media *);

LP_API void
lp_media_abort (lp_Media *);

LP_END_DECLS

#endif /* PLAY_H */
