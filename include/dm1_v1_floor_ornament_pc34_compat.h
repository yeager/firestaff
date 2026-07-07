#ifndef FIRESTAFF_DM1_V1_FLOOR_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_FLOOR_ORNAMENT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_FloorOrnamentBlitPc34 {
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
} DM1_FloorOrnamentBlitPc34;

typedef struct DM1_FloorOrnamentRenderPlanPc34 {
    int relForward;
    int relSide;
    int increment;
    int flipHorizontal;
    DM1_FloorOrnamentBlitPc34 blit;
} DM1_FloorOrnamentRenderPlanPc34;

int dm1_v1_floor_ornament_source_zone_pc34(
    int relForward,
    int relSide,
    DM1_FloorOrnamentRenderPlanPc34* outPlan);

int dm1_v1_floor_ornament_render_plan_pc34(
    int relForward,
    int relSide,
    int globalOrnamentIndex,
    DM1_FloorOrnamentRenderPlanPc34* outPlan);

int dm1_v1_floor_ornament_render_plan_at_pc34(
    int planIndex,
    int globalOrnamentIndex,
    DM1_FloorOrnamentRenderPlanPc34* outPlan);

int dm1_v1_floor_ornament_graphic_index_pc34(
    int globalOrnamentIndex,
    int increment);

int dm1_v1_floor_ornament_plan_count_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_FLOOR_ORNAMENT_PC34_COMPAT_H */
