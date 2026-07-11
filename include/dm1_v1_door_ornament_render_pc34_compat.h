#ifndef FIRESTAFF_DM1_V1_DOOR_ORNAMENT_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DOOR_ORNAMENT_RENDER_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_DoorOrnamentInfoPc34 {
    int globalIndex;
    int graphicIndex;
    int coordSet;
} DM1_DoorOrnamentInfoPc34;

typedef struct DM1_DoorOrnamentPanelPc34 {
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
} DM1_DoorOrnamentPanelPc34;

typedef struct DM1_DoorOrnamentRenderPlanPc34 {
    int graphicIndex;
    int srcX;
    int srcY;
    int srcW;
    int srcH;
    int dstX;
    int dstY;
    int dstW;
    int dstH;
    int transparentColor;
    int paletteMapValid;
    unsigned char paletteMap[16];
} DM1_DoorOrnamentRenderPlanPc34;

int dm1_v1_door_ornament_info_for_global_pc34(
    int globalIndex,
    DM1_DoorOrnamentInfoPc34* outInfo);

int dm1_v1_door_ornament_info_for_ordinal_pc34(
    int ornamentOrdinal,
    int cacheLoaded,
    const int localToGlobal[16],
    DM1_DoorOrnamentInfoPc34* outInfo);

int dm1_v1_door_ornament_render_plan_pc34(
    int globalIndex,
    int depthIndex,
    const DM1_DoorOrnamentPanelPc34* panel,
    int sourceWidth,
    int sourceHeight,
    DM1_DoorOrnamentRenderPlanPc34* outPlan);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DOOR_ORNAMENT_RENDER_PC34_COMPAT_H */
