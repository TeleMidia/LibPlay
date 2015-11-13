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
  GHashTable *table;             /* primary table */
  lp_properties_t *metatable;    /* fallback table */
};

/* Property descriptor data.  */
typedef struct _lp_properties_desc_t
{
  const char *name;             /* property name */
  GType type;                   /* property type */
  int inherited;                /* true if property is inherited */
  int has_default;              /* true if property has default */
  union                         /* property default value */
  {
    int i;
    double d;
    char *s;
    void *p;
  } default_value;
} lp_properties_desc_t;

/* List of known properties.  */
static const lp_properties_desc_t known_properties[] = {
  /* KEEP THIS SORTED ALPHABETICALLY */
  {"height", G_TYPE_INT, TRUE, FALSE, {NULL}},
  {"width", G_TYPE_INT, TRUE, FALSE, {NULL}},
  {"x", G_TYPE_INT, FALSE, TRUE, {_LP_PROPERTY_DEFAULT_X}},
  {"y", G_TYPE_INT, FALSE, TRUE, {_LP_PROPERTY_DEFAULT_Y}},
  {"z", G_TYPE_INT, FALSE, TRUE, {_LP_PROPERTY_DEFAULT_Z}},
};

/* Compares two property descriptors.  */

static ATTR_PURE int
__lp_properties_desc_compar (const void *desc1, const void *desc2)
{
  return strcmp (((const lp_properties_desc_t *) desc1)->name,
                 ((const lp_properties_desc_t *) desc2)->name);
}

/* Gets the descriptor of property @name and stores it in @desc.
   Returns %TRUE if successful, or %FALSE if @name is unknown.  */

static int
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

/* Sets #lp_properties_t metatable to @metatable.
   Returns the previous metatable value of @props.  */

lp_properties_t *
_lp_properties_set_metatable (lp_properties_t *props,
                              const lp_properties_t *metatable)
{
  lp_properties_t *prev;

  _lp_assert (props != NULL);
  _lp_assert (props != metatable);

  prev = props->metatable;
  props->metatable = deconst (lp_properties_t *, metatable);

  return prev;
}

/* Returns the number of key-value pairs defined in #lp_properties_t.  */

ATTR_USE_RESULT unsigned int
_lp_properties_size (const lp_properties_t *props)
{
  _lp_assert (props != NULL);
  return g_hash_table_size (props->table);
}

/* Gets #lp_properties_t property @name and stores it in @desc.
   Returns %TRUE if successful, or %FALSE if @name is not set.  */

ATTR_USE_RESULT int
_lp_properties_get (const lp_properties_t *props, const char *name,
                    GValue *value)
{
  lp_properties_desc_t *desc;
  GValue default_value = G_VALUE_INIT;
  GValue *result;

  _lp_assert (props != NULL);
  _lp_assert (name != NULL);
  _lp_assert (value != NULL);

  result = (GValue *) g_hash_table_lookup (props->table, name);
  if (result != NULL)
    {
      g_value_init (value, G_VALUE_TYPE (result));
      g_value_copy (result, value);
      return TRUE;
    }

  if (!__lp_properties_get_desc (name, &desc))
    return FALSE;               /* unknown property, nothing to do */

  if (desc->has_default)
    {
      ptrdiff_t ptr = (ptrdiff_t) desc;

      ptr += (ptrdiff_t) offsetof (lp_properties_desc_t, default_value);
      _lp_util_g_value_init_and_set (&default_value, desc->type,
                                     pointerof (ptr));
      g_value_init (value, G_VALUE_TYPE (&default_value));
      g_value_copy (&default_value, value);
      g_value_unset (&default_value);

      return TRUE;
    }

  if (desc->inherited && props->metatable != NULL)
    return _lp_properties_get (props->metatable, name, value);

  return FALSE;
}

/* Sets #lp_properties_t property @name to @value.
   Returns %TRUE if successful, or %FALSE otherwise.  */

ATTR_USE_RESULT int
_lp_properties_set (lp_properties_t *props, const char *name,
                    const GValue *value)
{
  lp_properties_desc_t *desc;
  GType type;

  _lp_assert (props != NULL);
  _lp_assert (name != NULL);
  _lp_assert (G_IS_VALUE (deconst (GValue *, value)));

  type = G_VALUE_TYPE (deconst (GValue *, value));
  if (unlikely (__lp_properties_get_desc (name, &desc)
                && type != desc->type))
    {
      return FALSE;             /* bad type */
    }

  g_hash_table_insert (props->table, g_strdup (name),
                       _lp_util_g_value_dup (value));
  return TRUE;
}

/* Resets #lp_properties_t property @name to its default value.
   Returns %TRUE if successful, or %FALSE otherwise.  */

ATTR_USE_RESULT int
_lp_properties_reset (lp_properties_t *props, const char *name)
{
  _lp_assert (props != NULL);
  _lp_assert (name != NULL);
  return g_hash_table_remove (props->table, name);
}

/* Resets all #lp_properties_t properties to their default values.  */

void
_lp_properties_reset_all (lp_properties_t *props)
{
  _lp_assert (props != NULL);
  g_hash_table_remove_all (props->table);
}
