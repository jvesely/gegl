/* This file is part of GEGL.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2006,2007 Øyvind Kolås <pippin@gimp.org>
 */

#include "config.h"

#include <string.h>
#include <math.h>

#include <glib-object.h>

#include "gegl-buffer-types.h"
#include "gegl-tile-handler-cache.h"
#include "gegl-tile-handler-private.h"
#include "gegl-tile-storage.h"
#include "gegl-buffer-private.h"
#include "gegl-config.h"

struct _GeglTileHandlerPrivate
{
  GeglTileStorage      *tile_storage;
  GeglTileHandlerCache *cache;
};

G_DEFINE_TYPE (GeglTileHandler, gegl_tile_handler, GEGL_TYPE_TILE_SOURCE)

enum
{
  PROP0,
  PROP_SOURCE
};

gboolean gegl_tile_storage_cached_release (GeglTileStorage *storage);

static void
gegl_tile_handler_dispose (GObject *object)
{
  GeglTileHandler *handler = GEGL_TILE_HANDLER (object);

  if (handler->source)
    {
      g_object_unref (handler->source);
      handler->source = NULL;
    }

  G_OBJECT_CLASS (gegl_tile_handler_parent_class)->dispose (object);
}

static gpointer
gegl_tile_handler_command (GeglTileSource *gegl_tile_source,
                           GeglTileCommand command,
                           gint            x,
                           gint            y,
                           gint            z,
                           gpointer        data)
{
  GeglTileHandler *handler = (GeglTileHandler*)gegl_tile_source;

  return gegl_tile_handler_source_command (handler, command, x, y, z, data);
}

static void
gegl_tile_handler_get_property (GObject    *gobject,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  GeglTileHandler *handler = GEGL_TILE_HANDLER (gobject);

  switch (property_id)
    {
      case PROP_SOURCE:
        g_value_set_object (value, handler->source);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
        break;
    }
}

static void
gegl_tile_handler_set_property (GObject      *gobject,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  GeglTileHandler *handler = GEGL_TILE_HANDLER (gobject);

  switch (property_id)
    {
      case PROP_SOURCE:
        gegl_tile_handler_set_source (handler, g_value_get_object (value));
        return;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
        break;
    }
}

static void
gegl_tile_handler_class_init (GeglTileHandlerClass *klass)
{
  GObjectClass *gobject_class  = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gegl_tile_handler_set_property;
  gobject_class->get_property = gegl_tile_handler_get_property;
  gobject_class->dispose      = gegl_tile_handler_dispose;

  g_object_class_install_property (gobject_class, PROP_SOURCE,
                                   g_param_spec_object ("source",
                                                        "GeglBuffer",
                                                        "The tilestore to be a facade for",
                                                        G_TYPE_OBJECT,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_type_class_add_private (gobject_class, sizeof (GeglTileHandlerPrivate));
}

static void
gegl_tile_handler_init (GeglTileHandler *self)
{
  ((GeglTileSource *) self)->command = gegl_tile_handler_command;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GEGL_TYPE_TILE_HANDLER,
                                            GeglTileHandlerPrivate);
}

void
gegl_tile_handler_set_source (GeglTileHandler *handler,
                              GeglTileSource  *source)
{
  if (source != handler->source)
    {
      if (handler->source)
        g_object_unref (handler->source);

      handler->source = source;

      if (handler->source)
        g_object_ref (handler->source);
    }
}

void
_gegl_tile_handler_set_tile_storage (GeglTileHandler *handler,
                                     GeglTileStorage *tile_storage)
{
  handler->priv->tile_storage = tile_storage;
}

void
_gegl_tile_handler_set_cache (GeglTileHandler      *handler,
                              GeglTileHandlerCache *cache)
{
  handler->priv->cache = cache;
}

GeglTileStorage *
_gegl_tile_handler_get_tile_storage (GeglTileHandler *handler)
{
  return handler->priv->tile_storage;
}

GeglTileHandlerCache *
_gegl_tile_handler_get_cache (GeglTileHandler *handler)
{
  return handler->priv->cache;
}

GeglTile *
gegl_tile_handler_create_tile (GeglTileHandler *handler,
                               gint             x,
                               gint             y,
                               gint             z)
{
  GeglTile *tile;

  tile = gegl_tile_new (handler->priv->tile_storage->tile_size);

  tile->tile_storage = handler->priv->tile_storage;
  tile->x            = x;
  tile->y            = y;
  tile->z            = z;

  if (handler->priv->cache)
    gegl_tile_handler_cache_insert (handler->priv->cache, tile, x, y, z);

  return tile;
}

GeglTile *
gegl_tile_handler_dup_tile (GeglTileHandler *handler,
                            GeglTile        *tile,
                            gint             x,
                            gint             y,
                            gint             z)
{
  tile = gegl_tile_dup (tile);

  tile->x = x;
  tile->y = y;
  tile->z = z;

  if (handler->priv->cache)
    gegl_tile_handler_cache_insert (handler->priv->cache, tile, x, y, z);

  return tile;
}

void
gegl_tile_handler_damage_rect (GeglTileHandler     *handler,
                               const GeglRectangle *rect)
{
  GeglTileSource *source;
  gint            tile_width;
  gint            tile_height;
  gint            X1, Y1;
  gint            X2, Y2;
  gint            x1, y1;
  gint            x2, y2;
  gint            z;

  g_return_if_fail (GEGL_IS_TILE_HANDLER (handler));
  g_return_if_fail (rect != NULL);

  if (! handler->priv->tile_storage            ||
      ! handler->priv->tile_storage->seen_zoom ||
      rect->width  <= 0                        ||
      rect->height <= 0)
    {
      return;
    }

  source = GEGL_TILE_SOURCE (handler);

  if (gegl_config_threads()>1)
    g_rec_mutex_lock (&handler->priv->tile_storage->mutex);

  tile_width  = handler->priv->tile_storage->tile_width;
  tile_height = handler->priv->tile_storage->tile_height;

  X1 = rect->x;
  Y1 = rect->y;
  X2 = rect->x + rect->width  - 1;
  Y2 = rect->y + rect->height - 1;

  x1 = floor ((gdouble) X1 / tile_width);
  y1 = floor ((gdouble) Y1 / tile_height);
  x2 = floor ((gdouble) X2 / tile_width);
  y2 = floor ((gdouble) Y2 / tile_height);

  for (z = 1; z <= handler->priv->tile_storage->seen_zoom; z++)
    {
      gint x,  y;

      X1 >>= 1;
      Y1 >>= 1;
      X2 >>= 1;
      Y2 >>= 1;

      x1 >>= 1;
      y1 >>= 1;
      x2 >>= 1;
      y2 >>= 1;

      for (x = x1; x <= x2; x++)
        {
          for (y = y1; y <= y2; y++)
            {
              gegl_tile_source_void (source, x, y, z);
            }
        }
    }

  if (gegl_config_threads()>1)
    g_rec_mutex_unlock (&handler->priv->tile_storage->mutex);
}
