#ifndef FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_ROOM_PASS_ORDER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_ROOM_PASS_ORDER_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CSB_V1_ROOM_PASS_EVENT_F0098_BASELINE = 0,
    CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS = 1,
    CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER = 2,
    CSB_V1_ROOM_PASS_EVENT_F0115_THING_PASS = 3
} CSB_V1_CustomBackgroundsRoomPassEventKind;

typedef struct {
    int ordinal;
    CSB_V1_CustomBackgroundsRoomPassEventKind kind;
    int room_num;
    int redmcsb_view_square;
    int relative_forward;
    int relative_side;
    int redmcsb_line;
    int csb_line;
    const char *label;
    const char *source_lines;
} CSB_V1_CustomBackgroundsRoomPassEvent;

typedef struct {
    int contract_only;
    int event_count;
    int locks_d3_d2_room_passes;
    int f0098_before_room_passes;
    int custom_backgrounds_before_room_draw;
    int custom_backgrounds_before_f0115;
    int d3l_custom_background_room;
    int d3r_custom_background_room;
    int d3c_custom_background_room;
    int d2l_custom_background_room;
    int d2r_custom_background_room;
    int d2c_custom_background_room;
    uint32_t expected_trace_hash;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_f0098_anchor;
    const char *redmcsb_f0115_anchor;
    const char *csb_lineage_applybackground_anchor;
    const char *csb_lineage_custombackgrounds_anchor;
    const char *csb_lineage_room_dispatch_anchor;
    const char *source_summary;
} CSB_V1_CustomBackgroundsRoomPassOrderContract;

const CSB_V1_CustomBackgroundsRoomPassOrderContract *
csb_v1_viewport_custom_backgrounds_room_pass_order_contract_pc34(void);

size_t csb_v1_viewport_custom_backgrounds_room_pass_order_trace_pc34(
    CSB_V1_CustomBackgroundsRoomPassEvent *out_events,
    size_t out_capacity);

uint32_t csb_v1_viewport_custom_backgrounds_room_pass_order_hash_pc34(
    const CSB_V1_CustomBackgroundsRoomPassEvent *events,
    size_t event_count);

int csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
    const CSB_V1_CustomBackgroundsRoomPassEvent *events,
    size_t event_count,
    CSB_V1_CustomBackgroundsRoomPassEventKind kind,
    int room_num);

const char *
csb_v1_viewport_custom_backgrounds_room_pass_order_source_evidence_pc34(void);

#endif
