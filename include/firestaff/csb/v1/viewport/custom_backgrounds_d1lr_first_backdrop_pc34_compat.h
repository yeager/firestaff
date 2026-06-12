/* CSB V1 contract-only CustomBackgrounds D1L/D1R first-backdrop gate.
 * CSB-lineage anchors: Viewport.cpp:5324-5337 relpos tables,
 * 6451-6505 ApplyBackground, 6574-6622 CustomBackgrounds, and
 * 7050-7070 D1L/D1R room dispatch.
 * ReDMCSB anchors: DUNVIEW.C F0128:8318-8542 and DEFS.H:2596-2614.
 */
#ifndef FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_D1LR_FIRST_BACKDROP_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_D1LR_FIRST_BACKDROP_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_D1LR_FIRST_BACKDROP_SIDE_D1L = 1,
    CSB_V1_D1LR_FIRST_BACKDROP_SIDE_D1R = 2
} CSB_V1_D1LRFirstBackdropSidePc34;

typedef struct {
    int contract_only;
    int no_game_data_dependency;
    int no_real_asset_pixels;
    int pair_count;
    int skin_def_min_words;
    int first_backdrop_bitmap_index;
    int first_backdrop_mask_index;
    int middle_backdrop_bitmap_index;
    int middle_backdrop_mask_index;
    int near_backdrop_bitmap_index;
    int near_backdrop_mask_index;
    int near_layer_room_limit;
    int first_backdrop_apply_order;
    int middle_backdrop_apply_order;
    int d1l_room_num;
    int d1r_room_num;
    int distinct_from_d0l2_d0r2_first_backdrop;
    int distinct_from_d0c_first_backdrop;
    int distinct_from_room_slot_backdrop1;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_relpos_anchor;
    const char *csb_lineage_applybackground_anchor;
    const char *csb_lineage_custom_backgrounds_anchor;
    const char *csb_lineage_d1lr_dispatch_anchor;
    const char *source_summary;
} CSB_V1_D1LRFirstBackdropContractPc34;

typedef struct {
    int side;
    int room_num;
    int room_slot_ordinal;
    int redmcsb_view_square;
    int relative_forward;
    int relative_side;
    int f0128_depth;
    int f0128_lateral;
    int custom_backgrounds_before_square_body;
    int near_layer_rejected_by_room_limit;
    const char *room_name;
    const char *source_lines;
} CSB_V1_D1LRFirstBackdropPairPc34;

typedef struct {
    int room_num;
    int target_x;
    int target_y;
    int selected_skin;
    int used_default_skin;
    int has_custom_background_entry;
    int first_bitmap_id;
    int first_mask_id;
    int middle_bitmap_id;
    int middle_mask_id;
    int near_bitmap_id;
    int near_mask_id;
    int first_backdrop_applied;
    int middle_backdrop_applied_after_first;
    int near_backdrop_rejected;
    int first_backdrop_apply_order;
    int middle_backdrop_apply_order;
    int near_backdrop_apply_order;
    uint32_t masked_sample_before;
    uint32_t masked_sample_source;
    uint16_t masked_sample_mask;
    uint32_t masked_sample_after;
    const char *source_lines;
} CSB_V1_D1LRFirstBackdropTracePc34;

const CSB_V1_D1LRFirstBackdropContractPc34 *
csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_contract_pc34(void);

size_t csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_count_pc34(void);

const CSB_V1_D1LRFirstBackdropPairPc34 *
csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_at_pc34(size_t index);

const CSB_V1_D1LRFirstBackdropPairPc34 *
csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_pair_for_room_pc34(int room_num);

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
    CSB_V1_D1LRFirstBackdropTracePc34 *out_trace);

uint32_t csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_apply_word_pc34(
    uint32_t destination,
    uint32_t source,
    uint16_t mask);

const char *
csb_v1_viewport_custom_backgrounds_d1lr_first_backdrop_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
