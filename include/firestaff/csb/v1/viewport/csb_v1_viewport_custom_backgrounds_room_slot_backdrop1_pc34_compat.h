#ifndef FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_ROOM_SLOT_BACKDROP1_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_ROOM_SLOT_BACKDROP1_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int contract_only;
    int room_slot_count;
    int skin_def_min_words;
    int backdrop0_bitmap_skin_def_index;
    int backdrop0_mask_skin_def_index;
    int backdrop1_bitmap_skin_def_index;
    int backdrop1_mask_skin_def_index;
    int backdrop1_bitmap_size_words;
    int backdrop1_mask_height;
    int backdrop1_room_num_limit;
    int backdrop1_after_backdrop0;
    int backdrop1_after_f0107_keepout;
    int reuses_room_slot_selector;
    uint32_t expected_trace_hash;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_f0098_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_defs_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *csb_lineage_applybackground_anchor;
    const char *csb_lineage_bitmap_application_anchor;
    const char *disjointness_note;
    const char *source_summary;
} CSB_V1_CustomBackgroundsRoomSlotBackdrop1Contract;

typedef struct {
    int room_num;
    int call_order;
    int redmcsb_view_square_ordinal;
    int relative_forward;
    int relative_side;
    int target_x;
    int target_y;
    int selected_skin;
    int room_lookup_used_same_slot_table;
    int backdrop0_bitmap_graphic_id;
    int backdrop0_mask_graphic_id;
    int backdrop1_bitmap_graphic_id;
    int backdrop1_mask_graphic_id;
    int backdrop0_applybackground_ordinal;
    int middle_applybackground_ordinal;
    int backdrop1_applybackground_ordinal;
    int f0107_keepout_order;
    int backdrop0_composite_order;
    int backdrop1_composite_order;
    int backdrop1_after_backdrop0;
    int backdrop1_after_f0107_keepout;
    int backdrop1_room_gate_allows_apply;
    int backdrop1_applied;
    int backdrop1_bitmap_size_words;
    int backdrop1_mask_height;
    const char *room_name;
    const char *source_lines;
} CSB_V1_CustomBackgroundsRoomSlotBackdrop1Trace;

const CSB_V1_CustomBackgroundsRoomSlotBackdrop1Contract *
csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_contract_pc34(void);

size_t csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_trace_pc34(
    CSB_V1_CustomBackgroundsRoomSlotBackdrop1Trace *out_trace,
    size_t out_capacity);

uint32_t csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_hash_pc34(
    const CSB_V1_CustomBackgroundsRoomSlotBackdrop1Trace *trace,
    size_t trace_count);

const char *
csb_v1_viewport_custom_backgrounds_room_slot_backdrop1_source_evidence_pc34(void);

#endif
