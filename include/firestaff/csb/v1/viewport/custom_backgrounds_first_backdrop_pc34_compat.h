/* ReDMCSB source-lock anchors:
 * DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542 (CPSF/custom backdrop order),
 * F0098_DUNGEONVIEW_DrawFloorAndCeiling:2962-3002 (floor/ceiling base),
 * F0107:3502-3938 (ornament keepout anchor requested for this gate),
 * DEFS.H:2596-2614 (I34E/P31J view-square ordinals).
 * CSB-lineage anchors:
 * Viewport.cpp:6451-6505 ApplyBackground masked composite and
 * Viewport.cpp:6599-6619 roomNum pSkinDef[0..6] bitmap application.
 */
#ifndef FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_FIRST_BACKDROP_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_FIRST_BACKDROP_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_ROOM0_PSKINDEF0 = 0,
    CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0098_D0L2_D0R2_BASE = 1,
    CSB_V1_FIRST_BACKDROP_SOURCE_LOCK_STEP_F0107_MASK_0X8000_KEEP_OUT = 2
} CSB_V1_CustomBackgroundsFirstBackdropSourceLockStepPc34;

typedef struct {
    int side;
    int view_square;
    int f0128_draw_index;
    int relative_depth;
    int relative_lateral;
    int first_backdrop_room_num;
    int skin_def_bitmap_index;
    int skin_def_mask_index;
    int first_backdrop_bitmap_id;
    int first_backdrop_mask_id;
    int f0098_base_color;
    int f0107_opaque_color;
    int final_keepout_x;
    int final_keepout_y;
    const char *name;
    const char *source_lines;
} CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34;

typedef struct {
    int contract_only;
    int no_game_data_dependency;
    int no_gui_dependency;
    int first_backdrop_room_num;
    int second_backdrop_room_num;
    int room0_rel_forward;
    int room0_rel_side;
    int skin_def_min_words;
    int pskin_first_backdrop_index;
    int pskin_first_backdrop_mask_index;
    int pskin_middle_backdrop_index;
    int pskin_near_backdrop_index;
    int first_backdrop_before_f0098_base;
    int d0l2_d0r2_pair_count;
    int f0107_mask_0x8000_keepout_preserves_destination;
    int f0107_keepout_does_not_erase_first_backdrop;
    int distinct_from_second_backdrop_gate;
    int distinct_from_room_slot_gate;
    int distinct_from_mask_after_floor_ceiling_gate;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_f0098_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_applybackground_anchor;
    const char *csb_lineage_pskindef_anchor;
    const char *source_summary;
} CSB_V1_CustomBackgroundsFirstBackdropSourceLockContractPc34;

typedef struct {
    int loaded_level;
    int room_num;
    int target_x;
    int target_y;
    int selected_skin;
    int used_default_skin;
    int selected_pskin_bitmap_index;
    int selected_pskin_mask_index;
    int selected_bitmap_id;
    int selected_mask_id;
    int selected_first_backdrop;
    int rejected_second_backdrop_path;
    const char *source_lines;
} CSB_V1_CustomBackgroundsFirstBackdropSelectionPc34;

typedef struct {
    int side;
    int ok;
    int step_count;
    CSB_V1_CustomBackgroundsFirstBackdropSourceLockStepPc34 steps[4];
    int first_backdrop_pixel_before_f0098;
    int f0098_pixel_outside_keepout;
    int pixel_before_f0107_keepout;
    int pixel_after_f0107_keepout;
    int final_first_backdrop_pixel;
    int final_f0098_base_pixel;
    int final_f0107_opaque_pixel;
    int f0098_base_after_first_backdrop;
    int f0107_keepout_after_f0098;
    int f0107_keepout_preserved_first_backdrop;
    int room0_pskindef0_applied;
    int room0_pskindef0_before_f0098;
    int room2_second_backdrop_not_used;
    uint64_t content_hash;
    const char *source_lines;
} CSB_V1_CustomBackgroundsFirstBackdropRunPc34;

const CSB_V1_CustomBackgroundsFirstBackdropSourceLockContractPc34 *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_contract_pc34(void);

size_t csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_count_pc34(void);

const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_pc34(size_t index);

const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_pair_for_side_pc34(int side);

size_t csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_order_pc34(
    CSB_V1_CustomBackgroundsFirstBackdropSourceLockStepPc34 *out_steps,
    size_t out_capacity);

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
    CSB_V1_CustomBackgroundsFirstBackdropSelectionPc34 *out_selection);

int csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_run_pc34(
    const CSB_V1_CustomBackgroundsFirstBackdropD0PairPc34 *pair,
    const CSB_V1_CustomBackgroundsFirstBackdropSelectionPc34 *selection,
    CSB_V1_CustomBackgroundsFirstBackdropRunPc34 *out_run);

uint64_t csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_hash_pc34(
    const CSB_V1_CustomBackgroundsFirstBackdropRunPc34 *run);

const char *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_lock_evidence_pc34(void);

#endif
