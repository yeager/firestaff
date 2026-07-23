#include "dm1_v1_action_spell_render_command_admission_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

int main(void)
{
    static unsigned char spellBackground[87 * 25] = { 1 };
    static unsigned char spellLines[14 * 39] = { 1 };
    static unsigned char font[1] = { 1 };
    DM1_V1_ActionSpellHudSurfacePc34 surfaces[] = {
        { 9, 87, 25, 87 * 25, spellBackground, 1 },
        { 11, 14, 39, 14 * 39, spellLines, 1 },
        { 695, 1, 1, 1, font, 1 }
    };
    DM1_V1_ActionSpellHudMaterialSetPc34 materials = {
        surfaces, 3, 0
    };
    DM1_V1_ActionSpellPresentationSequenceReceiptPc34 sequence;
    DM1_V1_ActionSpellRenderCommandReceiptPc34 commands;

    memset(&sequence, 0, sizeof(sequence));
    sequence.accepted = 1;
    sequence.drawable = 1;
    sequence.presentationKind =
        DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    sequence.stepCount = 4;
    sequence.steps[0] = (DM1_V1_ActionSpellPresentationSequenceStepPc34) {
        DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34, 9, 13, 1,
        0, 0, 87, 25
    };
    sequence.steps[1] = (DM1_V1_ActionSpellPresentationSequenceStepPc34) {
        DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34, 11, 245, 6,
        0, 13, 14, 12
    };
    sequence.steps[2] = (DM1_V1_ActionSpellPresentationSequenceStepPc34) {
        DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34, 11, 261, 4,
        0, 26, 14, 12
    };
    sequence.steps[3] = (DM1_V1_ActionSpellPresentationSequenceStepPc34) {
        DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34, 695, 221, 1,
        0, 0, 0, 0
    };

    CHECK(dm1_v1_action_spell_render_command_admit_pc34(
              &sequence, &materials, &commands));
    CHECK(commands.accepted && commands.drawable && commands.commandCount == 4);
    CHECK(commands.sourceOwnedCommandCount == 4);
    CHECK(commands.commands[0].graphicId == 9 &&
          commands.commands[0].sourceSurfaceIndex == 0);
    CHECK(commands.commands[1].graphicId == 11 &&
          commands.commands[1].sourceY == 13 &&
          commands.commands[1].zoneCount == 6);
    CHECK(commands.commands[2].graphicId == 11 &&
          commands.commands[2].sourceY == 26 &&
          commands.commands[2].zoneCount == 4);
    CHECK(commands.commands[3].kind ==
              DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34 &&
          commands.commands[3].sourceSurfaceIndex == 2);

    surfaces[1].sourceOwned = 0;
    CHECK(!dm1_v1_action_spell_render_command_admit_pc34(
              &sequence, &materials, &commands));
    surfaces[1].sourceOwned = 1;
    surfaces[1].height = 37;
    CHECK(!dm1_v1_action_spell_render_command_admit_pc34(
              &sequence, &materials, &commands));
    surfaces[1].height = 39;
    memset(spellLines, 0, sizeof(spellLines));
    CHECK(!dm1_v1_action_spell_render_command_admit_pc34(
              &sequence, &materials, &commands));
    spellLines[0] = 1;
    font[0] = 0;
    CHECK(!dm1_v1_action_spell_render_command_admit_pc34(
              &sequence, &materials, &commands));
    font[0] = 1;
    surfaces[2].graphicId = 653;
    sequence.steps[3].graphicId = 653;
    CHECK(!dm1_v1_action_spell_render_command_admit_pc34(
              &sequence, &materials, &commands));
    surfaces[2].graphicId = 695;
    sequence.steps[3].graphicId = 557;
    CHECK(!dm1_v1_action_spell_render_command_admit_pc34(
              &sequence, &materials, &commands));

    printf("%s\n", failures ? "failed" : "ok: action/spell render command admission");
    return failures ? 1 : 0;
}
