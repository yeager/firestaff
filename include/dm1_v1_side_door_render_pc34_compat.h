#ifndef FIRESTAFF_DM1_V1_SIDE_DOOR_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SIDE_DOOR_RENDER_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_SideDoorBlitPc34 {
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
} DM1_SideDoorBlitPc34;

typedef struct DM1_SideDoorRenderPlanPc34 {
    int relForward;
    int relSide;
    int depthIndex;
    DM1_SideDoorBlitPc34 panel;
    DM1_SideDoorBlitPc34 frameA;
    DM1_SideDoorBlitPc34 frameB;
    int frameCount;
} DM1_SideDoorRenderPlanPc34;

int dm1_v1_side_door_render_plan_count_pc34(void);

int dm1_v1_side_door_render_plan_at_pc34(
    int planIndex,
    DM1_SideDoorRenderPlanPc34* outPlan);

int dm1_v1_side_door_render_plan_for_rel_pc34(
    int relForward,
    int relSide,
    DM1_SideDoorRenderPlanPc34* outPlan);

int dm1_v1_side_door_panel_blits_for_cell_pc34(
    const DM1_SideDoorRenderPlanPc34* plan,
    int doorState,
    int doorVertical,
    DM1_SideDoorBlitPc34 outBlits[2]);

int dm1_v1_side_door_panel_blits_for_draw_pc34(
    const DM1_SideDoorRenderPlanPc34* plan,
    int doorState,
    int doorVertical,
    DM1_SideDoorBlitPc34 outBlits[2]);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SIDE_DOOR_RENDER_PC34_COMPAT_H */
