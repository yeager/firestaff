#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    /* ReDMCSB names the floor-pit square aspect C02_ELEMENT_PIT. */
    DM1_V1_D1C_DISPATCH_PC34_ELEMENT_WALL = 0,
    DM1_V1_D1C_DISPATCH_PC34_ELEMENT_FLOOR = 1,
    DM1_V1_D1C_DISPATCH_PC34_ELEMENT_FLOOR_PIT = 2,
    DM1_V1_D1C_DISPATCH_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D1C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT = 19,
    DM1_V1_D1C_DISPATCH_PC34_VIEW_SQUARE_D1C = 3,
    DM1_V1_D1C_DISPATCH_PC34_VIEW_WALL_D1C_FRONT = 14,
    DM1_V1_D1C_DISPATCH_PC34_CELL_ORDER_ALCOVE = 0x0000,
    DM1_V1_D1C_DISPATCH_PC34_CELL_ORDER_OPEN = 0x3421,
    DM1_V1_D1C_DISPATCH_PC34_STAIRS_UP_SLOT_D1C = 5,
    DM1_V1_D1C_DISPATCH_PC34_STAIRS_DOWN_SLOT_D1C = 12,
    DM1_V1_D1C_DISPATCH_PC34_FLOOR_PIT_D1C_GRAPHIC = 55,
    DM1_V1_D1C_DISPATCH_PC34_ZONE_STAIRS_UP_D1C = 809,
    DM1_V1_D1C_DISPATCH_PC34_ZONE_STAIRS_DOWN_D1C = 822,
    DM1_V1_D1C_DISPATCH_PC34_ZONE_FLOOR_PIT_D1C = 859,
    DM1_V1_D1C_DISPATCH_PC34_ZONE_FIELD_D1C = 712
};

typedef enum {
    DM1_V1_D1C_DISPATCH_PC34_ROUTE_UNSUPPORTED = 0,
    DM1_V1_D1C_DISPATCH_PC34_ROUTE_STAIRS_UP_FRONT,
    DM1_V1_D1C_DISPATCH_PC34_ROUTE_STAIRS_DOWN_FRONT,
    DM1_V1_D1C_DISPATCH_PC34_ROUTE_FLOOR_PIT,
    DM1_V1_D1C_DISPATCH_PC34_ROUTE_WALL_ALCOVE,
    DM1_V1_D1C_DISPATCH_PC34_ROUTE_WALL_NO_ALCOVE,
    DM1_V1_D1C_DISPATCH_PC34_ROUTE_OPEN_FLOOR,
    DM1_V1_D1C_DISPATCH_PC34_ROUTE_TELEPORTER_FIELD
} DM1_V1_D1CDispatchRoutePc34;

typedef struct {
    int element;
    bool has_stairs_up_bit;
    int floor_ornament_ordinal;
    int front_wall_ornament_ordinal;
    int first_thing_index;
    bool has_alcove;
} DM1_V1_D1CDispatchInputPc34;

typedef struct {
    const char *draw_square_source_lines;
    const char *defs_square_aspect_lines;
    const char *defs_zone_lines;
    const char *stairs_up_source_lines;
    const char *stairs_down_source_lines;
    const char *floor_pit_source_lines;
    const char *wall_source_lines;
    const char *field_source_lines;
    const char *dispatch_source_lines;
    const char *non_overlap_note;
} DM1_V1_D1CDispatchEvidencePc34;

typedef struct {
    DM1_V1_D1CDispatchRoutePc34 route_taken;
    int native_bitmap_index;
    int zone_index;
    int cell_order_called;
    int view_square_index;
    int view_wall_index;
    int floor_ornament_ordinal;
    int front_wall_ornament_ordinal;
    int first_thing_index;
    bool cell_order_called_valid;
    bool used_f0104;
    bool used_f0113;
    bool used_f0107_alcove;
    bool used_f0115;
    bool used_f0108_floor_ornament;
    bool used_f0112_ceiling_pit;
    bool used_f0115_after_dispatch;
    bool wall_blit_called;
    bool used_f0111_door;
    bool unsupported_element;
    DM1_V1_D1CDispatchEvidencePc34 evidence;
} DM1_V1_D1CDispatchOutputPc34;

bool dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat_probe(
    const DM1_V1_D1CDispatchInputPc34 *input,
    DM1_V1_D1CDispatchOutputPc34 *output);

const DM1_V1_D1CDispatchEvidencePc34 *
dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D1C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H */
