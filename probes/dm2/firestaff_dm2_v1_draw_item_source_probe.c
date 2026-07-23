/*
 * firestaff_dm2_v1_draw_item_source_probe.c — DM2 V1 DRAW_ITEM Source Probe
 *
 * Deterministic end-to-end exercise of the source-owned DRAW_ITEM placement
 * chain: display-order tables, per-cell 5x5 visibility mask assembly, the
 * view-rotated static-object source plan, the M11 delivery gate, the item
 * render-plan placement fill and the asset-blit scale/flip/slot rules.
 *
 * Source-lock:
 *   skproject/SKWIN/SkWinCore.cpp DRAW_ITEM (_32cb_3672)
 *   skproject/SKWIN/SkWinCore.cpp DRAW_PUT_DOWN_ITEM (_32cb_3991)
 *   skproject/SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT (_32cb_3b9d)
 *   skproject/SKWIN/SkWinCore.cpp QUERY_OBJECT_5x5_POS (_48ae_07fd)
 *   skproject/SKWIN/SkWinCore.cpp DIR_FROM_5x5_POS (_48ae_07bf)
 *   skproject/SKWIN/SkGlobal.cpp _4976_4a04/_4976_41b0/_4976_41de/
 *                                _4976_418e/tlbDisplayOrder*
 */

#include "dm2_v1_viewport_renderer.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int errors = 0;
static int passed = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

int main(int argc, char **argv)
{
    uint8_t order[25];
    DM2_V1_ViewportState viewport;
    DM2_V1_ItemRenderPlan plan;
    DM2_V1_ItemAssetBlit blit;
    uint8_t fb[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int center_entries;
    int d1c_ok = 1;
    int mask_ok = 1;
    int view;

    (void)argc; (void)argv;

    fprintf(stderr, "=== DM2 V1 DRAW_ITEM Source Placement Probe ===\n");
    fprintf(stderr, "Source: skproject/SKWIN/SkWinCore.cpp DRAW_ITEM / "
            "DRAW_PUT_DOWN_ITEM / DRAW_STATIC_OBJECT\n\n");

    /* Invariant 1: D1C iterates all 25 source center-order entries; the
     * party cell (0) is bounded to the first 15. */
    center_entries = dm2_v1_viewport_static_object_display_order(3, order);
    PROBE_ASSERT(center_entries == 25 && order[0] == 0 && order[1] == 4 &&
                 order[24] == 22,
                 "D1C center display order has 25 source entries");
    PROBE_ASSERT(dm2_v1_viewport_static_object_display_order(0, order) == 15,
                 "party cell iterates 15 display-order entries");
    PROBE_ASSERT(dm2_v1_viewport_static_object_display_order(2, order) == 25 &&
                 order[0] == 4 && order[1] == 3,
                 "D1R right display order starts 4,3 (source tlbDisplayOrderRight)");

    /* Invariant 2: a synthetic square with a north-facing weapon and a
     * south-facing container produces the exact source visibility mask for
     * every party direction. */
    for (view = 0; view < 4; ++view) {
        uint32_t weapon_bit =
            dm2_v1_viewport_static_object_visibility_bit(0, view);
        uint32_t container_bit =
            dm2_v1_viewport_static_object_visibility_bit(2, view);
        uint32_t mask = weapon_bit | container_bit;
        int weapon_pos = dm2_v1_viewport_object_5x5_pos(0, view);
        DM2_V1_StaticObjectSourcePlan splan;

        if (weapon_bit == 0u || container_bit == 0u ||
            weapon_bit == container_bit ||
            (mask & (1u << (unsigned)weapon_pos)) == 0u) {
            mask_ok = 0;
        }
        if (!dm2_v1_viewport_static_object_source_plan(
                3, 17, 0x10, 0, 0, 0, view, 1u, mask, &splan) ||
            splan.position_5x5 != weapon_pos ||
            (splan.visibility_mask_5x5 &
             (1u << (unsigned)splan.position_5x5)) == 0u) {
            d1c_ok = 0;
        }
    }
    PROBE_ASSERT(mask_ok, "visibility mask is exact for all 4 party views");
    PROBE_ASSERT(d1c_ok,
                 "D1C source plan anchors at the view-rotated record position");

    /* Invariant 3: render-plan placement fill + table1d7029 pass order. */
    memset(fb, 0, sizeof(fb));
    dm2_v1_viewport_init(&viewport, fb, DM2_VP_WIDTH);
    dm2_v1_viewport_set_party(&viewport, 0, 10, 10);
    viewport.item_count = 2;
    memset(viewport.items, 0, sizeof(viewport.items));
    viewport.items[0].item_category = 0x10;
    viewport.items[0].item_type = 0x22;
    viewport.items[0].screen_x = 96;
    viewport.items[0].screen_y = 88;
    viewport.items[0].direction = 0;
    viewport.items[0].source_gdat_field = 0xf9;
    viewport.items[0].source_g1_weapon = 1;
    viewport.items[0].source_static_object_admitted = 1;
    viewport.items[0].source_static_object_cell = 3;
    viewport.items[0].source_static_object_pass = 17;
    viewport.items[0].source_static_object_clip_rect_id = 5081;
    viewport.items[1].item_category = 0x14;
    viewport.items[1].item_type = 3;
    viewport.items[1].screen_x = 100;
    viewport.items[1].screen_y = 90;
    viewport.items[1].direction = 1;
    viewport.items[1].source_gdat_field = 0xf9;
    viewport.items[1].source_g1_container = 1;
    viewport.items[1].source_static_object_admitted = 1;
    viewport.items[1].source_static_object_cell = 6;
    viewport.items[1].source_static_object_pass = 14;
    viewport.items[1].source_static_object_clip_rect_id = 5158;
    PROBE_ASSERT(dm2_v1_viewport_build_item_render_plan(&viewport, &plan) == 1 &&
                 plan.item_count == 2 &&
                 plan.items[0].item_category == 0x14 &&
                 plan.items[1].item_category == 0x10,
                 "render plan preserves table1d7029 pass order (14 before 17)");
    PROBE_ASSERT(plan.items[0].source_static_object_placement_valid == 1 &&
                 plan.items[0].source_static_object_stretch_factor64 == 0x2b &&
                 plan.items[0].source_static_object_flip_mirror == 1,
                 "D2C container placement: stretch 0x2b, chest mirror");
    PROBE_ASSERT(plan.items[1].source_static_object_placement_valid == 1 &&
                 plan.items[1].source_static_object_stretch_factor64 == 0x40 &&
                 plan.items[1].source_static_object_slot_x_offset == -3 &&
                 plan.items[1].source_static_object_slot_y_offset == 2,
                 "D1C weapon placement: stretch 0x40, slot deltas -3/+2");

    /* Invariant 4: the asset blit applies DRAW_ITEM scale, slot and flip. */
    PROBE_ASSERT(dm2_v1_viewport_item_asset_blit(&plan.items[1], 8, 8, 8, 0,
                                                 4, 32, &blit) == 1 &&
                 blit.dst_rect.w == 8 && blit.dst_rect.h == 8 &&
                 blit.dst_rect.x == 96 - 4 - 3 &&
                 blit.dst_rect.y == 88 - 4 + 2 &&
                 blit.flip_mirror == 0,
                 "D1C weapon blit: identity scale with source slot deltas");
    PROBE_ASSERT(dm2_v1_viewport_item_asset_blit(&plan.items[0], 8, 8, 8, 0,
                                                 4, 32, &blit) == 1 &&
                 blit.dst_rect.w == 5 && blit.dst_rect.h == 5 &&
                 blit.flip_mirror == 1,
                 "D2C container blit: 0x2b distance stretch and chest mirror");

    fprintf(stderr, "\n=== %d passed, %d failed ===\n", passed, errors);
    return errors == 0 ? 0 : 1;
}
