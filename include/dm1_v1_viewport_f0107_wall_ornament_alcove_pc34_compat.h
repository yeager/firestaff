#ifndef FIRESTAFF_DM1_V1_VIEWPORT_F0107_WALL_ORNAMENT_ALCOVE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_F0107_WALL_ORNAMENT_ALCOVE_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

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

const char *dm1_v1_viewport_f0107_wall_ornament_alcove_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
