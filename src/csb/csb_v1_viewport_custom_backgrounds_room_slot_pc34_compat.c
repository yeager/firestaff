#include "csb_v1_viewport_custom_backgrounds_room_slot_pc34_compat.h"

#include <string.h>

enum {
    CSB_ROOM_SLOT_COUNT = 16,
    CSB_FIRST_BACKDROP_ROOM = 0,
    CSB_SECOND_BACKDROP_ROOM = 2,
    CSB_SKIN_DEF_MIN_WORDS = 7,
    CSB_LARGE_BITMAP_INDEX = 0,
    CSB_NEAR_BITMAP_INDEX = 1,
    CSB_MIDDLE_BITMAP_INDEX = 2,
    CSB_LARGE_MASK_INDEX = 4,
    CSB_NEAR_MASK_INDEX = 5,
    CSB_MIDDLE_MASK_INDEX = 6,
    CSB_NEAR_LAYER_ROOM_LIMIT = 5
};

static const char s_redmcsb_viewport_anchor[] =
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542 draws the "
    "PC 3.4 viewport square sequence after the floor/ceiling baseline.";

static const char s_redmcsb_floor_ceiling_anchor[] =
    "ReDMCSB DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002 "
    "draws G2109_Ceiling/G2108_Floor and clears "
    "G0297_B_DrawFloorAndCeilingRequested before CSB CustomBackgrounds.";

static const char s_redmcsb_defs_room_slot_anchor[] =
    "ReDMCSB DEFS.H:2596-2614 I34E/P31J view-square ordinals; D0C=0, "
    "D0L=1, D0R=2, D1C=3, D1L=4, D1R=5, D2C=6, D2L=7, D2R=8, "
    "D2L2=9, D2R2=10, D3C=11, D3L=12, D3R=13, D3L2=14, D3R2=15.";

static const char s_csb_relpos_anchor[] =
    "CSB-lineage Viewport.cpp:5324-5337 relposSid/relposFwd and "
    "DXFWD/DYFWD/DXSID/DYSID translate CustomBackgrounds room slots.";

static const char s_csb_custom_backgrounds_anchor[] =
    "CSB-lineage Viewport.cpp:6574-6622 CustomBackgrounds: bounds check, "
    "skinCache.GetSkin(d.LoadedLevel,x,y), default skin fallback, "
    "GetSkinDef(18), and roomNum<5 near-layer gate.";

static const char s_csb_bitmap_application_anchor[] =
    "CSB-lineage Viewport.cpp:6451-6505 ApplyBackground masked composite; "
    "Viewport.cpp:6599-6619 applies large pSkinDef[0]/[4], middle "
    "pSkinDef[2]/[6], and near pSkinDef[1]/[5] bitmaps by roomNum.";

static const char s_csb_room_dispatch_anchor[] =
    "CSB-lineage Viewport.cpp:6926-7147 dispatches CustomBackgrounds "
    "room slots 0,2,1,3,4,5,7,6,8,9,10,11,12,13,14,15 before each "
    "matching room draw.";

static const char s_source_evidence[] =
    "contract_only=1; ReDMCSB DUNVIEW.C "
    "F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002; "
    "F0128_DUNGEONVIEW_Draw_CPSF:8318-8542; DEFS.H:2596-2614 I34E/P31J "
    "view-square ordinals; CSB-lineage CSB.h:335-396 BACKGROUND_MASK/"
    "BACKGROUND_LIB; Viewport.cpp:5324-5337 relpos and facing deltas; "
    "Viewport.cpp:6451-6505 ApplyBackground; Viewport.cpp:6574-6622 "
    "CustomBackgrounds skin/default/mask/bitmap application; "
    "Viewport.cpp:6926-7147 room-slot dispatch.";

static const CSB_V1_CustomBackgroundsRoomSlotSpec s_room_slots[] = {
    { 0, 0, 14, 0, 3, -2, 1, "D3L2", s_csb_room_dispatch_anchor },
    { 2, 2, 12, 1, 3, -1, 1, "D3L", s_csb_room_dispatch_anchor },
    { 1, 1, 15, 2, 3, 2, 1, "D3R2", s_csb_room_dispatch_anchor },
    { 3, 3, 13, 3, 3, 1, 1, "D3R", s_csb_room_dispatch_anchor },
    { 4, 4, 11, 4, 3, 0, 1, "D3C", s_csb_room_dispatch_anchor },
    { 5, 5, 9, 5, 2, -2, 0, "D2L2", s_csb_room_dispatch_anchor },
    { 7, 7, 7, 6, 2, -1, 0, "D2L", s_csb_room_dispatch_anchor },
    { 6, 6, 10, 7, 2, 2, 0, "D2R2", s_csb_room_dispatch_anchor },
    { 8, 8, 8, 8, 2, 1, 0, "D2R", s_csb_room_dispatch_anchor },
    { 9, 9, 6, 9, 2, 0, 0, "D2C", s_csb_room_dispatch_anchor },
    { 10, 10, 4, 10, 1, -1, 0, "D1L", s_csb_room_dispatch_anchor },
    { 11, 11, 5, 11, 1, 1, 0, "D1R", s_csb_room_dispatch_anchor },
    { 12, 12, 3, 12, 1, 0, 0, "D1C", s_csb_room_dispatch_anchor },
    { 13, 13, 1, 13, 0, -1, 0, "D0L", s_csb_room_dispatch_anchor },
    { 14, 14, 2, 14, 0, 1, 0, "D0R", s_csb_room_dispatch_anchor },
    { 15, 15, 0, 15, 0, 0, 0, "D0C", s_csb_room_dispatch_anchor }
};

static const CSB_V1_CustomBackgroundsRoomSlotContract s_contract = {
    1,
    CSB_ROOM_SLOT_COUNT,
    CSB_FIRST_BACKDROP_ROOM,
    CSB_SECOND_BACKDROP_ROOM,
    CSB_SKIN_DEF_MIN_WORDS,
    CSB_LARGE_BITMAP_INDEX,
    CSB_LARGE_MASK_INDEX,
    CSB_MIDDLE_BITMAP_INDEX,
    CSB_MIDDLE_MASK_INDEX,
    CSB_NEAR_BITMAP_INDEX,
    CSB_NEAR_MASK_INDEX,
    CSB_NEAR_LAYER_ROOM_LIMIT,
    s_redmcsb_viewport_anchor,
    s_redmcsb_floor_ceiling_anchor,
    s_redmcsb_defs_room_slot_anchor,
    s_csb_relpos_anchor,
    s_csb_custom_backgrounds_anchor,
    s_csb_bitmap_application_anchor,
    s_csb_room_dispatch_anchor,
    s_source_evidence
};

static int translate_room_slot(
    const CSB_V1_CustomBackgroundsRoomSlotSpec *slot,
    int party_x,
    int party_y,
    int facing,
    int *out_x,
    int *out_y)
{
    static const int dx_fwd[4] = { 0, 1, 0, -1 };
    static const int dy_fwd[4] = { -1, 0, 1, 0 };
    static const int dx_side[4] = { 1, 0, -1, 0 };
    static const int dy_side[4] = { 0, 1, 0, -1 };

    if (!slot || !out_x || !out_y || facing < 0 || facing > 3) {
        return 0;
    }

    *out_x = party_x + dx_side[facing] * slot->relative_side +
             dx_fwd[facing] * slot->relative_forward;
    *out_y = party_y + dy_side[facing] * slot->relative_side +
             dy_fwd[facing] * slot->relative_forward;
    return 1;
}

static void apply_layer_counts(
    const CSB_V1_CustomBackgroundsBitmapApplication *application,
    CSB_V1_CustomBackgroundsViewportState *state)
{
    if (!application || !state) {
        return;
    }
    state->large_apply_count += application->large_applied;
    state->middle_apply_count += application->middle_applied;
    state->near_apply_count += application->near_applied;
}

const CSB_V1_CustomBackgroundsRoomSlotContract *
csb_v1_viewport_custom_backgrounds_room_slot_contract_pc34(void)
{
    return &s_contract;
}

size_t csb_v1_viewport_custom_backgrounds_room_slot_count_pc34(void)
{
    return sizeof(s_room_slots) / sizeof(s_room_slots[0]);
}

const CSB_V1_CustomBackgroundsRoomSlotSpec *
csb_v1_viewport_custom_backgrounds_room_slot_spec_pc34(size_t index)
{
    if (index >= csb_v1_viewport_custom_backgrounds_room_slot_count_pc34()) {
        return NULL;
    }
    return &s_room_slots[index];
}

const CSB_V1_CustomBackgroundsRoomSlotSpec *
csb_v1_viewport_custom_backgrounds_room_slot_spec_for_room_pc34(int room_num)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_custom_backgrounds_room_slot_count_pc34(); ++i) {
        if (s_room_slots[i].room_num == room_num) {
            return &s_room_slots[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_custom_backgrounds_room_slot_select_pc34(
    const uint8_t *level_cell_skins,
    int width,
    int height,
    int loaded_level,
    int party_x,
    int party_y,
    int facing,
    int room_num,
    int default_skin,
    CSB_V1_CustomBackgroundsRoomSlotSelection *out_selection)
{
    const CSB_V1_CustomBackgroundsRoomSlotSpec *slot;
    int target_x;
    int target_y;
    int skin = 0;

    if (out_selection) {
        memset(out_selection, 0, sizeof(*out_selection));
    }

    slot = csb_v1_viewport_custom_backgrounds_room_slot_spec_for_room_pc34(room_num);
    if (!slot || !out_selection ||
        !translate_room_slot(slot, party_x, party_y, facing, &target_x, &target_y)) {
        return 0;
    }

    out_selection->loaded_level = loaded_level;
    out_selection->room_num = room_num;
    out_selection->room_slot_ordinal = slot->room_slot_ordinal;
    out_selection->redmcsb_view_square_ordinal = slot->redmcsb_view_square_ordinal;
    out_selection->relative_forward = slot->relative_forward;
    out_selection->relative_side = slot->relative_side;
    out_selection->target_x = target_x;
    out_selection->target_y = target_y;
    out_selection->source_lines = s_csb_custom_backgrounds_anchor;

    if (level_cell_skins && width > 0 && height > 0 &&
        target_x >= 0 && target_y >= 0 && target_x < width && target_y < height) {
        skin = level_cell_skins[(size_t)target_y * (size_t)width + (size_t)target_x];
    }

    if (skin == 0 && default_skin > 0) {
        skin = default_skin;
        out_selection->used_default_skin = 1;
    }

    out_selection->selected_skin = skin;
    out_selection->has_custom_background_entry = skin != 0;
    out_selection->default_backdrop_selected = skin == 0;
    return 1;
}

int csb_v1_viewport_custom_backgrounds_room_slot_apply_bitmap_pc34(
    const CSB_V1_CustomBackgroundsRoomSlotSelection *selection,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    CSB_V1_CustomBackgroundsViewportState *state,
    CSB_V1_CustomBackgroundsBitmapApplication *out_application)
{
    CSB_V1_CustomBackgroundsBitmapApplication application;

    memset(&application, 0, sizeof(application));
    application.room_num = selection ? selection->room_num : -1;
    application.selected_skin = selection ? selection->selected_skin : 0;
    application.source_lines = s_csb_bitmap_application_anchor;

    if (!selection || !state || !out_application) {
        return 0;
    }

    if (!selection->has_custom_background_entry ||
        !skin_def_words ||
        skin_def_word_count < CSB_SKIN_DEF_MIN_WORDS) {
        application.default_backdrop_selected = 1;
        state->default_backdrop_selected = 1;
        *out_application = application;
        return 1;
    }

    application.large_bitmap_graphic_id = skin_def_words[CSB_LARGE_BITMAP_INDEX];
    application.near_bitmap_graphic_id = skin_def_words[CSB_NEAR_BITMAP_INDEX];
    application.middle_bitmap_graphic_id = skin_def_words[CSB_MIDDLE_BITMAP_INDEX];
    application.large_mask_graphic_id = skin_def_words[CSB_LARGE_MASK_INDEX];
    application.near_mask_graphic_id = skin_def_words[CSB_NEAR_MASK_INDEX];
    application.middle_mask_graphic_id = skin_def_words[CSB_MIDDLE_MASK_INDEX];

    application.large_applied =
        application.large_bitmap_graphic_id != 0 && application.large_mask_graphic_id != 0;
    application.middle_applied =
        application.middle_bitmap_graphic_id != 0 && application.middle_mask_graphic_id != 0;
    application.near_applied =
        selection->room_num < CSB_NEAR_LAYER_ROOM_LIMIT &&
        application.near_bitmap_graphic_id != 0 &&
        application.near_mask_graphic_id != 0;
    application.applybackground_call_count =
        application.large_applied + application.middle_applied + application.near_applied;

    state->last_room_num = selection->room_num;
    state->last_skin = selection->selected_skin;
    if (selection->room_num == CSB_FIRST_BACKDROP_ROOM) {
        state->first_backdrop_room_num = selection->room_num;
        state->first_backdrop_skin = selection->selected_skin;
        application.mutates_first_backdrop_selection = 1;
    }
    if (selection->room_num == CSB_SECOND_BACKDROP_ROOM) {
        state->second_backdrop_room_num = selection->room_num;
        state->second_backdrop_skin = selection->selected_skin;
        application.mutates_second_backdrop_selection = 1;
    }
    application.keeps_both_backdrops_gate_disjoint =
        !(application.mutates_first_backdrop_selection &&
          application.mutates_second_backdrop_selection);

    apply_layer_counts(&application, state);
    *out_application = application;
    return 1;
}

const char *
csb_v1_viewport_custom_backgrounds_room_slot_source_evidence_pc34(void)
{
    return s_source_evidence;
}
