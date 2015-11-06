/* media.c -- Media object.
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

/* Default parent media object.  */
static lp_media_t *default_parent = NULL;

ATTR_PURE lp_media_t *
_lp_media_get_default_parent (void)
{
  return default_parent;
}

void
_lp_media_destroy_default_parent (void)
{
  lp_media_destroy (default_parent);
}

static GValue *
__lp_media_g_value_alloc (GType type)
{
  GValue *value = g_slice_new0 (GValue);
  assert (value != NULL);
  g_value_init (value, type);
  return value;
}

static void
__lp_media_g_value_free (GValue *value)
{
  g_value_unset (value);
  g_slice_free (GValue, value);
}

static lp_media_t *
__lp_media_alloc (const char *uri)
{
  lp_media_t *media;

  media = (lp_media_t *) g_malloc (sizeof (*media));
  assert (media != NULL);
  media->parent = NULL;
  media->refcount = 1;
  g_mutex_init (&media->mutex);
  media->uri = g_strdup (uri);
  media->properties = g_hash_table_new_full
    (g_str_hash, g_str_equal,
     (GDestroyNotify) g_free,
     (GDestroyNotify) __lp_media_g_value_free);
  assert (media->properties != NULL);

  return media;
}

static void
__lp_media_free (lp_media_t *media)
{
  g_mutex_clear (&media->mutex);
  g_free (media->uri);
  g_hash_table_destroy (media->properties);
  g_free (media);
}

/*-
 * lp_media_create:
 * @uri: source URI for the media object
 *
 * Creates a new #lp_media_t with the given source @uri.
 *
 * Return value: a newly allocated #lp_media_t with a reference count of 1.
 * This function never returns %NULL.
 */
lp_media_t *
lp_media_create (const char *uri)
{
  lp_media_t *media;

  if (unlikely (default_parent == NULL))
    {
      default_parent = __lp_media_alloc (uri);
      /* TODO: Iinitialize GStreamer.  */
    }

  media = __lp_media_alloc (uri);
  media->parent = default_parent;

  return media;
}

/*-
 * lp_media_destroy:
 * @media: a #lp_media_t
 *
 * Decreases the reference count on @media by one.  If the result is zero,
 * then @media and all associated resources are freed.
 */
void
lp_media_destroy (lp_media_t *media)
{
  if (unlikely (media == NULL))
    return;
  if (!g_atomic_int_dec_and_test (&media->refcount))
    return;
  __lp_media_free (media);
}

/*-
 * lp_media_reference:
 * @media: a #lp_media_t
 *
 * Increases the reference count on @media by one.  This prevents @media
 * from being destroyed until a matching call to lp_media_destroy() is made.
 *
 * Return value: the referenced #lp_media_t.
 */
lp_media_t *
lp_media_reference (lp_media_t *media)
{
  if (unlikely (media == NULL))
    return media;
  g_atomic_int_inc (&media->refcount);
  return media;
}

/*-
 * lp_media_get_reference_count:
 * @media: a #lp_media_t
 *
 * Returns the current reference count of @media.
 *
 * Return value: the current reference count of @media.
 * If the object is a nil object, 0 will be returned.
 */
unsigned int
lp_media_get_reference_count (const lp_media_t *media)
{
  if (unlikely (media == NULL))
    return 0;
  return (guint) g_atomic_int_get (&media->refcount);
}

/*-
 * lp_media_get_parent:
 * @media: a #lp_media_t
 *
 * Returns the parent of @media.
 *
 * Return value: the parent of @media.
 */
ATTR_PURE lp_media_t *
lp_media_get_parent (const lp_media_t *media)
{
  return media->parent;
}

static int
__lp_media_get_property (lp_media_t *media, const char *name,
                         GType type, void *p)
{
  GValue *value;

  value = (GValue *) g_hash_table_lookup (media->properties, name);
  if (value == NULL)
    return FALSE;

  if (G_VALUE_TYPE (value) != type)
    return FALSE;

  switch (type)
    {
    case G_TYPE_INT:
      set_if_nonnull (((int *) p), g_value_get_int (value));
      break;
    case G_TYPE_DOUBLE:
      set_if_nonnull (((double *) p), g_value_get_double (value));
      break;
    case G_TYPE_STRING:
      set_if_nonnull (((char **) p), g_value_dup_string (value));
      break;
    case G_TYPE_POINTER:
      set_if_nonnull (((void **) p), g_value_get_pointer (value));
      break;
    default:
      ASSERT_NOT_REACHED;
    }
  return TRUE;
}

static int
__lp_media_set_property (lp_media_t *media, const char *name,
                         GType type, void *p)
{
  GValue *value;

  value = __lp_media_g_value_alloc (type);
  switch (type)
    {
    case G_TYPE_INT:
      g_value_set_int (value, *((int *) p));
      break;
    case G_TYPE_DOUBLE:
      g_value_set_double (value, *((double *) p));
      break;
    case G_TYPE_STRING:
      g_value_set_string (value, *((char **) p));
      break;
    case G_TYPE_POINTER:
      g_value_set_pointer (value, *((void **) p));
      break;
    default:
      ASSERT_NOT_REACHED;
    }

  g_hash_table_insert (media->properties, g_strdup (name), value);

  return TRUE;
}

/*-
 * lp_media_get_property_int:
 * @media: a #lp_media_t
 * @name: property name
 * @i: return value for the property
 *
 * Stores the current value of @media property @name into @i.
 *
 * Return value: %TRUE if successful.  %FALSE if property @name is not
 * defined or its current value is not of type #int.
 */
LP_API int
lp_media_get_property_int (lp_media_t *media, const char *name, int *i)
{
  return __lp_media_get_property (media, name, G_TYPE_INT, i);
}

/*-
 * lp_media_set_property_int:
 * @media: a #lp_media_t
 * @name: property name
 * @i: property value (#int)
 *
 * Sets @media property @name to @i.
 *
 * Return value: %TRUE if successful.  %FALSE otherwise.
 */
LP_API int
lp_media_set_property_int (lp_media_t *media, const char *name, int i)
{
  return __lp_media_set_property (media, name, G_TYPE_INT, &i);
}

/*-
 * lp_media_get_property_double:
 * @media: a #lp_media_t
 * @name: property name
 * @d: return value for the property
 *
 * Stores the current value of @media property @name into @d.
 *
 * Return value: %TRUE if successful.  %FALSE if property @name is not
 * defined or its current value is not of type #double.
 */
LP_API int
lp_media_get_property_double (lp_media_t *media, const char *name,
                              double *d)
{
  return __lp_media_get_property (media, name, G_TYPE_DOUBLE, d);
}

/*-
 * lp_media_set_property_double:
 * @media: a #lp_media_t
 * @name: property name
 * @d: property value (#double)
 *
 * Sets @media property @name to @d.
 *
 * Return value: %TRUE if successful.  %FALSE otherwise.
 */
LP_API int
lp_media_set_property_double (lp_media_t *media, const char *name, double d)
{
  return __lp_media_set_property (media, name, G_TYPE_DOUBLE, &d);
}

/*-
 * lp_media_get_property_string:
 * @media: a #lp_media_t
 * @name: property name
 * @s: return value for the property
 *
 * Stores a copy of the current value of @media property @name into @s.
 *
 * Return value: %TRUE if successful.  %FALSE if property @name is not
 * defined or its current value is not of type #string.
 */
LP_API int
lp_media_get_property_string (lp_media_t *media, const char *name, char **s)
{
  return __lp_media_get_property (media, name, G_TYPE_STRING, s);
}

/*-
 * lp_media_set_property_string:
 * @media: a #lp_media_t
 * @name: property name
 * @s: property value (#string)
 *
 * Sets @media property @name to @s.
 *
 * Return value: %TRUE if successful.  %FALSE otherwise.
 */
LP_API int
lp_media_set_property_string (lp_media_t *media, const char *name,
                              const char *s)
{
  return __lp_media_set_property (media, name, G_TYPE_STRING, &s);
}

/*-
 * lp_media_get_property_pointer:
 * @media: a #lp_media_t
 * @name: property name
 * @p: return value for the property
 *
 * Stores a copy of the current value of @media property @name into @p.
 *
 * Return value: %TRUE if successful.  %FALSE if property @name is not
 * defined or its current value is not of type #pointer.
 */
LP_API int
lp_media_get_property_pointer (lp_media_t *media, const char *name,
                               void **p)
{
  return __lp_media_get_property (media, name, G_TYPE_POINTER, p);
}

/*-
 * lp_media_set_property_pointer:
 * @media: a #lp_media_t
 * @name: property name
 * @p: property value (#pointer)
 *
 * Sets @media property @name to @p.
 *
 * Return value: %TRUE if successful.  %FALSE otherwise.
 */
LP_API int
lp_media_set_property_pointer (lp_media_t *media, const char *name, void *p)
{
  return __lp_media_set_property (media, name, G_TYPE_POINTER, &p);
}
