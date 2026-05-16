
#ifndef NEXUS_V1_RASTERIZER_H
#define NEXUS_V1_RASTERIZER_H

#include "nexus_v1_math3d.h"
#include <stdint.h>

/* Software rasterizer for Nexus 3D rendering.
 * Renders to a 320x200 framebuffer (Saturn VDP1 resolution).
 * Features:
 *   - Z-buffer per pixel
 *   - Flat shading + directional light
 *   - Affine texture mapping (Saturn-style, no perspective correction)
 *   - Back-face culling
 *   - Scissor clipping */

#define NEXUS_FB_W 320
#define NEXUS_FB_H 200

typedef struct {
    uint8_t color_buffer[NEXUS_FB_W * NEXUS_FB_H];   /* indexed color */
    float z_buffer[NEXUS_FB_W * NEXUS_FB_H];
    uint32_t palette[256];
    int clear_color;
} Nexus_Framebuffer;

typedef struct {
    Vec3 pos;
    Vec3 dir;      /* party facing direction */
    float fov;     /* degrees, typically 60 */
    Mat4 view;
    Mat4 proj;
    Mat4 view_proj;
} Nexus_Camera;

typedef struct {
    Vec3 position;
    Vec2 uv;
    uint8_t color;
} Nexus_RasterVertex;

/* Initialize framebuffer */
void nexus_fb_init(Nexus_Framebuffer *fb);
void nexus_fb_clear(Nexus_Framebuffer *fb);

/* Camera */
void nexus_camera_init(Nexus_Camera *cam, Vec3 pos, int facing_dir);
void nexus_camera_update(Nexus_Camera *cam);

/* Rasterize a single triangle (flat shaded) */
void nexus_raster_triangle(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam);

/* Rasterize a quad (two triangles) */
void nexus_raster_quad(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1,
    Nexus_RasterVertex v2, Nexus_RasterVertex v3,
    const Nexus_Camera *cam);

/* Draw dungeon walls for a square */
void nexus_draw_wall(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int wall_dir, uint8_t color);

/* Draw floor/ceiling tile */
void nexus_draw_floor(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, uint8_t floor_color, uint8_t ceil_color);

#endif

