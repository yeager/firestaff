
#include "nexus_v1_rasterizer.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

void nexus_fb_init(Nexus_Framebuffer *fb) {
    if (!fb) return;
    memset(fb, 0, sizeof(*fb));
    fb->clear_color = 0;
    /* Default Nexus palette — dark dungeon tones */
    fb->palette[0]  = 0xFF000000; /* black */
    fb->palette[1]  = 0xFF1A1A2E; /* deep navy */
    fb->palette[2]  = 0xFF16213E; /* dark blue */
    fb->palette[3]  = 0xFF0F3460; /* midnight blue */
    fb->palette[4]  = 0xFF533483; /* purple */
    fb->palette[5]  = 0xFF404040; /* dark gray (walls) */
    fb->palette[6]  = 0xFF606060; /* medium gray */
    fb->palette[7]  = 0xFF808080; /* light gray */
    fb->palette[8]  = 0xFF3C3C3C; /* floor */
    fb->palette[9]  = 0xFF2A2A2A; /* ceiling */
    fb->palette[10] = 0xFF8B4513; /* brown (doors) */
    fb->palette[11] = 0xFF556B2F; /* olive (creatures) */
    fb->palette[12] = 0xFFB22222; /* firebrick (lava) */
    fb->palette[13] = 0xFF4169E1; /* royal blue (water) */
    fb->palette[14] = 0xFFDAA520; /* goldenrod (treasure) */
    fb->palette[15] = 0xFFFFFFFF; /* white */
}

void nexus_fb_clear(Nexus_Framebuffer *fb) {
    if (!fb) return;
    memset(fb->color_buffer, fb->clear_color, sizeof(fb->color_buffer));
    /* Clear Z-buffer to far plane */
    for (int i = 0; i < NEXUS_FB_W * NEXUS_FB_H; i++)
        fb->z_buffer[i] = 1e30f;
}

/* ── Camera ── */

static const Vec3 g_dir_vectors[4] = {
    {0, 0, -1}, /* North */
    {1, 0, 0},  /* East */
    {0, 0, 1},  /* South */
    {-1, 0, 0}  /* West */
};

void nexus_camera_init(Nexus_Camera *cam, Vec3 pos, int facing) {
    if (!cam) return;
    cam->pos = pos;
    cam->dir = g_dir_vectors[facing & 3];
    cam->fov = 60.0f;
    nexus_camera_update(cam);
}

void nexus_camera_update(Nexus_Camera *cam) {
    if (!cam) return;
    Vec3 target = v3_add(cam->pos, cam->dir);
    Vec3 up = {0, 1, 0};
    cam->view = m4_look_at(cam->pos, target, up);
    cam->proj = m4_perspective(cam->fov, (float)NEXUS_FB_W / NEXUS_FB_H, 0.1f, 100.0f);
    cam->view_proj = m4_multiply(cam->proj, cam->view);
}

/* ── Triangle rasterization with Z-buffer ── */

static inline int mini(int a, int b) { return a < b ? a : b; }
static inline int maxi(int a, int b) { return a > b ? a : b; }
static inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

static float edge_fn(Vec2i a, Vec2i b, Vec2i p) {
    return (float)((p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x));
}

void nexus_raster_triangle(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1, Nexus_RasterVertex v2,
    const Nexus_Camera *cam)
{
    Vec2i s0, s1, s2;
    float z0, z1, z2;
    int min_x, max_x, min_y, max_y;
    float area;

    if (!fb || !cam) return;

    /* Project vertices */
    s0 = v3_project(v0.position, cam->view_proj, NEXUS_FB_W, NEXUS_FB_H);
    s1 = v3_project(v1.position, cam->view_proj, NEXUS_FB_W, NEXUS_FB_H);
    s2 = v3_project(v2.position, cam->view_proj, NEXUS_FB_W, NEXUS_FB_H);

    /* Compute Z in view space for depth test */
    {
        Vec4 vz0 = m4_transform(cam->view, (Vec4){v0.position.x, v0.position.y, v0.position.z, 1});
        Vec4 vz1 = m4_transform(cam->view, (Vec4){v1.position.x, v1.position.y, v1.position.z, 1});
        Vec4 vz2 = m4_transform(cam->view, (Vec4){v2.position.x, v2.position.y, v2.position.z, 1});
        z0 = -vz0.z; z1 = -vz1.z; z2 = -vz2.z;
    }

    /* Back-face culling */
    area = edge_fn(s0, s1, s2);
    if (area <= 0) return;

    /* Bounding box (scissored) */
    min_x = clampi(mini(s0.x, mini(s1.x, s2.x)), 0, NEXUS_FB_W - 1);
    max_x = clampi(maxi(s0.x, maxi(s1.x, s2.x)), 0, NEXUS_FB_W - 1);
    min_y = clampi(mini(s0.y, mini(s1.y, s2.y)), 0, NEXUS_FB_H - 1);
    max_y = clampi(maxi(s0.y, maxi(s1.y, s2.y)), 0, NEXUS_FB_H - 1);

    /* Rasterize */
    {
        float inv_area = 1.0f / area;
        Vec2i p;
        for (p.y = min_y; p.y <= max_y; p.y++) {
            for (p.x = min_x; p.x <= max_x; p.x++) {
                float w0 = edge_fn(s1, s2, p) * inv_area;
                float w1 = edge_fn(s2, s0, p) * inv_area;
                float w2 = 1.0f - w0 - w1;
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    float z = w0 * z0 + w1 * z1 + w2 * z2;
                    int idx = p.y * NEXUS_FB_W + p.x;
                    if (z < fb->z_buffer[idx] && z > 0) {
                        fb->z_buffer[idx] = z;
                        fb->color_buffer[idx] = v0.color;
                    }
                }
            }
        }
    }
}

void nexus_raster_quad(Nexus_Framebuffer *fb,
    Nexus_RasterVertex v0, Nexus_RasterVertex v1,
    Nexus_RasterVertex v2, Nexus_RasterVertex v3,
    const Nexus_Camera *cam)
{
    nexus_raster_triangle(fb, v0, v1, v2, cam);
    nexus_raster_triangle(fb, v0, v2, v3, cam);
}

/* ── Dungeon wall rendering ── */

/* Wall direction: 0=North, 1=East, 2=South, 3=West */
void nexus_draw_wall(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, int wall_dir, uint8_t color)
{
    Nexus_RasterVertex v[4];
    float wall_h = 1.0f;
    int i;

    for (i = 0; i < 4; i++) v[i].color = color;

    switch (wall_dir) {
    case 0: /* North wall (z face) */
        v[0].position = (Vec3){x, 0, z};
        v[1].position = (Vec3){x + 1, 0, z};
        v[2].position = (Vec3){x + 1, wall_h, z};
        v[3].position = (Vec3){x, wall_h, z};
        break;
    case 1: /* East wall (x face) */
        v[0].position = (Vec3){x + 1, 0, z};
        v[1].position = (Vec3){x + 1, 0, z + 1};
        v[2].position = (Vec3){x + 1, wall_h, z + 1};
        v[3].position = (Vec3){x + 1, wall_h, z};
        break;
    case 2: /* South wall */
        v[0].position = (Vec3){x + 1, 0, z + 1};
        v[1].position = (Vec3){x, 0, z + 1};
        v[2].position = (Vec3){x, wall_h, z + 1};
        v[3].position = (Vec3){x + 1, wall_h, z + 1};
        break;
    case 3: /* West wall */
        v[0].position = (Vec3){x, 0, z + 1};
        v[1].position = (Vec3){x, 0, z};
        v[2].position = (Vec3){x, wall_h, z};
        v[3].position = (Vec3){x, wall_h, z + 1};
        break;
    }
    nexus_raster_quad(fb, v[0], v[1], v[2], v[3], cam);
}

void nexus_draw_floor(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
    float x, float z, uint8_t floor_color, uint8_t ceil_color)
{
    Nexus_RasterVertex v[4];
    int i;

    /* Floor */
    for (i = 0; i < 4; i++) v[i].color = floor_color;
    v[0].position = (Vec3){x, 0, z};
    v[1].position = (Vec3){x + 1, 0, z};
    v[2].position = (Vec3){x + 1, 0, z + 1};
    v[3].position = (Vec3){x, 0, z + 1};
    nexus_raster_quad(fb, v[0], v[1], v[2], v[3], cam);

    /* Ceiling */
    for (i = 0; i < 4; i++) v[i].color = ceil_color;
    v[0].position = (Vec3){x, 1, z + 1};
    v[1].position = (Vec3){x + 1, 1, z + 1};
    v[2].position = (Vec3){x + 1, 1, z};
    v[3].position = (Vec3){x, 1, z};
    nexus_raster_quad(fb, v[0], v[1], v[2], v[3], cam);
}

