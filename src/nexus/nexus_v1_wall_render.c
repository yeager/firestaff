#include "nexus_v1_rendering.h"
#include "nexus_v1_textures.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/* ── Texture fetch helper ──────────────────────────────────── */

/* Sample a texture with affine UV mapping.
 * This mirrors Saturn VDP1's affine texture mapping (no perspective correction). */
static inline uint8_t tex_sample(const uint8_t *page,
    float u, float v)
{
    int px, py;
    if (!page) return 0;
    /* Wrap UV to [0, 255] */
    px = ((int)u) & 255;
    py = ((int)v) & 255;
    /* 2 pixels per byte: even=high nibble, odd=low nibble */
    {
        int byte_idx = (py * 256 + px) >> 1;
        int shift = (px & 1) ? 0 : 4;
        return (page[byte_idx] >> shift) & 0x0F;
    }
}

/* ── Textured wall quad with Z-buffer ─────────────────────── */

void nexus_render_wall_quad(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_z,
    int wall_dir,
    int tex_page,
    uint8_t u0, uint8_t v0, uint8_t u1, uint8_t v1,
    uint8_t light_level)
{
    Nexus_RasterVertex v[4];
    int i;
    const uint8_t *page = NULL;

    /* Get texture page data (NULL = fallback) */
    if (tex_page >= 0 && tex_page < 256)
        page = NULL; /* TODO: fetch from texture manager */

    /* Build wall quad vertices */
    for (i = 0; i < 4; i++) v[i].color = 5; /* default gray */

    switch (wall_dir) {
    case 0: /* North face (z−) */
        v[0].position = (Vec3){world_x,     0.0f, world_z};
        v[1].position = (Vec3){world_x + 1, 0.0f, world_z};
        v[2].position = (Vec3){world_x + 1, 1.0f, world_z};
        v[3].position = (Vec3){world_x,     1.0f, world_z};
        v[0].uv = (Vec2){u0, v0};
        v[1].uv = (Vec2){u1, v0};
        v[2].uv = (Vec2){u1, v1};
        v[3].uv = (Vec2){u0, v1};
        break;
    case 1: /* East face (x+) */
        v[0].position = (Vec3){world_x + 1, 0.0f, world_z};
        v[1].position = (Vec3){world_x + 1, 0.0f, world_z + 1};
        v[2].position = (Vec3){world_x + 1, 1.0f, world_z + 1};
        v[3].position = (Vec3){world_x + 1, 1.0f, world_z};
        v[0].uv = (Vec2){u0, v0};
        v[1].uv = (Vec2){u1, v0};
        v[2].uv = (Vec2){u1, v1};
        v[3].uv = (Vec2){u0, v1};
        break;
    case 2: /* South face (z+) */
        v[0].position = (Vec3){world_x + 1, 0.0f, world_z + 1};
        v[1].position = (Vec3){world_x,     0.0f, world_z + 1};
        v[2].position = (Vec3){world_x,     1.0f, world_z + 1};
        v[3].position = (Vec3){world_x + 1, 1.0f, world_z + 1};
        v[0].uv = (Vec2){u0, v0};
        v[1].uv = (Vec2){u1, v0};
        v[2].uv = (Vec2){u1, v1};
        v[3].uv = (Vec2){u0, v1};
        break;
    case 3: /* West face (x−) */
        v[0].position = (Vec3){world_x, 0.0f, world_z + 1};
        v[1].position = (Vec3){world_x, 0.0f, world_z};
        v[2].position = (Vec3){world_x, 1.0f, world_z};
        v[3].position = (Vec3){world_x, 1.0f, world_z + 1};
        v[0].uv = (Vec2){u0, v0};
        v[1].uv = (Vec2){u1, v0};
        v[2].uv = (Vec2){u1, v1};
        v[3].uv = (Vec2){u0, v1};
        break;
    default:
        return;
    }

    /* Modulate base color by light level */
    v[0].color = light_level;
    v[1].color = light_level;
    v[2].color = light_level;
    v[3].color = light_level;

    nexus_raster_quad(fb, v[0], v[1], v[2], v[3], cam);
}

/* ── Floor tile ─────────────────────────────────────────────── */

void nexus_render_floor_tile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_z,
    int floor_tex_page, uint8_t fu0, uint8_t fv0,
    uint8_t light_level)
{
    Nexus_RasterVertex v[4];
    int i;

    (void)floor_tex_page; /* TODO: use texture manager */
    (void)fu0;
    (void)fv0;

    for (i = 0; i < 4; i++) v[i].color = 8; /* floor gray */

    v[0].position = (Vec3){world_x,     0.0f, world_z};
    v[1].position = (Vec3){world_x + 1, 0.0f, world_z};
    v[2].position = (Vec3){world_x + 1, 0.0f, world_z + 1};
    v[3].position = (Vec3){world_x,     0.0f, world_z + 1};

    v[0].color = v[1].color = v[2].color = v[3].color = 8 + (light_level >> 2);
    nexus_raster_quad(fb, v[0], v[1], v[2], v[3], cam);
}

/* ── Ceiling tile ───────────────────────────────────────────── */

void nexus_render_ceiling_tile(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_z,
    int ceil_tex_page, uint8_t cu0, uint8_t cv0,
    uint8_t light_level)
{
    Nexus_RasterVertex v[4];
    int i;

    (void)ceil_tex_page; /* TODO: use texture manager */
    (void)cu0;
    (void)cv0;

    for (i = 0; i < 4; i++) v[i].color = 9; /* ceiling dark gray */

    v[0].position = (Vec3){world_x,     1.0f, world_z + 1};
    v[1].position = (Vec3){world_x + 1, 1.0f, world_z + 1};
    v[2].position = (Vec3){world_x + 1, 1.0f, world_z};
    v[3].position = (Vec3){world_x,     1.0f, world_z};

    v[0].color = v[1].color = v[2].color = v[3].color = 9 + (light_level >> 2);
    nexus_raster_quad(fb, v[0], v[1], v[2], v[3], cam);
}

/* ── Square wall rendering ───────────────────────────────────── */

/* Wall type → texture page (deterministic fallback table) */
static int nexus_wall_type_to_texpage(int wall_type) {
    /* Wall type 0 = solid stone wall */
    /* Types 1-31 = floor/door/pit variations */
    switch (wall_type) {
    case 0:  return 0;   /* solid wall */
    case 1:  return 1;   /* rough wall */
    case 2:  return 2;   /* brick wall */
    case 3:  return 3;   /* carved stone */
    default: return 0;   /* default solid */
    }
}

void nexus_render_square_walls(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    float world_x, float world_z,
    int square_type,
    int facing_dir,
    uint8_t light_level)
{
    if (square_type == 0) {
        /* Solid wall — draw front face */
        int front_face = (facing_dir + 2) & 3;
        int tex_page = nexus_wall_type_to_texpage(0);
        nexus_render_wall_quad(fb, cam, world_x, world_z,
            front_face, tex_page, 0, 0, 255, 255, light_level);
    }
    /* Floor squares: no solid wall to draw at center.
     * Side walls are handled by adjacent square checks in the viewport. */
}

/* ── DGN geometry blob → texture pages ─────────────────────── */

/* Parse DGN geometry blob to extract embedded VDP1 texture pages.
 * The geometry blob contains wall/floor/ceiling textures packed as
 * VDP1 BITMAP format (4-bit per pixel, 256×256 pages).
 *
 * DGN Geometry Blob Structure (hypothesis — not yet confirmed):
 *   Offset 0: uint32 texture_page_count
 *   Offset 4: uint32[page_count] byte offsets for each page
 *   Offset N: packed VDP1 BITMAP data for each page (32768 bytes each)
 *
 * Source: Phase 0 §4.3 geometry blob analysis
 * Status: NOT YET REVERSE-ENGINEERED — see docs/nexus_issues.md M2 */
int nexus_wall_load_textures_from_dgn(Nexus_TextureManager *tex,
    const uint8_t *dgn_data, int dgn_size,
    int geometry_offset)
{
    int off = geometry_offset;
    uint32_t page_count;
    int i;

    if (!tex || !dgn_data || dgn_size < off + 4)
        return 0;

    /* Read page count from geometry blob */
    page_count = ((uint32_t)dgn_data[off] << 24) |
                 ((uint32_t)dgn_data[off+1] << 16) |
                 ((uint32_t)dgn_data[off+2] << 8) |
                 (uint32_t)dgn_data[off+3];
    off += 4;

    if (page_count == 0 || page_count > 256)
        page_count = 0;  /* not confirmed yet */

    printf("Nexus wall textures: DGN geometry has %u texture pages (hypothetical)\n",
        page_count);

    /* TODO: Read page offsets and load each page from dgn_data
     * Requires reverse-engineering the DGN geometry blob format first.
     * See docs/nexus_issues.md M2. */
    (void)i;  /* suppress unused warning */
    return (int)page_count;
}
