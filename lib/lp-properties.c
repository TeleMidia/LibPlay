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
  GHashTable *hash;             /* hash table */
};

/* Property descriptor data.  */
typedef struct _lp_properties_desc_t
{
  const char *name;             /* property name */
  GType type;                   /* property type */
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
  {"height", G_TYPE_INT, TRUE, {LP_PROPERTY_DEFAULT_HEIGHT}},
  {"width", G_TYPE_INT, TRUE, {LP_PROPERTY_DEFAULT_WIDTH}},
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

/* Sets #lp_properties_t property @name to the default value of @desc.
   Returns %TRUE if successful, or %FALSE if @desc has no default value.  */

static int
__lp_properties_set_desc (lp_properties_t *prop, const char *name,
                          const lp_properties_desc_t *desc)
{
  GValue *value;

  assert (streq (name, desc->name));

  if (unlikely (!desc->has_default))
    return FALSE;

  value = _lp_util_g_value_alloc (desc->type);
  switch (desc->type)
    {
    case G_TYPE_INT:
      g_value_set_int (value, desc->default_value.i);
      break;
    case G_TYPE_DOUBLE:
      g_value_set_double (value, desc->default_value.d);
      break;
    case G_TYPE_STRING:
      g_value_set_string (value, desc->default_value.s);
      break;
    case G_TYPE_POINTER:
      g_value_set_pointer (value, desc->default_value.p);
      break;
    default:
      ASSERT_NOT_REACHED;
    }

  g_hash_table_insert (prop->hash, g_strdup (desc->name), value);
  return TRUE;
}

/*************************** Internal functions ***************************/

/* Allocates and returns an empty #lp_properties_t.
   This function always returns a valid pointer.  */

ATTR_USE_RESULT lp_properties_t *
_lp_properties_alloc (void)
{
  lp_properties_t *props;
  GHashTable *hash;

  hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                (GDestroyNotify) g_free,
                                (GDestroyNotify) _lp_util_g_value_free);
  assert (hash != NULL);

  props = (lp_properties_t *) g_malloc (sizeof (*props));
  assert (props != NULL);
  props->hash = hash;

  return props;
}

/* Frees a #lp_properties_t allocated with _lp_properties_alloc().  */

void
_lp_properties_free (lp_properties_t *props)
{
  if (unlikely (props == NULL))
    return;

  g_hash_table_destroy (props->hash);
  g_free (props);
}

/* Returns the number of key-value pairs defined in #lp_properties_t.  */

ATTR_USE_RESULT unsigned int
_lp_properties_size (const lp_properties_t *props)
{
  if (unlikely (props == NULL))
    return 0;

  return g_hash_table_size (props->hash);
}

/* Gets #lp_properties_t property @name and stores it in @desc.
   Returns %TRUE if successful, or %FALSE if @name is not set.  */

ATTR_USE_RESULT int
_lp_properties_get (const lp_properties_t *props, const char *name,
                    GValue *value)
{
  GValue *result;

  if (unlikely (props == NULL))
    return FALSE;

  if (unlikely (name == NULL))
    return FALSE;

  result = (GValue *) g_hash_table_lookup (props->hash, name);
  if (result == NULL)
    return FALSE;

  if (value != NULL)
  {
    g_value_init (value, G_VALUE_TYPE (result));
    g_value_copy (result, value);
  }

  return TRUE;
}

/* Sets #lp_properties_t property @name to @value.
   Returns %TRUE if successful, or %FALSE otherwise.  */

ATTR_USE_RESULT int
_lp_properties_set (lp_properties_t *props, const char *name,
                    const GValue *value)
{
  lp_properties_desc_t *desc;

  if (unlikely (props == NULL))
    return FALSE;

  if (unlikely (name == NULL))
    return FALSE;

  if (unlikely (value == NULL))
    return FALSE;

  if (unlikely (__lp_properties_get_desc (name, &desc)
                && G_VALUE_TYPE (value) != desc->type))
    {
      return FALSE;             /* bad type */
    }

  g_hash_table_insert (props->hash, g_strdup (name),
                       _lp_util_g_value_dup (value));
  return TRUE;
}

/* Resets #lp_properties_t property @name to its default value.
   Returns %TRUE if successful, or %FALSE otherwise.  */

ATTR_USE_RESULT int
_lp_properties_reset (lp_properties_t *props, const char *name)
{
  lp_properties_desc_t *desc;

  if (unlikely (props == NULL))
    return FALSE;

  if (unlikely (name == NULL))
    return FALSE;

  if (!__lp_properties_get_desc (name, &desc))
    return FALSE;

  return __lp_properties_set_desc (props, name, desc);
}

/* Resets all #lp_properties_t properties to their default values.  */

void
_lp_properties_reset_all (lp_properties_t *props)
{
  size_t i;

  if (unlikely (props == NULL))
    return;

  g_hash_table_remove_all (props->hash);
  for (i = 0; i < nelementsof (known_properties); i++)
    {
      const lp_properties_desc_t *desc = &known_properties[i];
      __lp_properties_set_desc (props, desc->name, desc);
    }
}
