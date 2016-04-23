/* gstx-macros.h -- Auxiliary GStreamer macros.
   Copyright (C) 2015-2016 PUC-Rio/Laboratorio TeleMidia

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

#ifndef GSTX_MACROS_H
#define GSTX_MACROS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <assert.h>
#include <stdarg.h>

#include "macros.h"

#define GSTX_INCLUDE_PROLOGUE                   \
  PRAGMA_DIAG_IGNORE (-Wbad-function-cast)      \
  PRAGMA_DIAG_PUSH ()                           \
  PRAGMA_DIAG_IGNORE (-Wcast-qual)              \
  PRAGMA_DIAG_IGNORE (-Wconversion)             \
  PRAGMA_DIAG_IGNORE (-Wsign-conversion)

#define GSTX_INCLUDE_EPILOGUE\
  PRAGMA_DIAG_POP ()

GSTX_INCLUDE_PROLOGUE
#include <glib.h>
#include <gst/gst.h>
GSTX_INCLUDE_EPILOGUE


/* Maps a GStreamer element name to an offset within a structure.  */
typedef struct _gstx_eltmap_t
{
  const char *name;
  ptrdiff_t offset;
} gstx_eltmap_t;

/* Allocates all fields of @obj specified in @map.
   Returns true all fields were allocated, otherwise returns false and sets
   @err to the name of the element that couldn't be allocated.  */

static ATTR_UNUSED gboolean
gstx_eltmap_alloc (const void *obj, const gstx_eltmap_t map[],
                   const char **err)
{
  size_t i;
  for (i = 0; map[i].name != NULL; i++)
    {
      GstElement **elt;
      elt = (GstElement **)(((ptrdiff_t) obj) + map[i].offset);
      *elt = gst_element_factory_make (map[i].name, NULL);
      if (unlikely (*elt == NULL))
        {
          size_t j;
          set_if_nonnull (err, map[i].name);
          for (j = 0; j < i; j++)
            {
              elt = (GstElement **)(((ptrdiff_t) obj) + map[i].offset);
              *elt = gst_element_factory_make (map[i].name, NULL);
              if (*elt != NULL)
                gst_object_unref (*elt);
            }
          return FALSE;
        }
    }
  return TRUE;
}


/* Returns the current time of @elt's clock,
   or GST_CLOCK_TIME_NONE if @elt doesn't have a clock.  */

static GstClockTime ATTR_UNUSED
gstx_element_get_clock_time (GstElement *elt)
{
  GstClock *clock;
  GstClockTime time;

  clock = gst_element_get_clock (elt);
  if (unlikely (clock == NULL))
    return GST_CLOCK_TIME_NONE;

  time = gst_clock_get_time (clock);
  g_object_unref (clock);

  return time;
}

/* Asserted version of gst_element_get_state().  */
#define gstx_element_get_state(elt, st, pend, tout)             \
  STMT_BEGIN                                                    \
  {                                                             \
    assert (gst_element_get_state ((elt), (st), (pend), (tout)) \
            != GST_STATE_CHANGE_FAILURE);                       \
  }                                                             \
  STMT_END

/* Synchronous version of gstx_element_get_state().  */
#define gstx_element_get_state_sync(elt, st, pend)                      \
  STMT_BEGIN                                                            \
  {                                                                     \
    gstx_element_get_state ((elt), (st), (pend), GST_CLOCK_TIME_NONE);  \
  }                                                                     \
  STMT_END

/* Asserted version of gst_element_set_state().  */
#define gstx_element_set_state(elt, st)         \
  STMT_BEGIN                                    \
  {                                             \
    assert (gst_element_set_state ((elt), (st)) \
            != GST_STATE_CHANGE_FAILURE);       \
  }                                             \
  STMT_END

/* Synchronous version of gstx_element_set_state().  */
#define gstx_element_set_state_sync(elt, st)            \
  STMT_BEGIN                                            \
  {                                                     \
    gstx_element_set_state ((elt), (st));               \
    gstx_element_get_state_sync ((elt), NULL, NULL);    \
  }                                                     \
  STMT_END

/* Posts an application message originating from element @orig and with the
   given @name and fields on the bus of element @dest.  */

static void ATTR_UNUSED
gstx_element_post_application_message (GstElement *dest, GstObject *orig,
                                       const char *name,
                                       const char *first_field, ...)
{
  GstStructure *st;
  GstMessage *msg;
  va_list args;

  va_start (args, first_field);
  st = gst_structure_new_valist (name, first_field, args);
  assert (st != NULL);
  va_end (args);

  msg = gst_message_new_application (orig, st);
  assert (msg != NULL);
  assert (gst_element_post_message (dest, msg));
}


/* Returns the value of pointer @field in structure @st.
   Aborts if there is no such field of if it does not contain a pointer.  */

static gpointer ATTR_UNUSED
gstx_structure_get_pointer (const GstStructure *st, const gchar *field)
{
  const GValue *value;

  value = gst_structure_get_value (st, field);
  assert (value != NULL);
  assert (G_VALUE_TYPE (deconst (GValue *, value)) == G_TYPE_POINTER);

  return g_value_get_pointer (value);
}

#endif /* GSTX_MACROS_H */
