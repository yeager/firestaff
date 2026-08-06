/* Nexus V1 rasterizer runtime boundary
 *
 * The indexed CPU rasterizer in nexus_v1_rasterizer.c is retained for
 * explicit source-format/material probes only.  It is not a Saturn VDP1
 * command consumer: a caller-supplied host texture cannot prove the retail
 * CLUT, VRAM destination, command order, or DGN/MNS owner.  The production
 * library therefore links this lifecycle-safe no-op adapter until an
 * authenticated VDP1 capture is available.
 */

#include "nexus_v1_rasterizer.h"
#include <string.h>

void nexus_fb_init(Nexus_Framebuffer *fb) {
    if (!fb) return;
    memset(fb, 0, sizeof(*fb));
}

void nexus_fb_clear(Nexus_Framebuffer *fb) {
    if (!fb) return;
    memset(fb->color_buffer, (uint8_t)fb->clear_color,
           sizeof(fb->color_buffer));
    for (int i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i)
        fb->z_buffer[i] = 1e30f;
}

void nexus_fb_set_palette(Nexus_Framebuffer *fb,
                          const uint32_t palette[256]) {
    if (!fb || !palette) return;
    /* Retain palette receipts for diagnostics; this does not authorize draw. */
    memcpy(fb->palette, palette, sizeof(fb->palette));
}

void nexus_camera_init(Nexus_Camera *cam, Vec3 pos, int facing_dir) {
    if (!cam) return;
    memset(cam, 0, sizeof(*cam));
    cam->pos = pos;
    cam->fov = 60.0f;
    (void)facing_dir;
}

void nexus_camera_update(Nexus_Camera *cam) {
    (void)cam;
}

void nexus_raster_triangle(Nexus_Framebuffer *fb, Nexus_RasterVertex v0,
                           Nexus_RasterVertex v1, Nexus_RasterVertex v2,
                           const Nexus_Camera *cam) {
    (void)fb; (void)v0; (void)v1; (void)v2; (void)cam;
}

void nexus_raster_triangle_tex(Nexus_Framebuffer *fb, Nexus_RasterVertex v0,
                               Nexus_RasterVertex v1, Nexus_RasterVertex v2,
                               const Nexus_Camera *cam,
                               const uint8_t *tex_data, int tex_w, int tex_h,
                               const uint32_t *tex_palette) {
    (void)fb; (void)v0; (void)v1; (void)v2; (void)cam;
    (void)tex_data; (void)tex_w; (void)tex_h; (void)tex_palette;
}

void nexus_raster_quad(Nexus_Framebuffer *fb, Nexus_RasterVertex v0,
                       Nexus_RasterVertex v1, Nexus_RasterVertex v2,
                       Nexus_RasterVertex v3, const Nexus_Camera *cam) {
    (void)fb; (void)v0; (void)v1; (void)v2; (void)v3; (void)cam;
}

void nexus_raster_quad_tex(Nexus_Framebuffer *fb, Nexus_RasterVertex v0,
                           Nexus_RasterVertex v1, Nexus_RasterVertex v2,
                           Nexus_RasterVertex v3, const Nexus_Camera *cam,
                           const uint8_t *tex_data, int tex_w, int tex_h,
                           const uint32_t *tex_palette) {
    (void)fb; (void)v0; (void)v1; (void)v2; (void)v3; (void)cam;
    (void)tex_data; (void)tex_w; (void)tex_h; (void)tex_palette;
}

void nexus_raster_quad_tex_mapped(Nexus_Framebuffer *fb,
                                  Nexus_RasterVertex v0,
                                  Nexus_RasterVertex v1,
                                  Nexus_RasterVertex v2,
                                  Nexus_RasterVertex v3,
                                  const Nexus_Camera *cam,
                                  const uint8_t *tex_data, int tex_w,
                                  int tex_h, const uint32_t *tex_palette,
                                  const uint8_t texel_map[256]) {
    (void)fb; (void)v0; (void)v1; (void)v2; (void)v3; (void)cam;
    (void)tex_data; (void)tex_w; (void)tex_h; (void)tex_palette;
    (void)texel_map;
}

void nexus_draw_wall(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
                     float x, float z, int wall_dir, uint8_t color,
                     int texture_id, const uint8_t *tex_data, int tex_w,
                     int tex_h, const uint32_t *tex_palette) {
    (void)fb; (void)cam; (void)x; (void)z; (void)wall_dir; (void)color;
    (void)texture_id; (void)tex_data; (void)tex_w; (void)tex_h;
    (void)tex_palette;
}

void nexus_draw_wall_simple(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
                            float x, float z, int wall_dir, uint8_t color) {
    (void)fb; (void)cam; (void)x; (void)z; (void)wall_dir; (void)color;
}

void nexus_draw_floor(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
                      float x, float z, uint8_t floor_color,
                      uint8_t ceil_color) {
    (void)fb; (void)cam; (void)x; (void)z; (void)floor_color;
    (void)ceil_color;
}

void nexus_draw_floor_tex(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
                          float x, float z, const uint8_t *tex_data,
                          int tex_w, int tex_h,
                          const uint32_t *tex_palette) {
    (void)fb; (void)cam; (void)x; (void)z; (void)tex_data;
    (void)tex_w; (void)tex_h; (void)tex_palette;
}

void nexus_draw_ceiling_tex(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
                            float x, float z, const uint8_t *tex_data,
                            int tex_w, int tex_h,
                            const uint32_t *tex_palette) {
    (void)fb; (void)cam; (void)x; (void)z; (void)tex_data;
    (void)tex_w; (void)tex_h; (void)tex_palette;
}

void nexus_draw_floor_tex_mapped(Nexus_Framebuffer *fb,
                                 const Nexus_Camera *cam, float x, float z,
                                 const uint8_t *tex_data, int tex_w,
                                 int tex_h, const uint32_t *tex_palette,
                                 const uint8_t texel_map[256]) {
    (void)fb; (void)cam; (void)x; (void)z; (void)tex_data; (void)tex_w;
    (void)tex_h; (void)tex_palette; (void)texel_map;
}

void nexus_draw_ceiling_tex_mapped(Nexus_Framebuffer *fb,
                                   const Nexus_Camera *cam, float x, float z,
                                   const uint8_t *tex_data, int tex_w,
                                   int tex_h, const uint32_t *tex_palette,
                                   const uint8_t texel_map[256]) {
    (void)fb; (void)cam; (void)x; (void)z; (void)tex_data; (void)tex_w;
    (void)tex_h; (void)tex_palette; (void)texel_map;
}

void nexus_draw_floor_tex_mapped_heights(
    Nexus_Framebuffer *fb, const Nexus_Camera *cam, float x, float z,
    const int8_t heights[4], uint8_t rotation, const uint8_t *tex_data,
    int tex_w, int tex_h, const uint32_t *tex_palette,
    const uint8_t texel_map[256]) {
    (void)fb; (void)cam; (void)x; (void)z; (void)heights; (void)rotation;
    (void)tex_data; (void)tex_w; (void)tex_h; (void)tex_palette;
    (void)texel_map;
}

void nexus_draw_ceiling_tex_mapped_heights(
    Nexus_Framebuffer *fb, const Nexus_Camera *cam, float x, float z,
    const int8_t heights[4], uint8_t rotation, const uint8_t *tex_data,
    int tex_w, int tex_h, const uint32_t *tex_palette,
    const uint8_t texel_map[256]) {
    (void)fb; (void)cam; (void)x; (void)z; (void)heights; (void)rotation;
    (void)tex_data; (void)tex_w; (void)tex_h; (void)tex_palette;
    (void)texel_map;
}

void nexus_draw_wall_tex_mapped(Nexus_Framebuffer *fb,
                                const Nexus_Camera *cam, float x, float z,
                                int wall_dir, const uint8_t *tex_data,
                                int tex_w, int tex_h,
                                const uint32_t *tex_palette,
                                const uint8_t texel_map[256]) {
    (void)fb; (void)cam; (void)x; (void)z; (void)wall_dir; (void)tex_data;
    (void)tex_w; (void)tex_h; (void)tex_palette; (void)texel_map;
}

void nexus_draw_door(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
                     float x, float z, int facing, int door_state,
                     int texture_id, const uint8_t *tex_data, int tex_w,
                     int tex_h, const uint32_t *tex_palette) {
    (void)fb; (void)cam; (void)x; (void)z; (void)facing; (void)door_state;
    (void)texture_id; (void)tex_data; (void)tex_w; (void)tex_h;
    (void)tex_palette;
}

Vec2i nexus_project_model_vert(const Vec3 *local_vert, float scale,
                               const Vec3 *world_pos, const Mat4 *view_proj,
                               int screen_w, int screen_h) {
    (void)local_vert; (void)scale; (void)world_pos; (void)view_proj;
    (void)screen_w; (void)screen_h;
    return (Vec2i){0, 0};
}

void nexus_raster_billboard(Nexus_RasterVertex quad[4], Vec3 world_pos,
                            float width, float height,
                            const Nexus_Camera *cam) {
    (void)world_pos; (void)width; (void)height; (void)cam;
    if (quad) memset(quad, 0, sizeof(Nexus_RasterVertex) * 4U);
}

void nexus_raster_creature_billboard(
    Nexus_Framebuffer *fb, const Nexus_Camera *cam, Vec3 world_pos,
    float height, int texture_id, const uint8_t *tex_data, int tex_w,
    int tex_h, const uint32_t *tex_palette, uint32_t creature_flags,
    uint8_t base_color) {
    (void)fb; (void)cam; (void)world_pos; (void)height; (void)texture_id;
    (void)tex_data; (void)tex_w; (void)tex_h; (void)tex_palette;
    (void)creature_flags; (void)base_color;
}

void nexus_raster_projectile(Nexus_Framebuffer *fb, const Nexus_Camera *cam,
                             Vec3 start, Vec3 end, const Vec3 *arc_points,
                             int n_points, enum Nexus_ProjectileType type,
                             const uint32_t *palette) {
    (void)fb; (void)cam; (void)start; (void)end; (void)arc_points;
    (void)n_points; (void)type; (void)palette;
}
