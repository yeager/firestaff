#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3L2_D3R2_DOOR_ORNAMENT_ZONE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3L2_D3R2_DOOR_ORNAMENT_ZONE_PC34_COMPAT_H

typedef struct {
    const char *case_name;
    int zone;
    int door_state;
    int door_type;
    int door_native_bitmap_index;
    int ornament_index;
    int expected_zone_mapping;
    int expected_early_return;
    int expected_destroyed_branch;
    int expected_thieves_eye_ornament_drawn;
    int event73_count_thieves_eye;
    const char *anchor;
} CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34;

int csb_v1_viewport_d3l2_d3r2_door_ornament_zone_pc34_compat_test(void);

#endif
