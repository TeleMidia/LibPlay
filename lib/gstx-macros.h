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

#include "macros.h"
#include "gx-macros.h"

#define GSTX_INCLUDE_PROLOGUE                   \
  PRAGMA_DIAG_PUSH ()                           \
  PRAGMA_DIAG_IGNORE (-Wcast-qual)              \
  PRAGMA_DIAG_IGNORE (-Wconversion)

#define GSTX_INCLUDE_EPILOGUE                   \
  PRAGMA_DIAG_POP ()

GSTX_INCLUDE_PROLOGUE
#include <gst/gst.h>
GSTX_INCLUDE_EPILOGUE


/* Maps a GStreamer element name to an offset within a structure.  */
typedef struct _gstx_eltmap_t
{
  const gchar *name;
  ptrdiff_t offset;
} gstx_eltmap_t;

/* Allocates all fields of @obj specified in @map.
   Returns true all fields were allocated, otherwise returns false and sets
   @err to the name of the element that couldn't be allocated.  */

static ATTR_UNUSED gboolean
gstx_eltmap_alloc (const void *obj, const gstx_eltmap_t map[],
                   const gchar **err)
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


/* Asserted version of gst_bin_add().  */
#define gstx_bin_add(bin, elt)                          \
  STMT_BEGIN                                            \
  {                                                     \
    g_assert (gst_bin_add (GST_BIN ((bin)), (elt)));    \
  }                                                     \
  STMT_END

/* Asserted version of gst_bin_remove().  */
#define gstx_bin_remove(bin, elt)                       \
  STMT_BEGIN                                            \
  {                                                     \
    g_assert (gst_bin_remove (GST_BIN ((bin)), (elt))); \
  }                                                     \
  STMT_END

/* Asserted version of gst_element_link().  */
#define gstx_element_link(elt1, elt2)                   \
  STMT_BEGIN                                            \
  {                                                     \
    g_assert (gst_element_link ((elt1), (elt2)));       \
  }                                                     \
  STMT_END

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
#define gstx_element_get_state(elt, st, pend, tout)                     \
  STMT_BEGIN                                                            \
  {                                                                     \
    g_assert (gst_element_get_state ((elt), (st), (pend), (tout))       \
              != GST_STATE_CHANGE_FAILURE);                             \
  }                                                                     \
  STMT_END

/* Synchronous version of gstx_element_get_state().  */
#define gstx_element_get_state_sync(elt, st, pend)                      \
  STMT_BEGIN                                                            \
  {                                                                     \
    gstx_element_get_state ((elt), (st), (pend), GST_CLOCK_TIME_NONE);  \
  }                                                                     \
  STMT_END

/* Asserted version of gst_element_set_state().  */
#define gstx_element_set_state(elt, st)                 \
  STMT_BEGIN                                            \
  {                                                     \
    g_assert (gst_element_set_state ((elt), (st))       \
              != GST_STATE_CHANGE_FAILURE);             \
  }                                                     \
  STMT_END

/* Synchronous version of gstx_element_set_state().  */
#define gstx_element_set_state_sync(elt, st)            \
  STMT_BEGIN                                            \
  {                                                     \
    gstx_element_set_state ((elt), (st));               \
    gstx_element_get_state_sync ((elt), NULL, NULL);    \
  }                                                     \
  STMT_END


/* Returns the value of pointer @field in structure @st.
   Aborts if there is no such field of if it does not contain a pointer.  */

static gpointer ATTR_UNUSED
gstx_structure_get_pointer (const GstStructure *st, const gchar *field)
{
  const GValue *value;

  value = gst_structure_get_value (st, field);
  g_assert_nonnull (value);
  g_assert (G_VALUE_TYPE (deconst (GValue *, value)) == G_TYPE_POINTER);

  return g_value_get_pointer (value);
}

#endif /* GSTX_MACROS_H */
