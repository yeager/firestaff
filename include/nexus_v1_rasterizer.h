#ifndef NEXUS_V1_RASTERIZER_H
#define NEXUS_V1_RASTERIZER_H

#include "nexus_v1_math3d.h"
#include "nexus_v1_doors.h"
#include <stdint.h>

/* Nexus V1 Software Rasterizer
 * ==============================
 * Renders to a 320x224 indexed framebuffer (DM.BIN 0x03B960: 319x223).
 *
 * Features:
 *   - Z-buffer (per-pixel depth test)
 *   - Indexed texturing via verified source surfaces
 *   - Affine texture mapping (UV -> screen-space, Saturn-style, no
 *     perspective correction -- appropriate for flat dungeon surfaces)
 *   - Back-face culling (CCW winding)
 *   - Scissor clipping (discard out-of-bounds fragments)
 *   - Texture atlas support (Nexus_Texture from nexus_v1_palette.h)
 *   - Door animation states (open/closed/locked visual variants)
 *   - Creature model projection (DMDF billboard -> rasterizer vertices)
 *   - Projectile rendering (fireball/lightning/poison/bolt paths)
 *   - No visual fallback for unsupported or unverified 3D assets
 *
 * Source-lock references:
 *   ReDMCSB DRAWVIEW.C   -- viewport blit to screen memory (F2172)
 *   ReDMCSB BLIT.C        -- F0132 blit rect primitive
 *   ReDMCSB DUNGEON.C     -- wall square drawing (F0108)
 *   Saturn VDP1 SDK       -- command list format, local coordinates
 *   docs/NEXUS_FILE_CLASSIFICATION.md -- DMDF .MNS + surface files  */

/* ─────────────────────────── Public Types ──────────────────────── */
#define NEXUS_FB_W 320
#define NEXUS_FB_H 224

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
/* Legacy flat triangle entry point; intentionally no-draw without a verified
 * Saturn material. */
void nexus_raster_triangle(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam);

/* Textured triangle (affine UV, Saturn-style) */
void nexus_raster_triangle_tex(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette);

/* Legacy flat quad entry point; intentionally no-draw without a verified
 * Saturn material. */
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

/* Indexed texture with a frame-local CLUT remap. A map entry of 0xff clips
 * the texel before depth is written. This lets independently decoded DMDF/BPK
 * surfaces share the indexed VDP1 framebuffer without replacing their CLUT. */
void nexus_raster_quad_tex_mapped(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1,
    Nexus_RasterVertex v2, Nexus_RasterVertex v3,
    const Nexus_Camera *cam,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256]);

/* ── Dungeon geometry ────────────────────────────────────────────── */
/* wall_dir: 0=North(z-), 1=East(x+), 2=South(z+), 3=West(x-)
 * tex_data and tex_palette are required; unbound surfaces are no-draw. */
void nexus_draw_wall(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int wall_dir, uint8_t color,
    int texture_id, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette);

/* Legacy convenience overload; no-draw because it has no source surface. */
void nexus_draw_wall_simple(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int wall_dir, uint8_t color);

/* Floor + ceiling for a passable square (type != 0) */
void nexus_draw_floor(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z,
    uint8_t floor_color, uint8_t ceil_color);

void nexus_draw_floor_tex(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette);
void nexus_draw_ceiling_tex(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette);

void nexus_draw_floor_tex_mapped(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256]);
void nexus_draw_ceiling_tex_mapped(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256]);

/* DGN Structure1B supplies one signed floor height per cell and optional
 * X/Y slopes. Heights use 1/32 of a world unit; ceiling is parallel. */
void nexus_draw_floor_tex_mapped_heights(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z, const int8_t heights[4],
    uint8_t rotation, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256]);
void nexus_draw_ceiling_tex_mapped_heights(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z, const int8_t heights[4],
    uint8_t rotation, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256]);
void nexus_draw_wall_tex_mapped(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z, int wall_dir,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256]);

/* Door state API retained for the gameplay layer. Rendering is currently
 * no-draw: Saturn door surfaces, animation frames, and VDP1 placement have
 * not been proven from Nexus DGN/MNS/capture evidence. */
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

/* Retained API boundary for the former generic creature billboard route.
 * No-draw until Saturn VDP1 command/CLUT/placement and DMDF/MNS ownership
 * are captured.  Host textures and inferred flags are not sufficient proof. */
void nexus_raster_creature_billboard(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    Vec3 world_pos, float height,
    int texture_id, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette,
    uint32_t creature_flags,
    uint8_t base_color);

/* ── Projectile rendering ─────────────────────────────────────────── */
/* DM1 F0823 projectile types: FIREBALL, LIGHTNING, POISON_CLOUD,
 * GRABBER_BOLT (F0402).  n_points for multi-point arcs.  Extended
 * values (ICEBOLT through ACID) are used by nexus_render_projectile
 * in nexus_v1_rendering.c; both layers share this enum.            */
enum Nexus_ProjectileType {
    NEXUS_PROJ_FIREBALL = 0,
    NEXUS_PROJ_LIGHTNING = 1,
    NEXUS_PROJ_POISON_CLOUD = 2,
    NEXUS_PROJ_GRABBER_BOLT = 3,
    NEXUS_PROJ_ICEBOLT = 4,
    NEXUS_PROJ_ARROW    = 5,
    NEXUS_PROJ_POISON   = 6,
    NEXUS_PROJ_DEATHRAY = 7,
    NEXUS_PROJ_ACID     = 8,
    NEXUS_PROJ_COUNT    = 9,
};
void nexus_raster_projectile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    Vec3 start, Vec3 end,
    const Vec3 *arc_points, int n_points,
    enum Nexus_ProjectileType type,
    const uint32_t *palette);

#endif
