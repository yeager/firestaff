#ifndef FIRESTAFF_DM1_V1_VIEWPORT_F0107_WALL_ORNAMENT_ALCOVE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_F0107_WALL_ORNAMENT_ALCOVE_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34 = 4,
    DM1_V1_F0107_SLOT_M552_FRONT_WALL_ORNAMENT_PC34 = 5,
    DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34 = 6,
    DM1_V1_F0107_SLOT_M554_MIRROR_FRONT_WALL_PC34 = 3
} DM1_V1_F0107WallOrnamentSlotPc34;

typedef enum {
    DM1_V1_F0107_WALL_CELL_C00_D3L2_RIGHT_PC34 = 0,
    DM1_V1_F0107_WALL_CELL_C01_D3R2_LEFT_PC34 = 1,
    DM1_V1_F0107_WALL_CELL_M575_D3L_RIGHT_PC34 = 2,
    DM1_V1_F0107_WALL_CELL_M576_D3R_LEFT_PC34 = 3,
    DM1_V1_F0107_WALL_CELL_M580_D2L_RIGHT_PC34 = 7,
    DM1_V1_F0107_WALL_CELL_M581_D2R_LEFT_PC34 = 8,
    DM1_V1_F0107_WALL_CELL_M582_D2L_FRONT_PC34 = 9,
    DM1_V1_F0107_WALL_CELL_M584_D2R_FRONT_PC34 = 11,
    DM1_V1_F0107_WALL_CELL_M585_D1L_RIGHT_PC34 = 12,
    DM1_V1_F0107_WALL_CELL_M586_D1R_LEFT_PC34 = 13,
    DM1_V1_F0107_WALL_CELL_M587_D1C_FRONT_PC34 = 14
} DM1_V1_F0107WallCellCodePc34;

typedef enum {
    DM1_V1_F0107_COORDINATE_UNKNOWN_PC34 = -1,
    DM1_V1_F0107_COORDINATE_D3L2_RIGHT_PC34 = 0,
    DM1_V1_F0107_COORDINATE_D3R2_LEFT_PC34 = 1,
    DM1_V1_F0107_COORDINATE_D3L_RIGHT_PC34 = 2,
    DM1_V1_F0107_COORDINATE_D3R_LEFT_PC34 = 3,
    DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34 = 4,
    DM1_V1_F0107_COORDINATE_D2R_LEFT_PC34 = 5,
    DM1_V1_F0107_COORDINATE_D1L_RIGHT_PC34 = 6,
    DM1_V1_F0107_COORDINATE_D1R_LEFT_PC34 = 7,
    DM1_V1_F0107_COORDINATE_BACK_WALL_FRONT_PC34 = 8
} DM1_V1_F0107CoordinateSetPc34;

typedef enum {
    DM1_V1_F0107_WALLSET_C02_D1R_PC34 = 2,
    DM1_V1_F0107_WALLSET_C03_D1L_PC34 = 3,
    DM1_V1_F0107_WALLSET_C07_D2R_PC34 = 7,
    DM1_V1_F0107_WALLSET_C08_D2L_PC34 = 8,
    DM1_V1_F0107_WALLSET_C10_D3R2_PC34 = 10,
    DM1_V1_F0107_WALLSET_C11_D3L2_PC34 = 11,
    DM1_V1_F0107_WALLSET_C12_D3R_PC34 = 12,
    DM1_V1_F0107_WALLSET_C13_D3L_PC34 = 13
} DM1_V1_F0107WallSetPc34;

typedef struct {
    int ordinal_slot;
    int wall_cell_code;
    int coordinate_set;
    int wall_set;
    bool returns_alcove;
    const char *redmcsb_anchor;
    const char *claim;
} DM1_V1_F0107WallOrnamentAlcoveCasePc34;

#define DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_WIDTH_PC34 8
#define DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_HEIGHT_PC34 4
#define DM1_V1_F0107_WALL_ORNAMENT_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_F0107_WALL_ORNAMENT_C10_COLOR_FLESH_PC34 10

typedef struct {
    size_t case_index;
    int row;
    int viewport_x;
    uint8_t transparent_color;
} DM1_V1_F0107WallOrnamentPixelInputPc34;

typedef struct {
    const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *source_case;
    bool route_valid;
    bool returns_alcove;
    bool draw_attempted;
    bool in_clip;
    bool writes_pixel;
    bool transparent_skip;
    bool no_write_metadata;
    int row;
    int viewport_x;
    int source_x;
    int source_y;
    size_t source_offset;
    size_t viewport_offset;
    uint8_t pixel_before;
    uint8_t source_pixel;
    uint8_t pixel_after;
} DM1_V1_F0107WallOrnamentPixelResultPc34;

bool dm1_v1_viewport_f0107_wall_ornament_alcove_decide(
    int ordinal_slot,
    int wall_cell_code,
    int coordinate_set,
    int wall_set);

int dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(int wall_cell_code);

const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *
dm1_v1_viewport_f0107_wall_ornament_alcove_cases_pc34(size_t *count);

const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *
dm1_v1_viewport_f0107_wall_ornament_alcove_case_at_pc34(size_t index);

bool dm1_v1_viewport_f0107_wall_ornament_alcove_decide_case_pc34(
    const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *entry);

bool dm1_v1_viewport_f0107_wall_ornament_apply_pixel_pc34(
    const DM1_V1_F0107WallOrnamentPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_F0107WallOrnamentPixelResultPc34 *out);

uint8_t dm1_v1_viewport_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_f0107_wall_ornament_alcove_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
