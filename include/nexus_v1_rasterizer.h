#ifndef NEXUS_V1_RASTERIZER_H
#define NEXUS_V1_RASTERIZER_H

#include "nexus_v1_math3d.h"
#include <stdint.h>

/* Nexus V1 Software Rasterizer
 * ==============================
 * Renders to a 320x200 indexed framebuffer (Saturn VDP1 resolution).
 *
 * Features:
 *   - Z-buffer (per-pixel depth test)
 *   - Flat shading via per-vertex color
 *   - Affine texture mapping (UV -> screen-space, Saturn-style, no
 *     perspective correction -- appropriate for flat dungeon surfaces)
 *   - Back-face culling (CCW winding)
 *   - Scissor clipping (discard out-of-bounds fragments)
 *   - Texture atlas support (Nexus_Texture from nexus_v1_palette.h)
 *   - Door animation states (open/closed/locked visual variants)
 *   - Creature model projection (DMDF billboard -> rasterizer vertices)
 *   - Projectile rendering (fireball/lightning/poison/bolt paths)
 *   - Deterministic fallback for unsupported 3D assets
 *
 * Source-lock references:
 *   ReDMCSB DRAWVIEW.C   -- viewport blit to screen memory (F2172)
 *   ReDMCSB BLIT.C        -- F0132 blit rect primitive
 *   ReDMCSB DUNGEON.C     -- wall square drawing (F0108)
 *   Saturn VDP1 SDK       -- command list format, local coordinates
 *   docs/NEXUS_FILE_CLASSIFICATION.md -- DMDF .MNS + surface files  */

/* ─────────────────────────── Public Types ──────────────────────── */
#define NEXUS_FB_W 320
#define NEXUS_FB_H 200

typedef struct {
    uint8_t  color_buffer[NEXUS_FB_W * NEXUS_FB_H];
    float    z_buffer  [NEXUS_FB_W * NEXUS_FB_H];
    uint32_t palette  [256];
    int clear_color;
} Nexus_Framebuffer;

typedef struct {
    Vec3 pos;
    Vec3 dir;   /* unit direction: (0,0,-1)=N, (1,0,0)=E, (0,0,+1)=S, (-1,0,0)=W */
    float fov;  /* degrees, typically 60 */
    Mat4 view, proj, view_proj;
} Nexus_Camera;

typedef struct {
    Vec3 position;
    Vec2 uv;    /* texture coordinates (0..1); (0,0)=top-left */
    uint8_t color;   /* palette index used when no texture */
    int texture_id;  /* >=0 use texture at atlas[texture_id], -1 solid color */
} Nexus_RasterVertex;

/* ── Initialization ─────────────────────────────────────────────── */
void nexus_fb_init(Nexus_Framebuffer *fb);
void nexus_fb_clear(Nexus_Framebuffer *fb);
/* Inject a 256-entry RGBA palette (0xAARRGGBB). */
void nexus_fb_set_palette(Nexus_Framebuffer *fb, const uint32_t palette[256]);

/* ── Camera ──────────────────────────────────────────────────────── */
/* facing_dir: 0=North, 1=East, 2=South, 3=West */
void nexus_camera_init(Nexus_Camera *cam, Vec3 pos, int facing_dir);
void nexus_camera_update(Nexus_Camera *cam);

/* ── Primitives ──────────────────────────────────────────────────── */
/* Flat shaded triangle */
void nexus_raster_triangle(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam);

/* Textured triangle (affine UV, Saturn-style) */
void nexus_raster_triangle_tex(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette);

/* Flat quad */
void nexus_raster_quad(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1,
    Nexus_RasterVertex v2, Nexus_RasterVertex v3,
    const Nexus_Camera *cam);

/* Textured quad */
void nexus_raster_quad_tex(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1,
    Nexus_RasterVertex v2, Nexus_RasterVertex v3,
    const Nexus_Camera *cam,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette);

/* ── Dungeon geometry ────────────────────────────────────────────── */
/* wall_dir: 0=North(z-), 1=East(x+), 2=South(z+), 3=West(x-)
 * tex_data optional (NULL -> flat shaded, texture_id ignored)      */
void nexus_draw_wall(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int wall_dir, uint8_t color,
    int texture_id, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette);

/* convenience overloads with no texture — flat-shaded wall */
void nexus_draw_wall_simple(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int wall_dir, uint8_t color);

/* Floor + ceiling for a passable square (type != 0) */
void nexus_draw_floor(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z,
    uint8_t floor_color, uint8_t ceil_color);

/* Door state drawing (DM1 DUNGEON.C door semantics):
 *   CLOSED: full-height quad
 *   OPEN:   narrowed side-offset quad (gap visible)
 *   LOCKED: full-height + color brighten (gold tint, key indicator)  */
enum { NEXUS_DOOR_CLOSED = 0, NEXUS_DOOR_OPEN = 1, NEXUS_DOOR_LOCKED = 2 };
void nexus_draw_door(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int facing, int door_state,
    int texture_id, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette);

/* ── Creature rendering ─────────────────────────────────────────── */
/* Project a model vertex (local-fixed -> world -> screen)        */
Vec2i nexus_project_model_vert(const Vec3 *local_vert, float scale,
    const Vec3 *world_pos, const Mat4 *view_proj,
    int screen_w, int screen_h);

/* Build a billboard quad (4 RasterVertices) for a creature.        */
void nexus_raster_billboard(Nexus_RasterVertex quad[4],
    Vec3 world_pos, float width, float height,
    const Nexus_Camera *cam);

/* Render creature billboard (textured or flat-shaded).
 *LEVITATION: hovers 0.2 above floor.
 * FIRE_RESIST: red-tinted flat shade.                            */
void nexus_raster_creature_billboard(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    Vec3 world_pos, float height,
    int texture_id, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette,
    uint32_t creature_flags,
    uint8_t base_color);

/* ── Projectile rendering ─────────────────────────────────────────── */
/* DM1 F0823 projectile types: FIREBALL, LIGHTNING, POISON_CLOUD,
 * GRABBER_BOLT (F0402).  n_points for multi-point arcs.            */
enum Nexus_ProjectileType {
    NEXUS_PROJ_FIREBALL = 0,
    NEXUS_PROJ_LIGHTNING = 1,
    NEXUS_PROJ_POISON_CLOUD = 2,
    NEXUS_PROJ_GRABBER_BOLT = 3,
};
void nexus_raster_projectile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    Vec3 start, Vec3 end,
    const Vec3 *arc_points, int n_points,
    enum Nexus_ProjectileType type,
    const uint32_t *palette);

#endif
