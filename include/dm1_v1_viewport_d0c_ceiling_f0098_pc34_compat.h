#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_CEILING_F0098_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_CEILING_F0098_PC34_COMPAT_H

/*
 * DM1 V1 D0C ceiling + F0098 floor row-ownership contract.
 *
 * Source-lock anchors:
 * - ReDMCSB DUNVIEW.C F0098:2962-3002 owns the viewport floor/ceiling
 *   refresh rows before square drawing.
 * - ReDMCSB DUNVIEW.C F0112:4341-4470 draws ceiling pits through the
 *   floor-pit/stairs blitters, preserving C10 transparent pixels.
 *
 * Contract only: this slice proves row ownership and C10 transparency for
 * synthetic pixels. It does not claim real-asset floor/ceiling bitmap parity.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D0C_CEILING_F0098_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D0C_CEILING_F0098_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D0C_CEILING_F0098_CEILING_FIRST_ROW_PC34 0
#define DM1_V1_D0C_CEILING_F0098_CEILING_LAST_ROW_PC34 28
#define DM1_V1_D0C_CEILING_F0098_GAP_FIRST_ROW_PC34 29
#define DM1_V1_D0C_CEILING_F0098_GAP_LAST_ROW_PC34 65
#define DM1_V1_D0C_CEILING_F0098_FLOOR_FIRST_ROW_PC34 66
#define DM1_V1_D0C_CEILING_F0098_FLOOR_LAST_ROW_PC34 135
#define DM1_V1_D0C_CEILING_F0098_DEPTH_PC34 0
#define DM1_V1_D0C_CEILING_F0098_LANE_PC34 0
#define DM1_V1_D0C_CEILING_F0098_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0C_CEILING_F0098_ZONE_VIEWPORT_CEILING_PC34 700
#define DM1_V1_D0C_CEILING_F0098_ZONE_VIEWPORT_FLOOR_PC34 701
#define DM1_V1_D0C_CEILING_F0098_ZONE_WALL_D3L2_PC34 702
#define DM1_V1_D0C_CEILING_F0098_ZONE_WALL_D3R2_PC34 703

typedef enum {
    DM1_V1_D0C_CEILING_F0098_ROW_OUTSIDE_PC34 = 0,
    DM1_V1_D0C_CEILING_F0098_ROW_CEILING_PC34 = 1,
    DM1_V1_D0C_CEILING_F0098_ROW_GAP_PC34 = 2,
    DM1_V1_D0C_CEILING_F0098_ROW_FLOOR_PC34 = 3
} DM1_V1_D0CCeilingF0098RowOwnerPc34;

typedef enum {
    DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_BASE_PC34 = 0,
    DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_BASE_PC34 = 1,
    DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_PIT_F0112_PC34 = 2,
    DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_PIT_D0C_PC34 = 3,
    DM1_V1_D0C_CEILING_F0098_PIXEL_F0108_FLOOR_ORNAMENT_PC34 = 4,
    DM1_V1_D0C_CEILING_F0098_PIXEL_F0107_WALL_ORNAMENT_PC34 = 5
} DM1_V1_D0CCeilingF0098PixelSourcePc34;

typedef struct {
    int row;
    DM1_V1_D0CCeilingF0098PixelSourcePc34 source;
    uint8_t destination_pixel;
    uint8_t source_pixel;
} DM1_V1_D0CCeilingF0098PixelProbePc34;

typedef struct {
    DM1_V1_D0CCeilingF0098RowOwnerPc34 row_owner;
    bool valid_row;
    bool row_owned_by_f0098_d0c;
    bool writes_pixel;
    bool transparent_skip;
    uint8_t pixel_after;
    bool calls_f0098;
    bool calls_f0112;
    bool calls_f0104_floor_pit;
    bool calls_f0108_floor_ornament;
    bool calls_f0107_wall_ornament;
} DM1_V1_D0CCeilingF0098PixelResultPc34;

typedef struct {
    int viewport_width;
    int viewport_height;
    int ceiling_first_row;
    int ceiling_last_row;
    int gap_first_row;
    int gap_last_row;
    int floor_first_row;
    int floor_last_row;
    int depth;
    int lane;
    int transparent_color;
    int viewport_ceiling_zone;
    int viewport_floor_zone;
    int d3l2_wall_zone_anchor;
    int d3r2_wall_zone_anchor;
    bool contract_only;
    bool real_asset_bitmap_parity;
    bool f0098_owns_floor_rows;
    bool f0098_owns_ceiling_rows;
    bool c10_transparency;
    bool d0c_uses_f0108_floor_ornament_path;
    bool d0c_uses_f0107_wall_ornament_path;
    const char *source_lines;
} DM1_V1_D0CCeilingF0098SpecPc34;

const DM1_V1_D0CCeilingF0098SpecPc34 *
dm1_v1_viewport_d0c_ceiling_f0098_spec_pc34(void);

bool dm1_v1_viewport_d0c_ceiling_f0098_apply_pixel_pc34(
    const DM1_V1_D0CCeilingF0098PixelProbePc34 *probe,
    DM1_V1_D0CCeilingF0098PixelResultPc34 *out);

const char *dm1_v1_viewport_d0c_ceiling_f0098_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0C_CEILING_F0098_PC34_COMPAT_H */
