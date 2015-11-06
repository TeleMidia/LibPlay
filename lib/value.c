/* value.c -- Value object.
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

#include "play.h"
#include "macros.h"

/*-
 * Initializes VALUE with the given integer value.
 */
void
lp_value_init_int (lp_value_t *value, int i)
{
  value->type = LP_VALUE_INT;
  value->u.i = i;
}

/*-
 * Initializes VALUE with the given double value.
 */
void
lp_value_init_double (lp_value_t *value, double d)
{
  value->type = LP_VALUE_DOUBLE;
  value->u.d = d;
}

/*-
 * Initializes VALUE with the given string value.
 */
void
lp_value_init_string (lp_value_t *value, const char *str)
{
  value->type = LP_VALUE_STRING;
  value->u.s = deconst (char *, str);
}

/*-
 * Returns true if values V1 and V2 are equal, i.e, if they have the same
 * type and carry identical data.  Otherwise, returns false.
 */
ATTR_PURE int
lp_value_equals (const lp_value_t *v1, const lp_value_t *v2)
{
  if (v1->type != v2->type)
    return FALSE;
  switch (v1->type)
    {
    case LP_VALUE_INT:
      return v1->u.i == v2->u.i;

    case LP_VALUE_DOUBLE:
      return memcmp (&v1->u.i, &v2->u.i, sizeof (double)) == 0;

    case LP_VALUE_STRING:
      if (v1->u.s == NULL && v2->u.s == NULL)
        return TRUE;
      if (v1->u.s == NULL || v2->u.s == NULL)
        return FALSE;
      else
        return streq (v1->u.s, v2->u.s);

    default:
      ASSERT_NOT_REACHED;
    }
}
