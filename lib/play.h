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

#ifndef LIBPLAY_H
#define LIBPLAY_H

#include <playconf.h>

LP_BEGIN_DECLS

/* value */

typedef enum _lp_value_type_t
{
  LP_VALUE_INT = 0,
  LP_VALUE_DOUBLE,
  LP_VALUE_STRING
} lp_value_type_t;

typedef struct _lp_value_t
{
  lp_value_type_t type;
  union
  {
    int i;
    double d;
    char *s;
  } u;
} lp_value_t;

LP_API void
lp_value_init_int (lp_value_t *, int);

LP_API void
lp_value_init_double (lp_value_t *, double);

LP_API void
lp_value_init_string (lp_value_t *, const char *);

LP_API int
lp_value_equals (const lp_value_t *, const lp_value_t *);

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

LP_API lp_media_t *
lp_media_create (const char *uri);

LP_API void
lp_media_destroy (lp_media_t *);

LP_API lp_media_t *
lp_media_reference (lp_media_t *);

LP_API unsigned int
lp_media_get_reference_count (const lp_media_t *);

LP_END_DECLS

#endif /* LIBPLAY_H */
