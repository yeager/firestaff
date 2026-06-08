#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_D2C_DISPATCH_PC34_ELEMENT_WALL = 0,
    DM1_V1_D2C_DISPATCH_PC34_ELEMENT_CORRIDOR = 1,
    DM1_V1_D2C_DISPATCH_PC34_ELEMENT_PIT = 2,
    DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS = 3,
    DM1_V1_D2C_DISPATCH_PC34_ELEMENT_TELEPORTER = 5,
    DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS_SIDE = 18,
    DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT = 19,
    DM1_V1_D2C_DISPATCH_PC34_MASK_PIT_INVISIBLE = 0x0004,
    DM1_V1_D2C_DISPATCH_PC34_MASK_PIT_OPEN = 0x0008,
    DM1_V1_D2C_DISPATCH_PC34_MASK_STAIRS_UP = 0x0004,
    DM1_V1_D2C_DISPATCH_PC34_MASK_STAIRS_NS = 0x0008,
    DM1_V1_D2C_DISPATCH_PC34_STAIRS_UP_SLOT_D2C = 3,
    DM1_V1_D2C_DISPATCH_PC34_STAIRS_DOWN_SLOT_D2C = 10,
    DM1_V1_D2C_DISPATCH_PC34_FLOOR_PIT_D2C_GRAPHIC = 53,
    DM1_V1_D2C_DISPATCH_PC34_INVISIBLE_FLOOR_PIT_D2C_GRAPHIC = 59,
    DM1_V1_D2C_DISPATCH_PC34_CEILING_PIT_D2C_GRAPHIC = 65,
    DM1_V1_D2C_DISPATCH_PC34_VIEW_SQUARE_D2C = 6,
    DM1_V1_D2C_DISPATCH_PC34_VIEW_FLOOR_D2C = 6,
    DM1_V1_D2C_DISPATCH_PC34_ZONE_STAIRS_UP_D2C = 806,
    DM1_V1_D2C_DISPATCH_PC34_ZONE_STAIRS_DOWN_D2C = 819,
    DM1_V1_D2C_DISPATCH_PC34_ZONE_FLOOR_PIT_D2C = 856,
    DM1_V1_D2C_DISPATCH_PC34_ZONE_CEILING_PIT_D2C = 865,
    DM1_V1_D2C_DISPATCH_PC34_ZONE_FIELD_D2C = 709,
    DM1_V1_D2C_DISPATCH_PC34_CELL_ORDER_OPEN = 0x3421,
    DM1_V1_D2C_DISPATCH_PC34_COLOR_TRANSPARENT = 10,
    DM1_V1_D2C_DISPATCH_PC34_THING_END_OF_LIST = 0xfffe
};

typedef enum {
    DM1_V1_D2C_DISPATCH_PC34_ROUTE_UNSUPPORTED = 0,
    DM1_V1_D2C_DISPATCH_PC34_ROUTE_STAIRS_UP_FRONT,
    DM1_V1_D2C_DISPATCH_PC34_ROUTE_STAIRS_DOWN_FRONT,
    DM1_V1_D2C_DISPATCH_PC34_ROUTE_OPEN_PIT,
    DM1_V1_D2C_DISPATCH_PC34_ROUTE_CORRIDOR_TAIL,
    DM1_V1_D2C_DISPATCH_PC34_ROUTE_TELEPORTER_FIELD,
    DM1_V1_D2C_DISPATCH_PC34_ROUTE_WALL_RETURN
} DM1_V1_D2CDispatchRoutePc34;

typedef struct {
    int element;
    bool stairs_up;
    bool pit_or_teleporter_visible;
    int floor_ornament_ordinal;
    int first_thing;
} DM1_V1_D2CDispatchInputPc34;

typedef struct {
    DM1_V1_D2CDispatchRoutePc34 route_taken;
    int native_bitmap_index;
    int zone_index;
    int ceiling_pit_graphic;
    int ceiling_pit_zone;
    int view_square_index;
    int view_floor_index;
    int field_zone_index;
    int floor_ornament_ordinal;
    int first_thing;
    int cell_order_called;
    int pit_bitmap_order;
    int floor_ornament_order;
    int ceiling_pit_order;
    int thing_pass_order;
    int field_order;
    bool used_f0104;
    bool used_f0105;
    bool used_f0108_floor_ornament;
    bool used_f0112_ceiling_pit;
    bool used_f0113_field;
    bool used_f0115;
    bool bug0_64_floor_ornament_after_open_pit;
    bool wall_returned_before_tail;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D2CDispatchOutputPc34;

typedef struct {
    uint8_t raw_square;
    int direction;
    int floor_ornament_ordinal;
    int first_thing_after_metadata;
} DM1_V1_D2CMetadataInputPc34;

typedef struct {
    int element;
    bool stairs_up;
    bool pit_or_teleporter_visible;
    bool footprints_allowed;
    int floor_ornament_ordinal;
    int first_thing;
} DM1_V1_D2CMetadataOutputPc34;

typedef struct {
    int base_map_x;
    int base_map_y;
    int relative_depth;
    int relative_lateral;
    int resolved_map_x;
    int resolved_map_y;
    int view_square_index;
    int field_zone_index;
} DM1_V1_D2CCenterGeometryPc34;

typedef struct {
    int relative_depth;
    int relative_lateral;
    int draw_order;
    const char *function_name;
} DM1_V1_D2CFollowUpWritePc34;

typedef struct {
    const uint8_t *source;
    size_t source_len;
    uint8_t *destination;
    size_t destination_len;
    size_t width;
    size_t height;
    size_t destination_stride;
    int native_bitmap_index;
    int zone_index;
} DM1_V1_D2CF0105BlitInputPc34;

typedef struct {
    bool ok;
    bool used_f0105;
    bool copied_with_horizontal_flip;
    int native_bitmap_index;
    int zone_index;
    int transparent_color;
    size_t writes;
    size_t transparent_skips;
    uint8_t first_destination_byte;
    uint8_t last_destination_byte;
} DM1_V1_D2CF0105BlitOutputPc34;

typedef struct {
    const char *f0121_body_lines;
    const char *f0104_lines;
    const char *f0105_lines;
    const char *f0115_lines;
    const char *f0128_lines;
    const char *dungeon_metadata_lines;
    const char *defs_lines;
    const char *contract_note;
} DM1_V1_D2CDispatchEvidencePc34;

bool dm1_v1_viewport_d2c_stairs_pit_dispatch_probe_pc34(
    const DM1_V1_D2CDispatchInputPc34 *input,
    DM1_V1_D2CDispatchOutputPc34 *output);

bool dm1_v1_viewport_d2c_stairs_pit_dispatch_metadata_pc34(
    const DM1_V1_D2CMetadataInputPc34 *input,
    DM1_V1_D2CMetadataOutputPc34 *output);

bool dm1_v1_viewport_d2c_stairs_pit_dispatch_center_geometry_pc34(
    int map_x,
    int map_y,
    DM1_V1_D2CCenterGeometryPc34 *output);

const DM1_V1_D2CFollowUpWritePc34 *
dm1_v1_viewport_d2c_stairs_pit_dispatch_followups_pc34(size_t *count);

bool dm1_v1_viewport_d2c_stairs_pit_dispatch_f0105_blit_pc34(
    const DM1_V1_D2CF0105BlitInputPc34 *input,
    DM1_V1_D2CF0105BlitOutputPc34 *output);

const DM1_V1_D2CDispatchEvidencePc34 *
dm1_v1_viewport_d2c_stairs_pit_dispatch_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2C_STAIRS_PIT_DISPATCH_PC34_COMPAT_H */
