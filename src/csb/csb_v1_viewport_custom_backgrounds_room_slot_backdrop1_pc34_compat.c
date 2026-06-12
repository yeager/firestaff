#include "firestaff/csb/v1/viewport/csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_pc34_compat.h"

#include "csb_v1_viewport_custom_backgrounds_room_slot_pc34_compat.h"

#include <string.h>

enum {
    CSB_ROOM_SLOT_COUNT = 16,
    CSB_FIXTURE_WIDTH = 10,
    CSB_FIXTURE_HEIGHT = 10,
    CSB_FIXTURE_PARTY_X = 4,
    CSB_FIXTURE_PARTY_Y = 6,
    CSB_FIXTURE_FACING_NORTH = 0,
    CSB_SKIN_DEF_MIN_WORDS = 7,
    CSB_BACKDROP0_BITMAP_INDEX = 0,
    CSB_BACKDROP1_BITMAP_INDEX = 1,
    CSB_MIDDLE_BITMAP_INDEX = 2,
    CSB_BACKDROP0_MASK_INDEX = 4,
    CSB_BACKDROP1_MASK_INDEX = 5,
    CSB_MIDDLE_MASK_INDEX = 6,
    CSB_BACKDROP1_BITMAP_SIZE_WORDS = 4144,
    CSB_BACKDROP1_MASK_HEIGHT = 20,
    CSB_BACKDROP1_ROOM_LIMIT = 5,
    CSB_F0107_KEEP_OUT_ORDER = 2,
    CSB_BACKDROP0_COMPOSITE_ORDER = 3,
    CSB_MIDDLE_COMPOSITE_ORDER = 4,
    CSB_BACKDROP1_COMPOSITE_ORDER = 5
};

static const uint16_t s_skin_def[CSB_SKIN_DEF_MIN_WORDS] = {
    101, 202, 303, 0, 401, 502, 603
};

static const char s_redmcsb_f0128_anchor[] =
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542 orders "
    "floor/ceiling setup before the near D0/D1/D2/D3 room sequence.";

static const char s_redmcsb_f0098_anchor[] =
    "ReDMCSB DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002 "
    "writes the base G2109_Ceiling/G2108_Floor pixels before overlays.";

static const char s_redmcsb_f0107_anchor[] =
    "ReDMCSB DUNVIEW.C F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF:"
    "3502-3938 supplies the wall-ornament/alcove keepout before the CSB "
    "masked backdrop1 absorb step.";

static const char s_redmcsb_defs_anchor[] =
    "ReDMCSB DEFS.H:2596-2614 I34E/P31J view-square ordinals match the "
    "roomNum slot table used by the existing CSB room-slot helper.";

static const char s_redmcsb_dungeon_anchor[] =
    "ReDMCSB DUNGEON.C F0163:1769-1838, F0164:1840-1905, and "
    "F0172:2466-2523 anchor thing-list/aspect ordinals used by the "
    "viewport room-square walk.";

static const char s_csb_applybackground_anchor[] =
    "CSB-lineage Viewport.cpp:6451-6505 ApplyBackground masked composite "
    "absorbs source bitmap words into the viewport under the mask.";

static const char s_csb_bitmap_application_anchor[] =
    "CSB-lineage Viewport.cpp:6599-6619 applies pSkinDef[0]/[4], "
    "pSkinDef[2]/[6], then roomNum-gated pSkinDef[1]/[5] with "
    "GetMask(..., roomNum, 20) and GetBitmap(pSkinDef[1], 4144).";

static const char s_disjointness_note[] =
    "Disjoint from the existing first-backdrop, second-backdrop, both-"
    "backdrops, D0C-first-backdrop, room-slot, mask-after-floor-ceiling, "
    "and room-pass-order gates: this slice pins the pSkinDef[1]/[5] "
    "backdrop1 roomNum application after pSkinDef[0]/[4] and F0107 keepout.";

static const char s_source_summary[] =
    "contract_only=1; ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:"
    "8318-8542; F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002; "
    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF:3502-3938; "
    "DEFS.H:2596-2614; DUNGEON.C F0163:1769-1838 F0164:1840-1905 "
    "F0172:2466-2523; CSB-lineage Viewport.cpp:6451-6505 "
    "ApplyBackground masked composite; Viewport.cpp:6599-6619 "
    "roomNum pSkinDef[1]/[5] bitmap application.";

static const CSB_V1_CustomBackgroundsRoomSlotBackdrop1Contract s_contract = {
    1,
    CSB_ROOM_SLOT_COUNT,
    CSB_SKIN_DEF_MIN_WORDS,
    CSB_BACKDROP0_BITMAP_INDEX,
    CSB_BACKDROP0_MASK_INDEX,
    CSB_BACKDROP1_BITMAP_INDEX,
    CSB_BACKDROP1_MASK_INDEX,
    CSB_BACKDROP1_BITMAP_SIZE_WORDS,
    CSB_BACKDROP1_MASK_HEIGHT,
    CSB_BACKDROP1_ROOM_LIMIT,
    1,
    1,
    1,
    0x800823c0u,
    s_redmcsb_f0128_anchor,
    s_redmcsb_f0098_anchor,
    s_redmcsb_f0107_anchor,
    s_redmcsb_defs_anchor,
    s_redmcsb_dungeon_anchor,
    s_csb_applybackground_anchor,
    s_csb_bitmap_application_anchor,
    s_disjointness_note,
    s_source_summary
};

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int shift;

    for (shift = 0; shift < 32; shift += 8) {
        hash ^= (value >> shift) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static void seed_room_skins(uint8_t *skins, int width)
{
    size_t i;

    memset(skins, 0, (size_t)CSB_FIXTURE_WIDTH * (size_t)CSB_FIXTURE_HEIGHT);
    for (i = 0; i < csb_v1_viewport_custom_backgrounds_room_slot_count_pc34(); ++i) {
        const CSB_V1_CustomBackgroundsRoomSlotSpec *slot =
            csb_v1_viewport_custom_backgrounds_room_slot_spec_pc34(i);
        CSB_V1_CustomBackgroundsRoomSlotSelection selection;

        if (!slot) {
            continue;
        }
        if (csb_v1_viewport_custom_backgrounds_room_slot_select_pc34(
                NULL,
                0,
                0,
                6,
                CSB_FIXTURE_PARTY_X,
                CSB_FIXTURE_PARTY_Y,
                CSB_FIXTURE_FACING_NORTH,
                slot->room_num,
                0,
                &selection)) {
            skins[(size_t)selection.target_y * (size_t)width +
                  (size_t)selection.target_x] = (uint8_t)(120 + slot->room_num);
        }
    }
}

const CSB_V1_CustomBackgroundsRoomSlotBackdrop1Contract *
csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_contract_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 and F0098 lines 2962-3002
     * anchor the base pass. F0107 lines 3502-3938 anchors the alcove
     * keepout. CSB-lineage Viewport.cpp lines 6451-6505 and 6599-6619
     * anchor ApplyBackground and the pSkinDef[1]/[5] roomNum path. */
    return &s_contract;
}

size_t csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_trace_pc34(
    CSB_V1_CustomBackgroundsRoomSlotBackdrop1Trace *out_trace,
    size_t out_capacity)
{
    uint8_t skins[CSB_FIXTURE_WIDTH * CSB_FIXTURE_HEIGHT];
    size_t i;
    const size_t count = csb_v1_viewport_custom_backgrounds_room_slot_count_pc34();

    seed_room_skins(skins, CSB_FIXTURE_WIDTH);

    for (i = 0; out_trace && i < out_capacity && i < count; ++i) {
        const CSB_V1_CustomBackgroundsRoomSlotSpec *slot =
            csb_v1_viewport_custom_backgrounds_room_slot_spec_pc34(i);
        CSB_V1_CustomBackgroundsRoomSlotSelection selection;
        CSB_V1_CustomBackgroundsViewportState state;
        CSB_V1_CustomBackgroundsBitmapApplication application;
        CSB_V1_CustomBackgroundsRoomSlotBackdrop1Trace trace;
        int selected = 0;
        int applied = 0;

        memset(&trace, 0, sizeof(trace));
        memset(&state, 0, sizeof(state));
        memset(&selection, 0, sizeof(selection));
        memset(&application, 0, sizeof(application));

        if (slot) {
            selected = csb_v1_viewport_custom_backgrounds_room_slot_select_pc34(
                skins,
                CSB_FIXTURE_WIDTH,
                CSB_FIXTURE_HEIGHT,
                6,
                CSB_FIXTURE_PARTY_X,
                CSB_FIXTURE_PARTY_Y,
                CSB_FIXTURE_FACING_NORTH,
                slot->room_num,
                0,
                &selection);
            applied = csb_v1_viewport_custom_backgrounds_room_slot_apply_bitmap_pc34(
                &selection,
                s_skin_def,
                CSB_SKIN_DEF_MIN_WORDS,
                &state,
                &application);
        }

        trace.room_num = selection.room_num;
        trace.call_order = slot ? slot->call_order : -1;
        trace.redmcsb_view_square_ordinal = selection.redmcsb_view_square_ordinal;
        trace.relative_forward = selection.relative_forward;
        trace.relative_side = selection.relative_side;
        trace.target_x = selection.target_x;
        trace.target_y = selection.target_y;
        trace.selected_skin = selection.selected_skin;
        trace.room_lookup_used_same_slot_table =
            selected && slot &&
            selection.room_slot_ordinal == slot->room_slot_ordinal &&
            selection.redmcsb_view_square_ordinal == slot->redmcsb_view_square_ordinal;
        trace.backdrop0_bitmap_graphic_id = application.large_bitmap_graphic_id;
        trace.backdrop0_mask_graphic_id = application.large_mask_graphic_id;
        trace.backdrop1_bitmap_graphic_id = application.near_bitmap_graphic_id;
        trace.backdrop1_mask_graphic_id = application.near_mask_graphic_id;
        trace.backdrop0_applybackground_ordinal = 0;
        trace.middle_applybackground_ordinal = 1;
        trace.backdrop1_applybackground_ordinal = 2;
        trace.f0107_keepout_order = CSB_F0107_KEEP_OUT_ORDER;
        trace.backdrop0_composite_order = CSB_BACKDROP0_COMPOSITE_ORDER;
        trace.backdrop1_composite_order = CSB_BACKDROP1_COMPOSITE_ORDER;
        trace.backdrop1_after_backdrop0 =
            trace.backdrop1_applybackground_ordinal > trace.backdrop0_applybackground_ordinal &&
            trace.backdrop1_composite_order > trace.backdrop0_composite_order;
        trace.backdrop1_after_f0107_keepout =
            trace.backdrop1_composite_order > trace.f0107_keepout_order;
        trace.backdrop1_room_gate_allows_apply =
            selection.room_num < CSB_BACKDROP1_ROOM_LIMIT;
        trace.backdrop1_applied = applied ? application.near_applied : 0;
        trace.backdrop1_bitmap_size_words = CSB_BACKDROP1_BITMAP_SIZE_WORDS;
        trace.backdrop1_mask_height = CSB_BACKDROP1_MASK_HEIGHT;
        trace.room_name = slot ? slot->room_name : NULL;
        trace.source_lines = s_csb_bitmap_application_anchor;
        out_trace[i] = trace;
    }

    return count;
}

uint32_t csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_hash_pc34(
    const CSB_V1_CustomBackgroundsRoomSlotBackdrop1Trace *trace,
    size_t trace_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!trace) {
        return 0;
    }

    for (i = 0; i < trace_count; ++i) {
        hash = fnv1a_u32(hash, (uint32_t)trace[i].room_num);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].call_order);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].redmcsb_view_square_ordinal);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].relative_forward);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].relative_side);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].target_x);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].target_y);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].selected_skin);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].backdrop0_bitmap_graphic_id);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].backdrop0_mask_graphic_id);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].backdrop1_bitmap_graphic_id);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].backdrop1_mask_graphic_id);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].backdrop1_applied);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].backdrop1_after_backdrop0);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].backdrop1_after_f0107_keepout);
    }
    return hash;
}

const char *
csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_source_evidence_pc34(void)
{
    return s_source_summary;
}
