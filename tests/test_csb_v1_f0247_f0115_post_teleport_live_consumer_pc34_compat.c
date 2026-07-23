/* ReDMCSB PROJEXPL.C F0219 -> DUNVIEW.C F0115 live sprite consumption. */
#include "csb_v1_viewport_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\n", message); } \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

typedef struct {
    int calls;
    CSB_V1_ViewportRuntimeProjectileSpriteBlit last;
} DrawerProbe;

static int draw_real_projectile_bitmap(
    void *user,
    const CSB_V1_ViewportRuntimeProjectileSpriteBlit *blit,
    uint8_t *pixels,
    int stride)
{
    DrawerProbe *probe = (DrawerProbe *)user;

    if (!probe || !blit || !pixels || stride < 320 ||
        blit->graphic_index < 454 || blit->graphic_index > 670) {
        return 0;
    }
    ++probe->calls;
    probe->last = *blit;
    pixels[33 * stride + 112] = 0x5a;
    return 1;
}

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static int make_admitted_handoff(
    CSB_V1_DungeonData *dungeon,
    unsigned char raw[224],
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 *out_handoff)
{
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 handoff;
    int i;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 224u);
    dungeon->level_count = 2;
    dungeon->level_widths[0] = dungeon->level_widths[1] = 2;
    dungeon->level_heights[0] = dungeon->level_heights[1] = 2;
    dungeon->level_offsets[0] = 0;
    dungeon->level_offsets[1] = 4;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 224;
    dungeon->square_first_thing_base = 96;
    dungeon->square_first_thing_count = 2;
    dungeon->thing_data_bases[1] = 144;
    dungeon->thing_type_counts[1] = 1;
    dungeon->thing_data_bases[3] = 160;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[14] = 184;
    dungeon->thing_type_counts[14] = 1;
    for (i = 0; i < 8; ++i) raw[i] = (unsigned char)(1u << 5);

    /* C05 source, then the C03 -> C14 chain F0219 resolves on map 1. */
    raw[2] = (unsigned char)((5u << 5) | 0x18u);
    put_le16(raw, 76, 0u);
    put_le16(raw, 96, (unsigned short)(1u << 10));
    put_le16(raw, 144, 0xfffeu);
    put_le16(raw, 148, 0u);
    put_le16(raw, 150, 0x0100u);
    raw[4] = (unsigned char)((1u << 5) | 0x10u);
    put_le16(raw, 80, 1u);
    put_le16(raw, 98, (unsigned short)(3u << 10));
    put_le16(raw, 160, (unsigned short)(14u << 10));
    put_le16(raw, 184, 0xfffeu);
    put_le16(raw, 186, 0x1407u);
    raw[188] = 200u;
    raw[189] = 45u;
    put_le16(raw, 190, 17u);

    memset(&handoff, 0, sizeof(handoff));
    if (!csb_v1_f0219_post_teleport_projectile_impact_material_handoff_pc34(
            dungeon, 1, 0, 0, (uint16_t)(14u << 10),
            CSB_V1_F0115_PROJECTILE_ORDINAL_M715,
            CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
            CSB_V1_F0115_PROJECTILE_COORDINATE_SET_MIDDLE,
            &handoff)) {
        return 0;
    }
    *out_handoff = handoff;
    return 1;
}

static void test_live_draw_consumes_only_real_handoff(void)
{
    CSB_V1_ViewportConfig cfg;
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 handoff;
    DrawerProbe probe;
    unsigned char raw[224];
    uint8_t pixels[320 * 200];

    memset(&cfg, 0, sizeof(cfg));
    memset(&probe, 0, sizeof(probe));
    memset(pixels, 0, sizeof(pixels));
    CHECK(make_admitted_handoff(&dungeon, raw, &handoff) == 1,
          "fixture derives the handoff from the real C03 -> C14 destination list");
    cfg.viewport_pixels = pixels;
    cfg.viewport_stride = 320;
    cfg.post_teleport_projectile_handoffs = &handoff;
    cfg.post_teleport_projectile_handoff_count = 1;
    cfg.projectile_sprite_drawer = draw_real_projectile_bitmap;
    cfg.projectile_sprite_user = &probe;

    csb_v1_viewport_render_frame(&cfg, 0, 0, 0);
    CHECK(probe.calls == 1 && cfg.runtime_post_teleport_projectile_handoff_drawn_count == 1,
          "F0115 live path calls the real projectile bitmap drawer once");
    CHECK(probe.last.graphic_index == 487 && probe.last.aspect_index == 1 &&
              probe.last.transparent_color == 10 && probe.last.uses_f0791_blit,
          "live blit preserves admitted F0115 graphics metadata");
    CHECK(cfg.runtime_projectile_marker_drawn_count == 0 &&
              cfg.runtime_post_teleport_projectile_handoff_blocked_count == 0 &&
              pixels[33 * 320 + 112] == 0x5a,
          "accepted handoff draws no synthetic marker");
}

static void test_missing_drawer_is_no_draw_not_marker(void)
{
    CSB_V1_ViewportConfig cfg;
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 handoff;
    unsigned char raw[224];
    uint8_t pixels[320 * 200];

    memset(&cfg, 0, sizeof(cfg));
    memset(pixels, 0, sizeof(pixels));
    CHECK(make_admitted_handoff(&dungeon, raw, &handoff) == 1,
          "fixture admits an owned destination C14 before the live draw");
    cfg.viewport_pixels = pixels;
    cfg.viewport_stride = 320;
    cfg.post_teleport_projectile_handoffs = &handoff;
    cfg.post_teleport_projectile_handoff_count = 1;

    csb_v1_viewport_render_frame(&cfg, 0, 0, 0);
    CHECK(cfg.runtime_post_teleport_projectile_handoff_drawn_count == 0 &&
              cfg.runtime_post_teleport_projectile_handoff_blocked_count == 1 &&
              cfg.runtime_projectile_marker_drawn_count == 0,
          "missing real bitmap drawer fails closed without a marker");
}

int main(void)
{
    test_live_draw_consumes_only_real_handoff();
    test_missing_drawer_is_no_draw_not_marker();
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
