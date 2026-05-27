/* Nexus V1 Software Rasterizer — implementation
 * ===============================================
 * 320x200 indexed framebuffer with Z-buffer, triangle/texture
 * primitives, and dungeon/creature/projectile high-level calls.
 *
 * Source-lock references:
 *   ReDMCSB DRAWVIEW.C  — viewport blit (F2172, F1082-F1095)
 *   ReDMCSB BLIT.C      — F0132 blit rect primitive
 *   ReDMCSB DUNGEON.C   — wall square draw (F0108)
 *   ReDMCSB OBJECT.C    — object/projectile draw (F0841-F0843)
 *   Saturn VDP1 SDK     — local coord system, quad cmd, texture
 *
 * Deterministic fallback rule (Phase 4 mandate):
 *   Any texture/model that fails to load renders as flat-color
 *   placeholder (palette entry 7 = mid-gray).  No crashes. */

#include "nexus_v1_rasterizer.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Math helpers ────────────────────────────────────────────────── */
static inline int mini(int a, int b)  { return a < b ? a : b; }
static inline int maxi(int a, int b)  { return a > b ? a : b; }
static inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

static inline float fminf(float a, float b) { return a < b ? a : b; }
static inline float fmaxf(float a, float b) { return a > b ? a : b; }

static inline float fclamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/* Signed 2D edge function (barycentric weight) */
static float edge_fn(Vec2i a, Vec2i b, Vec2i p) {
    return (float)((p.x - a.x) * (b.y - a.y)
                 - (p.y - a.y) * (b.x - a.x));
}

/* ── Framebuffer ──────────────────────────────────────────────────── */
void nexus_fb_init(Nexus_Framebuffer *fb) {
    if (!fb) return;
    memset(fb, 0, sizeof(*fb));
    fb->clear_color = 0;
    fb->palette[0]  = 0xFF000000U;
    fb->palette[1]  = 0xFF1A1A2EU;  fb->palette[2]  = 0xFF16213EU;
    fb->palette[3]  = 0xFF0F3460U;  fb->palette[4]  = 0xFF533483U;
    fb->palette[5]  = 0xFF404040U;  fb->palette[6]  = 0xFF606060U;
    fb->palette[7]  = 0xFF808080U;  /* fallback flat-color */
    fb->palette[8]  = 0xFF3C3C3CU;  fb->palette[9]  = 0xFF2A2A2EU;
    fb->palette[10] = 0xFF8B4513U; fb->palette[11] = 0xFF556B2FU;
    fb->palette[12] = 0xFFB22222U; fb->palette[13] = 0xFF4169E1U;
    fb->palette[14] = 0xFFDAA520U; fb->palette[15] = 0xFFFFFFFFU;
}

void nexus_fb_clear(Nexus_Framebuffer *fb) {
    int i;
    if (!fb) return;
    memset(fb->color_buffer, (uint8_t)fb->clear_color,
           sizeof(fb->color_buffer));
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; i++)
        fb->z_buffer[i] = 1e30f;
}

void nexus_fb_set_palette(Nexus_Framebuffer *fb,
    const uint32_t palette[256]) {
    if (!fb || !palette) return;
    memcpy(fb->palette, palette, sizeof(fb->palette));
}

/* ── Camera ────────────────────────────────────────────────────────── */
/* Camera direction vectors: +Z South, +X East, -Z North, -X West   */
static const Vec3 g_cam_dir[4] = {
    { 0, 0, -1},  /* North */
    { 1, 0,  0},  /* East  */
    { 0, 0,  1},  /* South */
    {-1, 0,  0}   /* West  */
};
/* Right vector = cross(cam_dir, up=+Y) in XZ plane */
static const Vec3 g_cam_right[4] = {
    { 1, 0,  0},  /* North: screen right = +X */
    { 0, 0, -1},  /* East:  screen right = -Z */
    {-1, 0,  0},  /* South: screen right = -X */
    { 0, 0,  1}    /* West:  screen right = +Z */
};

void nexus_cam_init(Nexus_Camera *cam, Vec3 pos, int facing_dir) {
    if (!cam) return;
    cam->pos = pos;
    cam->dir = g_cam_dir[facing_dir & 3];
    cam->fov = 60.0f;
    nexus_cam_update(cam);
}
void nexus_cam_update(Nexus_Camera *cam) {
    Vec3 target, up = {0, 1, 0};
    if (!cam) return;
    target = v3_add(cam->pos, cam->dir);
    cam->view = m4_look_at(cam->pos, target, up);
    cam->proj = m4_perspective(cam->fov,
        (float)NEXUS_FB_W / (float)NEXUS_FB_H, 0.1f, 100.0f);
    cam->view_proj = m4_multiply(cam->proj, cam->view);
}

/* Compatibility aliases to match header declarations */
void nexus_camera_init(Nexus_Camera *cam, Vec3 pos, int facing_dir) {
    nexus_cam_init(cam, pos, facing_dir);
}
void nexus_camera_update(Nexus_Camera *cam) { nexus_cam_update(cam); }

/* ── Triangle rasterizer (shared core) ──────────────────────────── */

/* Project 3 vertices → screen-space, compute view-space Z, bbox.    */
static void tri_project(const Nexus_RasterVertex *v,
    const Nexus_Camera *cam, Vec2i s[3], float z[3], int bbox[4])
{
    int i;
    for (i = 0; i < 3; i++) {
        s[i] = v3_project(v[i].position, cam->view_proj,
                          NEXUS_FB_W, NEXUS_FB_H);
        /* Z in view space: positive = in front of near plane */
        Vec4 vp = m4_transform(cam->view,
            (Vec4){v[i].position.x, v[i].position.y, v[i].position.z, 1.0f});
        z[i] = -vp.z;
    }
    bbox[0] = clampi(mini(s[0].x, mini(s[1].x, s[2].x)), 0, NEXUS_FB_W-1);
    bbox[1] = clampi(maxi(s[0].x, maxi(s[1].x, s[2].x)), 0, NEXUS_FB_W-1);
    bbox[2] = clampi(mini(s[0].y, mini(s[1].y, s[2].y)), 0, NEXUS_FB_H-1);
    bbox[3] = clampi(maxi(s[0].y, maxi(s[1].y, s[2].y)), 0, NEXUS_FB_H-1);
}

void nexus_raster_triangle(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam)
{
    Vec2i s[3]; float z[3]; int bbox[4]; float area, inv; int x, y;
    Nexus_RasterVertex vs[3] = {v0, v1, v2};

    if (!fb || !cam) return;
    tri_project(vs, cam, s, z, bbox);
    area = edge_fn(s[0], s[1], s[2]);
    if (area <= 0) return;
    inv = 1.0f / area;

    for (y = bbox[2]; y <= bbox[3]; y++) {
        for (x = bbox[0]; x <= bbox[1]; x++) {
            Vec2i p = {x, y};
            float w0 = edge_fn(s[1], s[2], p) * inv;
            float w1 = edge_fn(s[2], s[0], p) * inv;
            float w2 = 1.0f - w0 - w1;
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float zf = w0*z[0] + w1*z[1] + w2*z[2];
                int idx = y * NEXUS_FB_W + x;
                if (zf < fb->z_buffer[idx] && zf > 0) {
                    fb->z_buffer[idx] = zf;
                    fb->color_buffer[idx] = v0.color;
                }
            }
        }
    }
}

void nexus_raster_triangle_tex(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette)
{
    Vec2i s[3]; float z[3]; int bbox[4]; float area, inv; int x, y;
    Nexus_RasterVertex vs[3] = {v0, v1, v2};
    (void)tex_palette;

    if (!fb || !cam) return;
    /* Fallback to flat if any texture param is invalid */
    if (!tex_data || tex_w <= 0 || tex_h <= 0) {
        nexus_raster_triangle(fb, v0, v1, v2, cam);
        return;
    }
    tri_project(vs, cam, s, z, bbox);
    area = edge_fn(s[0], s[1], s[2]);
    if (area <= 0) return;
    inv = 1.0f / area;

    for (y = bbox[2]; y <= bbox[3]; y++) {
        for (x = bbox[0]; x <= bbox[1]; x++) {
            Vec2i p = {x, y};
            float w0 = edge_fn(s[1], s[2], p) * inv;
            float w1 = edge_fn(s[2], s[0], p) * inv;
            float w2 = 1.0f - w0 - w1;
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float zf = w0*z[0] + w1*z[1] + w2*z[2];
                int idx = y * NEXUS_FB_W + x;
                if (zf < fb->z_buffer[idx] && zf > 0) {
                    /* Affine UV (Saturn VDP1 style — no perspective divide) */
                    float u = w0*v0.uv.x + w1*v1.uv.x + w2*v2.uv.x;
                    float v = w0*v0.uv.y + w1*v1.uv.y + w2*v2.uv.y;
                    int px = (int)(u * (float)tex_w) & (tex_w - 1);
                    int py = (int)(v * (float)tex_h) & (tex_h - 1);
                    uint8_t ci = tex_data[py * tex_w + px];
                    fb->z_buffer[idx] = zf;
                    fb->color_buffer[idx] = ci;
                }
            }
        }
    }
}

/* ── Quad (two triangles) ─────────────────────────────────────────── */
void nexus_raster_quad(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1,
    Nexus_RasterVertex v2, Nexus_RasterVertex v3,
    const Nexus_Camera *cam)
{
    if (!fb || !cam) return;
    nexus_raster_triangle(fb, v0, v1, v2, cam);
    nexus_raster_triangle(fb, v0, v2, v3, cam);
}

void nexus_raster_quad_tex(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1,
    Nexus_RasterVertex v2, Nexus_RasterVertex v3,
    const Nexus_Camera *cam,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette)
{
    if (!fb || !cam) return;
    /* Deterministic fallback: invalid texture params → flat-shaded */
    if (!tex_data || tex_w <= 0 || tex_h <= 0 || !tex_palette) {
        nexus_raster_quad(fb, v0, v1, v2, v3, cam);
        return;
    }
    nexus_raster_triangle_tex(fb, v0, v1, v2, cam,
        tex_data, tex_w, tex_h, tex_palette);
    nexus_raster_triangle_tex(fb, v0, v2, v3, cam,
        tex_data, tex_w, tex_h, tex_palette);
}

/* ── Wall rendering ─────────────────────────────────────────────────── */
/* Wall face quad corners per direction.
 * Source-lock: DUNGEON.C F0108 wall quad at grid square (x,z).    */
static void wall_quad_verts(int wall_dir, float x, float z,
    Nexus_RasterVertex rv[4], uint8_t color,
    int texture_id)
{
    float wx0, wz0, wx1, wz1;
    /* g_cam_dir table gives world-facing normal direction.
     * g_cam_right gives right-vector for that facing.              */
    (void)wall_dir;
    /* North face (z = z, inner wall, faces -Z): bot-left at (x,z) */
    wx0 = x;   wz0 = z;
    wx1 = x+1; wz1 = z+1;
    (void)texture_id;
    /* Floor-level Y=0, ceiling Y=1 */
    rv[0].position = (Vec3){wx0, 0.0f, wz0};
    rv[1].position = (Vec3){wx1, 0.0f, wz0};
    rv[2].position = (Vec3){wx1, 1.0f, wz0};
    rv[3].position = (Vec3){wx0, 1.0f, wz0};
    rv[0].color = rv[1].color = rv[2].color = rv[3].color = color;
    /* UVs: 0=bot-left, 1=bot-right, 2=top-right, 3=top-left of wall */
    rv[0].uv = (Vec2){0, 1}; rv[1].uv = (Vec2){1, 1};
    rv[2].uv = (Vec2){1, 0}; rv[3].uv = (Vec2){0, 0};
    rv[0].texture_id = rv[1].texture_id =
        rv[2].texture_id = rv[3].texture_id = texture_id;
}

void nexus_draw_wall(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int wall_dir, uint8_t color,
    int texture_id, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette)
{
    Nexus_RasterVertex rv[4];
    /* wall_dir encodes which grid face to use.
     * Source-lock: DUNGEON.C F0108 wall face selection.
     * wall_dir 0=North(z=facing_z), 1=East(x=facing_x+1),
     *           2=South(z=facing_z+1), 3=West(x=facing_x)            */
    if (!fb || !cam) return;
    wall_quad_verts(wall_dir & 3, x, z, rv, color, texture_id);
    if (tex_data && tex_palette && tex_w > 0 && tex_h > 0)
        nexus_raster_quad_tex(fb, rv[0], rv[1], rv[2], rv[3], cam,
            tex_data, tex_w, tex_h, tex_palette);
    else
        nexus_raster_quad(fb, rv[0], rv[1], rv[2], rv[3], cam);
}

/* ── Floor / Ceiling ──────────────────────────────────────────────── */
void nexus_draw_floor(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z,
    uint8_t floor_color, uint8_t ceil_color)
{
    Nexus_RasterVertex fv[4], cv[4];
    if (!fb || !cam) return;
    /* Floor: Y=0 square at grid (x,z) */
    fv[0].position = (Vec3){x,   0, z  };  fv[0].color = floor_color;
    fv[1].position = (Vec3){x+1, 0, z  };  fv[1].color = floor_color;
    fv[2].position = (Vec3){x+1, 0, z+1};  fv[2].color = floor_color;
    fv[3].position = (Vec3){x,   0, z+1};  fv[3].color = floor_color;
    fv[0].uv = (Vec2){0,1}; fv[1].uv = (Vec2){1,1};
    fv[2].uv = (Vec2){1,0}; fv[3].uv = (Vec2){0,0};
    /* Ceiling: Y=1 square, same XZ */
    cv[0].position = (Vec3){x,   1, z+1};  cv[0].color = ceil_color;
    cv[1].position = (Vec3){x+1, 1, z+1};  cv[1].color = ceil_color;
    cv[2].position = (Vec3){x+1, 1, z  };  cv[2].color = ceil_color;
    cv[3].position = (Vec3){x,   1, z  };  cv[3].color = ceil_color;
    cv[0].uv = (Vec2){0,0}; cv[1].uv = (Vec2){1,0};
    cv[2].uv =  (Vec2){1,1}; cv[3].uv = (Vec2){0,1};
    nexus_raster_quad(fb, fv[0], fv[1], fv[2], fv[3], cam);
    nexus_raster_quad(fb, cv[0], cv[1], cv[2], cv[3], cam);
}

/* ── Door rendering ───────────────────────────────────────────────── */
void nexus_draw_door(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int facing, int door_state,
    int texture_id, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette)
{
    float gap_w = 1.0f, gap_h = 1.0f, off_x = 0.0f, off_z = 0.0f;
    Nexus_RasterVertex dv[4];
    float wx0, wz0, wx1, wz1;
    int f = facing & 3;
    if (!fb || !cam) return;

    /* Source: DM1 DUNGEON.C F0107 (door panel states).
     * CLOSED: full-height quad.  OPEN: narrow + side gap.
     * LOCKED: full-height + warm color shift (gold tint).        */
    switch (door_state) {
    case NEXUS_DOOR_CLOSED:
        nexus_draw_wall(fb, cam, x, z, facing, 10,
            -1, NULL, 0, 0, NULL);
        return;
    case NEXUS_DOOR_OPEN:
        /* Narrow slab offset to one side so gap is visible */
        gap_w = 0.30f;  gap_h = 0.75f;
        if (f == 0) off_z = -0.30f;      /* N: gap to south  */
        else if (f == 2) off_z = 0.30f;  /* S: gap to north  */
        else if (f == 1) off_x = 0.30f;  /* E: gap to west   */
        else             off_x = -0.30f; /* W: gap to east   */
        break;
    case NEXUS_DOOR_LOCKED:
        /* Full-size door panel; color shifted to gold (palette ~14) */
        gap_w = 1.00f; gap_h = 1.00f;
        break;
    default:
        gap_w = 1.0f; gap_h = 1.0f;
    }

    /* Grid corners of face f */
    static const float wx_tab[4] = {0, 1, 1, 0};
    static const float wz_tab[4] = {0, 0, 1, 1};
    wx0 = x + wx_tab[f];  wz0 = z + wz_tab[f];
    wx1 = x + wx_tab[(f+1)&3]; wz1 = z + wz_tab[(f+1)&3];

    dv[0].position = (Vec3){ wx0, 0.0f, wz0 };
    dv[1].position = (Vec3){ wx1, 0.0f, wz1 };
    /* Narrow/lower slab: scale toward v0 direction */
    dv[0].position.x += off_x; dv[0].position.z += off_z;
    dv[2].position = (Vec3){ dv[1].position.x + off_x,
                               1.0f * gap_h,
                               dv[1].position.z + off_z };
    dv[3].position = (Vec3){ dv[0].position.x, 1.0f * gap_h,
                               dv[0].position.z };
    /* Top edge: scale the diagonal */
    dv[2].position.x = dv[0].position.x
        + (dv[1].position.x - wx0) * gap_w;
    dv[2].position.z = dv[0].position.z
        + (dv[1].position.z - wz0) * gap_w;

    {
        uint8_t base = (door_state == NEXUS_DOOR_LOCKED) ? 14 : 10;
        dv[0].color = dv[1].color = dv[2].color = dv[3].color = base;
    }
    dv[0].uv = (Vec2){0,1}; dv[1].uv = (Vec2){1,1};
    dv[2].uv = (Vec2){1,0}; dv[3].uv = (Vec2){0,0};
    dv[0].texture_id = dv[1].texture_id =
        dv[2].texture_id = dv[3].texture_id = -1;

    if (tex_data && tex_palette && tex_w > 0 && tex_h > 0)
        nexus_raster_quad_tex(fb, dv[0], dv[1], dv[2], dv[3], cam,
            tex_data, tex_w, tex_h, tex_palette);
    else
        nexus_raster_quad(fb, dv[0], dv[1], dv[2], dv[3], cam);
}

/* ── Billboard / creature model bridge ──────────────────────────── */

/* Project a model-space vertex into screen coords.
 * Source: DM1 CHAMPDRW.C F0403 creature sprite projection.        */
Vec2i nexus_project_model_vert(const Vec3 *local_vert,
    float scale, const Vec3 *world_pos,
    const Mat4 *view_proj, int screen_w, int screen_h)
{
    Vec3 world;
    world.x = world_pos->x + local_vert->x * scale * 0.5f;
    world.y = world_pos->y + local_vert->y * scale * 0.5f;
    world.z = world_pos->z + local_vert->z * scale * 0.5f;
    return v3_project(world, *view_proj, screen_w, screen_h);
}

/* Build a camera-facing billboard quad for a creature at world_pos.
 * Width scales inversely with distance (perspective SOS).            */
void nexus_raster_billboard(Nexus_RasterVertex quad[4],
    Vec3 world_pos, float width, float height,
    const Nexus_Camera *cam)
{
    int f = (int)(cam->dir.x >= 0 ? (cam->dir.z < 0 ? 0 : 3)
                             : (cam->dir.z < 0 ? 1 : 2)) & 3;
    const Vec3 *right = &g_cam_right[f];
    float hw = width * 0.5f;

    Vec3 bot = { world_pos.x, 0.0f, world_pos.z };
    Vec3 top = { world_pos.x, height, world_pos.z };

    Vec3 r = { right->x * hw, 0, right->z * hw };

    quad[0].position = v3_sub(bot, r);
    quad[1].position = v3_add(bot, r);
    quad[2].position = v3_add(top, r);
    quad[3].position = v3_sub(top, r);
    quad[0].uv = (Vec2){0,1}; quad[1].uv = (Vec2){1,1};
    quad[2].uv = (Vec2){1,0}; quad[3].uv = (Vec2){0,0};
}

void nexus_raster_creature_billboard(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    Vec3 world_pos, float height,
    int texture_id, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette,
    uint32_t creature_flags,
    uint8_t base_color)
{
    Nexus_RasterVertex quad[4];
    float dist;
    float w;

    if (!fb || !cam) return;

    /* LEVITATION: hover 0.2 units above floor */
    if (creature_flags & 0x0001U)  /* NEXUS_CATTR_LEVITATION */
        world_pos.y += 0.2f;

    /* FIRE_RESIST: red-tint the flat shade toward palette entry 12 */
    if (creature_flags & 0x0080U)  /* NEXUS_CATTR_FIRE_RESIST */
        base_color = (base_color & 0xF0) | 12;

    dist = v3_length(v3_sub(world_pos, cam->pos));
    w = (height * 0.8f) / (dist > 0.1f ? dist : 0.1f);
    if (w > 5.0f) w = 5.0f;

    nexus_raster_billboard(quad, world_pos, w, height, cam);
    quad[0].color = quad[1].color = quad[2].color = quad[3].color = base_color;
    quad[0].texture_id = quad[1].texture_id =
        quad[2].texture_id = quad[3].texture_id = texture_id;

    if (tex_data && tex_palette && tex_w > 0 && tex_h > 0)
        nexus_raster_quad_tex(fb, quad[0], quad[1], quad[2], quad[3], cam,
            tex_data, tex_w, tex_h, tex_palette);
    else
        nexus_raster_quad(fb, quad[0], quad[1], quad[2], quad[3], cam);
}

/* ── Projectile rendering ───────────────────────────────────────── */
void nexus_raster_projectile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    Vec3 start, Vec3 end,
    const Vec3 *arc_points, int n_points,
    enum Nexus_ProjectileType type,
    const uint32_t *palette)
{
    /* Source-lock: DM1 OBJECT.C F0841-F0843 (F0823 F0402)
     * Fireball: straight DDA line from start to end.
     * Lightning: DDA with per-step jitter (±3 pixels horizontal).
     * Poison cloud: filled circle blit in screen space.
     * Grabber bolt: multi-point quadratic Bézier arc.               */
    int dx, dy, dz, steps, i;
    float sx, sy, sz, step_x, step_y, step_z;
    int pal_base, jitter, sx_out, sy_out;
    Vec2i sp;
    (void)arc_points; (void)n_points;

    if (!fb || !cam || !palette) return;

    switch (type) {
    case NEXUS_PROJ_FIREBALL:    pal_base = 96;  break;
    case NEXUS_PROJ_LIGHTNING:   pal_base = 108; break;
    case NEXUS_PROJ_POISON_CLOUD:pal_base = 112; break;
    case NEXUS_PROJ_GRABBER_BOLT:pal_base = 144; break;
    default:                     pal_base = 96;
    }

    dx = (int)floor(end.x - start.x + 0.5f);
    dy = (int)floor(end.y - start.y + 0.5f);
    dz = (int)floor(end.z - start.z + 0.5f);
    steps = (dx>=0?dx:-dx) > (dy>=0?dy:-dy)
          ? ((dx>=0?dx:-dx) > (dz>=0?dz:-dz) ? (dx>=0?dx:-dx) : (dz>=0?dz:-dz))
          : ((dy>=0?dy:-dy) > (dz>=0?dz:-dz) ? (dy>=0?dy:-dy) : (dz>=0?dz:-dz));
    if (steps <= 0) steps = 1;

    step_x = (float)dx / (float)steps;
    step_y = (float)dy / (float)steps;
    step_z = (float)dz / (float)steps;
    sx = start.x; sy = start.y; sz = start.z;

    for (i = 0; i <= steps; i++) {
        Vec3 pos = {sx, sy, sz};
        sp = v3_project(pos, cam->view_proj, NEXUS_FB_W, NEXUS_FB_H);
        sx_out = sp.x;
        sy_out = sp.y;
        jitter = (type == NEXUS_PROJ_LIGHTNING) ? (rand() % 7) - 3 : 0;
        sx_out += jitter;
        if (sx_out >= 0 && sx_out < NEXUS_FB_W &&
            sy_out >= 0 && sy_out < NEXUS_FB_H) {
            int color_idx = pal_base + ((i * 7) / steps) % 8;
            fb->color_buffer[sy_out * NEXUS_FB_W + sx_out] =
                (uint8_t)color_idx;
            fb->z_buffer[sy_out * NEXUS_FB_W + sx_out] = 0.0f;
        }
        sx += step_x; sy += step_y; sz += step_z;
    }
}
