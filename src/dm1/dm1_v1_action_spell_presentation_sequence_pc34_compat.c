#include "dm1_v1_action_spell_presentation_sequence_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

enum {
    kGraphicCreatureDamage = 14,
    kGraphicHudFontPrimary = 695,
    kGraphicHudFontAlternate = 557
};

static int
dm1_v1_action_spell_sequence_append_pc34(
    DM1_V1_ActionSpellPresentationSequenceReceiptPc34 *receipt,
    int kind,
    int graphicId,
    int zoneId,
    int zoneCount,
    int sourceX,
    int sourceY,
    int sourceW,
    int sourceH)
{
    DM1_V1_ActionSpellPresentationSequenceStepPc34 *step;
    if (!receipt || receipt->stepCount >=
            DM1_V1_ACTION_SPELL_SEQUENCE_MAX_STEPS_PC34) {
        return 0;
    }
    step = &receipt->steps[receipt->stepCount++];
    step->kind = kind;
    step->graphicId = graphicId;
    step->zoneId = zoneId;
    step->zoneCount = zoneCount;
    step->sourceX = sourceX;
    step->sourceY = sourceY;
    step->sourceW = sourceW;
    step->sourceH = sourceH;
    return 1;
}

static int
dm1_v1_action_spell_sequence_has_bound_material_pc34(
    const DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials)
{
    return presentation && materials && presentation->valid &&
           presentation->drawable && presentation->suppressSyntheticFallback &&
           materials->accepted && materials->drawable &&
           materials->presentationKind == presentation->presentationKind &&
           materials->sourceSurfaceCount > 0;
}

int
dm1_v1_action_spell_presentation_sequence_build_pc34(
    const DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    int actionMenuRowCount,
    DM1_V1_ActionSpellPresentationSequenceReceiptPc34 *outReceipt)
{
    int row;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!dm1_v1_action_spell_sequence_has_bound_material_pc34(
            presentation, materials)) {
        return 0;
    }
    outReceipt->presentationKind = presentation->presentationKind;
    outReceipt->sourceSurfaceCount = materials->sourceSurfaceCount;

    switch (presentation->presentationKind) {
        case DM1_V1_ACTION_HUD_PRESENTATION_DAMAGE_PC34:
            if (materials->primaryGraphicId != kGraphicCreatureDamage ||
                materials->primaryZoneId != DM1_V1_ACTION_RESULT_ZONE_ID_PC34 ||
                materials->fontGraphicId != kGraphicHudFontPrimary ||
                !dm1_v1_action_spell_sequence_append_pc34(
                    outReceipt, DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34,
                    kGraphicCreatureDamage,
                    DM1_V1_ACTION_RESULT_ZONE_ID_PC34, 1, 0, 0, 88, 45) ||
                !dm1_v1_action_spell_sequence_append_pc34(
                    outReceipt,
                    DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34,
                    kGraphicHudFontPrimary,
                    DM1_V1_ACTION_RESULT_ZONE_ID_PC34, 1, 0, 0, 0, 0)) {
                return 0;
            }
            break;

        case DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34:
            if (!presentation->requiresRealActionMenuLayout ||
                materials->primaryGraphicId !=
                    DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 ||
                materials->primaryZoneId != DM1_V1_ACTION_AREA_ZONE_ID_PC34 ||
                materials->fontGraphicId != kGraphicHudFontPrimary ||
                actionMenuRowCount < 1 || actionMenuRowCount > 3 ||
                materials->secondaryZoneId !=
                    dm1_v1_action_menu_graphic_zone_id_pc34(
                        actionMenuRowCount) ||
                !dm1_v1_action_spell_sequence_append_pc34(
                    outReceipt, DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34,
                    DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34,
                    DM1_V1_ACTION_AREA_ZONE_ID_PC34, 1, 0, 0, 87, 45) ||
                !dm1_v1_action_spell_sequence_append_pc34(
                    outReceipt,
                    DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34,
                    kGraphicHudFontPrimary,
                    DM1_V1_ACTION_MENU_HEADER_ZONE_ID_PC34, 1, 0, 0, 0, 0)) {
                return 0;
            }
            for (row = 0; row < actionMenuRowCount; ++row) {
                if (!dm1_v1_action_spell_sequence_append_pc34(
                        outReceipt,
                        DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34,
                        kGraphicHudFontPrimary,
                        dm1_v1_action_menu_row_zone_id_pc34(row), 1,
                        0, 0, 0, 0)) {
                    return 0;
                }
            }
            break;

        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_FAILURE_PC34:
            if (!presentation->requiresRealSpellAreaLayout ||
                materials->primaryGraphicId !=
                    DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 ||
                materials->secondaryGraphicId !=
                    DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 ||
                materials->primaryZoneId != DM1_V1_SPELL_AREA_ZONE_ID_PC34 ||
                (materials->fontGraphicId != kGraphicHudFontPrimary &&
                 materials->fontGraphicId != kGraphicHudFontAlternate) ||
                !dm1_v1_action_spell_sequence_append_pc34(
                    outReceipt, DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34,
                    DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34,
                    DM1_V1_SPELL_AREA_ZONE_ID_PC34, 1, 0, 0, 87, 25) ||
                !dm1_v1_action_spell_sequence_append_pc34(
                    outReceipt, DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34,
                    DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34,
                    DM1_V1_SPELL_AVAILABLE_SYMBOL_PARENT_ZONE_ID_BASE_PC34,
                    DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34,
                    0, DM1_V1_SPELL_AREA_LINES_AVAILABLE_Y_PC34, 14, 12) ||
                !dm1_v1_action_spell_sequence_append_pc34(
                    outReceipt, DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34,
                    DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34,
                    DM1_V1_SPELL_CHAMPION_SYMBOL_ZONE_ID_BASE_PC34,
                    DM1_V1_SPELL_RUNE_SEQUENCE_MAX_PC34,
                    0, DM1_V1_SPELL_AREA_LINES_SELECTED_Y_PC34, 14, 12) ||
                !dm1_v1_action_spell_sequence_append_pc34(
                    outReceipt,
                    DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34,
                    materials->fontGraphicId,
                    DM1_V1_SPELL_CASTER_PANEL_ZONE_ID_PC34, 1,
                    0, 0, 0, 0)) {
                return 0;
            }
            break;

        default:
            return 0;
    }

    outReceipt->accepted = 1;
    outReceipt->drawable = 1;
    return 1;
}
