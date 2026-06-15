#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1C_CENTER_FIELD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1C_CENTER_FIELD_PC34_COMPAT_H

#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_D1C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX = 3,
    DM1_V1_D1C_CENTER_FIELD_PC34_DEPTH = 1,
    DM1_V1_D1C_CENTER_FIELD_PC34_LATERAL = 0,
    DM1_V1_D1C_CENTER_FIELD_PC34_FIELD_ASPECT = 10,
    DM1_V1_D1C_CENTER_FIELD_PC34_FIELD_ZONE = 712,
    DM1_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_OPEN = 0x3421,
    DM1_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_OPEN_DOOR = 0x3421,
    DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_WALL = 0,
    DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR = 1,
    DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_PIT = 2,
    DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_STAIRS = 3,
    DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_DOOR = 4,
    DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_FAKEWALL = 6
};

typedef enum {
    DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_INVALID = 0,
    DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_OPEN_FIELD,
    DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_TELEPORTER_FIELD,
    DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_PIT_FIELD,
    DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_STAIRS_FIELD,
    DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_OPEN_DOOR_FIELD,
    DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_WALL_BLOCKED,
    DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_CLOSED_DOOR_BLOCKED
} DM1_V1_D1CCenterFieldRoutePc34;

typedef struct {
    const uint8_t *cells;
    int width;
    int height;
    bool has_item;
    bool has_creature;
    bool has_projectile;
    bool has_explosion;
    bool door_is_open;
} DM1_V1_D1CCenterFieldDungeonPc34;

typedef struct {
    int16_t map_x;
    int16_t map_y;
    int16_t direction;
} DM1_V1_D1CCenterFieldPartyPc34;

typedef struct {
    uint8_t *pixels;
    int width;
    int height;
    int stride;
    int zone;
} DM1_V1_D1CCenterFieldTargetPc34;

typedef struct {
    DM1_V1_D1CCenterFieldRoutePc34 route;
    int16_t target_map_x;
    int16_t target_map_y;
    uint8_t square;
    int square_type;
    uint16_t cell_order;
    int view_square_index;
    int field_aspect;
    int field_zone;
    bool called_relative_square;
    bool called_relative_square_type;
    bool called_get_square;
    bool called_floor_ceiling_helper;
    bool called_d0c_reference_helper;
    bool called_f0115_thing_pass;
    bool called_f0113_field;
    bool called_f0108_floor_ornament;
    bool called_f0112_ceiling_pit;
    bool called_f0104_pit_or_stairs;
    bool drew_item;
    bool drew_creature;
    bool drew_projectile;
    bool drew_explosion;
    bool drew_wall_bitmap;
    bool drew_wall_ornament;
    bool drew_door_bitmap;
    const char *source_evidence;
} DM1_V1_D1CCenterFieldRenderPc34;

DM1_V1_D1CCenterFieldRenderPc34
dm1_v1_viewport_d1c_center_field_pc34_compat_render_square(
    const DM1_V1_D1CCenterFieldDungeonPc34 *dungeon,
    const DM1_V1_D1CCenterFieldPartyPc34 *party,
    DM1_Viewport3DState *viewport,
    DM1_V1_D1CCenterFieldTargetPc34 *target);

#ifdef __cplusplus
}
#endif

#endif
