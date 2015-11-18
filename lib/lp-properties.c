/* lp-properties.c -- Property table.
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
#include "play-internal.h"

/* Property table data.  */
struct _lp_properties_t
{
  GHashTable *table;            /* primary table */
  lp_properties_t *metatable;   /* fallback table */
};

/* Property descriptor data.  */
typedef struct _lp_properties_desc_t
{
  const char *name;             /* property name */
  GType type;                   /* property type */
  lp_bool_t inherited;          /* true if property is inherited */
  lp_bool_t has_default;        /* true if property has default */
  union                         /* property default value */
  {
    int i;
    double d;
    char *s;
    void *p;
  } default_value;
} lp_properties_desc_t;

/* *INDENT-ON* */
PRAGMA_DIAG_PUSH ()PRAGMA_DIAG_IGNORE (-Wpedantic)
/* List of known properties.  */
     static const
       lp_properties_desc_t
       known_properties[] = {
       /* KEEP THIS SORTED ALPHABETICALLY */
       {"alpha", G_TYPE_DOUBLE, FALSE, TRUE,
        {.d = _LP_PROPERTY_DEFAULT_ALPHA}},
       {"height", G_TYPE_INT, TRUE, FALSE, {.i = 0}},
       {"width", G_TYPE_INT, TRUE, FALSE, {.i = 0}},
       {"x", G_TYPE_INT, FALSE, TRUE, {.i = _LP_PROPERTY_DEFAULT_X}},
       {"y", G_TYPE_INT, FALSE, TRUE, {.i = _LP_PROPERTY_DEFAULT_Y}},
       {"z", G_TYPE_INT, FALSE, TRUE, {.i = _LP_PROPERTY_DEFAULT_Z}},
     };

PRAGMA_DIAG_POP ()
/* *INDENT-OFF* */

  /* Compares two property descriptors.  */

static ATTR_PURE int
__lp_properties_desc_compar (const void *desc1, const void *desc2)
{
  return strcmp (((const lp_properties_desc_t *) desc1)->name,
                 ((const lp_properties_desc_t *) desc2)->name);
}

/* Gets the descriptor of property @name and stores it in @desc.
   Returns %TRUE if successful, or %FALSE if @name is unknown.  */

static lp_bool_t
__lp_properties_get_desc (const char *name, lp_properties_desc_t **desc)
{
  lp_properties_desc_t key;
  lp_properties_desc_t *match;

  key.name = name;
  match = (lp_properties_desc_t *)
    bsearch (&key, known_properties, nelementsof (known_properties),
             sizeof (*known_properties), __lp_properties_desc_compar);
  if (match == NULL)
    return FALSE;

  set_if_nonnull (desc, match);
  return TRUE;
}

/*************************** Internal functions ***************************/

/* Allocates and returns an empty #lp_properties_t.
   This function always returns a valid pointer.  */

ATTR_USE_RESULT lp_properties_t *
_lp_properties_alloc (void)
{
  lp_properties_t *props;
  GHashTable *table;

  table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                 (GDestroyNotify) g_free,
                                 (GDestroyNotify) _lp_util_g_value_free);
  _lp_assert (table != NULL);

  props = (lp_properties_t *) g_malloc (sizeof (*props));
  _lp_assert (props != NULL);
  props->table = table;
  props->metatable = NULL;

  return props;
}

/* Frees a #lp_properties_t allocated with _lp_properties_alloc().  */

void
_lp_properties_free (lp_properties_t *props)
{
  if (unlikely (props == NULL))
    return;
  g_hash_table_destroy (props->table);
  g_free (props);
}

/* Returns #lp_properties_t metatable.  */

ATTR_PURE ATTR_USE_RESULT lp_properties_t *
_lp_properties_get_metatable (const lp_properties_t *props)
{
  _lp_assert (props != NULL);
  return props->metatable;
}

/* Sets #lp_properties_t metatable to @metatab