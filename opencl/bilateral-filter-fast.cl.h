static const char* bilateral_filter_fast_cl_source =
"/* This file is part of GEGL                                                  \n"
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
" * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.       \n"
" *                                                                            \n"
" * Copyright 2013 Victor Oliveira (victormatheus@gmail.com)                   \n"
" */                                                                           \n"
"                                                                              \n"
"#define GRID(x,y,z) grid[x+sw*(y + z * sh)]                                   \n"
"                                                                              \n"
"#define LOCAL_W 8                                                             \n"
"#define LOCAL_H 8                                                             \n"
"                                                                              \n"
"/* found by trial and error on a NVidia GPU */                                \n"
"                                                                              \n"
"// optimum value                                                              \n"
"// #define DEPTH_CHUNK 12                                                     \n"
"                                                                              \n"
"// a little less than 16k, works on most GPUs                                 \n"
"#define DEPTH_CHUNK 7                                                         \n"
"                                                                              \n"
"__attribute__((reqd_work_group_size(8, 8, 1)))                                \n"
"__kernel void bilateral_downsample(__global const float4 *input,              \n"
"                                   __global       float8 *grid,               \n"
"                                   int   width,                               \n"
"                                   int   height,                              \n"
"                                   int   sw,                                  \n"
"                                   int   sh,                                  \n"
"                                   int   depth,                               \n"
"                                   int   s_sigma,                             \n"
"                                   float r_sigma)                             \n"
"{                                                                             \n"
"  const int gid_x = get_global_id(0);                                         \n"
"  const int gid_y = get_global_id(1);                                         \n"
"                                                                              \n"
"  __local float8 grid_chunk[DEPTH_CHUNK][LOCAL_H][LOCAL_W];                   \n"
"                                                                              \n"
"  if (gid_x > sw || gid_y > sh) return;                                       \n"
"                                                                              \n"
"  for (int d = 0; d < depth; d+=DEPTH_CHUNK)                                  \n"
"    {                                                                         \n"
"      for (int k=0; k < DEPTH_CHUNK; k++)                                     \n"
"        {                                                                     \n"
"          grid_chunk[k][get_local_id(1)][get_local_id(0)] = (float8)(0.0f);   \n"
"        }                                                                     \n"
"                                                                              \n"
"      barrier (CLK_LOCAL_MEM_FENCE);                                          \n"
"                                                                              \n"
"      for (int ry=0; ry < s_sigma; ry++)                                      \n"
"        for (int rx=0; rx < s_sigma; rx++)                                    \n"
"          {                                                                   \n"
"            const int x = clamp(gid_x * s_sigma - s_sigma/2 + rx, 0, width -1);\n"
"            const int y = clamp(gid_y * s_sigma - s_sigma/2 + ry, 0, height-1);\n"
"                                                                              \n"
"            const float4 val = input[y * width + x];                          \n"
"                                                                              \n"
"            const int4 z = convert_int4(val * (1.0f/r_sigma) + 0.5f);         \n"
"                                                                              \n"
"            // z >= d && z < d+DEPTH_CHUNK                                    \n"
"            int4 inbounds = (z >= d & z < d+DEPTH_CHUNK);                     \n"
"                                                                              \n"
"            if (inbounds.x) grid_chunk[z.x-d][get_local_id(1)][get_local_id(0)].s01 += (float2)(val.x, 1.0f);\n"
"            if (inbounds.y) grid_chunk[z.y-d][get_local_id(1)][get_local_id(0)].s23 += (float2)(val.y, 1.0f);\n"
"            if (inbounds.z) grid_chunk[z.z-d][get_local_id(1)][get_local_id(0)].s45 += (float2)(val.z, 1.0f);\n"
"            if (inbounds.w) grid_chunk[z.w-d][get_local_id(1)][get_local_id(0)].s67 += (float2)(val.w, 1.0f);\n"
"                                                                              \n"
"            barrier (CLK_LOCAL_MEM_FENCE);                                    \n"
"          }                                                                   \n"
"                                                                              \n"
"      for (int s=d, e=d+min(DEPTH_CHUNK, depth-d); s < e; s++)                \n"
"        {                                                                     \n"
"          grid[gid_x+sw*(gid_y + s * sh)] = grid_chunk[s-d][get_local_id(1)][get_local_id(0)];\n"
"        }                                                                     \n"
"    }                                                                         \n"
"}                                                                             \n"
"                                                                              \n"
"#undef LOCAL_W                                                                \n"
"#undef LOCAL_H                                                                \n"
"                                                                              \n"
"#define LOCAL_W 16                                                            \n"
"#define LOCAL_H 16                                                            \n"
"                                                                              \n"
"__attribute__((reqd_work_group_size(16, 16, 1)))                              \n"
"__kernel void bilateral_blur(__global const float8 *grid,                     \n"
"                             __global       float2 *blurz_r,                  \n"
"                             __global       float2 *blurz_g,                  \n"
"                             __global       float2 *blurz_b,                  \n"
"                             __global       float2 *blurz_a,                  \n"
"                             int sw,                                          \n"
"                             int sh,                                          \n"
"                             int depth)                                       \n"
"{                                                                             \n"
"  const int gid_x = get_global_id(0);                                         \n"
"  const int gid_y = get_global_id(1);                                         \n"
"                                                                              \n"
"  const int lx = get_local_id(0);                                             \n"
"  const int ly = get_local_id(1);                                             \n"
"                                                                              \n"
"  float8 vpp = (float8)(0.0f);                                                \n"
"  float8 vp  = (float8)(0.0f);                                                \n"
"  float8 v   = (float8)(0.0f);                                                \n"
"                                                                              \n"
"  __local float8 data[LOCAL_H+2][LOCAL_W+2];                                  \n"
"                                                                              \n"
"  for (int d=0; d<depth; d++)                                                 \n"
"    {                                                                         \n"
"        for (int ky=get_local_id(1)-1; ky<LOCAL_H+1; ky+=get_local_size(1))   \n"
"          for (int kx=get_local_id(0)-1; kx<LOCAL_W+1; kx+=get_local_size(0)) \n"
"            {                                                                 \n"
"              int xx = clamp((int)get_group_id(0)*LOCAL_W+kx, 0, sw-1);       \n"
"              int yy = clamp((int)get_group_id(1)*LOCAL_H+ky, 0, sh-1);       \n"
"                                                                              \n"
"              data[ky+1][kx+1] = GRID(xx, yy, d);                             \n"
"            }                                                                 \n"
"                                                                              \n"
"        barrier (CLK_LOCAL_MEM_FENCE);                                        \n"
"                                                                              \n"
"      /* blur x */                                                            \n"
"                                                                              \n"
"        data[ly  ][lx+1] = (data[ly  ][lx] + 2.0f * data[ly  ][lx+1] + data[ly  ][lx+2]) / 4.0f;\n"
"        data[ly+1][lx+1] = (data[ly+1][lx] + 2.0f * data[ly+1][lx+1] + data[ly+1][lx+2]) / 4.0f;\n"
"        data[ly+2][lx+1] = (data[ly+2][lx] + 2.0f * data[ly+2][lx+1] + data[ly+2][lx+2]) / 4.0f;\n"
"                                                                              \n"
"        barrier (CLK_LOCAL_MEM_FENCE);                                        \n"
"                                                                              \n"
"      /* blur y */                                                            \n"
"                                                                              \n"
"      if (d==0) {                                                             \n"
"        v = (data[ly][lx+1] + 2.0f * data[ly+1][lx+1] + data[ly+2][lx+1]) / 4.0f;\n"
"        vpp = v;                                                              \n"
"        vp  = v;                                                              \n"
"      }                                                                       \n"
"      else {                                                                  \n"
"        vpp = vp;                                                             \n"
"        vp  = v;                                                              \n"
"                                                                              \n"
"        v = (data[ly][lx+1] + 2.0f * data[ly+1][lx+1] + data[ly+2][lx+1]) / 4.0f;\n"
"                                                                              \n"
"        float8 blurred = (vpp + 2.0f * vp + v) / 4.0f;                        \n"
"                                                                              \n"
"        if (gid_x < sw && gid_y < sh) {                                       \n"
"          blurz_r[gid_x+sw*(gid_y+sh*(d-1))] = blurred.s01;                   \n"
"          blurz_g[gid_x+sw*(gid_y+sh*(d-1))] = blurred.s23;                   \n"
"          blurz_b[gid_x+sw*(gid_y+sh*(d-1))] = blurred.s45;                   \n"
"          blurz_a[gid_x+sw*(gid_y+sh*(d-1))] = blurred.s67;                   \n"
"        }                                                                     \n"
"      }                                                                       \n"
"    }                                                                         \n"
"                                                                              \n"
"  vpp = vp;                                                                   \n"
"  vp  = v;                                                                    \n"
"                                                                              \n"
"  float8 blurred = (vpp + 2.0f * vp + v) / 4.0f;                              \n"
"                                                                              \n"
"  if (gid_x < sw && gid_y < sh) {                                             \n"
"    blurz_r[gid_x+sw*(gid_y+sh*(depth-1))] = blurred.s01;                     \n"
"    blurz_g[gid_x+sw*(gid_y+sh*(depth-1))] = blurred.s23;                     \n"
"    blurz_b[gid_x+sw*(gid_y+sh*(depth-1))] = blurred.s45;                     \n"
"    blurz_a[gid_x+sw*(gid_y+sh*(depth-1))] = blurred.s67;                     \n"
"  }                                                                           \n"
"}                                                                             \n"
"                                                                              \n"
"#undef LOCAL_W                                                                \n"
"#undef LOCAL_H                                                                \n"
"                                                                              \n"
"/* I could use texture memory here, but it's not worth the cost of            \n"
"   converting blurz_[r,g,b,a] from buffers to textures */                     \n"
"                                                                              \n"
"__kernel void bilateral_interpolate(__global    const float4  *input,         \n"
"                                    __global    const float2  *blurz_r,       \n"
"                                    __global    const float2  *blurz_g,       \n"
"                                    __global    const float2  *blurz_b,       \n"
"                                    __global    const float2  *blurz_a,       \n"
"                                    __global          float4  *smoothed,      \n"
"                                    int   width,                              \n"
"                                    int   sw,                                 \n"
"                                    int   sh,                                 \n"
"                                    int   depth,                              \n"
"                                    int   s_sigma,                            \n"
"                                    float r_sigma)                            \n"
"{                                                                             \n"
"  const int x = get_global_id(0);                                             \n"
"  const int y = get_global_id(1);                                             \n"
"                                                                              \n"
"  const float  xf = (float)(x) / s_sigma;                                     \n"
"  const float  yf = (float)(y) / s_sigma;                                     \n"
"  const float4 zf = input[y * width + x] / r_sigma;                           \n"
"                                                                              \n"
"  float8 val;                                                                 \n"
"                                                                              \n"
"  int  x1 = (int)xf;                                                          \n"
"  int  y1 = (int)yf;                                                          \n"
"  int4 z1 = convert_int4(zf);                                                 \n"
"                                                                              \n"
"  int  x2 = min(x1+1, sw-1);                                                  \n"
"  int  y2 = min(y1+1, sh-1);                                                  \n"
"  int4 z2 = min(z1+1, depth-1);                                               \n"
"                                                                              \n"
"  float  x_alpha = xf - x1;                                                   \n"
"  float  y_alpha = yf - y1;                                                   \n"
"  float4 z_alpha = zf - floor(zf); /* it's weird that z1 doesn't work here */ \n"
"                                                                              \n"
"  #define BLURZ_R(x,y,z) blurz_r[x+sw*(y+z*sh)]                               \n"
"  #define BLURZ_G(x,y,z) blurz_g[x+sw*(y+z*sh)]                               \n"
"  #define BLURZ_B(x,y,z) blurz_b[x+sw*(y+z*sh)]                               \n"
"  #define BLURZ_A(x,y,z) blurz_a[x+sw*(y+z*sh)]                               \n"
"                                                                              \n"
"  /* trilinear interpolation */                                               \n"
"                                                                              \n"
"  val.s04 = mix(mix(mix(BLURZ_R(x1, y1, z1.x), BLURZ_R(x2, y1, z1.x), x_alpha),\n"
"                    mix(BLURZ_R(x1, y2, z1.x), BLURZ_R(x2, y2, z1.x), x_alpha), y_alpha),\n"
"                mix(mix(BLURZ_R(x1, y1, z2.x), BLURZ_R(x2, y1, z2.x), x_alpha),\n"
"                    mix(BLURZ_R(x1, y2, z2.x), BLURZ_R(x2, y2, z2.x), x_alpha), y_alpha), z_alpha.x);\n"
"                                                                              \n"
"  val.s15 = mix(mix(mix(BLURZ_G(x1, y1, z1.y), BLURZ_G(x2, y1, z1.y), x_alpha),\n"
"                    mix(BLURZ_G(x1, y2, z1.y), BLURZ_G(x2, y2, z1.y), x_alpha), y_alpha),\n"
"                mix(mix(BLURZ_G(x1, y1, z2.y), BLURZ_G(x2, y1, z2.y), x_alpha),\n"
"                    mix(BLURZ_G(x1, y2, z2.y), BLURZ_G(x2, y2, z2.y), x_alpha), y_alpha), z_alpha.y);\n"
"                                                                              \n"
"  val.s26 = mix(mix(mix(BLURZ_B(x1, y1, z1.z), BLURZ_B(x2, y1, z1.z), x_alpha),\n"
"                    mix(BLURZ_B(x1, y2, z1.z), BLURZ_B(x2, y2, z1.z), x_alpha), y_alpha),\n"
"                mix(mix(BLURZ_B(x1, y1, z2.z), BLURZ_B(x2, y1, z2.z), x_alpha),\n"
"                    mix(BLURZ_B(x1, y2, z2.z), BLURZ_B(x2, y2, z2.z), x_alpha), y_alpha), z_alpha.z);\n"
"                                                                              \n"
"  val.s37 = mix(mix(mix(BLURZ_A(x1, y1, z1.w), BLURZ_A(x2, y1, z1.w), x_alpha),\n"
"                    mix(BLURZ_A(x1, y2, z1.w), BLURZ_A(x2, y2, z1.w), x_alpha), y_alpha),\n"
"                mix(mix(BLURZ_A(x1, y1, z2.w), BLURZ_A(x2, y1, z2.w), x_alpha),\n"
"                    mix(BLURZ_A(x1, y2, z2.w), BLURZ_A(x2, y2, z2.w), x_alpha), y_alpha), z_alpha.w);\n"
"                                                                              \n"
"  smoothed[y * width + x] = val.s0123/val.s4567;                              \n"
"}                                                                             \n"
;
