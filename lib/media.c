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
_lp_media_lock (lp_media_t *media)
{
  g_mutex_lock (&media->mutex);
}

void
_lp_media_unlock (lp_media_t *media)
{
  g_mutex_unlock (&media->mutex);
}

static void
__lp_media_destroy_property_value (GValue *value)
{
  g_value_unset (value);
  /* g_free (value); */
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
     (GDestroyNotify) __lp_media_destroy_property_value);
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

/* COMMENT */
ATTR_PURE lp_media_t *
lp_media_get_parent (const lp_media_t *media)
{
  return media->parent;
}

/* COMMENT */
LP_API int
lp_media_get_property_int (lp_media_t *media, const char *name, int *i)
{
  GValue *value;

  value = (GValue *) g_hash_table_lookup (media->properties, name);
  if (value == NULL)
    return FALSE;

  if (G_VALUE_TYPE (value) != G_TYPE_INT)
    return FALSE;

  set_if_nonnull (i, g_value_get_int (value));
  return TRUE;
}

/* COMMENT */
LP_API int
lp_media_set_property_int (lp_media_t *media, const char *name, int i)
{
  GValue value = G_VALUE_INIT;

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, i);
  g_hash_table_insert (media->properties, deconst (gpointer, name),
                       g_memdup (&value, sizeof (value)));
  return TRUE;
}
