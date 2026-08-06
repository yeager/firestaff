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
 * Missing or unproven texture material is no-draw. */

#include "nexus_v1_rasterizer.h"
#include "nexus_v1_doors.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Math helpers ────────────────────────────────────────────────── */
static inline int mini(int a, int b)  { return a < b ? a : b; }
static inline int maxi(int a, int b)  { return a > b ? a : b; }
static inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

static inline float __attribute__((unused)) fminf_local (float a, float b) { return a < b ? a : b; }
static inline float __attribute__((unused)) fmaxf_local (float a, float b) { return a > b ? a : b; }

static inline float __attribute__((unused)) fclamp (float v, float lo, float hi) {
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
    /* No invented palette: only verified TITLE/STABG/VDP1 material may
     * populate the framebuffer palette through nexus_fb_set_palette(). */
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

/* Public camera API — matches nexus_v1_rasterizer.h declarations */
void nexus_camera_init(Nexus_Camera *cam, Vec3 pos, int facing_dir) {
    if (!cam) return;
    cam->pos = pos;
    cam->dir = g_cam_dir[facing_dir & 3];
    cam->fov = 60.0f;
    nexus_camera_update(cam);
}
void nexus_camera_update(Nexus_Camera *cam) {
    Vec3 target, up = {0, 1, 0};
    if (!cam) return;
    target = v3_add(cam->pos, cam->dir);
    cam->view = m4_look_at(cam->pos, target, up);
    cam->proj = m4_perspective(cam->fov,
        (float)NEXUS_FB_W / (float)NEXUS_FB_H, 0.1f, 100.0f);
    cam->view_proj = m4_multiply(cam->proj, cam->view);
}

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

static int texture_repeat_coordinate(float uv, int extent) {
    int coordinate = (int)floorf(uv * (float)extent);
    coordinate %= extent;
    return coordinate < 0 ? coordinate + extent : coordinate;
}

static void nexus_raster_triangle_tex_mapped(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256])
{
    Vec2i s[3]; float z[3]; int bbox[4]; float area, inv; int x, y;
    Nexus_RasterVertex vs[3] = {v0, v1, v2};

    if (!fb || !cam || !tex_data || !tex_palette || !texel_map ||
        tex_w <= 0 || tex_h <= 0) return;
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
                    float u = w0*v0.uv.x + w1*v1.uv.x + w2*v2.uv.x;
                    float v = w0*v0.uv.y + w1*v1.uv.y + w2*v2.uv.y;
                    int px = texture_repeat_coordinate(u, tex_w);
                    int py = texture_repeat_coordinate(v, tex_h);
                    uint8_t source_index = tex_data[py * tex_w + px];
                    uint8_t framebuffer_index = texel_map[source_index];

                    /* A missing CLUT entry and transparent source texels are
                     * clipped, matching VDP1's no-write path. */
                    if ((tex_palette[source_index] >> 24) == 0U ||
                        framebuffer_index == 0xffU) continue;
                    fb->z_buffer[idx] = zf;
                    fb->color_buffer[idx] = framebuffer_index;
                }
            }
        }
    }
}

void nexus_raster_triangle(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam)
{
    /* Flat-colour geometry has no verified Saturn material contract. */
    (void)fb; (void)v0; (void)v1; (void)v2; (void)cam;
}

void nexus_raster_triangle_tex(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette)
{
    Vec2i s[3]; float z[3]; int bbox[4]; float area, inv; int x, y;
    Nexus_RasterVertex vs[3] = {v0, v1, v2};

    if (!fb || !cam) return;
    /* Strict route: an unbound texture is not a license to synthesize a
     * flat-colour surface.  The caller must submit a verified material. */
    if (!tex_data || !tex_palette || tex_w <= 0 || tex_h <= 0) {
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
                    int px = texture_repeat_coordinate(u, tex_w);
                    int py = texture_repeat_coordinate(v, tex_h);
                    uint8_t ci = tex_data[py * tex_w + px];
                    if ((tex_palette[ci] >> 24) == 0U) continue;
                    fb->z_buffer[idx] = zf;
                    fb->color_buffer[idx] = ci;
                }
            }
        }
    }
}

void nexus_raster_quad_tex_mapped(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1,
    Nexus_RasterVertex v2, Nexus_RasterVertex v3,
    const Nexus_Camera *cam,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256])
{
    if (!fb || !cam || !tex_data || !tex_palette || !texel_map ||
        tex_w <= 0 || tex_h <= 0) return;
    nexus_raster_triangle_tex_mapped(fb, v0, v1, v2, cam,
        tex_data, tex_w, tex_h, tex_palette, texel_map);
    nexus_raster_triangle_tex_mapped(fb, v0, v2, v3, cam,
        tex_data, tex_w, tex_h, tex_palette, texel_map);
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
    /* Unverified material must not become a flat-color substitute. */
    if (!tex_data || tex_w <= 0 || tex_h <= 0 || !tex_palette) {
        return;
    }
    nexus_raster_triangle_tex(fb, v0, v1, v2, cam,
        tex_data, tex_w, tex_h, tex_palette);
    nexus_raster_triangle_tex(fb, v0, v2, v3, cam,
        tex_data, tex_w, tex_h, tex_palette);
}

void nexus_draw_wall_simple(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int wall_dir, uint8_t color) {
    nexus_draw_wall(fb, cam, x, z, wall_dir, color,
        -1, NULL, 0, 0, NULL);
}
/* Wall face quad corners per direction.
 * Source-lock: DUNGEON.C F0108 wall quad at grid square (x,z).    */
static void wall_quad_verts(int wall_dir, float x, float z,
    Nexus_RasterVertex rv[4], uint8_t color,
    int texture_id)
{
    (void)texture_id;
    /* Counter-clockwise faces viewed from inside their grid square. */
    switch (wall_dir & 3) {
    case 0:
        rv[0].position = (Vec3){x, 0.0f, z};
        rv[1].position = (Vec3){x + 1.0f, 0.0f, z};
        rv[2].position = (Vec3){x + 1.0f, 1.0f, z};
        rv[3].position = (Vec3){x, 1.0f, z};
        break;
    case 1:
        rv[0].position = (Vec3){x + 1.0f, 0.0f, z};
        rv[1].position = (Vec3){x + 1.0f, 0.0f, z + 1.0f};
        rv[2].position = (Vec3){x + 1.0f, 1.0f, z + 1.0f};
        rv[3].position = (Vec3){x + 1.0f, 1.0f, z};
        break;
    case 2:
        rv[0].position = (Vec3){x + 1.0f, 0.0f, z + 1.0f};
        rv[1].position = (Vec3){x, 0.0f, z + 1.0f};
        rv[2].position = (Vec3){x, 1.0f, z + 1.0f};
        rv[3].position = (Vec3){x + 1.0f, 1.0f, z + 1.0f};
        break;
    default:
        rv[0].position = (Vec3){x, 0.0f, z + 1.0f};
        rv[1].position = (Vec3){x, 0.0f, z};
        rv[2].position = (Vec3){x, 1.0f, z};
        rv[3].position = (Vec3){x, 1.0f, z + 1.0f};
        break;
    }
    rv[0].color = rv[1].color = rv[2].color = rv[3].color = color;
    /* UVs: 0=bot-left, 1=bot-right, 2=top-right, 3=top-left of wall */
    rv[0].uv = (Vec2){0, 1}; rv[1].uv = (Vec2){1, 1};
    rv[2].uv = (Vec2){1, 0}; rv[3].uv = (Vec2){0, 0};
    rv[0].texture_id = rv[1].texture_id =
        rv[2].texture_id = rv[3].texture_id = texture_id;
}

void nexus_draw_wall_tex_mapped(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z, int wall_dir,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256])
{
    Nexus_RasterVertex rv[4];
    if (!fb || !cam || !tex_data || !tex_palette || !texel_map ||
        tex_w <= 0 || tex_h <= 0) return;
    wall_quad_verts(wall_dir, x, z, rv, 0, -1);
    nexus_raster_quad_tex_mapped(fb, rv[0], rv[1], rv[2], rv[3], cam,
        tex_data, tex_w, tex_h, tex_palette, texel_map);
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
    if (!fb || !cam || !tex_data || !tex_palette || tex_w <= 0 || tex_h <= 0)
        return;
    wall_quad_verts(wall_dir & 3, x, z, rv, color, texture_id);
    nexus_raster_quad_tex(fb, rv[0], rv[1], rv[2], rv[3], cam,
        tex_data, tex_w, tex_h, tex_palette);
}

/* ── Floor / Ceiling ──────────────────────────────────────────────── */
void nexus_draw_floor(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z,
    uint8_t floor_color, uint8_t ceil_color)
{
    (void)x; (void)z; (void)floor_color; (void)ceil_color;
    if (!fb || !cam) return;
    /* Legacy floor/ceiling calls have no verified Saturn surface argument;
     * do not turn their colour indices into invented geometry pixels. */
    return;
}

void nexus_draw_floor_tex(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette)
{
    Nexus_RasterVertex v[4];
    if (!fb || !cam || !tex_data || !tex_palette || tex_w <= 0 || tex_h <= 0)
        return;
    v[0].position = (Vec3){x, 0, z};     v[0].uv = (Vec2){0, 1};
    v[1].position = (Vec3){x + 1, 0, z}; v[1].uv = (Vec2){1, 1};
    v[2].position = (Vec3){x + 1, 0, z + 1}; v[2].uv = (Vec2){1, 0};
    v[3].position = (Vec3){x, 0, z + 1}; v[3].uv = (Vec2){0, 0};
    v[0].color = v[1].color = v[2].color = v[3].color = 0;
    nexus_raster_quad_tex(fb, v[0], v[1], v[2], v[3], cam,
                          tex_data, tex_w, tex_h, tex_palette);
}

void nexus_draw_ceiling_tex(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette)
{
    Nexus_RasterVertex v[4];
    if (!fb || !cam || !tex_data || !tex_palette || tex_w <= 0 || tex_h <= 0)
        return;
    v[0].position = (Vec3){x, 1, z + 1}; v[0].uv = (Vec2){0, 0};
    v[1].position = (Vec3){x + 1, 1, z + 1}; v[1].uv = (Vec2){1, 0};
    v[2].position = (Vec3){x + 1, 1, z}; v[2].uv = (Vec2){1, 1};
    v[3].position = (Vec3){x, 1, z}; v[3].uv = (Vec2){0, 1};
    v[0].color = v[1].color = v[2].color = v[3].color = 0;
    nexus_raster_quad_tex(fb, v[0], v[1], v[2], v[3], cam,
                          tex_data, tex_w, tex_h, tex_palette);
}

void nexus_draw_floor_tex_mapped(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256])
{
    Nexus_RasterVertex v[4];
    if (!fb || !cam || !tex_data || !tex_palette || !texel_map ||
        tex_w <= 0 || tex_h <= 0) return;
    v[0].position = (Vec3){x, 0, z}; v[0].uv = (Vec2){0, 1};
    v[1].position = (Vec3){x + 1, 0, z}; v[1].uv = (Vec2){1, 1};
    v[2].position = (Vec3){x + 1, 0, z + 1}; v[2].uv = (Vec2){1, 0};
    v[3].position = (Vec3){x, 0, z + 1}; v[3].uv = (Vec2){0, 0};
    nexus_raster_quad_tex_mapped(fb, v[0], v[1], v[2], v[3], cam,
        tex_data, tex_w, tex_h, tex_palette, texel_map);
}

void nexus_draw_ceiling_tex_mapped(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z,
    const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256])
{
    Nexus_RasterVertex v[4];
    if (!fb || !cam || !tex_data || !tex_palette || !texel_map ||
        tex_w <= 0 || tex_h <= 0) return;
    v[0].position = (Vec3){x, 1, z + 1}; v[0].uv = (Vec2){0, 0};
    v[1].position = (Vec3){x + 1, 1, z + 1}; v[1].uv = (Vec2){1, 0};
    v[2].position = (Vec3){x + 1, 1, z}; v[2].uv = (Vec2){1, 1};
    v[3].position = (Vec3){x, 1, z}; v[3].uv = (Vec2){0, 1};
    nexus_raster_quad_tex_mapped(fb, v[0], v[1], v[2], v[3], cam,
        tex_data, tex_w, tex_h, tex_palette, texel_map);
}

static void dgn_surface_uv(Nexus_RasterVertex v[4], uint8_t rotation,
                           int ceiling) {
    static const Vec2 uv[4] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
    for (int i = 0; i < 4; ++i) {
        int source = ceiling ? 3 - i : i;
        v[i].uv = uv[(source + (rotation & 3U)) & 3];
        v[i].color = 0;
    }
}

static void dgn_surface_positions(Nexus_RasterVertex v[4], float x, float z,
                                  const int8_t heights[4], int ceiling) {
    static const float dx[4] = {0, 1, 1, 0};
    static const float dz[4] = {0, 0, 1, 1};
    for (int i = 0; i < 4; ++i) {
        int source = ceiling ? 3 - i : i;
        v[i].position = (Vec3){x + dx[source],
            (float)heights[source] / 32.0f, z + dz[source]};
    }
}

void nexus_draw_floor_tex_mapped_heights(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z, const int8_t heights[4],
    uint8_t rotation, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256])
{
    Nexus_RasterVertex v[4];
    if (!fb || !cam || !heights || !tex_data || !tex_palette || !texel_map ||
        tex_w <= 0 || tex_h <= 0) return;
    dgn_surface_positions(v, x, z, heights, 0);
    dgn_surface_uv(v, rotation, 0);
    nexus_raster_quad_tex_mapped(fb, v[0], v[1], v[2], v[3], cam,
        tex_data, tex_w, tex_h, tex_palette, texel_map);
}

void nexus_draw_ceiling_tex_mapped_heights(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam, float x, float z, const int8_t heights[4],
    uint8_t rotation, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette, const uint8_t texel_map[256])
{
    Nexus_RasterVertex v[4];
    if (!fb || !cam || !heights || !tex_data || !tex_palette || !texel_map ||
        tex_w <= 0 || tex_h <= 0) return;
    dgn_surface_positions(v, x, z, heights, 1);
    dgn_surface_uv(v, rotation, 1);
    nexus_raster_quad_tex_mapped(fb, v[0], v[1], v[2], v[3], cam,
        tex_data, tex_w, tex_h, tex_palette, texel_map);
}

/* ── Door rendering ───────────────────────────────────────────────── */
void nexus_draw_door(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int facing, int door_state,
    int texture_id, const uint8_t *tex_data, int tex_w, int tex_h,
    const uint32_t *tex_palette)
{
    /* The former implementation copied DM1 DUNGEON.C F0107 semantics into
     * Nexus: guessed gap geometry and palette indices 10/14. Saturn door
     * surfaces, animation frames, and VDP1 destination placement are not yet
     * bound to a verified DGN/MNS/VDP1 receipt. Keep the public API for the
     * gameplay state layer, but fail closed instead of emitting synthetic
     * pixels when a caller supplies an arbitrary host texture. */
    (void)fb; (void)cam; (void)x; (void)z; (void)facing;
    (void)door_state; (void)texture_id; (void)tex_data; (void)tex_w;
    (void)tex_h; (void)tex_palette;
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
    /* A Saturn creature draw needs the original VDP1 command, CLUT,
     * placement and DMDF/MNS owner.  This legacy API accepts only a host
     * texture and inferred gameplay flags, so it cannot prove those facts.
     * Retain the symbol for source-study callers, but never admit its
     * synthetic billboard to the production viewport. */
    (void)fb;
    (void)cam;
    (void)world_pos;
    (void)height;
    (void)texture_id;
    (void)tex_data;
    (void)tex_w;
    (void)tex_h;
    (void)tex_palette;
    (void)creature_flags;
    (void)base_color;
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
    (void)arc_points; (void)n_points;

    if (!fb || !cam || !palette) return;

    /* No verified Saturn effect stream or VDP1 binding exists, so Nexus
     * deliberately emits no projectile pixels.  Keep this boundary explicit
     * until an original effect asset/capture supplies the pixel owner. */
    (void)start; (void)end; (void)type;
}
