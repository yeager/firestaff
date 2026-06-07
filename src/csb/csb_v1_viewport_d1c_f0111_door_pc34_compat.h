#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0111_DOOR_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int contract_only;
    int view_square_d1c;
    int view_depth;
    int view_lane;
    int element_door_front;
    int door_native_width;
    int door_native_height;
    int door_native_byte_count;
    int view_door_ornament_d1lcr;
    int door_zone_d1c;
    int doorpass1_order;
    int doorpass2_order;
    int f0124_wall_case_precedes_door_case;
    int f0124_door_precedes_terminal_f0115;
    int f0128_dispatch_after_d1l_d1r;
    int f0128_dispatches_d1c;
    int uses_f0122_d1l;
    int uses_f0123_d1r;
    const char *redmcsb_d1c_call_anchor;
    const char *redmcsb_defs_anchor;
    const char *redmcsb_f0124_order_anchor;
    const char *redmcsb_f0128_dispatch_anchor;
    const char *csb_lineage_viewport_anchor;
    const char *door_bitmap_index_symbol;
    const char *door_byte_count_macro;
    const char *door_view_symbol;
    const char *door_frame_symbol;
    const char *source_evidence;
} CSB_V1_ViewportD1CF0111DoorPc34Contract;

const CSB_V1_ViewportD1CF0111DoorPc34Contract *
csb_v1_viewport_d1c_f0111_door_pc34_contract(void);

const char *
csb_v1_viewport_d1c_f0111_door_pc34_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
