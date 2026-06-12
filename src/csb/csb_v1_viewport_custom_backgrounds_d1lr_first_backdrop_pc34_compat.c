#include "firestaff/csb/v1/viewport/custom_backgrounds_d1lr_first_backdrop_pc34_compat.h"

#include <string.h>

enum {
    CSB_ROOM_D1L = 10,
    CSB_ROOM_D1R = 11,
    CSB_VIEW_SQUARE_D1L = 4,
    CSB_VIEW_SQUARE_D1R = 5,
    CSB_SKIN_DEF_MIN_WORDS = 7,
    CSB_PSKIN_FIRST_BITMAP = 0,
    CSB_PSKIN_NEAR_BITMAP = 1,
    CSB_PSKIN_MIDDLE_BITMAP = 2,
    CSB_PSKIN_FIRST_MASK = 4,
    CSB_PSKIN_NEAR_MASK = 5,
    CSB_PSKIN_MIDDLE_MASK = 6,
    CSB_NEAR_ROOM_LIMIT = 5,
    CSB_FIRST_APPLY_ORDER = 0,
    CSB_MIDDLE_APPLY_ORDER = 1
};

static const char s_redmcsb_f0128_anchor[] =
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542 dispatches "
    "D1L at 8524-8525 and D1R at 8528-8529 after the CSB backdrop pass.";

static const char s_redmcsb_defs_anchor[] =
    "ReDMCSB DEFS.H:2596-2614 anchors M607_VIEW_SQUARE_D1L=4 and "
    "M608_VIEW_SQUARE_D1R=5 for the selected side pair.";

static const char s_csb_relpos_anchor[] =
    "CSB-lineage Viewport.cpp:5324-5337 relposSid/relposFwd and facing "
    "deltas translate roomNum 10/11 to D1L/D1R.";

static const char s_csb_applybackground_anchor[] =
    "CSB-lineage Viewport.cpp:6451-6505 ApplyBackground merges source "
    "bitmap words with destination words through the room mask.";

static const char s_csb_custom_backgrounds_anchor[] =
    "CSB-lineage Viewport.cpp:6574-6622 CustomBackgrounds gets the skin, "
    "falls back to default skin, then applies pSkinDef[0]/[4] before "
    "pSkinDef[2]/[6] and the roomNum<5 pSkinDef[1]/[5] near layer.";

static const char s_csb_d1lr_dispatch_anchor[] =
    "CSB-lineage Viewport.cpp:7050-7070 dispatches CustomBackgrounds room "
    "10 before D1L and room 11 before D1R.";

static const char s_source_summary[] =
    "contract_only=1; no_game_data=1; CSB V1 D1L/D1R first-backdrop slice; "
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542; "
    "DEFS.H:2596-2614; CSB-lineage Viewport.cpp:5324-5337 relpos; "
    "Viewport.cpp:6451-6505 ApplyBackground; Viewport.cpp:6574-6622 "
    "CustomBackgrounds pSkinDef[0]/[4] first; Viewport.cpp:7050-7070 "
    "room 10/11 D1L/D1R dispatch; disjoint from D0L2/D0R2 first-backdrop, "
    "D0C first-backdrop, and room-slot backdrop1 gates.";

static const CSB_V1_D1LRFirstBackdropContractPc34 s_contract = {
    1,
    1,
    1,
    2,
    CSB_SKIN_DEF_MIN_WORDS,
    CSB_PSKIN_FIRST_BITMAP,
    CSB_PSKIN_FIRST_MASK,
    CSB_PSKIN_MIDDLE_BITMAP,
    CSB_PSKIN_MIDDLE_MASK,
    CSB_PSKIN_NEAR_BITMAP,
    CSB_PSKIN_NEAR_MASK,
    CSB_NEAR_ROOM_LIMIT,
    CSB_FIRST_APPLY_ORDER,
    CSB_MIDDLE_APPLY_ORDER,
    CSB_ROOM_D1L,
    CSB_ROOM_D1R,
    1,
    1,
    1,
    s_redmcsb_f0128_anchor,
    s_redmcsb_defs_anchor,
    s_csb_relpos_anchor,
    s_csb_applybackground_anchor,
    s_csb_custom_backgrounds_anchor,
    s_csb_d1lr_dispatch_anchor,
    s_source_summary
};

static const CSB_V1_D1LRFirstBackdropPairPc34 s_pairs[] = {
    {
        CSB_V1_D1LR_FIRST_BACKDROP_SIDE_D1L,
        CSB_ROOM_D1L,
        10,
        CSB_VIEW_SQUARE_D1L,
        1,
        -1,
        1,
        -1,
        1,
        1,
        "D1L",
        s_csb_d1lr_dispatch_anchor
    },
    {
        CSB_V1_D1LR_FIRST_BACKDROP_SIDE_D1R,
        CSB_ROOM_D1R,
        11,
        CSB_VIEW_SQUARE_D1R,
        1,
        1,
        1,
        1,
        1,
        1,
        "D1R",
        s_csb_d1lr_dispatch_anchor
    }
};

static int translate_room(
    const CSB_V1_D1LRFirstBackdropPairPc34 *pair,
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

    if (!pair || !out_x || !out_y || facing < 0 || facing > 3) {
        return 0;
    }

    *out_x = party_x + dx_side[facing] * pair->relative_side +
             dx_fwd[facing] * pair->relative_forward;
    *out_y = party_y + dy_side[facing] * pair->relative_side +
             dy_fwd[facing] * pair->relative_forward;
    return 1;
}

const CSB_V1_D1LRFirstBackdropContractPc34 *
csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_contract_pc34(void)
{
    /* CSB-lineage Viewport.cpp:6574-6622 anchors the pSkinDef[0]/[4]
     * first-backdrop application; Viewport.cpp:7050-7070 pins this gate to
     * the D1L/D1R room slots only. */
    return &s_contract;
}

size_t csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_count_pc34(void)
{
    return sizeof(s_pairs) / sizeof(s_pairs[0]);
}

const CSB_V1_D1LRFirstBackdropPairPc34 *
csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_count_pc34()) {
        return NULL;
    }
    return &s_pairs[index];
}

const CSB_V1_D1LRFirstBackdropPairPc34 *
csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_for_room_pc34(int room_num)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_count_pc34(); ++i) {
        if (s_pairs[i].room_num == room_num) {
            return &s_pairs[i];
        }
    }
    return NULL;
}

uint32_t csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_apply_word_pc34(
    uint32_t destination,
    uint32_t source,
    uint16_t mask)
{
    const uint32_t expanded_mask = (uint32_t)mask | ((uint32_t)mask << 16);

    /* CSB-lineage Viewport.cpp:6494-6497: dst=(dst & ~mask)|(src & mask). */
    return (destination & ~expanded_mask) | (source & expanded_mask);
}

int csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_select_pc34(
    const CSB_V1_D1LRFirstBackdropPairPc34 *pair,
    const uint8_t *level_cell_skins,
    int width,
    int height,
    int party_x,
    int party_y,
    int facing,
    int default_skin,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    CSB_V1_D1LRFirstBackdropTracePc34 *out_trace)
{
    CSB_V1_D1LRFirstBackdropTracePc34 trace;
    int skin = 0;

    if (!out_trace || !pair) {
        return 0;
    }

    memset(&trace, 0, sizeof(trace));
    if (!translate_room(pair, party_x, party_y, facing,
                        &trace.target_x, &trace.target_y)) {
        return 0;
    }
    trace.room_num = pair->room_num;
    trace.source_lines = s_csb_custom_backgrounds_anchor;

    if (level_cell_skins && width > 0 && height > 0 &&
        trace.target_x >= 0 && trace.target_y >= 0 &&
        trace.target_x < width && trace.target_y < height) {
        skin = level_cell_skins[(size_t)trace.target_y * (size_t)width +
                                (size_t)trace.target_x];
    }
    if (skin == 0 && default_skin > 0) {
        skin = default_skin;
        trace.used_default_skin = 1;
    }
    trace.selected_skin = skin;
    trace.has_custom_background_entry = skin != 0;

    if (trace.has_custom_background_entry &&
        skin_def_words &&
        skin_def_word_count >= CSB_SKIN_DEF_MIN_WORDS) {
        trace.first_bitmap_id = skin_def_words[CSB_PSKIN_FIRST_BITMAP];
        trace.first_mask_id = skin_def_words[CSB_PSKIN_FIRST_MASK];
        trace.middle_bitmap_id = skin_def_words[CSB_PSKIN_MIDDLE_BITMAP];
        trace.middle_mask_id = skin_def_words[CSB_PSKIN_MIDDLE_MASK];
        trace.near_bitmap_id = skin_def_words[CSB_PSKIN_NEAR_BITMAP];
        trace.near_mask_id = skin_def_words[CSB_PSKIN_NEAR_MASK];
        trace.first_backdrop_applied = trace.first_bitmap_id != 0 && trace.first_mask_id != 0;
        trace.middle_backdrop_applied_after_first =
            trace.first_backdrop_applied &&
            trace.middle_bitmap_id != 0 &&
            trace.middle_mask_id != 0;
    }

    trace.near_backdrop_rejected = pair->room_num >= CSB_NEAR_ROOM_LIMIT;
    trace.first_backdrop_apply_order = trace.first_backdrop_applied ? CSB_FIRST_APPLY_ORDER : -1;
    trace.middle_backdrop_apply_order =
        trace.middle_backdrop_applied_after_first ? CSB_MIDDLE_APPLY_ORDER : -1;
    trace.near_backdrop_apply_order = -1;
    trace.masked_sample_before = 0x11223344u;
    trace.masked_sample_source = 0xaabbccddu;
    trace.masked_sample_mask = 0x00ffu;
    trace.masked_sample_after =
        csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_apply_word_pc34(
            trace.masked_sample_before,
            trace.masked_sample_source,
            trace.masked_sample_mask);

    *out_trace = trace;
    return 1;
}

const char *
csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_source_evidence_pc34(void)
{
    return s_source_summary;
}
