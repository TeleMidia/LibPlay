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

#include <playconf.h>

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

LP_API const char *
lp_version_string (void);

typedef int lp_bool_t;

/* status */

typedef enum _lp_status_t
{
  LP_STATUS_SUCCESS = 0,
  LP_STATUS_NULL_POINTER,
  LP_STATUS_READ_ERROR,
  LP_STATUS_WRITE_ERROR,
  LP_STATUS_FILE_NOT_FOUND,
  LP_STATUS_NEGATIVE_COUNT,
  LP_STATUS_INVALID_PARENT,
  LP_STATUS_LAST_STATUS
} lp_status_t;

LP_API const char *
lp_status_to_string (lp_status_t);

/* event */

typedef enum _lp_event_type_t
{
  LP_EVENT_START = 0,
  LP_EVENT_STOP,
  LP_EVENT_USER
} lp_event_type_t;

typedef struct _lp_event_t
{
  lp_event_type_t type;
} lp_event_t;

LP_API void
lp_event_init_start (lp_event_t *);

LP_API void
lp_event_init_stop (lp_event_t *);

LP_API void
lp_event_init_user (lp_event_t *);

LP_API int
lp_event_equals (const lp_event_t *, const lp_event_t *);

/* media */

typedef struct _lp_media_t lp_media_t;
typedef int (*lp_event_func_t) (lp_media_t *, lp_media_t *, lp_event_t *);

LP_API lp_media_t *
lp_media_create (const char *uri);

LP_API lp_media_t *
lp_media_create_for_parent (lp_media_t *, const char *);

LP_API void
lp_media_destroy (lp_media_t *);

LP_API lp_status_t
lp_media_status (const lp_media_t *);

LP_API lp_media_t *
lp_media_reference (lp_media_t *);

LP_API unsigned int
lp_media_get_reference_count (const lp_media_t *);

LP_API lp_media_t *
lp_media_get_parent (const lp_media_t *);

LP_API int
lp_media_add_child (lp_media_t *, lp_media_t *);

LP_API int
lp_media_remove_child (lp_media_t *, lp_media_t *);

LP_API int
lp_media_post (lp_media_t *, lp_event_t *);

LP_API int
lp_media_register (lp_media_t *, lp_event_func_t);

LP_API int
lp_media_unregister (lp_media_t *, lp_event_func_t);

LP_API int
lp_media_get_property_int (lp_media_t *, const char *, int *);

LP_API int
lp_media_set_property_int (lp_media_t *, const char *, int);

LP_API int
lp_media_get_property_double (lp_media_t *, const char *, double *);

LP_API int
lp_media_set_property_double (lp_media_t *, const char *, double);

LP_API int
lp_media_get_property_string (lp_media_t *, const char *, char **);

LP_API int
lp_media_set_property_string (lp_media_t *, const char *, const char *);

LP_API int
lp_media_get_property_pointer (lp_media_t *, const char *, void **);

LP_API int
lp_media_set_property_pointer (lp_media_t *, const char *, const void *);

LP_END_DECLS

#endif /* PLAY_H */
