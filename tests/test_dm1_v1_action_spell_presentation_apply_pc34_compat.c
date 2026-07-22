#include "dm1_v1_action_spell_presentation_apply_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void setup(
    DM1_V1_ActionSpellExecutionReceiptPc34 *execution,
    DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    DM1_V1_ActionSpellHudSurfacePc34 surfaces[3],
    const unsigned char *background,
    const unsigned char *lines,
    const unsigned char *font)
{
    memset(execution, 0, sizeof(*execution));
    memset(commands, 0, sizeof(*commands));
    execution->accepted = 1;
    execution->readyForPresentation = 1;
    execution->presentationKind =
        DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    execution->championIndex = 1;
    execution->spellKind = 2;
    execution->spellPowerOrdinal = 3;
    execution->sourceTick = 50;
    execution->serial = 8;
    execution->commandCount = 4;
    execution->sourceOwnedCommandCount = 4;
    commands->accepted = 1;
    commands->drawable = 1;
    commands->presentationKind = execution->presentationKind;
    commands->commandCount = 4;
    commands->sourceOwnedCommandCount = 4;
    commands->commands[0] = (DM1_V1_ActionSpellRenderCommandPc34) {
        1, 9, 13, 1, 0, 0, 87, 25, 0
    };
    commands->commands[1] = (DM1_V1_ActionSpellRenderCommandPc34) {
        1, 11, 245, 6, 0, 13, 14, 12, 1
    };
    commands->commands[2] = (DM1_V1_ActionSpellRenderCommandPc34) {
        1, 11, 261, 4, 0, 26, 14, 12, 1
    };
    commands->commands[3] = (DM1_V1_ActionSpellRenderCommandPc34) {
        2, 695, 221, 1, 0, 0, 0, 0, 2
    };
    surfaces[0] = (DM1_V1_ActionSpellHudSurfacePc34) {
        9, 87, 25, 87 * 25, background, 1
    };
    surfaces[1] = (DM1_V1_ActionSpellHudSurfacePc34) {
        11, 14, 39, 14 * 39, lines, 1
    };
    surfaces[2] = (DM1_V1_ActionSpellHudSurfacePc34) {
        695, 1, 1, 1, font, 1
    };
    materials->surfaces = surfaces;
    materials->surfaceCount = 3;
    materials->actionMenuRowCount = 0;
}

int main(void)
{
    static const unsigned char background[87 * 25] = { 1 };
    static const unsigned char lines[14 * 39] = { 1 };
    static const unsigned char font[1] = { 1 };
    DM1_V1_ActionSpellExecutionReceiptPc34 execution;
    DM1_V1_ActionSpellRenderCommandReceiptPc34 commands;
    DM1_V1_ActionSpellHudMaterialSetPc34 materials;
    DM1_V1_ActionSpellHudSurfacePc34 surfaces[3];
    DM1_V1_ActionSpellPresentationFrameStatePc34 state;
    DM1_V1_ActionSpellPresentationApplyReceiptPc34 receipt;

    memset(&state, 0, sizeof(state));
    setup(&execution, &commands, &materials, surfaces, background, lines, font);
    /* Fingerprint is normally supplied by the preceding execution receipt. */
    execution.commandFingerprint = 1898491539u;
    dm1_v1_action_spell_presentation_frame_begin_pc34(&state, 50);
    CHECK(!dm1_v1_action_spell_presentation_apply_pc34(
              &state, &execution, &commands, &materials, &receipt));
    /* Reuse the actual prior receipt builder to get the canonical fingerprint. */
    {
        DM1_V1_LiveActionEffectPc34 effect;
        DM1_V1_ActionSpellHudPresentationReceiptPc34 presentation;
        memset(&effect, 0, sizeof(effect));
        memset(&presentation, 0, sizeof(presentation));
        effect.kind = DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34;
        effect.championIndex = 1;
        effect.damage = 3;
        effect.combatOutcome = 2;
        effect.sourceTick = 50;
        effect.serial = 8;
        presentation.valid = 1;
        presentation.drawable = 1;
        presentation.suppressSyntheticFallback = 1;
        presentation.presentationKind = execution.presentationKind;
        presentation.championIndex = 1;
        presentation.spellKind = 2;
        presentation.spellPowerOrdinal = 3;
        CHECK(dm1_v1_action_spell_execution_receipt_build_pc34(
                  &effect, &presentation, &commands, &execution));
    }
    CHECK(dm1_v1_action_spell_presentation_apply_pc34(
              &state, &execution, &commands, &materials, &receipt));
    CHECK(receipt.applied && !receipt.alreadyApplied && state.hasPresentation &&
          state.commandCount == 4 && state.commands[1].zoneId == 245);
    CHECK(dm1_v1_action_spell_presentation_apply_pc34(
              &state, &execution, &commands, &materials, &receipt));
    CHECK(receipt.alreadyApplied && !receipt.applied);
    surfaces[1].sourceOwned = 0;
    CHECK(!dm1_v1_action_spell_presentation_apply_pc34(
              &state, &execution, &commands, &materials, &receipt));
    surfaces[1].sourceOwned = 1;
    dm1_v1_action_spell_presentation_frame_begin_pc34(&state, 51);
    commands.commands[1].zoneId = 246;
    CHECK(!dm1_v1_action_spell_presentation_apply_pc34(
              &state, &execution, &commands, &materials, &receipt));

    printf("%s\n", failures ? "failed" : "ok: action/spell presentation apply");
    return failures ? 1 : 0;
}
