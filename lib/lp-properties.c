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

typedef struct _lp_properties_desc_t
{
  const char *name;
  GType type;
  int has_default;
  union {
    int i;
    double d;
    char *s;
    void *p;
  } default_value;
} lp_properties_desc_t;

static const lp_properties_desc_t known_properties[] =
{
  /* KEEP THIS SORTED ALPHABETICALLY */
  {"height", G_TYPE_INT, TRUE, {LP_PROPERTY_DEFAULT_HEIGHT}},
  {"width",  G_TYPE_INT, TRUE, {LP_PROPERTY_DEFAULT_WIDTH}},
};

#if 0
static ATTR_PURE int
__lp_properties_desc_compar (const void *desc1, const void *desc2)
{
  return strcmp (((const lp_properties_desc_t *) desc1)->name,
                 ((const lp_properties_desc_t *) desc2)->name);
}

/* Checks if property @name is a known property.
   If successful, store its description in @desc and returns %TRUE.
   Otherwise, returns %FALSE.  */

static int
__lp_properties_desc (const char *name, lp_properties_desc_t **desc)
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
#endif


/* Internal functions.  */

/* Allocates a new (empty) #lp_properties_t.
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

/* Checks if property @name is in @props.
   If successful, stores its current value into @value and returns %TRUE.
   Otherwise, returns %FALSE.  */

ATTR_USE_RESULT int
_lp_properties_get (lp_properties_t *props, const char *name,
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

/* Sets property @name to @value in @props.
   Returns %TRUE if successful, or %FALSE otherwise.  */

ATTR_USE_RESULT int
_lp_properties_set (lp_properties_t *props, const char *name,
                     GValue *value)
{
  if (unlikely (props == NULL))
    return FALSE;

  if (unlikely (name == NULL))
    return FALSE;

  if (unlikely (value == NULL))
    return FALSE;

  g_hash_table_insert (props->hash, g_strdup (name),
                       _lp_util_g_value_dup (value));
  return TRUE;
}

/* Resets all properties in #lp_property_t to their default values.  */

void
_lp_properties_reset_all (lp_properties_t *props)
{
  GHashTable *hash;
  size_t i;

  if (unlikely (props == NULL))
    return;

  hash = props->hash;
  g_hash_table_remove_all (hash);
  for (i = 0; i < nelementsof (known_properties); i++)
    {
      const lp_properties_desc_t *desc;
      GValue *value;

      desc = &known_properties[i];
      if (!desc->has_default)
        continue;

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
      assert (g_hash_table_insert (hash, g_strdup (desc->name), value));
    }
}
