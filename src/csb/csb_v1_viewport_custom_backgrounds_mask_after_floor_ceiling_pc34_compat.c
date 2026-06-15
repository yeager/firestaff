#include "csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_pc34_compat.h"

#include <string.h>

enum {
    CSB_SKIN_DEF_MIN_WORDS = 7,
    CSB_LARGE_BITMAP_INDEX = 0,
    CSB_NEAR_BITMAP_INDEX = 1,
    CSB_MIDDLE_BITMAP_INDEX = 2,
    CSB_LARGE_MASK_INDEX = 4,
    CSB_NEAR_MASK_INDEX = 5,
    CSB_MIDDLE_MASK_INDEX = 6,
    CSB_NEAR_LAYER_ROOM_LIMIT = 5
};

static const char s_redmcsb_f0128_anchor[] =
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542 gates "
    "F0098 before the CSB V1 viewport room pass.";

static const char s_redmcsb_f0098_anchor[] =
    "ReDMCSB DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002 "
    "draws G2109_Ceiling/G2108_Floor and resets "
    "G0297_B_DrawFloorAndCeilingRequested.";

static const char s_redmcsb_defs_view_square_anchor[] =
    "ReDMCSB DEFS.H:2596-2614 I34E/P31J view-square ordinals anchor the "
    "roomNum viewport slots without reselecting them here.";

static const char s_csb_applybackground_anchor[] =
    "CSBWin/CSB-lineage Viewport.cpp:6451-6505 ApplyBackground performs "
    "the masked composite after the source-locked floor/ceiling baseline.";

static const char s_csb_bitmap_application_anchor[] =
    "CSBWin/CSB-lineage Viewport.cpp:6599-6619 applies roomNum pSkinDef "
    "large [0]/[4], middle [2]/[6], and near [1]/[5] bitmaps.";

static const char s_csbwin_viewport_url[] =
    "https://github.com/BeipDev/CSBWin/blob/master/CSBWin/Viewport.cpp";

static const char s_source_evidence[] =
    "contract_only=1; ReDMCSB DUNVIEW.C "
    "F0128_DUNGEONVIEW_Draw_CPSF:8318-8542; "
    "F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002 "
    "G2109_Ceiling/G2108_Floor and G0297_B_DrawFloorAndCeilingRequested "
    "reset; DEFS.H:2596-2614 I34E/P31J view-square ordinals; "
    "CSBWin/CSB-lineage Viewport.cpp:6451-6505 ApplyBackground masked "
    "composite; Viewport.cpp:6599-6619 roomNum pSkinDef bitmap order; "
    "url=https://github.com/BeipDev/CSBWin/blob/master/CSBWin/Viewport.cpp";

static const CSB_V1_CustomBackgroundsMaskAfterFloorCeilingContract s_contract = {
    1,
    CSB_V1_MASK_AFTER_FLOOR_STEP_F0098_FLOOR_CEILING,
    CSB_V1_MASK_AFTER_FLOOR_STEP_G0297_RESET,
    CSB_V1_MASK_AFTER_FLOOR_STEP_APPLYBACKGROUND_MASK,
    CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_LARGE,
    CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_MIDDLE,
    CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_NEAR,
    CSB_V1_MASK_AFTER_FLOOR_STEP_STATE_DEFAULT,
    CSB_SKIN_DEF_MIN_WORDS,
    CSB_LARGE_BITMAP_INDEX,
    CSB_LARGE_MASK_INDEX,
    CSB_MIDDLE_BITMAP_INDEX,
    CSB_MIDDLE_MASK_INDEX,
    CSB_NEAR_BITMAP_INDEX,
    CSB_NEAR_MASK_INDEX,
    CSB_NEAR_LAYER_ROOM_LIMIT,
    s_redmcsb_f0128_anchor,
    s_redmcsb_f0098_anchor,
    s_redmcsb_defs_view_square_anchor,
    s_csb_applybackground_anchor,
    s_csb_bitmap_application_anchor,
    s_csbwin_viewport_url,
    s_source_evidence
};

static const CSB_V1_CustomBackgroundsMaskAfterFloorCeilingStep s_order[] = {
    CSB_V1_MASK_AFTER_FLOOR_STEP_F0098_FLOOR_CEILING,
    CSB_V1_MASK_AFTER_FLOOR_STEP_G0297_RESET,
    CSB_V1_MASK_AFTER_FLOOR_STEP_APPLYBACKGROUND_MASK,
    CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_LARGE,
    CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_MIDDLE,
    CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_NEAR
};

static void append_step(
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingResult *result,
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingStep step)
{
    if (result && result->order_count < sizeof(result->order) / sizeof(result->order[0])) {
        result->order[result->order_count++] = step;
    }
}

static void set_layer(
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingLayer *layer,
    int ordinal,
    int room_num,
    const char *name,
    int bitmap_index,
    int mask_index,
    const uint16_t *skin_def_words,
    int applies)
{
    memset(layer, 0, sizeof(*layer));
    layer->layer_ordinal = ordinal;
    layer->room_num = room_num;
    layer->layer_name = name;
    layer->bitmap_skin_def_index = bitmap_index;
    layer->mask_skin_def_index = mask_index;
    layer->bitmap_graphic_id = skin_def_words[bitmap_index];
    layer->mask_graphic_id = skin_def_words[mask_index];
    layer->applies_for_room_num = applies;
    layer->source_lines = s_csb_bitmap_application_anchor;
}

const CSB_V1_CustomBackgroundsMaskAfterFloorCeilingContract *
csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_contract_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 enters F0098 before the
     * viewport room pass. F0098 lines 2962-3002 draws G2109/G2108 and
     * clears G0297 before CSBWin Viewport.cpp lines 6451-6505 and
     * 6599-6619 composite masks and roomNum skin bitmaps.
     * CSBWin URL: https://github.com/BeipDev/CSBWin/blob/master/CSBWin/Viewport.cpp */
    return &s_contract;
}

size_t csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_order_pc34(
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingStep *out_steps,
    size_t out_capacity)
{
    const size_t count = sizeof(s_order) / sizeof(s_order[0]);

    /* Source-lock order: ReDMCSB DUNVIEW.C F0098:2962-3002 base pixels
     * and G0297 reset, then CSBWin Viewport.cpp:6451-6505 mask composite,
     * then Viewport.cpp:6599-6619 roomNum pSkinDef bitmap application. */
    if (out_steps && out_capacity > 0) {
        const size_t copy_count = out_capacity < count ? out_capacity : count;
        memcpy(out_steps, s_order, copy_count * sizeof(s_order[0]));
    }
    return count;
}

int csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_run_pc34(
    int room_num,
    int draw_floor_and_ceiling_requested,
    int has_custom_background,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingResult *out_result)
{
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingResult result;
    int can_apply;

    if (!out_result || room_num < 0 || room_num > 15) {
        return 0;
    }

    memset(&result, 0, sizeof(result));
    result.room_num = room_num;
    result.had_custom_background = has_custom_background ? 1 : 0;
    result.g0297_initial_requested = draw_floor_and_ceiling_requested ? 1 : 0;
    result.source_lines = s_source_evidence;

    if (draw_floor_and_ceiling_requested) {
        /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 copies/decodes the
         * G2109_Ceiling and G2108_Floor baseline, then clears G0297. */
        result.ceiling_drawn = 1;
        result.floor_drawn = 1;
        append_step(&result, CSB_V1_MASK_AFTER_FLOOR_STEP_F0098_FLOOR_CEILING);
        result.g0297_after_floor_ceiling = 0;
        append_step(&result, CSB_V1_MASK_AFTER_FLOOR_STEP_G0297_RESET);
    } else {
        result.g0297_after_floor_ceiling = 0;
    }

    result.g0297_before_applybackground_mask = result.g0297_after_floor_ceiling;
    can_apply = result.had_custom_background &&
                skin_def_words &&
                skin_def_word_count >= CSB_SKIN_DEF_MIN_WORDS;

    if (!can_apply) {
        result.state_default = 1;
        append_step(&result, CSB_V1_MASK_AFTER_FLOOR_STEP_STATE_DEFAULT);
        result.g0297_after_room_bitmap = result.g0297_before_applybackground_mask;
        result.floor_ceiling_before_mask = result.floor_drawn && result.ceiling_drawn;
        result.reset_before_mask = result.g0297_before_applybackground_mask == 0;
        *out_result = result;
        return 1;
    }

    /* CSBWin/CSB-lineage Viewport.cpp lines 6451-6505: the mask composite
     * is applied only after the ReDMCSB floor/ceiling baseline is reset. */
    result.applybackground_mask_applied = 1;
    append_step(&result, CSB_V1_MASK_AFTER_FLOOR_STEP_APPLYBACKGROUND_MASK);

    set_layer(&result.layers[0], 0, room_num, "large",
              CSB_LARGE_BITMAP_INDEX, CSB_LARGE_MASK_INDEX, skin_def_words, 1);
    set_layer(&result.layers[1], 1, room_num, "middle",
              CSB_MIDDLE_BITMAP_INDEX, CSB_MIDDLE_MASK_INDEX, skin_def_words, 1);
    set_layer(&result.layers[2], 2, room_num, "near",
              CSB_NEAR_BITMAP_INDEX, CSB_NEAR_MASK_INDEX, skin_def_words,
              room_num < CSB_NEAR_LAYER_ROOM_LIMIT);

    append_step(&result, CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_LARGE);
    append_step(&result, CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_MIDDLE);
    result.room_bitmap_large_applied =
        result.layers[0].bitmap_graphic_id != 0 &&
        result.layers[0].mask_graphic_id != 0;
    result.room_bitmap_middle_applied =
        result.layers[1].bitmap_graphic_id != 0 &&
        result.layers[1].mask_graphic_id != 0;
    if (result.layers[2].applies_for_room_num) {
        append_step(&result, CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_NEAR);
        result.room_bitmap_near_applied =
            result.layers[2].bitmap_graphic_id != 0 &&
            result.layers[2].mask_graphic_id != 0;
    }

    result.room_bitmap_apply_count =
        result.room_bitmap_large_applied +
        result.room_bitmap_middle_applied +
        result.room_bitmap_near_applied;
    result.g0297_after_room_bitmap = result.g0297_before_applybackground_mask;
    result.floor_ceiling_before_mask =
        result.floor_drawn && result.ceiling_drawn && result.applybackground_mask_applied;
    result.mask_before_room_bitmap =
        result.applybackground_mask_applied && result.room_bitmap_apply_count > 0;
    result.reset_before_mask = result.g0297_before_applybackground_mask == 0;

    *out_result = result;
    return 1;
}

const char *
csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_source_evidence_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128:8318-8542 and F0098:2962-3002 anchor the
     * baseline/reset timing; DEFS.H:2596-2614 anchors I34E/P31J view-square
     * ordinals. CSBWin Viewport.cpp:6451-6505 and 6599-6619 anchor the
     * mask-before-room-bitmap order. */
    return s_source_evidence;
}
