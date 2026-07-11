#include "dm1_v1_door_ornament_render_pc34_compat.h"

enum {
    DM1_GFX_DOOR_ORNAMENT_BASE_PC34 = 441,
    DM1_DOOR_ORNAMENT_TRANSPARENT_COLOR_PC34 = 9
};

static const unsigned char s_doorOrnamentCoordSetByGlobal[12] = {
    0, 1, 1, 1, 0, 1, 2, 1, 1, 1, 1, 1
};

/* ReDMCSB: DUNVIEW.C G0207_aaauc_Graphic558_DoorOrnamentCoordinateSets.
 * Format per row: X1, X2, Y1, Y2, ByteWidth, Height.
 * View index 0=D3LCR, 1=D2LCR, 2=D1LCR. */
static const unsigned char s_doorOrnamentCoordSets[4][3][6] = {
    {{17,31, 8,17, 8,10}, {22,42,11,23,16,13}, {32,63,13,31,16,19}},
    {{ 0,47, 0,40,24,41}, { 0,63, 0,60,32,61}, { 0,95, 0,87,48,88}},
    {{17,31,15,24, 8,10}, {22,42,22,34,16,13}, {32,63,31,49,16,19}},
    {{23,35,31,39, 8, 9}, {30,48,41,52,16,12}, {44,75,61,79,16,19}}
};

static const unsigned char s_doorOrnamentPaletteD3[16] = {
    0, 12, 1, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 2, 0, 13
};

static const unsigned char s_doorOrnamentPaletteD2[16] = {
    0, 1, 2, 3, 4, 3, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15
};

static void copy_palette(unsigned char dst[16], const unsigned char src[16])
{
    int i;
    for (i = 0; i < 16; ++i) {
        dst[i] = src[i];
    }
}

int dm1_v1_door_ornament_info_for_global_pc34(
    int globalIndex,
    DM1_DoorOrnamentInfoPc34* outInfo)
{
    if (!outInfo || globalIndex < 0) {
        return 0;
    }
    outInfo->globalIndex = globalIndex;
    outInfo->graphicIndex = DM1_GFX_DOOR_ORNAMENT_BASE_PC34 + globalIndex;
    outInfo->coordSet = globalIndex < 12 ?
        s_doorOrnamentCoordSetByGlobal[globalIndex] : 1;
    return 1;
}

int dm1_v1_door_ornament_info_for_ordinal_pc34(
    int ornamentOrdinal,
    int cacheLoaded,
    const int localToGlobal[16],
    DM1_DoorOrnamentInfoPc34* outInfo)
{
    int localIndex;
    int globalIndex;
    if (ornamentOrdinal <= 0 || !outInfo) {
        return 0;
    }
    localIndex = ornamentOrdinal - 1;
    if (localIndex < 0 || localIndex >= 16) {
        return 0;
    }
    /* ReDMCSB F0096 map graphics setup resolves map-local door ornament
     * ordinals through the current map metadata.  If no metadata is loaded,
     * Firestaff keeps the original identity fallback used by old loose
     * fixtures. */
    globalIndex = (cacheLoaded && localToGlobal) ? localToGlobal[localIndex] : localIndex;
    return dm1_v1_door_ornament_info_for_global_pc34(globalIndex, outInfo);
}

int dm1_v1_door_ornament_render_plan_pc34(
    int globalIndex,
    int depthIndex,
    const DM1_DoorOrnamentPanelPc34* panel,
    int sourceWidth,
    int sourceHeight,
    DM1_DoorOrnamentRenderPlanPc34* outPlan)
{
    DM1_DoorOrnamentInfoPc34 info;
    const unsigned char* coord;
    int viewIndex;
    int ornW;
    int ornH;
    int relX;
    int relY;
    int visibleTop;
    int visibleBottom;
    int cropTop;
    int cropH;
    int srcY;
    int srcH;

    if (!panel || !outPlan || sourceWidth <= 0 || sourceHeight <= 0 ||
        depthIndex < 0 || depthIndex > 2 ||
        !dm1_v1_door_ornament_info_for_global_pc34(globalIndex, &info)) {
        return 0;
    }
    viewIndex = depthIndex == 0 ? 2 : (depthIndex == 1 ? 1 : 0);
    coord = s_doorOrnamentCoordSets[info.coordSet][viewIndex];
    relX = coord[0];
    relY = coord[2];
    ornW = coord[1] - coord[0] + 1;
    ornH = coord[3] - coord[2] + 1;
    if (ornW <= 0 || ornH <= 0 || panel->height <= 0) {
        return 0;
    }

    visibleTop = relY;
    visibleBottom = relY + ornH;
    if (visibleTop < panel->srcY) {
        visibleTop = panel->srcY;
    }
    if (visibleBottom > panel->srcY + panel->height) {
        visibleBottom = panel->srcY + panel->height;
    }
    if (visibleBottom <= visibleTop) {
        return 0;
    }

    cropTop = visibleTop - relY;
    cropH = visibleBottom - visibleTop;
    srcY = (cropTop * sourceHeight) / ornH;
    srcH = ((cropTop + cropH) * sourceHeight) / ornH - srcY;
    if (srcH <= 0) {
        srcH = 1;
    }

    outPlan->graphicIndex = info.graphicIndex;
    outPlan->srcX = 0;
    outPlan->srcY = srcY;
    outPlan->srcW = sourceWidth;
    outPlan->srcH = srcH;
    outPlan->dstX = panel->dstX + relX;
    outPlan->dstY = panel->dstY + visibleTop - panel->srcY;
    outPlan->dstW = ornW;
    outPlan->dstH = cropH;
    outPlan->transparentColor = DM1_DOOR_ORNAMENT_TRANSPARENT_COLOR_PC34;
    outPlan->paletteMapValid = 0;
    if (depthIndex == 2) {
        copy_palette(outPlan->paletteMap, s_doorOrnamentPaletteD3);
        outPlan->paletteMapValid = 1;
    } else if (depthIndex == 1) {
        copy_palette(outPlan->paletteMap, s_doorOrnamentPaletteD2);
        outPlan->paletteMapValid = 1;
    }
    return 1;
}
