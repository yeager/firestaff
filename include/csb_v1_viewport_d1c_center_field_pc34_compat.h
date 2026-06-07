#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1C_CENTER_FIELD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1C_CENTER_FIELD_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_INVALID = 0,
    CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_WALL_FRONT_ALCOVE,
    CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_WALL_FRONT_NO_ALCOVE,
    CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_CORRIDOR_OPEN,
    CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_TELEPORTER_FIELD,
    CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_DOOR_FRONT
} CSB_V1_D1CCenterFieldRoutePc34;

typedef struct {
    int view_square_index;
    int lane;
    int depth;
    int field_aspect;
    bool has_alcove;
    int element;
} CSB_V1_D1CCenterFieldInputPc34;

typedef struct {
    const char *array_anchor;
    const char *def_anchor;
    const char *dispatch_anchor;
    const char *body_anchor;
    const char *wall_anchor;
    const char *alcove_anchor;
    const char *door_anchor;
    const char *field_anchor;
    const char *zone_anchor;
    const char *cell_order_anchor;
    const char *source_note;
} CSB_V1_D1CCenterFieldEvidencePc34;

typedef struct {
    CSB_V1_D1CCenterFieldRoutePc34 route_taken;
    int wall_zone_index;
    int field_zone_index;
    uint16_t cell_order;
    bool used_f0100;
    bool used_f0107_alcove;
    bool used_f0113_field;
    bool used_f0115_thing_pass;
    CSB_V1_D1CCenterFieldEvidencePc34 evidence;
} CSB_V1_D1CCenterFieldOutputPc34;

enum {
    CSB_V1_D1C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX = 3,
    CSB_V1_D1C_CENTER_FIELD_PC34_LANE = 0,
    CSB_V1_D1C_CENTER_FIELD_PC34_DEPTH = 1,
    CSB_V1_D1C_CENTER_FIELD_PC34_FIELD_ASPECT = 10,
    CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_WALL = 0,
    CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR = 1,
    CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_TELEPORTER = 5,
    CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_DOOR_FRONT = 17,
    CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA508_ZONE_WALL_D1C = 710,
    CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_WALL_D1C = 712,
    CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_WALL_D1R = 714,
    CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_STAIRS_UP_FRONT_D1C = 809,
    CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_STAIRS_UP_FRONT_D1R = 810,
    CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_STAIRS_DOWN_FRONT_D1C = 822,
    CSB_V1_D1C_CENTER_FIELD_PC34_VIEW_WALL_D1C_FRONT = 14,
    CSB_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_ALCOVE = 0x0000,
    CSB_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT = 0x0218
};

CSB_V1_D1CCenterFieldOutputPc34
csb_v1_viewport_d1c_center_field_pc34_compat_probe(
    CSB_V1_D1CCenterFieldInputPc34 input);

const CSB_V1_D1CCenterFieldEvidencePc34 *
csb_v1_viewport_d1c_center_field_pc34_compat_evidence(void);

const char *
csb_v1_viewport_d1c_center_field_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_D1C_CENTER_FIELD_PC34_COMPAT_H */
