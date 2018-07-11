
/* !!!! AUTOGENERATED FILE generated by math.rb !!!!!
 *
 * This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright 2006 Øyvind Kolås <pippin@gimp.org>
 *
 * !!!! AUTOGENERATED FILE !!!!!
 */
#include "config.h"
#include <glib/gi18n-lib.h>


#ifdef GEGL_PROPERTIES

property_double (value, _("Value"), 1.0)
   description(_("global value used if aux doesn't contain data"))
   ui_range (-1.0, 1.0)

#else

#define GEGL_OP_POINT_COMPOSER
#define GEGL_OP_NAME         multiply
#define GEGL_OP_C_FILE       "multiply.c"

#include "gegl-op.h"

#include <math.h>
#ifdef _MSC_VER
#define powf(a,b) ((gfloat)pow(a,b))
#endif


static void prepare (GeglOperation *operation)
{
  const Babl *space = gegl_operation_get_source_space (operation, "input");
  const Babl *format;
  if (!space)
     space = gegl_operation_get_source_space (operation, "aux");
  format  = babl_format_with_space ("RGBA float", space);

  gegl_operation_set_format (operation, "input", format);
  gegl_operation_set_format (operation, "aux", babl_format_with_space ("RGB float", space));
  gegl_operation_set_format (operation, "output", format);
}

static gboolean
process (GeglOperation       *op,
         void                *in_buf,
         void                *aux_buf,
         void                *out_buf,
         glong                n_pixels,
         const GeglRectangle *roi,
         gint                 level)
{
  gfloat * GEGL_ALIGNED in = in_buf;
  gfloat * GEGL_ALIGNED out = out_buf;
  gfloat * GEGL_ALIGNED aux = aux_buf;
  gint    i;

  if (aux == NULL)
    {
      gfloat value = GEGL_PROPERTIES (op)->value;
      for (i=0; i<n_pixels; i++)
        {
          gint   j;
          for (j=0; j<3; j++)
            {
              gfloat result;
              gfloat input=in[j];
              result = input * value;
              out[j]=result;
            }
          out[3]=in[3];
          in += 4;
          out+= 4;
        }
    }
  else
    {
      for (i=0; i<n_pixels; i++)
        {
          gint   j;
          gfloat value;
          for (j=0; j<3; j++)
            {
              gfloat input =in[j];
              gfloat result;
              value=aux[j];
              result = input * value;
              out[j]=result;
            }
          out[3]=in[3];
          in += 4;
          aux += 3;
          out+= 4;
        }
    }

  return TRUE;
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass              *operation_class;
  GeglOperationPointComposerClass *point_composer_class;

  operation_class  = GEGL_OPERATION_CLASS (klass);
  point_composer_class     = GEGL_OPERATION_POINT_COMPOSER_CLASS (klass);

  point_composer_class->process = process;
  operation_class->prepare = prepare;

  gegl_operation_class_set_keys (operation_class,
  "name"        , "gegl:multiply",
  "title"       , "Multiply",
  "categories"  , "compositors:math",
  "reference-hash"  , "c80bb8504f405bb0a5ce2be4fad6af69",
  "description" ,
       _("Math operation multiply, performs the operation per pixel, using either the constant provided in 'value' or the corresponding pixel from the buffer on aux as operands. The result is the evaluation of the expression result = input * value"),
       NULL);
}
#endif
