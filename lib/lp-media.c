/* lp-media.c -- Media object.
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

#define DEFINE_NIL_MEDIA(status)                \
  {                                             \
    status,         /* status */                \
    -1,             /* ref_count */             \
    NULL,           /* parent */                \
    NULL,           /* uri */                   \
    NULL,           /* children */              \
    NULL,           /* handlers */              \
    NULL,           /* properties */            \
  }

static const lp_media_t __lp_media_nil[] = {
  DEFINE_NIL_MEDIA (LP_STATUS_NULL_POINTER),
  DEFINE_NIL_MEDIA (LP_STATUS_READ_ERROR),
  DEFINE_NIL_MEDIA (LP_STATUS_WRITE_ERROR),
  DEFINE_NIL_MEDIA (LP_STATUS_FILE_NOT_FOUND),
  DEFINE_NIL_MEDIA (LP_STATUS_NEGATIVE_COUNT),
  DEFINE_NIL_MEDIA (LP_STATUS_INVALID_PARENT)
};

G_STATIC_ASSERT (nelementsof (__lp_media_nil) == LP_STATUS_LAST_STATUS - 1);

/* Forward declarations:  */
static lp_media_t *__lp_media_create_in_error (lp_status_t);
static lp_media_t *__lp_media_alloc (const char *);
static void __lp_media_free (lp_media_t *);

/* Returns a reference to an invalid #lp_media_t.  */

static ATTR_PURE ATTR_USE_RESULT lp_media_t *
__lp_media_create_in_error (lp_status_t status)
{
  lp_media_t *media;

  assert (status != LP_STATUS_SUCCESS);
  media = deconst (lp_media_t *,
                   &__lp_media_nil[status - LP_STATUS_NULL_POINTER]);
  assert (status == media->status);

  return media;
}

/* Allocates and returns a #lp_media_t with the given @uri.
   This function always return a valid pointer.  */

static ATTR_USE_RESULT lp_media_t *
__lp_media_alloc (const char *uri)
{
  lp_media_t *media;

  media = (lp_media_t *) g_malloc (sizeof (*media));
  assert (media != NULL);
  memset (media, 0, sizeof (*media));

  media->status = LP_STATUS_SUCCESS;
  media->ref_count = 1;
  media->parent = NULL;

  media->uri = g_strdup (uri);
  media->children = NULL;
  media->handlers = NULL;
  media->properties = _lp_properties_alloc ();
  assert (media->properties != NULL);

  return media;
}

/* Frees a #lp_media_t allocated with __lp_media_alloc().  */

static void
__lp_media_free (lp_media_t *media)
{
  g_free (media->uri);
  g_list_free_full (media->children, (GDestroyNotify) lp_media_destroy);
  g_list_free (media->handlers);
  _lp_properties_free (media->properties);
  g_free (media);
}

/*************************** Exported functions ***************************/

/*-
 * lp_media_create:
 * @uri: content URI for the object or %NULL
 *
 * Creates a new #lp_media_t.
 * If @uri is not %NULL, sets the object's URI to @uri.
 *
 * Return value: a newly allocated #lp_media_t with a reference count of 1.
 * The caller owns the returned object and should call lp_media_destroy()
 * when finished with it.
 *
 * This function always returns a valid pointer.
 */
ATTR_USE_RESULT lp_media_t *
lp_media_create (const char *uri)
{
  return __lp_media_alloc (uri);
}

/*-
 * lp_media_create_for_parent:
 * @parent: parent #lp_media_t
 * @uri: content URI for the object or %NULL
 *
 * Creates a new #lp_media_t and add it to @parent.
 * If @uri is not %NULL, sets the object content URI to @uri.
 *
 * Return value: a newly allocated #lp_media_t.  The returned object is
 * owned by the @parent; the caller should call lp_media_reference() if the
 * returned object is to be retained after the @parent is destroyed.
 *
 * This function always returns a valid pointer.
 */
ATTR_USE_RESULT lp_media_t *
lp_media_create_for_parent (lp_media_t *parent, const char *uri)
{
  lp_media_t *media;

  if (unlikely (parent == NULL))
    return __lp_media_create_in_error (LP_STATUS_NULL_POINTER);

  if (unlikely (parent->status))
    return __lp_media_create_in_error (LP_STATUS_INVALID_PARENT);

  media = __lp_media_alloc (uri);
  assert (media != NULL);
  parent->children = g_list_append (parent->children, media);
  media->parent = parent;

  return media;
}

/*-
 * lp_media_destroy:
 * @media: a #lp_media_t
 *
 * Decreases the reference count of @media by 1.
 * If the result is 0, then @media and all associated resources are freed.
 */
void
lp_media_destroy (lp_media_t *media)
{
  if (unlikely (!_lp_media_is_valid (media)))
    return;

  if (unlikely (g_atomic_int_get (&(media)->ref_count) < 0))
    return;

  if (!g_atomic_int_dec_and_test (&media->ref_count))
    return;

  assert (g_atomic_int_get (&(media)->ref_count) == 0);
  __lp_media_free (media);
}

/*-
 * lp_media_status:
 * @media: a #lp_media_t
 *
 * Checks whether an error has previously occurred for @media.
 *
 * Return value: the current status of @media, see #lp_status_t.
 */
lp_status_t ATTR_PURE ATTR_USE_RESULT
lp_media_status (const lp_media_t *media)
{
  assert (media != NULL);
  return media->status;
}

/*-
 * lp_media_reference:
 * @media: a #lp_media_t
 *
 * Increases the reference count on @media by 1.  This prevents the
 * #lp_media_t from being destroyed until a matching call to
 * lp_media_destroy() is made.
 *
 * Return value: the referenced #lp_media_t.
 */
lp_media_t *
lp_media_reference (lp_media_t *media)
{
  if (likely (_lp_media_is_valid (media)))
    g_atomic_int_inc (&media->ref_count);
  return media;
}

/*-
 * lp_media_get_reference_count:
 * @media: a #lp_media_t
 *
 * Returns the current reference count of @media.
 *
 * Return value: the current reference count of @media.
 */
ATTR_USE_RESULT unsigned int
lp_media_get_reference_count (const lp_media_t *media)
{
  gint ref_count;

  if (unlikely (!_lp_media_is_valid (media)))
    return 0;

  ref_count = g_atomic_int_get (&media->ref_count);
  assert (ref_count > 0);

  return (unsigned int) ref_count;
}

/*-
 * lp_media_register:
 * @media: a #lp_media_t
 * @func: a #lp_event_func_t
 *
 * Appends @func to @media's event-handler list.
 *
 * Return value: %TRUE if successful, or %FALSE if @func is already
 * in handler list.
 */
int
lp_media_register (lp_media_t *media, lp_event_func_t func)
{
  if (unlikely (!_lp_media_is_valid (media)))
    return FALSE;

  if (unlikely (func == NULL))
    return FALSE;

  if (unlikely (g_list_find (media->handlers, pointerof (func)) != NULL))
    return FALSE;

  media->handlers = g_list_append (media->handlers, pointerof (func));
  assert (media->handlers != NULL);
  return TRUE;
}

/*-
 * lp_media_unregister:
 * @media: a #lp_media_t
 * @func: a #lp_event_func_t
 *
 * Removes @func from @media's event-handler list.
 *
 * Return value: %TRUE if successful, or %FALSE if @func is not in handler
 * list.
 */
int
lp_media_unregister (lp_media_t *media, lp_event_func_t func)
{
  GList *link;

  if (unlikely (!_lp_media_is_valid (media)))
    return FALSE;

  if (unlikely (func == NULL))
    return FALSE;

  link = g_list_find (media->handlers, pointerof (func));
  if (unlikely (link == NULL))
    return FALSE;

  media->handlers = g_list_remove_link (media->handlers, link);
  g_list_free (link);
  return TRUE;
}
