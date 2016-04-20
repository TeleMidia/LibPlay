/* play.h -- Simple multimedia library.
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

/* Library version.  */
LP_API int
lp_version (void);

LP_API const char *
lp_version_string (void);

/* Scene type.  */
G_DECLARE_FINAL_TYPE (lp_Scene, lp_scene, LP, SCENE, GObject)

/* Gets the GType of an lp_Scene.  */
#define LP_TYPE_SCENE (lp_scene_get_type ())

/* Media type.  */
G_DECLARE_FINAL_TYPE (lp_Media, lp_media, LP, MEDIA, GObject)

/* Gets the GType of an lp_Media.  */
#define LP_TYPE_MEDIA (lp_media_get_type ())

LP_API gboolean
lp_media_start (lp_Media *);

LP_END_DECLS

#endif /* PLAY_H */
