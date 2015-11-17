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
    {                                           \
      NULL,         /* back-end data */         \
      NULL,         /* free */                  \
      NULL,         /* add_child */             \
      NULL,         /* remove_child */          \
      NULL,         /* post */                  \
      NULL,         /* get_property */          \
      NULL,         /* set_property */          \
    }                                           \
  }

static const lp_media_t __lp_media_nil[] = {
  DEFINE_NIL_MEDIA (LP_STATUS_NULL_POINTER),
  DEFINE_NIL_MEDIA (LP_STATUS_READ_ERROR),
  DEFINE_NIL_MEDIA (LP_STATUS_WRITE_ERROR),
  DEFINE_NIL_MEDIA (LP_STATUS_FILE_NOT_FOUND),
  DEFINE_NIL_MEDIA (LP_STATUS_NEGATIVE_COUNT),
  DEFINE_NIL_MEDIA (LP_STATUS_INVALID_PARENT)
};

_LP_STATIC_ASSERT (nelementsof (__lp_media_nil)
                   == LP_STATUS_LAST_STATUS - 1);

/* Forward declarations:  */
/* *INDENT-OFF* */
static lp_media_t *__lp_media_create_in_error (lp_status_t);
static lp_media_t *__lp_media_alloc (const char *);
static void __lp_media_free (lp_media_t *);
static void __lp_media_set_parent (lp_media_t *, lp_media_t *);
static unsigned int __lp_media_dispatch_helper (lp_media_t *, lp_media_t *, lp_event_t *);
static lp_bool_t __lp_media_get_property_helper (lp_media_t *, const char *, GType, void *);
static lp_bool_t __lp_media_set_property_helper (lp_media_t *, const char *, GType, void *);
/* *INDENT-ON* */

/* Checks if @media is valid.  */
#define __lp_media_is_valid(media)\
  ((media) != NULL && !(media)->status)

/* Returns a reference to an invalid #lp_media_t.  */

static ATTR_PURE ATTR_USE_RESULT lp_media_t *
__lp_media_create_in_error (lp_status_t status)
{
  lp_media_t *media;

  _lp_assert (status != LP_STATUS_SUCCESS);
  media = deconst (lp_media_t *,
                   &__lp_media_nil[status - LP_STATUS_NULL_POINTER]);
  _lp_assert (status == media->status);

  return media;
}

/* Allocates and returns a #lp_media_t with the given @uri.
   This function always return a valid pointer.  */

static ATTR_USE_RESULT lp_media_t *
__lp_media_alloc (const char *uri)
{
  lp_media_t *media;

  media = (lp_media_t *) g_malloc (sizeof (*media));
  _lp_assert (media != NULL);
  memset (media, 0, sizeof (*media));

  media->status = LP_STATUS_SUCCESS;
  media->ref_count = 1;
  media->uri = g_strdup (uri);
  media->children = NULL;
  media->handlers = NULL;
  media->properties = _lp_properties_alloc ();
  _lp_assert (media->properties != NULL);
  __lp_media_set_parent (media, NULL);
  _lp_media_gst_init (media);

  return media;
}

/* Frees a #lp_media_t allocated with __lp_media_alloc().  */

static void
__lp_media_free (lp_media_t *media)
{
  _lp_assert (media != NULL);
  g_free (media->uri);
  g_list_free_full (media->children, (GDestroyNotify) lp_media_destroy);
  g_list_free (media->handlers);
  _lp_properties_free (media->properties);
  _lp_assert (media->backend.free != NULL);
  media->backend.free (media->backend.data);
  g_free (media);
}

/* Sets @media parent to @parent if it is a valid pointer, or resets it to
   %NULL if @parent is %NULL.  */

static void
__lp_media_set_parent (lp_media_t *media, lp_media_t *parent)
{
  _lp_assert (media != NULL);
  _lp_assert (media->properties != NULL);

  if (parent == NULL)
    {
      media->parent = NULL;
      _lp_properties_set_metatable (media->properties, NULL);
    }
  else
    {
      media->parent = parent;
      _lp_assert (parent->properties != NULL);
      assert (_lp_properties_get_metatable (media->properties) == NULL);
      _lp_properties_set_metatable (media->properties, parent->properties);
    }
}

/* Helper function used by _lp_media_dispatch().  */

static unsigned int
__lp_media_dispatch_helper (lp_media_t *media, lp_media_t *target,
                            lp_event_t *event)
{
  GList *list;
  unsigned int count;

  _lp_assert (__lp_media_is_valid (media));
  _lp_assert (__lp_media_is_valid (target));
  _lp_assert (event != NULL);

  list = media->handlers;
  count = 0;

  while (list != NULL)
    {
      lp_event_func_t func;

      func = (lp_event_func_t) integralof (list->data);
      _lp_assert (func != NULL);
      count++;

      if (func (media, target, event))
        return count;

      list = g_list_next (list);
    }

  if (media->parent == NULL)
    return count;

  return count + __lp_media_dispatch_helper (media->parent, target, event);
}

/* Helper function used by property getters.  */

static lp_bool_t
__lp_media_get_property_helper (lp_media_t *media, const char *name,
                                GType type, void *ptr)
{
  GValue value = G_VALUE_INIT;

  if (unlikely (!__lp_media_is_valid (media)))
    return FALSE;

  if (unlikely (name == NULL))
    return FALSE;

  if (!_lp_properties_get (media->properties, name, &value))
    return FALSE;

  _lp_assert (G_IS_VALUE (&value));
  if (G_VALUE_TYPE (&value) != type)
    {
      g_value_unset (&value);
      return FALSE;
    }

  switch (type)
    {
    case G_TYPE_INT:
      set_if_nonnull (((int *) ptr), g_value_get_int (&value));
      break;
    case G_TYPE_DOUBLE:
      set_if_nonnull (((double *) ptr), g_value_get_double (&value));
      break;
    case G_TYPE_STRING:
      set_if_nonnull (((char **) ptr), g_value_dup_string (&value));
      break;
    case G_TYPE_POINTER:
      set_if_nonnull (((void **) ptr), g_value_get_pointer (&value));
      break;
    default:
      _LP_ASSERT_NOT_REACHED;
    }
  g_value_unset (&value);
  return TRUE;
}

/* Helper function used by property setters.  */

static lp_bool_t
__lp_media_set_property_helper (lp_media_t *media, const char *name,
                                GType type, void *ptr)
{
  GValue value = G_VALUE_INIT;
  lp_bool_t status;

  if (unlikely (!__lp_media_is_valid (media)))
    return FALSE;

  if (unlikely (name == NULL))
    return FALSE;

  g_value_init (&value, type);
  switch (type)
    {
    case G_TYPE_INT:
      g_value_set_int (&value, *((int *) ptr));
      break;
    case G_TYPE_DOUBLE:
      g_value_set_double (&value, *((double *) ptr));
      break;
    case G_TYPE_STRING:
      g_value_set_string (&value, *((char **) ptr));
      break;
    case G_TYPE_POINTER:
      g_value_set_pointer (&value, *((void **) ptr));
      break;
    default:
      _LP_ASSERT_NOT_REACHED;
    }
  status = _lp_properties_set (media->properties, name, &value);
  g_value_unset (&value);
  return status;
}

/*************************** Internal functions ***************************/

/* Gets the root ancestor of @media.
   Returns @media if it has no parent.  */

lp_media_t *
_lp_media_get_root_ancestor (lp_media_t *media)
{
  _lp_assert (__lp_media_is_valid (media));
  if (media->parent == NULL)
    return media;
  return _lp_media_get_root_ancestor (media->parent);
}

/* Dispatches @event to all handlers registered in @media and its ancestors.
   Stops processing (calling handlers) if any of them returns %TRUE.
   Returns the total number of handlers called.  */

unsigned int
_lp_media_dispatch (lp_media_t *media, lp_event_t *event)
{
  return __lp_media_dispatch_helper (media, media, event);
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
  _lp_assert (media != NULL);
  _lp_assert (lp_media_add_child (parent, media));

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
  if (unlikely (!__lp_media_is_valid (media)))
    return;

  if (unlikely (g_atomic_int_get (&(media)->ref_count) < 0))
    return;

  if (!g_atomic_int_dec_and_test (&media->ref_count))
    return;

  _lp_assert (g_atomic_int_get (&(media)->ref_count) == 0);
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
  _lp_assert (media != NULL);
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
  if (likely (__lp_media_is_valid (media)))
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

  if (unlikely (!__lp_media_is_valid (media)))
    return 0;

  ref_count = g_atomic_int_get (&media->ref_count);
  _lp_assert (ref_count > 0);

  return (unsigned int) ref_count;
}

/*-
 * lp_media_get_parent:
 * @media: a #lp_media_t
 *
 * Returns the parent of @media.
 *
 * Return value: the parent of @media, or %NULL if @media has no parent.
 */
ATTR_PURE ATTR_USE_RESULT lp_media_t *
lp_media_get_parent (const lp_media_t *media)
{
  if (unlikely (!__lp_media_is_valid (media)))
    return NULL;
  return media->parent;
}

/*-
 * lp_media_add_child:
 * @parent: a #lp_media_t
 * @child: a #lp_media_t
 *
 * Appends @child to @parent's children list and sets its parent to @parent.
 *
 * Return value: %TRUE if successful, or %FALSE if @child is already in
 * children list or if its current parent is not %NULL.
 */
lp_bool_t
lp_media_add_child (lp_media_t *parent, lp_media_t *child)
{
  if (unlikely (!__lp_media_is_valid (parent)
                || !__lp_media_is_valid (child)))
    return FALSE;

  if (unlikely (parent == child))
    return FALSE;

  if (unlikely (child->parent != NULL))
    return FALSE;

  if (unlikely (g_list_find (parent->children, child) != NULL))
    return FALSE;

  __lp_media_set_parent (child, parent);
  parent->children = g_list_append (parent->children, child);
  _lp_assert (parent->children != NULL);
  return TRUE;
}

/*-
 * lp_media_remove_child:
 * @parent: a #lp_media_t
 * @child: a #lp_media_t
 *
 * Removes @child from @parent's children list and set its parent to %NULL.
 *
 * Return value: %TRUE if successful, or %FALSE if @child is not in children
 * list.
 */
lp_bool_t
lp_media_remove_child (lp_media_t *parent, lp_media_t *child)
{
  GList *link;

  if (unlikely (!__lp_media_is_valid (parent)))
    return FALSE;

  if (unlikely (child->parent != parent))
    return FALSE;

  link = g_list_find (parent->children, child);
  if (unlikely (link == NULL))
    return FALSE;

  __lp_media_set_parent (child, NULL);
  parent->children = g_list_remove_link (parent->children, link);
  g_list_free (link);
  return TRUE;
}

/*-
 * lp_media_post:
 * @media: a #lp_media_t
 * @event: a #lp_event_t
 *
 * Posts @event to @media.
 *
 * Return value: %TRUE if successful, or %FALSE otherwise.
 */

lp_bool_t
lp_media_post (lp_media_t *media, lp_event_t *event)
{
  if (unlikely (!__lp_media_is_valid (media)))
    return FALSE;

  if (unlikely (event == NULL))
    return FALSE;

  _lp_assert (media->backend.post != NULL);
  return media->backend.post (media, event);
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
lp_bool_t
lp_media_register (lp_media_t *media, lp_event_func_t func)
{
  if (unlikely (!__lp_media_is_valid (media)))
    return FALSE;

  if (unlikely (func == NULL))
    return FALSE;

  if (unlikely (g_list_find (media->handlers, pointerof (func)) != NULL))
    return FALSE;

  media->handlers = g_list_append (media->handlers, pointerof (func));
  _lp_assert (media->handlers != NULL);
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
lp_bool_t
lp_media_unregister (lp_media_t *media, lp_event_func_t func)
{
  GList *link;

  if (unlikely (!__lp_media_is_valid (media)))
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

/*-
 * lp_media_get_property_int:
 * @media: a #lp_media_t
 * @name: property name
 * @i: return value for the property
 *
 * Gets the value of @media property @name and stores it into @i.
 *
 * Return value: %TRUE if successful.  %FALSE if property @name is not
 * defined or its current value is not of type #int.
 */
lp_bool_t
lp_media_get_property_int (lp_media_t *media, const char *name, int *i)
{
  return __lp_media_get_property_helper (media, name, G_TYPE_INT, i);
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
lp_bool_t
lp_media_set_property_int (lp_media_t *media, const char *name, int i)
{
  return __lp_media_set_property_helper (media, name, G_TYPE_INT, &i);
}

/*-
 * lp_media_get_property_double:
 * @media: a #lp_media_t
 * @name: property name
 * @d: return value for the property
 *
 * Gets the value of @media property @name and stores it into @d.
 *
 * Return value: %TRUE if successful.  %FALSE if property @name is not
 * defined or its current value is not of type #double.
 */
lp_bool_t
lp_media_get_property_double (lp_media_t *media, const char *name,
                              double *d)
{
  return __lp_media_get_property_helper (media, name, G_TYPE_DOUBLE, d);
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
lp_bool_t
lp_media_set_property_double (lp_media_t *media, const char *name,
                              double d)
{
  return __lp_media_set_property_helper (media, name, G_TYPE_DOUBLE, &d);
}

/*-
 * lp_media_get_property_string:
 * @media: a #lp_media_t
 * @name: property name
 * @s: return value for the property
 *
 * Gets the value of @media property @name and stores a copy of it into @s.
 * The caller owns the copy and should call free() after done with it.
 *
 * Return value: %TRUE if successful.  %FALSE if property @name is not
 * defined or its current value is not of type #string.
 */
lp_bool_t
lp_media_get_property_string (lp_media_t *media, const char *name,
                              char **s)
{
  return __lp_media_get_property_helper (media, name, G_TYPE_STRING, s);
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
lp_bool_t
lp_media_set_property_string (lp_media_t *media, const char *name,
                              const char *s)
{
  return __lp_media_set_property_helper (media, name, G_TYPE_STRING, &s);
}

/*-
 * lp_media_get_property_pointer:
 * @media: a #lp_media_t
 * @name: property name
 * @p: return value for the property
 *
 * Gets the value of @media property @name and stores it into @p.
 *
 * Return value: %TRUE if successful.  %FALSE if property @name is not
 * defined or its current value is not of type #pointer.
 */
lp_bool_t
lp_media_get_property_pointer (lp_media_t *media, const char *name,
                               void **p)
{
  return __lp_media_get_property_helper (media, name, G_TYPE_POINTER, p);
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
lp_bool_t
lp_media_set_property_pointer (lp_media_t *media, const char *name,
                               const void *p)
{
  return __lp_media_set_property_helper (media, name, G_TYPE_POINTER, &p);
}
