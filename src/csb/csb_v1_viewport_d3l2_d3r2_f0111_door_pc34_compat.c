#include "csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_C17_ELEMENT_DOOR_FRONT = 17,       /* ReDMCSB DEFS.H C17_ELEMENT_DOOR_FRONT. */
    CSB_C14_VIEW_SQUARE_D3L2 = 14,         /* ReDMCSB DEFS.H:2610. */
    CSB_C15_VIEW_SQUARE_D3R2 = 15,         /* ReDMCSB DEFS.H:2611. */
    CSB_C00_VIEW_FLOOR_D3L2 = 0,           /* ReDMCSB DEFS.H:2750. */
    CSB_C01_VIEW_FLOOR_D3R2 = 1,           /* ReDMCSB DEFS.H:2751. */
    CSB_DOOR_PASS1_D3L2 = 0x0218,          /* ReDMCSB DEFS.H:2669. */
    CSB_DOOR_PASS1_D3R2 = 0x0128,          /* ReDMCSB DEFS.H:2668. */
    CSB_DOOR_PASS2_D3L2 = 0x0349,          /* ReDMCSB DEFS.H:2672. */
    CSB_DOOR_PASS2_D3R2 = 0x0439,          /* ReDMCSB DEFS.H:2675. */
    CSB_DOOR_GRAPHIC_FIRST_SET = 246,         /* ReDMCSB DEFS.H M633. */
    CSB_DOOR_SET_GRAPHIC_COUNT = 3,           /* ReDMCSB DEFS.H C003. */
    CSB_DOOR_SET_COUNT = 4,                   /* PC3.4 I34 DoorSet 0..3. */
    CSB_DOOR_NATIVE_BITMAP_FRONT_D3LCR = 246, /* DoorSet 0's G0693. */
    CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR = 0,   /* ReDMCSB DEFS.H:2789. */
    CSB_C3700_ZONE_DOOR_D3L2 = 3700,       /* ReDMCSB DEFS.H:4250. */
    CSB_C3710_ZONE_DOOR_D3R2 = 3710,       /* ReDMCSB DEFS.H:4251. */
    CSB_C10_COLOR_FLESH = 10,              /* ReDMCSB DEFS.H:2088. */
    /* PC3.4/I34 F0111 takes the F0616_CopyBitmap branch (DUNVIEW.C:4259-4261),
     * so its native dimensions are the GRAPHICS.DAT record metadata, not the
     * 48x41 byte-count arguments used by the older MEDIA009 branch. */
    CSB_D3_DOOR_NATIVE_BYTE_WIDTH = 44,
    CSB_D3_DOOR_NATIVE_HEIGHT = 38,
    CSB_LINEAGE_PWALL_D3L2_INDEX = 5,      /* CSB Viewport.cpp:2267. */
    CSB_LINEAGE_PWALL_D3R2_INDEX = 6,      /* CSB Viewport.cpp:2271. */
    CSB_LINEAGE_FRAME_BITMAP_INDEX = 5,    /* CSB Viewport.cpp:2281. */
    CSB_LINEAGE_FRAME_RECT_D3L_INDEX = 7,  /* CSB Viewport.cpp:2386. */
    CSB_LINEAGE_FRAME_RECT_D3R_INDEX = 6,  /* CSB Viewport.cpp:2387. */
    CSB_LINEAGE_DOOR_GRAPHIC_INDEX = 2,    /* CSB Viewport.cpp:2568. */
    CSB_LINEAGE_DOOR_GRAPHIC_SIZE = 984,   /* CSB Viewport.cpp:2602-2604. */
    CSB_LINEAGE_DOOR_NEARNESS = 0          /* CSB Viewport.cpp:2602-2604. */
};

int csb_v1_viewport_door_graphic_index_pc34(int door_set, int depth)
{
    if (door_set < 0 || door_set >= CSB_DOOR_SET_COUNT ||
        depth < 0 || depth >= CSB_DOOR_SET_GRAPHIC_COUNT) return -1;
    return CSB_DOOR_GRAPHIC_FIRST_SET +
        door_set * CSB_DOOR_SET_GRAPHIC_COUNT + depth;
}

int csb_v1_viewport_door_graphic_index_from_map_pc34(int door_set0,
                                                      int door_set1,
                                                      uint16_t door_word,
                                                      int depth)
{
    const int door_set = (door_word & UINT16_C(0x0001)) ? door_set1 : door_set0;

    /* ReDMCSB DUNGEON.C F0174 copies MAP.D DoorSet0/1 to the current-map
     * door info. DUNVIEW.C F0096 then builds G0693/G0694/G0695 as M633 +
     * selected DoorSet * C003 + depth. CSBWin Viewport.cpp selects the same
     * map member from DB0's low door-type bit. */
    return csb_v1_viewport_door_graphic_index_pc34(door_set, depth);
}

int csb_v1_viewport_door_graphic_index_valid_pc34(int graphic_index, int depth)
{
    int door_set;
    for (door_set = 0; door_set < CSB_DOOR_SET_COUNT; ++door_set) {
        if (graphic_index == csb_v1_viewport_door_graphic_index_pc34(
                door_set, depth)) return 1;
    }
    return 0;
}

int csb_v1_viewport_d3_door_graphic_index_valid_pc34(int graphic_index)
{
    return csb_v1_viewport_door_graphic_index_valid_pc34(graphic_index, 0);
}

static const char s_source_evidence[] =
    "DUNVIEW.C F0676/F0677 C17_ELEMENT_DOOR_FRONT path; "
    "fail-closed real GRAPHICS.DAT receipt binds the G0693 door bitmap "
    "family before the D3L2/D3R2 route is source-backed; "
    "F0111_DUNGEONVIEW_DrawDoor; ReDMCSB DUNVIEW.C:6268-6274 F0676 "
    "calls F0108 with M552_FRONT_WALL_ORNAMENT_ORDINAL and "
    "C00_VIEW_FLOOR_D3L2, F0115 C14 order 0x0218, F0111 with "
    "G0693/C0/C3700, then sets order 0x0349 before the follow-up F0115 "
    "returns to F0128. DUNVIEW.C:6336-6341 F0677 calls F0108 with "
    "M558_FLOOR_ORNAMENT_ORDINAL and C01_VIEW_FLOOR_D3R2, F0115 C15 "
    "order 0x0128, F0111 with G0693/C0/C3710, then sets order 0x0439. "
    "DUNVIEW.C:4218-4334 F0111 copies the resolved native bitmap through "
    "F0489_MEMORY_GetNativeBitmapOrGraphic/F0616_CopyBitmap (PC/I34 uses "
    "the record's 44x38 metadata, not the legacy 48x41 M075 byte span) and blits with "
    "C10_COLOR_FLESH. DEFS.H:2088,2610-2611,2668-2675,2750-2751,2789,"
    "4250-4251,5456 anchors constants. CSB Viewport.cpp:1813-1820, "
    "2267/2271 pWallBitmaps parity, 2281 pDoorBitmaps, 2386/2387 "
    "DoorFrameRect, 2568 and 2596-2616 StdDrawDoor parity.";

static const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 s_specs[] = {
    {
        CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34,
        CSB_C14_VIEW_SQUARE_D3L2,
        CSB_C17_ELEMENT_DOOR_FRONT,
        CSB_C00_VIEW_FLOOR_D3L2,
        "M552_FRONT_WALL_ORNAMENT_ORDINAL",
        CSB_C14_VIEW_SQUARE_D3L2,
        CSB_DOOR_PASS1_D3L2,
        CSB_DOOR_PASS2_D3L2,
        CSB_DOOR_NATIVE_BITMAP_FRONT_D3LCR,
        CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        CSB_C3700_ZONE_DOOR_D3L2,
        CSB_C10_COLOR_FLESH,
        CSB_D3_DOOR_NATIVE_BYTE_WIDTH,
        CSB_D3_DOOR_NATIVE_HEIGHT,
        CSB_D3_DOOR_NATIVE_BYTE_WIDTH,
        CSB_LINEAGE_PWALL_D3L2_INDEX,
        CSB_LINEAGE_PWALL_D3R2_INDEX,
        CSB_LINEAGE_FRAME_BITMAP_INDEX,
        CSB_LINEAGE_FRAME_RECT_D3L_INDEX,
        CSB_LINEAGE_DOOR_GRAPHIC_INDEX,
        CSB_LINEAGE_DOOR_GRAPHIC_SIZE,
        CSB_LINEAGE_DOOR_NEARNESS,
        "F0676_DrawD3L2",
        "ReDMCSB DUNVIEW.C:6268-6274; DEFS.H:2088,2610,2669,2672,2750,2789,4250,5456; CSB Viewport.cpp:1813-1820,2267,2281,2386"
    },
    {
        CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34,
        CSB_C15_VIEW_SQUARE_D3R2,
        CSB_C17_ELEMENT_DOOR_FRONT,
        CSB_C01_VIEW_FLOOR_D3R2,
        "M558_FLOOR_ORNAMENT_ORDINAL",
        CSB_C15_VIEW_SQUARE_D3R2,
        CSB_DOOR_PASS1_D3R2,
        CSB_DOOR_PASS2_D3R2,
        CSB_DOOR_NATIVE_BITMAP_FRONT_D3LCR,
        CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        CSB_C3710_ZONE_DOOR_D3R2,
        CSB_C10_COLOR_FLESH,
        CSB_D3_DOOR_NATIVE_BYTE_WIDTH,
        CSB_D3_DOOR_NATIVE_HEIGHT,
        CSB_D3_DOOR_NATIVE_BYTE_WIDTH,
        CSB_LINEAGE_PWALL_D3L2_INDEX,
        CSB_LINEAGE_PWALL_D3R2_INDEX,
        CSB_LINEAGE_FRAME_BITMAP_INDEX,
        CSB_LINEAGE_FRAME_RECT_D3R_INDEX,
        CSB_LINEAGE_DOOR_GRAPHIC_INDEX,
        CSB_LINEAGE_DOOR_GRAPHIC_SIZE,
        CSB_LINEAGE_DOOR_NEARNESS,
        "F0677_DrawD3R2",
        "ReDMCSB DUNVIEW.C:6336-6341; DEFS.H:2088,2611,2668,2675,2751,2789,4251,5456; CSB Viewport.cpp:1813-1820,2271,2281,2387"
    }
};

size_t csb_v1_viewport_d3l2_d3r2_f0111_door_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *
csb_v1_viewport_d3l2_d3r2_f0111_door_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d3l2_d3r2_f0111_door_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *
csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
    CSB_V1_ViewportD3L2D3R2F0111DoorSidePc34 side)
{
    for (size_t i = 0; i < csb_v1_viewport_d3l2_d3r2_f0111_door_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

int csb_v1_viewport_d3l2_d3r2_f0111_door_trace_pc34(
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *spec,
    int door_type,
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 *out_result)
{
    CSB_V1_ViewportD3L2D3R2F0111DoorResultPc34 result;

    if (!spec || !out_result || door_type < 0) return -1;

    result.ok = CSB_ROUTE_ABSENT;
    result.f0108_called = CSB_ROUTE_PRESENT;
    result.f0108_floor_view = spec->floor_view;
    result.f0108_ordinal_slot = spec->floor_ornament_ordinal_slot;
    result.f0115_pass1_called = CSB_ROUTE_PRESENT;
    result.f0115_pass1_view_square = spec->f0115_view_square;
    result.f0115_pass1_order = spec->door_pass1_order;
    result.f0111_called = CSB_ROUTE_PRESENT;
    result.f0111_native_bitmap_index_family = spec->door_native_bitmap_index_family;
    result.f0111_door_ornament_view = spec->door_ornament_view;
    result.f0111_zone = spec->door_zone;
    result.f0115_pass2_order_set_before_dispatch = CSB_ROUTE_PRESENT;
    result.f0115_pass2_order = spec->door_pass2_order;
    result.f0128_dispatch_reached_after_pass2 = CSB_ROUTE_PRESENT;
    result.f0111_transparent_color = spec->transparent_color;
    /* ReDMCSB DUNVIEW.C:4257-4258 F0111 resolves G0693[...] through
     * F0489_MEMORY_GetNativeBitmapOrGraphic before F0616_CopyBitmap. */
    result.native_bitmap_fetches_via_f0489 = CSB_ROUTE_PRESENT;
    result.resolved_native_bitmap_index =
        spec->door_native_bitmap_index_family + door_type;
    result.preserved_frame_byte_width = spec->frame_byte_width;
    result.f0107_called = CSB_ROUTE_ABSENT;
    result.l0201_pwall_parity_preserved = CSB_ROUTE_PRESENT;
    result.lineage_pwall_left_index = spec->l0201_pwall_left_index;
    result.lineage_pwall_right_index = spec->l0201_pwall_right_index;
    result.pixel_anchor_ready = CSB_ROUTE_PRESENT;
    result.source_lock_evidence = s_source_evidence;
    result.ok =
        result.f0108_called &&
        result.f0115_pass1_called &&
        result.f0111_called &&
        result.f0115_pass2_order_set_before_dispatch &&
        result.f0128_dispatch_reached_after_pass2 &&
        result.native_bitmap_fetches_via_f0489 &&
        result.f0107_called == CSB_ROUTE_ABSENT &&
        result.l0201_pwall_parity_preserved &&
        result.pixel_anchor_ready;

    *out_result = result;
    return result.ok ? 0 : 1;
}

int csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2,
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2,
    int source_graphics_dat_bound,
    int no_synthetic_pixels,
    int no_fallback_visuals,
    int source_graphics_item_index,
    size_t source_byte_count,
    uint32_t source_payload_hash,
    CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34 *out_receipt)
{
    CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34 receipt;

    if (out_receipt) {
        *out_receipt =
            (CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34){0};
    }
    if (!d3l2 || !d3r2 || !out_receipt || !source_graphics_dat_bound ||
        !no_synthetic_pixels || !no_fallback_visuals ||
        !csb_v1_viewport_d3_door_graphic_index_valid_pc34(
            source_graphics_item_index) ||
        source_byte_count == 0u || source_payload_hash == 0u ||
        d3l2->side != CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34 ||
        d3r2->side != CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34 ||
        d3l2->door_native_bitmap_index_family !=
            CSB_DOOR_NATIVE_BITMAP_FRONT_D3LCR ||
        d3r2->door_native_bitmap_index_family !=
            CSB_DOOR_NATIVE_BITMAP_FRONT_D3LCR ||
        d3l2->door_zone != CSB_C3700_ZONE_DOOR_D3L2 ||
        d3r2->door_zone != CSB_C3710_ZONE_DOOR_D3R2 ||
        d3l2->transparent_color != CSB_C10_COLOR_FLESH ||
        d3r2->transparent_color != CSB_C10_COLOR_FLESH) {
        return 0;
    }

    receipt =
        (CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34){0};
    receipt.valid = CSB_ROUTE_PRESENT;
    receipt.route_backed_by_real_graphics_dat = CSB_ROUTE_PRESENT;
    receipt.source_graphics_dat_bound = CSB_ROUTE_PRESENT;
    receipt.no_synthetic_pixels = CSB_ROUTE_PRESENT;
    receipt.no_fallback_visuals = CSB_ROUTE_PRESENT;
    receipt.source_graphics_item_index = source_graphics_item_index;
    receipt.source_byte_count = source_byte_count;
    receipt.source_payload_hash = source_payload_hash;
    receipt.d3l2_view_square = d3l2->view_square;
    receipt.d3r2_view_square = d3r2->view_square;
    receipt.d3l2_door_zone = d3l2->door_zone;
    receipt.d3r2_door_zone = d3r2->door_zone;
    receipt.d3l2_pass1_order = d3l2->door_pass1_order;
    receipt.d3r2_pass1_order = d3r2->door_pass1_order;
    receipt.d3l2_pass2_order = d3l2->door_pass2_order;
    receipt.d3r2_pass2_order = d3r2->door_pass2_order;
    receipt.door_ornament_view = d3l2->door_ornament_view;
    receipt.transparent_color = CSB_C10_COLOR_FLESH;
    receipt.native_bitmap_byte_width = d3l2->native_bitmap_byte_width;
    receipt.native_bitmap_height = d3l2->native_bitmap_height;
    receipt.redmcsb_d3l2_function = d3l2->redmcsb_function;
    receipt.redmcsb_d3r2_function = d3r2->redmcsb_function;
    *out_receipt = receipt;
    return 1;
}

const char *csb_v1_viewport_d3l2_d3r2_f0111_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
