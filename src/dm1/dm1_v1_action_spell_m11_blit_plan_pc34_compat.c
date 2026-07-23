#include "dm1_v1_action_spell_m11_blit_plan_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static void dm1_v1_action_spell_m11_blit_set_pc34(
    DM1_V1_ActionSpellM11BlitPc34 *outBlit, int graphicId, int zoneId,
    int zoneCount, int sourceX, int sourceY, int sourceW, int sourceH,
    int destinationX, int destinationY)
{
    outBlit->graphicId = graphicId;
    outBlit->zoneId = zoneId;
    outBlit->zoneCount = zoneCount;
    outBlit->sourceX = sourceX;
    outBlit->sourceY = sourceY;
    outBlit->sourceW = sourceW;
    outBlit->sourceH = sourceH;
    outBlit->destinationX = destinationX;
    outBlit->destinationY = destinationY;
}

int dm1_v1_action_spell_m11_blit_plan_build_pc34(
    int presentationKind, int actionMenuRowCount,
    DM1_V1_ActionSpellM11BlitPlanPc34 *outPlan)
{
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->presentationKind = presentationKind;

    if (presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34) {
        DM1_V1_ActionAreaRectPc34 graphic;
        if (actionMenuRowCount < 1 || actionMenuRowCount > 3) return 0;
        graphic = dm1_v1_action_menu_graphic_rect_pc34(actionMenuRowCount);
        outPlan->clearX = dm1_v1_box_action_area_x_pc34();
        outPlan->clearY = dm1_v1_box_action_area_y_pc34();
        outPlan->clearW = dm1_v1_box_action_area_w_pc34();
        outPlan->clearH = dm1_v1_box_action_area_h_pc34();
        outPlan->blitCount = 1;
        dm1_v1_action_spell_m11_blit_set_pc34(&outPlan->blits[0],
            DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34,
            dm1_v1_action_menu_graphic_zone_id_pc34(actionMenuRowCount), 1,
            0, 0, graphic.w, graphic.h, graphic.x, graphic.y);
    } else if (presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34 ||
               presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34 ||
               presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34 ||
               presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_SPELL_FAILURE_PC34) {
        DM1_V1_SpellAreaRectPc34 sourceBox = dm1_v1_spell_area_source_box_rect_pc34();
        DM1_V1_SpellAreaRectPc34 background = dm1_v1_spell_area_graphic_rect_pc34();
        outPlan->clearX = sourceBox.x;
        outPlan->clearY = sourceBox.y;
        outPlan->clearW = sourceBox.w;
        outPlan->clearH = sourceBox.h;
        outPlan->blitCount = 3;
        dm1_v1_action_spell_m11_blit_set_pc34(&outPlan->blits[0],
            DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34,
            DM1_V1_SPELL_AREA_ZONE_ID_PC34, 1, 0, 0, background.w,
            background.h, background.x, background.y);
        dm1_v1_action_spell_m11_blit_set_pc34(&outPlan->blits[1],
            DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34,
            DM1_V1_SPELL_AVAILABLE_SYMBOL_PARENT_ZONE_ID_BASE_PC34,
            DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34, 0,
            DM1_V1_SPELL_AREA_LINES_AVAILABLE_Y_PC34,
            DM1_V1_SPELL_LABEL_CELL_W_PC34,
            DM1_V1_SPELL_LABEL_CELL_H_PC34, 224, 50);
        dm1_v1_action_spell_m11_blit_set_pc34(&outPlan->blits[2],
            DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34,
            DM1_V1_SPELL_CHAMPION_SYMBOL_ZONE_ID_BASE_PC34,
            DM1_V1_SPELL_RUNE_SEQUENCE_MAX_PC34, 0,
            DM1_V1_SPELL_AREA_LINES_SELECTED_Y_PC34,
            DM1_V1_SPELL_LABEL_CELL_W_PC34,
            DM1_V1_SPELL_LABEL_CELL_H_PC34, 224, 62);
    } else {
        return 0;
    }
    outPlan->accepted = 1;
    return 1;
}
