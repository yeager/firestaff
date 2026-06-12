#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2C_CENTER_WALL_COMPOSITION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2C_CENTER_WALL_COMPOSITION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D2C_CENTER_COMPOSITION_PC34_MAX_STEPS 8
#define DM1_V1_D2C_CENTER_COMPOSITION_PC34_C10_COLOR_FLESH 10

typedef enum {
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_WALL_PLAIN = 0,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_WALL_ALCOVE,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_DOOR_FRONT,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_CORRIDOR,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_TELEPORTER
} DM1_V1_D2CCenterCompositionRoutePc34;

typedef enum {
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_DISPATCH_D2C = 0,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_WALL_BODY,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0107_FRONT_WALL_ORNAMENT,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0108_FLOOR_ORNAMENT,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0111_DOOR_BODY,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_REAR,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_FRONT,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_OPEN_OR_ALCOVE,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0112_CEILING_PIT,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0113_CENTER_FIELD,
    DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_RETURN
} DM1_V1_D2CCenterCompositionStepPc34;

typedef struct {
    DM1_V1_D2CCenterCompositionStepPc34 step;
    uint16_t cell_order;
    int view_square_index;
    int zone;
    int transparent_color;
    const char *anchor;
} DM1_V1_D2CCenterCompositionOpPc34;

typedef struct {
    DM1_V1_D2CCenterCompositionRoutePc34 route;
    size_t step_count;
    DM1_V1_D2CCenterCompositionOpPc34 steps[DM1_V1_D2C_CENTER_COMPOSITION_PC34_MAX_STEPS];
    bool calls_f0107_front_wall_ornament;
    bool calls_f0108_floor_ornament;
    bool calls_f0111_door_body;
    bool calls_f0112_ceiling_pit;
    bool calls_f0113_center_field;
    bool calls_f0115_rear;
    bool calls_f0115_front;
    bool calls_f0115_open_or_alcove;
    bool wall_case_returns_before_f0108;
    bool field_uses_c10_transparency;
    int view_square_index;
    int view_depth;
    int view_lane;
    int field_aspect_index;
    int wall_zone_pc34;
    const char *source_evidence;
} DM1_V1_D2CCenterCompositionTracePc34;

typedef struct {
    uint16_t rear_cell_order;
    uint16_t front_cell_order;
    int rear_cell_count;
    int front_cell_count;
    int rear_cells[2];
    int front_cells[2];
    uint8_t initial_pixel;
    uint8_t after_floor_ornament;
    uint8_t after_rear_f0115;
    uint8_t after_f0111_door;
    uint8_t after_front_f0115;
    bool floor_transparent;
    bool rear_transparent;
    bool door_transparent;
    bool front_transparent;
} DM1_V1_D2CDoorFrontPixelTracePc34;

const char *dm1_v1_viewport_d2c_center_wall_composition_source_evidence_pc34(void);

DM1_V1_D2CCenterCompositionTracePc34
dm1_v1_viewport_d2c_center_wall_composition_trace_pc34(
    DM1_V1_D2CCenterCompositionRoutePc34 route);

int dm1_v1_viewport_d2c_center_wall_composition_apply_c10_layer_pc34(
    const uint8_t *source,
    uint8_t *destination,
    size_t count,
    uint8_t transparent_color);

int dm1_v1_viewport_d2c_door_front_compose_pixel_pc34(
    uint8_t initial_pixel,
    uint8_t floor_ornament_pixel,
    uint8_t rear_f0115_pixel,
    uint8_t f0111_door_pixel,
    uint8_t front_f0115_pixel,
    uint8_t transparent_color,
    DM1_V1_D2CDoorFrontPixelTracePc34 *out_trace);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2C_CENTER_WALL_COMPOSITION_PC34_COMPAT_H */
