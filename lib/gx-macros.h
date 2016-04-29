/* gx-macros.h -- Auxiliary GLib macros.
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

#ifndef GX_MACROS_H
#define GX_MACROS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <assert.h>

#include "macros.h"

#define GX_INCLUDE_PROLOGUE                     \
  PRAGMA_DIAG_PUSH ()                           \
  PRAGMA_DIAG_IGNORE (-Wbad-function-cast)

#define GX_INCLUDE_EPILOGUE                     \
  PRAGMA_DIAG_POP ()

GX_INCLUDE_PROLOGUE
#include <glib.h>
#include <glib-object.h>
GX_INCLUDE_EPILOGUE

/* Warning-free G_DEFINE_TYPE wrapper.  */
#define GX_DEFINE_TYPE(TN, t_n, T_P)            \
  GX_INCLUDE_PROLOGUE                           \
  G_DEFINE_TYPE (TN, t_n, T_P)                  \
  GX_INCLUDE_EPILOGUE


/* Gets the #GParamSpec of property @name of @obj.   */
#define gx_object_find_property(obj, name)\
  g_object_class_find_property (G_OBJECT_GET_CLASS ((obj)), (name))

/* Gets the #GType of property @name of @obj.  */

static ATTR_UNUSED GType
gx_object_find_property_type (GObject *obj, const char *name)
{
  GParamSpec *pspec;

  pspec = gx_object_find_property (obj, name);
  if (unlikely (pspec == NULL))
    return G_TYPE_INVALID;
  return G_PARAM_SPEC_VALUE_TYPE (pspec);
}

#endif /* GX_MACROS_H */
