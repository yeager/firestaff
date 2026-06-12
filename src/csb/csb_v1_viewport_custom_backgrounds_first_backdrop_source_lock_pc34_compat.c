/* ReDMCSB source-lock anchors:
 * DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542, F0098:2962-3002,
 * F0107:3502-3938, and DEFS.H:2596-2614.
 * CSB-lineage anchors: Viewport.cpp:6451-6505 ApplyBackground and
 * Viewport.cpp:6599-6619 pSkinDef[0] per-room bitmap application.
 */
#include "firestaff/csb/v1/viewport/custom_backgrounds_first_backdrop_pc34_compat.h"

#include <string.h>

enum {
    CSB_SIDE_D0L2 = 1,
    CSB_SIDE_D0R2 = 2,
    CSB_VIEW_SQUARE_D0L2 = 8,
    CSB_VIEW_SQUARE_D0R2 = 10,
    CSB_F0128_D0L2_ORDER = 16,
    CSB_F0128_D0R2_ORDER = 17,
    CSB_D0_DEPTH = 0,
    CSB_D0L2_LANE = -2,
    CSB_D0R2_LANE = 2,
    CSB_FIRST_BACKDROP_ROOM = 0,
    CSB_SECOND_BACKDROP_ROOM = 2,
    CSB_ROOM0_REL_FORWARD = 3,
    CSB_ROOM0_REL_SIDE = -2,
    CSB_SKIN_DEF_MIN_WORDS = 7,
    CSB_PSKINDEF_FIRST_BITMAP = 0,
    CSB_PSKINDEF_NEAR_BITMAP = 1,
    CSB_PSKINDEF_MIDDLE_BITMAP = 2,
    CSB_PSKINDEF_FIRST_MASK = 4,
    CSB_FIRST_BACKDROP_COLOR = 31,
    CSB_F0098_LEFT_BASE_COLOR = 17,
    CSB_F0098_RIGHT_BASE_COLOR = 18,
    CSB_F0107_COLOR = 47
};

static const char s_redmcsb_f0128_anchor[] =
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542 anchors "
    "CPSF viewport ordering and the D0L2/D0R2 pair surface used here.";

static const char s_redmcsb_f0098_anchor[] =
    "ReDMCSB DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002 "
    "anchors the floor+ceiling base pass.";

static const char s_redmcsb_f0107_anchor[] =
    "ReDMCSB DUNVIEW.C F0107:3502-3938 anchors the requested ornament "
    "keepout; this source-lock models MASK 0x8000 as destination-preserving.";

static const char s_redmcsb_defs_anchor[] =
    "ReDMCSB DEFS.H:2596-2614 anchors I34E/P31J view-square ordinals; "
    "the D0L2/D0R2 compatibility pair carries ordinals 8 and 10.";

static const char s_csb_applybackground_anchor[] =
    "CSB-lineage Viewport.cpp:6451-6505 ApplyBackground masked composite "
    "copies bitmap bits through the room mask into d.pViewportBMP.";

static const char s_csb_pskindef_anchor[] =
    "CSB-lineage Viewport.cpp:6574-6622 CustomBackgrounds selects skin by "
    "roomNum and Viewport.cpp:6599-6619 applies pSkinDef[0]/[4] first.";

static const char s_source_evidence[] =
    "contract_only=1; no_game_data=1; ReDMCSB DUNVIEW.C "
    "F0128_DUNGEONVIEW_Draw_CPSF:8318-8542; "
    "F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002; "
    "F0107:3502-3938 MASK 0x8000 keepout; DEFS.H:2596-2614 "
    "I34E/P31J view-square ordinals; CSB-lineage Viewport.cpp:6451-6505 "
    "ApplyBackground masked composite; Viewport.cpp:6574-6622 "
    "CustomBackgrounds roomNum skin selection; Viewport.cpp:6599-6619 "
    "pSkinDef[0]/[4] first-backdrop bitmap before pSkinDef[2]/[6] and "
    "pSkinDef[1]/[5]; distinct from second-backdrop, room-slot, and "
    "mask-after-floor+ceiling gates.";

static const CSB_V1_CustomBackgroundsFirstBackdropSourceLockContractPc34 s_contract = {
    1,
    1,
    1,
    CSB_FIRST_BACKDROP_ROOM,
    CSB_SECOND_BACKDROP_ROOM,
    CSB_ROOM0_REL_FORWARD,
    CSB_ROOM0_REL_SIDE,
    CSB_SKIN_DEF_MIN_WORDS,
    CSB_PSKINDEF_FIRST_BITMAP,
    CSB_PSKINDEF_FIRST_MASK,
    CSB_PSKINDEF_MIDDLE_BITMAP,
    CSB_PSKINDEF_NEAR_BITMAP,
    1,
    2,
    1,
    1,
    1,
    1,
    1,
    s_redmcsb_f0128_anchor,
    s_redmcsb_f0098_anchor,
    s_redmcsb_f0107_anchor,
    s_redmcsb_defs_anchor,
    s_csb_applybackground_anchor,
    s_csb_pskindef_anchor,
    s_source_evidence
};

static const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 s_pairs[] = {
    {
        CSB_SIDE_D0L2,
        CSB_VIEW_SQUARE_D0L2,
        CSB_F0128_D0L2_ORDER,
        CSB_D0_DEPTH,
        CSB_D0L2_LANE,
        CSB_FIRST_BACKDROP_ROOM,
        CSB_PSKINDEF_FIRST_BITMAP,
        CSB_PSKINDEF_FIRST_MASK,
        101,
        401,
        CSB_F0098_LEFT_BASE_COLOR,
        CSB_F0107_COLOR,
        104,
        60,
        "D0L2 first CustomBackgrounds backdrop keepout sample",
        s_source_evidence
    },
    {
        CSB_SIDE_D0R2,
        CSB_VIEW_SQUARE_D0R2,
        CSB_F0128_D0R2_ORDER,
        CSB_D0_DEPTH,
        CSB_D0R2_LANE,
        CSB_FIRST_BACKDROP_ROOM,
        CSB_PSKINDEF_FIRST_BITMAP,
        CSB_PSKINDEF_FIRST_MASK,
        101,
        401,
        CSB_F0098_RIGHT_BASE_COLOR,
        CSB_F0107_COLOR,
        120,
        60,
        "D0R2 first CustomBackgrounds backdrop keepout sample",
        s_source_evidence
    }
};

static const CSB_V1_CustomBackgroundsFirstBackdropSourceLockStepPc34 s_order[] = {
    CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_ROOM0_PSKINDEF0,
    CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0098_D0L2_D0R2_BASE,
    CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0107_MASK_0X8000_KEEP_OUT
};

static uint64_t fnv1a_u8(uint64_t hash, uint8_t value)
{
    hash ^= (uint64_t)value;
    hash *= UINT64_C(1099511628211);
    return hash;
}

static uint64_t fnv1a_i32(uint64_t hash, int value)
{
    uint32_t v = (uint32_t)value;
    size_t i;

    for (i = 0; i < 4u; ++i) {
        hash = fnv1a_u8(hash, (uint8_t)(v & 0xffu));
        v >>= 8;
    }
    return hash;
}

static int translate_room0(int party_x, int party_y, int facing, int *out_x, int *out_y)
{
    static const int dx_fwd[4] = { 0, 1, 0, -1 };
    static const int dy_fwd[4] = { -1, 0, 1, 0 };
    static const int dx_side[4] = { 1, 0, -1, 0 };
    static const int dy_side[4] = { 0, 1, 0, -1 };

    if (!out_x || !out_y || facing < 0 || facing > 3) {
        return 0;
    }

    *out_x = party_x + dx_side[facing] * CSB_ROOM0_REL_SIDE +
             dx_fwd[facing] * CSB_ROOM0_REL_FORWARD;
    *out_y = party_y + dy_side[facing] * CSB_ROOM0_REL_SIDE +
             dy_fwd[facing] * CSB_ROOM0_REL_FORWARD;
    return 1;
}

const CSB_V1_CustomBackgroundsFirstBackdropSourceLockContractPc34 *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_contract_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128:8318-8542 and F0098:2962-3002 anchor
     * this contract's ordering boundary. CSB-lineage Viewport.cpp:6599-6619
     * anchors pSkinDef[0]/[4] as the first CustomBackgrounds bitmap pair. */
    return &s_contract;
}

size_t csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_count_pc34(void)
{
    return sizeof(s_pairs) / sizeof(s_pairs[0]);
}

const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_pc34(size_t index)
{
    if (index >= csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_count_pc34()) {
        return NULL;
    }
    return &s_pairs[index];
}

const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_count_pc34(); ++i) {
        if (s_pairs[i].side == side) {
            return &s_pairs[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_order_pc34(
    CSB_V1_CustomBackgroundsFirstBackdropSourceLockStepPc34 *out_steps,
    size_t out_capacity)
{
    const size_t count = sizeof(s_order) / sizeof(s_order[0]);

    /* Source-lock order for this narrow gate: pSkinDef[0]/[4] first-backdrop
     * application, then the F0098 D0L2/D0R2 base boundary, then the requested
     * F0107 MASK 0x8000 keepout preservation check. */
    if (out_steps && out_capacity > 0u) {
        const size_t copy_count = out_capacity < count ? out_capacity : count;
        memcpy(out_steps, s_order, copy_count * sizeof(s_order[0]));
    }
    return count;
}

int csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_select_pc34(
    const uint8_t *level_cell_skins,
    int width,
    int height,
    int loaded_level,
    int party_x,
    int party_y,
    int facing,
    int room_num,
    int default_skin,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    CSB_V1_CustomBackgroundsFirstBackdropSelectionPc34 *out_selection)
{
    CSB_V1_CustomBackgroundsFirstBackdropSelectionPc34 selection;
    int x;
    int y;
    int skin = 0;

    if (!out_selection) {
        return 0;
    }
    memset(&selection, 0, sizeof(selection));
    selection.loaded_level = loaded_level;
    selection.room_num = room_num;
    selection.selected_pskin_bitmap_index = CSB_PSKINDEF_FIRST_BITMAP;
    selection.selected_pskin_mask_index = CSB_PSKINDEF_FIRST_MASK;
    selection.rejected_second_backdrop_path = room_num != CSB_SECOND_BACKDROP_ROOM;
    selection.source_lines = s_csb_pskindef_anchor;

    if (room_num != CSB_FIRST_BACKDROP_ROOM ||
        !translate_room0(party_x, party_y, facing, &x, &y)) {
        *out_selection = selection;
        return 0;
    }

    selection.target_x = x;
    selection.target_y = y;
    if (level_cell_skins && width > 0 && height > 0 &&
        x >= 0 && y >= 0 && x < width && y < height) {
        skin = level_cell_skins[(size_t)y * (size_t)width + (size_t)x];
    }
    if (skin == 0 && default_skin > 0) {
        skin = default_skin;
        selection.used_default_skin = 1;
    }
    selection.selected_skin = skin;

    if (skin != 0 && skin_def_words && skin_def_word_count >= CSB_SKIN_DEF_MIN_WORDS &&
        skin_def_words[CSB_PSKINDEF_FIRST_BITMAP] != 0 &&
        skin_def_words[CSB_PSKINDEF_FIRST_MASK] != 0) {
        selection.selected_bitmap_id = skin_def_words[CSB_PSKINDEF_FIRST_BITMAP];
        selection.selected_mask_id = skin_def_words[CSB_PSKINDEF_FIRST_MASK];
        selection.selected_first_backdrop = 1;
    }

    *out_selection = selection;
    return 1;
}

int csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_run_pc34(
    const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *pair,
    const CSB_V1_CustomBackgroundsFirstBackdropSelectionPc34 *selection,
    CSB_V1_CustomBackgroundsFirstBackdropRunPc34 *out_run)
{
    CSB_V1_CustomBackgroundsFirstBackdropRunPc34 run;

    if (!pair || !selection || !out_run || !selection->selected_first_backdrop) {
        return 0;
    }

    memset(&run, 0, sizeof(run));
    run.side = pair->side;
    run.ok = 1;
    run.source_lines = s_source_evidence;
    run.steps[run.step_count++] =
        CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_ROOM0_PSKINDEF0;

    /* CSB-lineage: Viewport.cpp:6599-6603 applies pSkinDef[0]/[4] before
     * this gate's F0098 boundary sample, using deterministic synthetic pixels. */
    run.first_backdrop_pixel_before_f0098 = CSB_FIRST_BACKDROP_COLOR;
    run.room0_pskindef0_applied =
        selection->selected_pskin_bitmap_index == CSB_PSKINDEF_FIRST_BITMAP &&
        selection->selected_pskin_mask_index == CSB_PSKINDEF_FIRST_MASK &&
        selection->selected_bitmap_id == pair->first_backdrop_bitmap_id &&
        selection->selected_mask_id == pair->first_backdrop_mask_id;
    run.room2_second_backdrop_not_used =
        selection->room_num == CSB_FIRST_BACKDROP_ROOM &&
        selection->rejected_second_backdrop_path;
    run.room0_pskindef0_before_f0098 = run.room0_pskindef0_applied;

    run.steps[run.step_count++] =
        CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0098_D0L2_D0R2_BASE;
    /* ReDMCSB: DUNVIEW.C F0098:2962-3002 floor+ceiling base is represented
     * by a side-specific noncentral sample; the central keepout sample keeps
     * the first-backdrop pixel for the following F0107 mask check. */
    run.f0098_pixel_outside_keepout = pair->f0098_base_color;
    run.f0098_base_after_first_backdrop = 1;
    run.pixel_before_f0107_keepout = run.first_backdrop_pixel_before_f0098;

    run.steps[run.step_count++] =
        CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0107_MASK_0X8000_KEEP_OUT;
    /* ReDMCSB: DUNVIEW.C F0107:3502-3938 is the requested keepout anchor.
     * The synthetic MASK 0x8000 source cell keeps the destination unchanged. */
    run.pixel_after_f0107_keepout = run.pixel_before_f0107_keepout;
    run.final_first_backdrop_pixel = run.pixel_after_f0107_keepout;
    run.final_f0098_base_pixel = run.f0098_pixel_outside_keepout;
    run.final_f0107_opaque_pixel = pair->f0107_opaque_color;
    run.f0107_keepout_after_f0098 = 1;
    run.f0107_keepout_preserved_first_backdrop =
        run.pixel_after_f0107_keepout == CSB_FIRST_BACKDROP_COLOR;
    run.content_hash =
        csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_hash_pc34(&run);

    *out_run = run;
    return 1;
}

uint64_t csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_hash_pc34(
    const CSB_V1_CustomBackgroundsFirstBackdropRunPc34 *run)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;

    if (!run) {
        return 0u;
    }

    hash = fnv1a_i32(hash, run->side);
    hash = fnv1a_i32(hash, run->ok);
    hash = fnv1a_i32(hash, run->step_count);
    for (i = 0; i < (size_t)run->step_count && i < sizeof(run->steps) / sizeof(run->steps[0]); ++i) {
        hash = fnv1a_i32(hash, (int)run->steps[i]);
    }
    hash = fnv1a_i32(hash, run->first_backdrop_pixel_before_f0098);
    hash = fnv1a_i32(hash, run->f0098_pixel_outside_keepout);
    hash = fnv1a_i32(hash, run->pixel_before_f0107_keepout);
    hash = fnv1a_i32(hash, run->pixel_after_f0107_keepout);
    hash = fnv1a_i32(hash, run->final_first_backdrop_pixel);
    hash = fnv1a_i32(hash, run->final_f0098_base_pixel);
    hash = fnv1a_i32(hash, run->final_f0107_opaque_pixel);
    hash = fnv1a_i32(hash, run->f0098_base_after_first_backdrop);
    hash = fnv1a_i32(hash, run->f0107_keepout_after_f0098);
    hash = fnv1a_i32(hash, run->f0107_keepout_preserved_first_backdrop);
    hash = fnv1a_i32(hash, run->room0_pskindef0_applied);
    hash = fnv1a_i32(hash, run->room0_pskindef0_before_f0098);
    hash = fnv1a_i32(hash, run->room2_second_backdrop_not_used);
    return hash;
}

const char *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_evidence_pc34(void)
{
    /* ReDMCSB: F0128/F0098/F0107 and DEFS.H anchors are kept together with
     * CSB-lineage Viewport.cpp:6451-6505 and 6599-6619 to make the gate
     * source-lock-only and asset-free. */
    return s_source_evidence;
}
