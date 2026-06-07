#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_D0C_STAIRS_PIT_PC34_ELEMENT_PIT = 2,
    DM1_V1_D0C_STAIRS_PIT_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D0C_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT = 19,
    DM1_V1_D0C_STAIRS_PIT_PC34_STAIRS_UP_SLOT_LEFT = 6,
    DM1_V1_D0C_STAIRS_PIT_PC34_STAIRS_DOWN_SLOT_LEFT = 13,
    DM1_V1_D0C_STAIRS_PIT_PC34_FLOOR_PIT_D0C_GRAPHIC = 57,
    DM1_V1_D0C_STAIRS_PIT_PC34_INVISIBLE_FLOOR_PIT_D0C_GRAPHIC = 63,
    DM1_V1_D0C_STAIRS_PIT_PC34_CEILING_PIT_D0C_GRAPHIC = 69,
    DM1_V1_D0C_STAIRS_PIT_PC34_MEDIA720_VIEW_SQUARE_D0C = 0,
    DM1_V1_D0C_STAIRS_PIT_PC34_LEGACY_VIEW_SQUARE_D0C = 9,
    DM1_V1_D0C_STAIRS_PIT_PC34_CELL_ORDER_BACKLEFT_BACKRIGHT = 0x0021,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_STAIRS_UP_D0L = 811,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_STAIRS_UP_D0R = 812,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_D0L = 824,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_D0R = 825,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D0C = 862,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_CEILING_PIT_D0C = 871,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_FIELD_D0C = 715
};

typedef struct {
    bool contract_only;
    const char *s_contract_anchor;
    const char *s_f0098_anchor;
    const char *s_dispatch_anchor;
    const char *s_d0c_anchor;
    const char *s_defs_element_anchor;
    const char *s_defs_square_aspect_anchor;
    const char *s_defs_stairs_bitmap_anchor;
    const char *s_defs_cell_order_anchor;
    const char *s_defs_zone_anchor;
    const char *s_stairs_anchor;
    const char *s_stairs_up_anchor;
    const char *s_stairs_down_anchor;
    const char *s_pit_anchor;
    const char *s_ceiling_pit_anchor;
    const char *s_thing_pass_anchor;
    const char *s_field_anchor;
    const char *s_no_f0099_anchor;
    const char *s_no_f0108_anchor;
    const char *s_source_evidence;
    int f0098_order;
    int d0c_switch_order;
    int stairs_override_order;
    int pit_override_order;
    int ceiling_pit_order;
    int thing_pass_order;
    int field_order;
    int stairs_up_slot_left;
    int stairs_down_slot_left;
    int floor_pit_graphic;
    int invisible_floor_pit_graphic;
    int ceiling_pit_graphic;
    int media720_view_square_d0c;
    int legacy_view_square_d0c;
    int cell_order_backleft_backright;
    int stairs_up_zone_left;
    int stairs_up_zone_right;
    int stairs_down_zone_left;
    int stairs_down_zone_right;
    int floor_pit_zone;
    int ceiling_pit_zone;
    int field_zone;
    bool f0098_precedes_pit_override;
    bool f0098_precedes_stairs_override;
    bool stairs_down_calls_f0115;
    bool pit_calls_f0115;
    bool ceiling_dispatch_calls_f0099;
    bool stairs_down_calls_f0108_floor_ornament;
} DM1_V1_D0CStairsPitDispatchContractPc34;

const DM1_V1_D0CStairsPitDispatchContractPc34 *
dm1_v1_viewport_d0c_stairs_pit_dispatch_contract_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
