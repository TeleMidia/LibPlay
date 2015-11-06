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

LP_BEGIN_DECLS

/* event */

typedef enum _lp_event_type_t
{
  LP_EVENT_START = 0,
  LP_EVENT_STOP
} lp_event_type_t;

typedef struct _lp_event_t
{
  lp_event_type_t type;
} lp_event_t;

LP_API void
lp_event_init_start (lp_event_t *);

LP_API void
lp_event_init_stop (lp_event_t *);

LP_API int
lp_event_equals (const lp_event_t *, const lp_event_t *);

/* media */

typedef struct _lp_media_t lp_media_t;
typedef int (*lp_event_func_t) (lp_media_t *, lp_event_t *);

LP_API lp_media_t *
lp_media_create (const char *uri);

LP_API void
lp_media_destroy (lp_media_t *);

LP_API lp_media_t *
lp_media_reference (lp_media_t *);

LP_API unsigned int
lp_media_get_reference_count (const lp_media_t *);

LP_API lp_media_t *
lp_media_get_parent (const lp_media_t *);

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
lp_media_set_property_pointer (lp_media_t *, const char *, void *);

LP_API void
lp_media_post (lp_media_t *, const lp_event_t *);

LP_API void
lp_media_register (lp_media_t *, lp_event_func_t);

LP_API void
lp_media_unregister (lp_media_t *, lp_event_func_t);

LP_END_DECLS

#endif /* PLAY_H */
