#include "dm1_v1_action_spell_input_command_admission_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

int main(void)
{
    DM1_V1_ActionSpellInputCommandRequestPc34 request;
    DM1_V1_ActionSpellHudMaterialReceiptPc34 materials;
    DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34 receipt;

    memset(&request, 0, sizeof(request));
    memset(&materials, 0, sizeof(materials));
    request.kind = DM1_V1_ACTION_SPELL_INPUT_ACTION_SELECT_PC34;
    request.active = 1;
    request.championIndex = 1;
    request.championCount = 4;
    request.actionIndex = 17;
    request.actionMenuRowCount = 2;
    request.selectedActionRow = 1;
    request.sourceTick = 40;
    materials.accepted = 1;
    materials.drawable = 1;
    materials.sourceSurfaceCount = 2;
    materials.primaryGraphicId = 10;
    materials.primaryZoneId = 11;
    materials.secondaryZoneId = 77;
    materials.fontGraphicId = 695;
    CHECK(dm1_v1_action_spell_input_command_admit_pc34(
              &request, &materials, &receipt));
    CHECK(receipt.presentationKind ==
              DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 &&
          receipt.commandGraphicId == 10 && receipt.commandZoneId == 86 &&
          receipt.fontGraphicId == 695);
    materials.secondaryZoneId = 11;
    CHECK(!dm1_v1_action_spell_input_command_admit_pc34(
              &request, &materials, &receipt));
    materials.secondaryZoneId = 77;
    request.candidatePanelActive = 1;
    CHECK(!dm1_v1_action_spell_input_command_admit_pc34(
              &request, &materials, &receipt));

    memset(&request, 0, sizeof(request));
    memset(&materials, 0, sizeof(materials));
    request.kind = DM1_V1_ACTION_SPELL_INPUT_SPELL_RUNE_COMMIT_PC34;
    request.championIndex = 2;
    request.sourceTick = 41;
    request.runeSymbolIndex = 3;
    request.spellPanel.active = 1;
    request.spellPanel.panel_open = 1;
    request.spellPanel.rune_row = 1;
    request.spellPanel.rune_count = 1;
    materials.accepted = 1;
    materials.drawable = 1;
    materials.sourceSurfaceCount = 3;
    materials.primaryGraphicId = 9;
    materials.secondaryGraphicId = 11;
    materials.primaryZoneId = 13;
    materials.fontGraphicId = 557;
    CHECK(dm1_v1_action_spell_input_command_admit_pc34(
              &request, &materials, &receipt));
    CHECK(receipt.commandGraphicId == 9 && receipt.secondaryGraphicId == 11 &&
          receipt.commandZoneId == 258 && receipt.runeValue == 0x69 &&
          receipt.runeRow == 1 && receipt.runeCount == 2 &&
          receipt.fontGraphicId == 557);
    materials.sourceSurfaceCount = 2;
    CHECK(!dm1_v1_action_spell_input_command_admit_pc34(
              &request, &materials, &receipt));
    materials.sourceSurfaceCount = 3;
    request.spellPanel.candidate_panel_active = 1;
    CHECK(!dm1_v1_action_spell_input_command_admit_pc34(
              &request, &materials, &receipt));

    printf("%s\n", failures ? "failed" : "ok: action/spell input command admission");
    return failures ? 1 : 0;
}
