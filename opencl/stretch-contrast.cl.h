static const char* stretch_contrast_cl_source =
"/* This file is an image processing operation for GEGL                        \n"
" *                                                                            \n"
" * GEGL is free software; you can redistribute it and/or                      \n"
" * modify it under the terms of the GNU Lesser General Public                 \n"
" * License as published by the Free Software Foundation; either               \n"
" * version 3 of the License, or (at your option) any later version.           \n"
" *                                                                            \n"
" * GEGL is distributed in the hope that it will be useful,                    \n"
" * but WITHOUT ANY WARRANTY; without even the implied warranty of             \n"
" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          \n"
" * Lesser General Public License for more details.                            \n"
" *                                                                            \n"
" * You should have received a copy of the GNU Lesser General Public           \n"
" * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.      \n"
" *                                                                            \n"
" * Copyright 2013 Carlos Zubieta <czubieta.dev@gmail.com>                     \n"
" */                                                                           \n"
"                                                                              \n"
"                                                                              \n"
"__kernel void init_stretch (__global float4 *out_min,                         \n"
"                            __global float4 *out_max)                         \n"
"{                                                                             \n"
"  int gid = get_global_id (0);                                                \n"
"                                                                              \n"
"  out_min[gid] = (float4)( FLT_MAX);                                          \n"
"  out_max[gid] = (float4)(-FLT_MAX);                                          \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void two_stages_local_min_max_reduce (__global const float4 *in,     \n"
"                                               __global       float4 *out_min,\n"
"                                               __global       float4 *out_max,\n"
"                                               __local        float4 *aux_min,\n"
"                                               __local        float4 *aux_max,\n"
"                                                              int     n_pixels)\n"
"{                                                                             \n"
"  int    gid   = get_global_id(0);                                            \n"
"  int    gsize = get_global_size(0);                                          \n"
"  int    lid   = get_local_id(0);                                             \n"
"  int    lsize = get_local_size(0);                                           \n"
"  float4 min_v = (float4)( FLT_MAX);                                          \n"
"  float4 max_v = (float4)(-FLT_MAX);                                          \n"
"  float4 in_v;                                                                \n"
"  float4 aux0, aux1;                                                          \n"
"  int    it;                                                                  \n"
"                                                                              \n"
"  /* Loop sequentially over chunks of input vector */                         \n"
"  for (it = gid; it < n_pixels; it += gsize)                                  \n"
"    {                                                                         \n"
"      in_v  =  in[it];                                                        \n"
"      min_v =  min (min_v, in_v);                                             \n"
"      max_v =  max (max_v, in_v);                                             \n"
"    }                                                                         \n"
"                                                                              \n"
"  /* Perform parallel reduction */                                            \n"
"  aux_min[lid] = min_v;                                                       \n"
"  aux_max[lid] = max_v;                                                       \n"
"                                                                              \n"
"  barrier (CLK_LOCAL_MEM_FENCE);                                              \n"
"                                                                              \n"
"  for(it = lsize / 2; it > 0; it >>= 1)                                       \n"
"    {                                                                         \n"
"      if (lid < it)                                                           \n"
"        {                                                                     \n"
"          aux0         = aux_min[lid + it];                                   \n"
"          aux1         = aux_min[lid];                                        \n"
"          aux_min[lid] = min (aux0, aux1);                                    \n"
"                                                                              \n"
"          aux0         = aux_max[lid + it];                                   \n"
"          aux1         = aux_max[lid];                                        \n"
"          aux_max[lid] = max (aux0, aux1);                                    \n"
"        }                                                                     \n"
"      barrier (CLK_LOCAL_MEM_FENCE);                                          \n"
"  }                                                                           \n"
"  if (lid == 0)                                                               \n"
"    {                                                                         \n"
"      out_min[get_group_id(0)] = aux_min[0];                                  \n"
"      out_max[get_group_id(0)] = aux_max[0];                                  \n"
"    }                                                                         \n"
"                                                                              \n"
"  /* the work-group size is the size of the buffer.                           \n"
"   * Make sure it's fully initialized */                                      \n"
"  if (gid == 0)                                                               \n"
"    {                                                                         \n"
"      /* No special case handling, gsize is a multiple of lsize */            \n"
"      int nb_wg = gsize / lsize;                                              \n"
"      for (it = nb_wg; it < lsize; it++)                                      \n"
"        {                                                                     \n"
"          out_min[it] = (float4)( FLT_MAX);                                   \n"
"          out_max[it] = (float4)(-FLT_MAX);                                   \n"
"        }                                                                     \n"
"    }                                                                         \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void global_min_max_reduce (__global float4 *in_min,                 \n"
"                                     __global float4 *in_max,                 \n"
"                                     __global float4 *out_min_max)            \n"
"{                                                                             \n"
"  int    gid   = get_global_id(0);                                            \n"
"  int    lid   = get_local_id(0);                                             \n"
"  int    lsize = get_local_size(0);                                           \n"
"  float4 aux0, aux1;                                                          \n"
"  int    it;                                                                  \n"
"                                                                              \n"
"  /* Perform parallel reduction */                                            \n"
"  for (it = lsize / 2; it > 0; it >>= 1)                                      \n"
"    {                                                                         \n"
"      if (lid < it)                                                           \n"
"        {                                                                     \n"
"          aux0        = in_min[lid + it];                                     \n"
"          aux1        = in_min[lid];                                          \n"
"          in_min[gid] = min (aux0, aux1);                                     \n"
"                                                                              \n"
"          aux0        = in_max[lid + it];                                     \n"
"          aux1        = in_max[lid];                                          \n"
"          in_max[gid] = max (aux0, aux1);                                     \n"
"        }                                                                     \n"
"      barrier (CLK_GLOBAL_MEM_FENCE);                                         \n"
"  }                                                                           \n"
"  if (lid == 0)                                                               \n"
"    {                                                                         \n"
"      out_min_max[0] = in_min[gid];                                           \n"
"      out_min_max[1] = in_max[gid];                                           \n"
"    }                                                                         \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void cl_stretch_contrast (__global const float4 *in,                 \n"
"                                   __global       float4 *out,                \n"
"                                                  float4  min,                \n"
"                                                  float4  diff)               \n"
"{                                                                             \n"
"  int    gid  = get_global_id(0);                                             \n"
"  float4 in_v = in[gid];                                                      \n"
"                                                                              \n"
"  in_v = (in_v - min) / diff;                                                 \n"
"                                                                              \n"
"  out[gid] = in_v;                                                            \n"
"}                                                                             \n"
;
