#ifndef FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_ROOM_SLOT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_ROOM_SLOT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int room_num;
    int room_slot_ordinal;
    int redmcsb_view_square_ordinal;
    int call_order;
    int relative_forward;
    int relative_side;
    int applies_near_layer;
    const char *room_name;
    const char *source_lines;
} CSB_V1_CustomBackgroundsRoomSlotSpec;

typedef struct {
    int loaded_level;
    int room_num;
    int room_slot_ordinal;
    int redmcsb_view_square_ordinal;
    int relative_forward;
    int relative_side;
    int target_x;
    int target_y;
    int selected_skin;
    int used_default_skin;
    int has_custom_background_entry;
    int default_backdrop_selected;
    const char *source_lines;
} CSB_V1_CustomBackgroundsRoomSlotSelection;

typedef struct {
    int room_num;
    int selected_skin;
    int large_bitmap_graphic_id;
    int large_mask_graphic_id;
    int middle_bitmap_graphic_id;
    int middle_mask_graphic_id;
    int near_bitmap_graphic_id;
    int near_mask_graphic_id;
    int large_applied;
    int middle_applied;
    int near_applied;
    int applybackground_call_count;
    int mutates_first_backdrop_selection;
    int mutates_second_backdrop_selection;
    int keeps_both_backdrops_gate_disjoint;
    int default_backdrop_selected;
    const char *source_lines;
} CSB_V1_CustomBackgroundsBitmapApplication;

typedef struct {
    int first_backdrop_room_num;
    int second_backdrop_room_num;
    int first_backdrop_skin;
    int second_backdrop_skin;
    int last_room_num;
    int last_skin;
    int large_apply_count;
    int middle_apply_count;
    int near_apply_count;
    int default_backdrop_selected;
} CSB_V1_CustomBackgroundsViewportState;

typedef struct {
    int contract_only;
    int room_slot_count;
    int first_backdrop_room_num;
    int second_backdrop_room_num;
    int skin_def_min_words;
    int large_bitmap_skin_def_index;
    int large_mask_skin_def_index;
    int middle_bitmap_skin_def_index;
    int middle_mask_skin_def_index;
    int near_bitmap_skin_def_index;
    int near_mask_skin_def_index;
    int near_layer_room_num_limit;
    const char *redmcsb_viewport_anchor;
    const char *redmcsb_floor_ceiling_anchor;
    const char *redmcsb_defs_room_slot_anchor;
    const char *csb_lineage_relpos_anchor;
    const char *csb_lineage_custom_backgrounds_anchor;
    const char *csb_lineage_bitmap_application_anchor;
    const char *csb_lineage_room_dispatch_anchor;
    const char *source_summary;
} CSB_V1_CustomBackgroundsRoomSlotContract;

const CSB_V1_CustomBackgroundsRoomSlotContract *
csb_v1_viewport_custom_backgrounds_room_slot_contract_pc34(void);

size_t csb_v1_viewport_custom_backgrounds_room_slot_count_pc34(void);

const CSB_V1_CustomBackgroundsRoomSlotSpec *
csb_v1_viewport_custom_backgrounds_room_slot_spec_pc34(size_t index);

const CSB_V1_CustomBackgroundsRoomSlotSpec *
csb_v1_viewport_custom_backgrounds_room_slot_spec_for_room_pc34(int room_num);

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
    CSB_V1_CustomBackgroundsRoomSlotSelection *out_selection);

int csb_v1_viewport_custom_backgrounds_room_slot_apply_bitmap_pc34(
    const CSB_V1_CustomBackgroundsRoomSlotSelection *selection,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    CSB_V1_CustomBackgroundsViewportState *state,
    CSB_V1_CustomBackgroundsBitmapApplication *out_application);

const char *
csb_v1_viewport_custom_backgrounds_room_slot_source_evidence_pc34(void);

#endif
