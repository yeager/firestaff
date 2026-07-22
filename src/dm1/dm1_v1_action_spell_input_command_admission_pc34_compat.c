#include "dm1_v1_action_spell_input_command_admission_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

enum {
    kGraphicHudFontPrimary = 695,
    kGraphicHudFontAlternate = 557
};

static int
dm1_v1_action_spell_input_action_material_valid_pc34(
    const DM1_V1_ActionSpellInputCommandRequestPc34 *request,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials)
{
    return request->active && !request->candidatePanelActive &&
           request->championIndex >= 0 &&
           request->championIndex < request->championCount &&
           request->actionIndex >= 0 && request->actionMenuRowCount >= 1 &&
           request->actionMenuRowCount <= 3 && request->selectedActionRow >= 0 &&
           request->selectedActionRow < request->actionMenuRowCount &&
           materials->accepted && materials->drawable &&
           materials->primaryGraphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
           materials->primaryZoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34 &&
           materials->secondaryGraphicId == 0 &&
           materials->secondaryZoneId ==
               dm1_v1_action_menu_graphic_zone_id_pc34(
                   request->actionMenuRowCount) &&
           materials->fontGraphicId == kGraphicHudFontPrimary &&
           materials->sourceSurfaceCount == 2;
}

static int
dm1_v1_action_spell_input_spell_material_valid_pc34(
    const DM1_V1_ActionSpellInputCommandRequestPc34 *request,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials)
{
    return request->spellPanel.active &&
           !request->spellPanel.party_dead &&
           !request->spellPanel.candidate_panel_active &&
           request->spellPanel.panel_open &&
           request->runeSymbolIndex >= 0 &&
           request->runeSymbolIndex < DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34 &&
           materials->accepted && materials->drawable &&
           materials->primaryGraphicId ==
               DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
           materials->secondaryGraphicId ==
               DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 &&
           materials->primaryZoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
           (materials->fontGraphicId == kGraphicHudFontPrimary ||
            materials->fontGraphicId == kGraphicHudFontAlternate) &&
           materials->sourceSurfaceCount == 3;
}

int
dm1_v1_action_spell_input_command_admit_pc34(
    const DM1_V1_ActionSpellInputCommandRequestPc34 *request,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34 *outReceipt)
{
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!request || !materials || request->sourceTick == 0) return 0;

    if (request->kind == DM1_V1_ACTION_SPELL_INPUT_ACTION_SELECT_PC34) {
        if (!dm1_v1_action_spell_input_action_material_valid_pc34(
                request, materials)) {
            return 0;
        }
        outReceipt->accepted = 1;
        outReceipt->kind = request->kind;
        outReceipt->presentationKind =
            DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34;
        outReceipt->championIndex = request->championIndex;
        outReceipt->actionIndex = request->actionIndex;
        outReceipt->commandZoneId =
            dm1_v1_action_menu_row_zone_id_pc34(request->selectedActionRow);
        outReceipt->commandGraphicId = DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34;
        outReceipt->fontGraphicId = kGraphicHudFontPrimary;
        outReceipt->sourceTick = request->sourceTick;
        return 1;
    }

    if (request->kind == DM1_V1_ACTION_SPELL_INPUT_SPELL_RUNE_COMMIT_PC34) {
        DM1_V1_SpellPanelReceiptPc34 runeReceipt;
        if (!dm1_v1_action_spell_input_spell_material_valid_pc34(
                request, materials)) {
            return 0;
        }
        runeReceipt = dm1_v1_spell_panel_enter_rune_pc34(
            &request->spellPanel, request->runeSymbolIndex);
        if (!runeReceipt.accepted || !runeReceipt.append_rune ||
            runeReceipt.rune_value != dm1_v1_spell_rune_value_pc34(
                request->spellPanel.rune_row, request->runeSymbolIndex)) {
            return 0;
        }
        outReceipt->accepted = 1;
        outReceipt->kind = request->kind;
        outReceipt->presentationKind =
            DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
        outReceipt->championIndex = request->championIndex;
        outReceipt->commandZoneId =
            dm1_v1_spell_available_symbol_zone_id_pc34(request->runeSymbolIndex);
        outReceipt->commandGraphicId =
            DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34;
        outReceipt->secondaryGraphicId =
            DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34;
        outReceipt->fontGraphicId = materials->fontGraphicId;
        outReceipt->runeValue = runeReceipt.rune_value;
        outReceipt->runeRow = request->spellPanel.rune_row;
        outReceipt->runeCount = runeReceipt.rune_count;
        outReceipt->sourceTick = request->sourceTick;
        return 1;
    }

    return 0;
}
