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

static int
__lp_gst_media_set_dimension (lp_media_t *media, const char *dimension,
    GValue *value)
{
  GstElement *videofilter;
  int code = FALSE;

  videofilter = _lp_media_get_element (media, "videofilter");
  if (videofilter != NULL)
  {
    GstCaps *caps;
    GstPad *pad;
    
    pad = gst_element_get_static_pad (videofilter, "sink");
    
    caps = gst_pad_get_current_caps (pad);
    caps = gst_caps_make_writable (caps);
    gst_caps_set_value (caps, dimension, value);

    g_object_set (G_OBJECT(videofilter), "caps", caps, NULL);

    gst_object_unref (caps);

    code = TRUE;
  }

  return code;
}

static int
__lp_gst_media_set_property (lp_media_t *media, const char *property, 
    GValue *value)
{
  int code = FALSE; 
  if ((streq(property, "width") || streq(property, "height")) && 
      G_VALUE_TYPE(value) == G_TYPE_INT)
    code = __lp_gst_media_set_dimension (media, property, value);

  return code;
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
__lp_media_alloc (const char *uri, lp_media_type_t type)
{
  lp_media_t *media;

  media = (lp_media_t *) g_malloc (sizeof (*media));
  assert (media != NULL);
  g_mutex_init (&media->mutex);
  media->parent = NULL;
  media->refcount = 1;
  media->uri = g_strdup (uri);
  media->type = type;
  media->handlers = NULL;
  media->elements = g_hash_table_new_full
    (g_str_hash, g_str_equal,
     (GDestroyNotify) g_free, (GDestroyNotify) gst_object_unref);

  media->properties = g_hash_table_new_full
    (g_str_hash, g_str_equal,
     (GDestroyNotify) g_free, (GDestroyNotify) __lp_media_g_value_free);

  
  assert (media->properties != NULL);
  return media;
}

static void
__lp_media_free (lp_media_t *media)
{
  lp_media_t *parent;

  parent = lp_media_get_parent (media);
  if (parent == NULL)
  {
    g_main_loop_quit (media->loop);
    g_thread_unref (media->loop_thread);
  }
  
  g_thread_unref (media->loop_thread);
  g_main_loop_unref (media->loop);
  g_mutex_clear (&media->mutex);
  g_free (media->uri);
  g_hash_table_destroy (media->elements);
  g_hash_table_destroy (media->properties);
  
  while (media->handlers != NULL)
    media->handlers = g_slist_remove_link (media->handlers, media->handlers);

  g_free (media);
}

static void
__lp_media_pad_added_callback (GstElement * source, GstPad * new_pad,
                               gpointer data)
{
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;
  lp_media_t *media = (lp_media_t *) data;

  assert (media);

  new_pad_caps = gst_pad_query_caps (new_pad, NULL);
  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  new_pad_type = gst_structure_get_name (new_pad_struct);

  if (g_str_has_prefix (new_pad_type, "video"))
  {
    _lp_media_set_video_bin (media, new_pad);
  }
  else if (g_str_has_prefix (new_pad_type, "audio"))
  {
    _lp_media_set_audio_bin (media, new_pad);
  }

  (void) source;
}

static void
__lp_gst_bus_state_change_message (GstMessage *message)
{
  GstState old_state, new_state;

  gst_message_parse_state_changed (message, &old_state, &new_state, NULL);
  if (new_state == GST_STATE_PLAYING)
  {
    const char *type_name = G_OBJECT_TYPE_NAME (G_OBJECT (message->src));
    if (strcmp (type_name, "GstBin") == 0)
    {
      lp_event_t event;
      lp_media_t *media;

      lp_event_init_start (&event);

      media = (lp_media_t *)
        g_object_get_data (G_OBJECT(message->src), "lp_media_t_pointer");
      assert (media);
      
      _lp_media_notify_handlers(media, media, &event);
    }
  }
}

static gpointer
__lp_gst_bus_loop_thread (gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;
  g_main_loop_run (loop);
  g_thread_exit (EXIT_SUCCESS);
  return NULL;
}

static gboolean
__lp_gst_bus_callback (GstBus *bus, GstMessage *message, gpointer data)
{
  (void)bus;
  (void)data;
  switch (GST_MESSAGE_TYPE (message))
  {
    case GST_MESSAGE_STATE_CHANGED:
    {
      __lp_gst_bus_state_change_message (message);
      break;
    }
    case GST_MESSAGE_ELEMENT:
    {
      /* Not supported yet */
    }
    case GST_MESSAGE_APPLICATION:
    {
      /* TODO */
      break;
    }
    case GST_MESSAGE_ERROR:
    {
      GError *err;
      
      gst_message_parse_error (message, &err, NULL);
      g_debug ("Error: %s\n", err->message);
      g_error_free (err);
      break;
    }
    case GST_MESSAGE_EOS:
    {
      /*
       * end-of-stream 
       */
      break;
    }
    default:
      /*
       * unhandled message 
       */
      break;
  }

  return TRUE;
}

static void
__lp_gst_setup (lp_media_t *media)
{
  GstElement *e;
  GstBus *bus;
  
  gst_init (NULL, NULL);

  e = gst_pipeline_new ("pipeline");
  assert (e);

  _lp_media_add_element (media, strdup ("pipeline"), e);  
  
  bus = gst_pipeline_get_bus (GST_PIPELINE (e));
  gst_bus_add_watch (bus, __lp_gst_bus_callback, NULL);
  
  media->loop = g_main_loop_new (NULL, FALSE);

  /* The following code is causing memory leak. */
  media->loop_thread =  g_thread_new (NULL, __lp_gst_bus_loop_thread, 
      media->loop);
  assert (media->loop_thread);
  
  gst_object_unref (bus);
}

void
_lp_media_atom_start (lp_media_t *media)
{
  GstElement *bin = NULL, *decoder = NULL, *pipeline = NULL;
  lp_media_t *parent = NULL;
  GstState parent_state;
  char *name;

  assert (media != NULL);
  assert (media->type == LP_MEDIA_ATOM);

  parent = lp_media_get_parent (media);
  assert (parent != NULL);
  assert (parent->type == LP_MEDIA_SCENE);

  pipeline = _lp_media_get_element (parent, "pipeline");
  assert (pipeline != NULL);
  parent_state = GST_STATE (pipeline);

  lp_media_get_property_string (media, "name", &name);
  bin = gst_bin_new (name);
  assert (bin != NULL);

  decoder = gst_element_factory_make ("uridecodebin", NULL);
  assert (decoder != NULL);

  gst_bin_add (GST_BIN (bin), decoder);

  g_object_set (G_OBJECT (decoder), "uri", media->uri, NULL);
  g_signal_connect (G_OBJECT (decoder), "pad-added",
                    G_CALLBACK (__lp_media_pad_added_callback), media);

  
  g_object_set_data (G_OBJECT(bin), "lp_media_t_pointer", media); 
  
  g_hash_table_insert (media->elements, strdup ("bin"), bin);
  g_hash_table_insert (media->elements, strdup ("decoder"), decoder);

  gst_element_set_state (bin, parent_state);

  if (gst_bin_add (GST_BIN (pipeline), bin))
  {
    media->start_offset =
      gst_clock_get_time (gst_pipeline_get_clock (GST_PIPELINE (pipeline)));

    if (unlikely (media->start_offset == GST_CLOCK_TIME_NONE))
      media->start_offset = 0;

    gst_element_set_state (bin, GST_STATE_PLAYING);
    if (GST_STATE (pipeline) != GST_STATE_PLAYING)
      gst_element_set_state (pipeline, GST_STATE_PLAYING);
  }
}

void
_lp_media_scene_start (lp_media_t *media)
{
  (void) media;
  /* Not implemented yet */
}

void
_lp_media_atom_stop (lp_media_t *media)
{
  (void) media;
  /* Not implemented yet */
}

void
_lp_media_scene_stop (lp_media_t *media)
{
  (void) media;
  /* Not implemented yet */
}

void
_lp_media_notify_handlers (lp_media_t *to_notify, lp_media_t *media, 
    lp_event_t *event)
{
  lp_media_t *parent;
  GSList *l;
  
  for (l = to_notify->handlers; l != NULL; l = l->next)
  {
    lp_event_func_t f;
    f = (lp_event_func_t) integralof (l->data);
    if (f(media, event) == FALSE)
      l = g_slist_remove_link (l, l);
  }

  parent = lp_media_get_parent (to_notify);
  if (parent != NULL)
    _lp_media_notify_handlers (parent, media, event);
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
    default_parent = __lp_media_alloc (NULL, LP_MEDIA_SCENE);

    /* GStreamer stuffs */
    __lp_gst_setup (default_parent);
  }
  media = __lp_media_alloc (uri, LP_MEDIA_ATOM);
  media->parent = default_parent;
  media->loop = media->parent->loop;
  media->loop_thread = media->parent->loop_thread;

  g_thread_ref (media->loop_thread);
  g_main_loop_ref (media->loop);

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

  _lp_media_lock (media);

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
  
  _lp_media_unlock (media);
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

  _lp_media_lock (media);
  g_hash_table_insert (media->properties, g_strdup (name), value);
  __lp_gst_media_set_property (media, name, value);
  
  _lp_media_unlock (media);
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

int
_lp_media_recursive_get_property_int (lp_media_t *media, const char *name,
                                      int *i)
{
  int found;
  found = __lp_media_get_property (media, name, G_TYPE_INT, i);

  if (found == FALSE)
  {
    lp_media_t *parent;
    parent = lp_media_get_parent (media);
    if (parent != NULL)
      found = _lp_media_recursive_get_property_int (parent, name, i);
  }

  return found;
}

int
_lp_media_recursive_get_property_double (lp_media_t *media,
                                         const char *name, double *i)
{
  int found;
  found = __lp_media_get_property (media, name, G_TYPE_DOUBLE, i);

  if (found == FALSE)
  {
    lp_media_t *parent;
    parent = lp_media_get_parent (media);
    if (parent != NULL)
      found = _lp_media_recursive_get_property_double (parent, name, i);
  }

  return found;
}

GstElement *
_lp_media_recursive_get_element (lp_media_t *media, const char *name)
{
  GstElement *e;
  e = _lp_media_get_element (media, name);

  if (e == NULL)
  {
    lp_media_t *parent;
    parent = lp_media_get_parent (media);
    if (parent != NULL)
      e = _lp_media_recursive_get_element (parent, name);
  }

  return e;
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
lp_media_set_property_double (lp_media_t *media, const char *name,
                              double d)
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
lp_media_get_property_string (lp_media_t *media, const char *name,
                              char **s)
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
lp_media_set_property_pointer (lp_media_t *media, const char *name,
                               void *p)
{
  return __lp_media_set_property (media, name, G_TYPE_POINTER, &p);
}

/*-
 * lp_media_register:
 * @media: a #lp_media_t
 * @handler: a lp_event_func_t 
 *
 * Adds @handler to the @media handler list.
 *
 * Return value: %TRUE if successful.  %FALSE otherwise.
 */
LP_API int
lp_media_register (lp_media_t *media, lp_event_func_t handler)
{
  GSList *l = NULL, *p = NULL;
  int found = 0;
  
  assert (handler != NULL);
  l = media->handlers;
  while (l != NULL && !found)
  {
    if (((lp_event_func_t) integralof (l->data)) == handler)
      found = 1;
    p = l;
    l = l->next;
  }

  if (!found)
  {
    l = g_slist_append (l, pointerof(handler));
    if (p != NULL)
      p->next = l;
    else
      p = l;

    if (media->handlers == NULL)
      media->handlers = p;

    return TRUE;
  }
  else
    return FALSE;
}

/*-
 * lp_media_unregister:
 * @media: a #lp_media_t
 * @handler: a lp_event_func_t 
 *
 * Removes @handler of the @media handler list.
 *
 * Return value: %TRUE if successful.  %FALSE otherwise.
 */
LP_API int
lp_media_unregister (lp_media_t *media, lp_event_func_t handler)
{
  GSList *l = NULL;
  int found = 0;
  
  assert (handler != NULL);
  l = media->handlers;
  while (l != NULL)
  {
    if (((lp_event_func_t) integralof (l->data)) == handler)
    {
      GSList *p = l;
      l = g_slist_remove_link (l, l);
      if (media->handlers == p)
        media->handlers = l;
        
      found = 1;
      break;
    }
    l = l->next;
  }
  if (!found)
    return FALSE;
  else
    return TRUE;
}

LP_API void
lp_media_post (lp_media_t *media, const lp_event_t *event)
{
  assert (event != NULL);
  switch (event->type)
  {
    case LP_EVENT_START:
      if (media->type == LP_MEDIA_ATOM)
        _lp_media_atom_start (media);
      else
        _lp_media_scene_start (media);

      break;
    case LP_EVENT_STOP:
      if (media->type == LP_MEDIA_ATOM)
        _lp_media_atom_stop (media);
      else
        _lp_media_scene_stop (media);

    default:
      ASSERT_NOT_REACHED;
      break;
  }
}
