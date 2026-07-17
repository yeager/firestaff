#ifndef FIRESTAFF_DM2_V1_GDAT_WALL_TRIM_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_WALL_TRIM_RECEIPT_H

#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_gdat_wall_m11_command.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_viewport_renderer.h"

typedef struct {
    int valid, no_draw;
    uint8_t depth, graphicsset;
    uint16_t trim_word;
    uint16_t left, top, right, bottom;
    uint32_t wall_material_hash, scene_identity_hash, identity_hash;
    DM2_V1_ViewportSurfaceSnapshot surface;
} DM2_V1_GdatWallTrimReceipt;

typedef struct {
    int valid, no_draw;
    uint8_t view_square, normal_scale, source_flip;
    int8_t movement_offset_y;
    uint16_t rect_number;
    uint32_t trim_identity_hash, wall_material_hash, composition_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatWallTrimM11Receipt;

int dm2_v1_gdat_wall_trim_receipt_build(const DM2_V1_GdatSceneM11CommandPlan *,
    const DM2_V1_GdatWallM11CommandPlan *, const DM2_V1_ViewportState *,
    uint8_t depth, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    DM2_V1_GdatWallTrimReceipt *);
int dm2_v1_gdat_wall_trim_m11_receipt_build(const DM2_V1_GdatWallTrimReceipt *,
    const DM2_V1_GdatWallM11CommandPlan *, uint8_t command_index,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *,
    const DM2_V1_ViewportState *, DM2_V1_GdatWallTrimM11Receipt *);

#endif
