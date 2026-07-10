#include "nexus_v1_rasterizer.h"
#include "nexus_v1_viewport.h"
#include "nexus_v1_dmdf_model.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int count_color(const Nexus_Framebuffer *fb, uint8_t color) {
    int i;
    int count = 0;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        if (fb->color_buffer[i] == color) ++count;
    }
    return count;
}

static int count_written_depth(const Nexus_Framebuffer *fb) {
    int i;
    int count = 0;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i) {
        if (fb->z_buffer[i] < 1e30f) ++count;
    }
    return count;
}

static void wb16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffU);
}

static void wb32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xffU);
    p[2] = (uint8_t)((v >> 8) & 0xffU);
    p[3] = (uint8_t)(v & 0xffU);
}

static uint8_t *dgn_cell(uint8_t *structure1, int structure1b_rel,
                         int x, int y) {
    return structure1 + structure1b_rel +
           ((y * NEXUS_MAX_MAP_SIZE + x) *
            NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
}

static void set_dgn_collision_ref(uint8_t *structure1,
                                  int structure1b_rel,
                                  int x,
                                  int y,
                                  int ref) {
    uint8_t *cell = dgn_cell(structure1, structure1b_rel, x, y);
    cell[6] = (uint8_t)((ref >> 8) & 0x0f);
    cell[7] = (uint8_t)(ref & 0xff);
}

static int build_viewport_dgn(uint8_t *buf, int buf_size,
                              int structure1b_rel, int geometry_bytes) {
    const int structure1_blocks = 19;
    uint8_t *structure1;
    int useful;
    if (!buf || buf_size <= 0) return -1;
    memset(buf, 0, (size_t)buf_size);
    useful = structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + geometry_bytes;
    if (useful > structure1_blocks * NEXUS_DGN_BLOCK_SIZE) return -1;
    wb16(buf + 0x0c, 1U);
    wb16(buf + 0x0e, (uint16_t)structure1_blocks);
    wb32(buf + 0x10, (uint32_t)useful);
    structure1 = buf + NEXUS_DGN_BLOCK_SIZE;
    structure1[2] = 0x40;
    structure1[3] = 0x40;
    wb32(structure1 + 0x14, (uint32_t)structure1b_rel);
    return 0;
}

static void seed_surface(Nexus_DMDFTextureSurface *surface,
                         uint8_t *pixel,
                         uint32_t rgba) {
    memset(surface, 0, sizeof(*surface));
    *pixel = 1u;
    surface->pixels = pixel;
    surface->width = 1;
    surface->height = 1;
    surface->palette[1] = rgba;
    surface->valid = 1;
}

static void build_prs3_indexed_material_bpk(uint8_t *data, size_t size) {
    const uint32_t trailer_off = 64U;
    const uint32_t surface_off = 96U;
    memset(data, 0, size);
    memcpy(data, "BPPK", 4);
    wb32(data + 4, (uint32_t)size);
    memcpy(data + 12, "BMPD", 4);
    wb32(data + 16, (uint32_t)size - 20U);
    wb32(data + 20, 2U);
    wb32(data + 24, trailer_off);
    wb32(data + 28, surface_off);
    data[trailer_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_TRAILER;
    wb16(data + surface_off + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 1U);
    data[surface_off + NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 1U;
    data[surface_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_16BPP;
    memcpy(data + surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);
    wb32(data + surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 4U, 1U);
    wb32(data + surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 8U, 1U);
    wb32(data + surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 12U, 2U);
    data[surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 16U] = 0x01U;
    data[surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 17U] = 0xf8U;
}

int main(void) {
    Nexus_Framebuffer fb;
    Nexus_Camera cam;
    /* 3x2 is deliberately not a power-of-two surface. */
    const uint8_t pixels[6] = {1, 2, 3, 2, 1, 3};
    uint32_t palette[256] = {0};
    uint8_t map[256];
    int written_before;
    Nexus_V1_Engine engine;
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportRenderReceipt receipt;
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    uint8_t floor_pixel;
    uint8_t wall_bpk[160];
    uint8_t *structure1;

    nexus_fb_init(&fb);
    nexus_fb_clear(&fb);
    nexus_camera_init(&cam, (Vec3){0.5f, 0.5f, 3.5f}, 0);
    memset(map, 0xff, sizeof(map));
    palette[1] = 0xff102030U;
    palette[2] = 0xff405060U;
    palette[3] = 0xff708090U;
    map[1] = 41;
    map[2] = 42;
    map[3] = 43;
    fb.palette[41] = palette[1];
    fb.palette[42] = palette[2];
    fb.palette[43] = palette[3];

    nexus_draw_wall_tex_mapped(&fb, &cam, 0.0f, 2.0f, 0,
                               pixels, 3, 2, palette, map);
    expect(count_color(&fb, 41) > 0 && count_color(&fb, 42) > 0 &&
               count_color(&fb, 43) > 0,
           "wall rasterizer samples all texels from a 3x2 material surface");
    expect(fb.palette[41] == palette[1] && fb.palette[42] == palette[2] &&
               fb.palette[43] == palette[3],
           "material CLUT is preserved through framebuffer remapping");

    nexus_fb_clear(&fb);
    palette[2] = 0x00000000U;
    written_before = count_written_depth(&fb);
    nexus_draw_wall_tex_mapped(&fb, &cam, 0.0f, 2.0f, 0,
                               pixels, 3, 2, palette, map);
    expect(count_color(&fb, 42) == 0,
           "transparent material texels never use their mapped replacement color");
    expect(count_written_depth(&fb) > written_before,
           "opaque material texels still write depth after transparent clipping");

    nexus_fb_clear(&fb);
    palette[2] = 0xff405060U;
    map[2] = 0xff;
    nexus_draw_wall_tex_mapped(&fb, &cam, 0.0f, 2.0f, 0,
                               pixels, 3, 2, palette, map);
    expect(count_color(&fb, 42) == 0,
           "undefined palette remaps clip instead of falling back to flat color");

    memset(&engine, 0, sizeof(engine));
    expect(build_viewport_dgn(dgn, (int)sizeof(dgn), 0x40, 2048) == 0,
           "viewport DGN fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_dgn_collision_ref(structure1, 0x40, 3, 4, 1);
    set_dgn_collision_ref(structure1, 0x40, 2, 4, 0x0fff);
    set_dgn_collision_ref(structure1, 0x40, 4, 4, 0x0fff);
    set_dgn_collision_ref(structure1, 0x40, 3, 3, 0x0fff);
    expect(nexus_v1_level_load(&engine.current_level,
                               dgn,
                               (int)sizeof(dgn),
                               0) == 0,
           "viewport DGN fixture loads through real Structure1B parser");
    for (int y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (int x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            for (int dir = 0; dir < 4; ++dir) {
                engine.current_level.wall_material_refs[y][x][dir] = 1U;
            }
        }
    }
    engine.level_loaded = 1;
    engine.game.current_level = 0;
    engine.game.party_x = 3;
    engine.game.party_y = 4;
    engine.game.party_dir = 0;
    engine.floor_materials.valid = 1;
    engine.floor_materials.surface_count = 1;
    seed_surface(&engine.floor_materials.surfaces[0],
                 &floor_pixel,
                 0xff204060U);
    engine.wall_materials.valid = 1;
    engine.wall_materials.surface_count = 1;
    engine.wall_materials.surfaces[0].valid = 1;
    engine.wall_materials.surfaces[0].palette[0xf8] = 0xfff80000U;
    build_prs3_indexed_material_bpk(wall_bpk, sizeof(wall_bpk));
    expect(nexus_v1_dmdf_import_bpk_material_bank(
               wall_bpk, sizeof(wall_bpk), &engine.wall_materials) == 1 &&
               engine.wall_materials.surfaces[1].valid &&
               engine.wall_materials.surfaces[1].pixels[0] == 0xf8U &&
               engine.wall_materials.bpk_prs3_surface_count == 1,
           "viewport imports PRS3-decoded BPK wall material surface");
    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, &engine);
    expect(nexus_viewport_last_dgn_render_receipt(&viewport, &receipt) == 0,
           "viewport exposes last DGN render receipt");
    expect(receipt.attempted && receipt.ready &&
               receipt.used_real_dgn_route &&
               !receipt.blocked &&
               !receipt.fallback_visuals_permitted,
           "viewport renders real DGN route without legacy fallback");
    expect(receipt.command_count > 0 &&
               receipt.command_count == receipt.material_surface_count &&
               receipt.floor_count == receipt.floor_material_surface_count &&
               receipt.ceiling_count == receipt.ceiling_material_surface_count &&
               receipt.wall_count == receipt.wall_material_surface_count &&
               receipt.command_count == receipt.rasterized_command_count,
           "viewport consumes every DGN command through decoded material surfaces");
    expect(receipt.floor_count > 0 &&
               receipt.ceiling_count > 0 &&
               receipt.wall_count > 0 &&
               receipt.palette_synced &&
               receipt.written_pixels > 0,
           "viewport rasterizes floor, ceiling and wall DGN geometry to pixels");

    engine.wall_materials.valid = 0;
    engine.wall_materials.surface_count = 0;
    engine.wall_materials.surfaces[1].valid = 0;
    nexus_v1_invalidate_dgn_material_plan(&engine);
    nexus_viewport_render(&viewport, &engine);
    expect(nexus_viewport_last_dgn_render_receipt(&viewport, &receipt) == 0,
           "viewport exposes blocked DGN material receipt");
    expect(receipt.attempted && receipt.used_real_dgn_route &&
               receipt.blocked &&
               !receipt.fallback_visuals_permitted &&
               !receipt.palette_synced &&
               receipt.rasterized_command_count == 0,
           "viewport blocks real DGN route when required BPK/DMDF material is missing");
    expect(receipt.command_count > 0 &&
               receipt.missing_material_count > 0 &&
               receipt.first_missing_material_id == 1 &&
               (receipt.first_missing_material_kind ==
                    NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT ||
                receipt.first_missing_material_kind ==
                    NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT ||
                receipt.first_missing_material_kind ==
                    NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT),
           "DGN material plan reports the first missing wall material instead of falling back");

    return failures ? 1 : 0;
}
