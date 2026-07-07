#include "dm1_v1_floor_ornament_pc34_compat.h"

#include "firestaff/dm1/v1/G0195_pc34_compat.h"
#include "firestaff/dm1/v1/G0206_pc34_compat.h"

enum {
    DM1_FLOOR_ORNAMENT_FOOTPRINTS_BASE_PC34 = 379,
    DM1_FLOOR_ORNAMENT_FOOTPRINTS_INDEX_PC34 = 15,
    DM1_FLOOR_ORNAMENT_BASE_PC34 = 385,
    DM1_FLOOR_ORNAMENT_VARIANTS_PC34 = 6
};

typedef struct DM1_FloorOrnamentSlotPc34 {
    int relForward;
    int relSide;
    int frameIndex;
    int increment;
    int renderDstX;
    int renderDstY;
    int renderWidth;
    int renderHeight;
} DM1_FloorOrnamentSlotPc34;

/* ReDMCSB DUNVIEW.C F0108 uses G0195 to select a G0206 coordinate set and
 * G0191 to select a native bitmap increment.  PC34 G0195 is all zero, so
 * every floor ornament visible slot uses G0206 set 0. */
static const DM1_FloorOrnamentSlotPc34 s_floorSlots[] = {
    {3, -1, 0, 0, 36,  66, 40,  6},
    {3,  0, 1, 1, 99,  66, 26,  6},
    {3,  1, 2, 0, 148, 66, 40,  6},
    {2, -1, 3, 2, 2,   77, 60, 11},
    {2,  0, 4, 3, 91,  77, 42, 11},
    {2,  1, 5, 2, 162, 77, 60, 11},
    {1, -1, 6, 4, 3,   94, 25, 21},
    {1,  0, 7, 5, 81,  93, 62, 23},
    {1,  1, 8, 4, 195, 94, 25, 21}
};

static const DM1_FloorOrnamentSlotPc34*
dm1_v1_floor_ornament_slot_pc34(int relForward, int relSide)
{
    int i;
    for (i = 0; i < (int)(sizeof(s_floorSlots) / sizeof(s_floorSlots[0])); ++i) {
        if (s_floorSlots[i].relForward == relForward &&
            s_floorSlots[i].relSide == relSide) {
            return &s_floorSlots[i];
        }
    }
    return 0;
}

static int
dm1_v1_floor_ornament_fill_source_plan_pc34(
    const DM1_FloorOrnamentSlotPc34* slot,
    DM1_FloorOrnamentRenderPlanPc34* outPlan)
{
    int coordSet;
    int x1;
    int x2;
    int y1;
    int y2;
    if (!slot || !outPlan) {
        return 0;
    }
    coordSet = dm1_v1_g0195_get_pc34(slot->frameIndex);
    x1 = dm1_v1_g0206_get_pc34(coordSet, slot->frameIndex, 0);
    x2 = dm1_v1_g0206_get_pc34(coordSet, slot->frameIndex, 1);
    y1 = dm1_v1_g0206_get_pc34(coordSet, slot->frameIndex, 2);
    y2 = dm1_v1_g0206_get_pc34(coordSet, slot->frameIndex, 3);
    if (coordSet < 0 || x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0) {
        return 0;
    }
    outPlan->relForward = slot->relForward;
    outPlan->relSide = slot->relSide;
    outPlan->increment = slot->increment;
    outPlan->flipHorizontal = slot->relSide > 0;
    outPlan->blit.graphicIndex = 0;
    outPlan->blit.srcX = 0;
    outPlan->blit.srcY = 0;
    outPlan->blit.dstX = x1;
    outPlan->blit.dstY = y1;
    outPlan->blit.width = x2 - x1 + 1;
    outPlan->blit.height = y2 - y1 + 1;
    return outPlan->blit.width > 0 && outPlan->blit.height > 0;
}

int dm1_v1_floor_ornament_source_zone_pc34(
    int relForward,
    int relSide,
    DM1_FloorOrnamentRenderPlanPc34* outPlan)
{
    return dm1_v1_floor_ornament_fill_source_plan_pc34(
        dm1_v1_floor_ornament_slot_pc34(relForward, relSide), outPlan);
}

int dm1_v1_floor_ornament_graphic_index_pc34(
    int globalOrnamentIndex,
    int increment)
{
    if (globalOrnamentIndex < 0 || increment < 0 ||
        increment >= DM1_FLOOR_ORNAMENT_VARIANTS_PC34) {
        return -1;
    }
    if (globalOrnamentIndex == DM1_FLOOR_ORNAMENT_FOOTPRINTS_INDEX_PC34) {
        return DM1_FLOOR_ORNAMENT_FOOTPRINTS_BASE_PC34 + increment;
    }
    return DM1_FLOOR_ORNAMENT_BASE_PC34 +
        globalOrnamentIndex * DM1_FLOOR_ORNAMENT_VARIANTS_PC34 + increment;
}

int dm1_v1_floor_ornament_render_plan_pc34(
    int relForward,
    int relSide,
    int globalOrnamentIndex,
    DM1_FloorOrnamentRenderPlanPc34* outPlan)
{
    const DM1_FloorOrnamentSlotPc34* slot =
        dm1_v1_floor_ornament_slot_pc34(relForward, relSide);
    int graphicIndex;
    if (!slot || !outPlan) {
        return 0;
    }
    if (!dm1_v1_floor_ornament_fill_source_plan_pc34(slot, outPlan)) {
        return 0;
    }
    outPlan->blit.dstX = slot->renderDstX;
    outPlan->blit.dstY = slot->renderDstY;
    outPlan->blit.width = slot->renderWidth;
    outPlan->blit.height = slot->renderHeight;
    graphicIndex = dm1_v1_floor_ornament_graphic_index_pc34(
        globalOrnamentIndex, outPlan->increment);
    if (graphicIndex < 0) {
        return 0;
    }
    outPlan->blit.graphicIndex = graphicIndex;
    return 1;
}

int dm1_v1_floor_ornament_render_plan_at_pc34(
    int planIndex,
    int globalOrnamentIndex,
    DM1_FloorOrnamentRenderPlanPc34* outPlan)
{
    const DM1_FloorOrnamentSlotPc34* slot;
    int graphicIndex;
    if (planIndex < 0 ||
        planIndex >= (int)(sizeof(s_floorSlots) / sizeof(s_floorSlots[0])) ||
        !outPlan) {
        return 0;
    }
    slot = &s_floorSlots[planIndex];
    if (!dm1_v1_floor_ornament_fill_source_plan_pc34(slot, outPlan)) {
        return 0;
    }
    outPlan->blit.dstX = slot->renderDstX;
    outPlan->blit.dstY = slot->renderDstY;
    outPlan->blit.width = slot->renderWidth;
    outPlan->blit.height = slot->renderHeight;
    graphicIndex = dm1_v1_floor_ornament_graphic_index_pc34(
        globalOrnamentIndex, outPlan->increment);
    if (graphicIndex < 0) {
        return 0;
    }
    outPlan->blit.graphicIndex = graphicIndex;
    return 1;
}

int dm1_v1_floor_ornament_plan_count_pc34(void)
{
    return (int)(sizeof(s_floorSlots) / sizeof(s_floorSlots[0]));
}
