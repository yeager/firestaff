#include "dm1_v1_action_spell_execution_receipt_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void setup_spell(
    DM1_V1_LiveActionEffectPc34 *effect,
    DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands)
{
    memset(effect, 0, sizeof(*effect));
    memset(presentation, 0, sizeof(*presentation));
    memset(commands, 0, sizeof(*commands));
    effect->kind = DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34;
    effect->championIndex = 2;
    effect->damage = 3;
    effect->combatOutcome = 2;
    effect->sourceTick = 77;
    effect->serial = 9;
    presentation->valid = 1;
    presentation->drawable = 1;
    presentation->suppressSyntheticFallback = 1;
    presentation->presentationKind =
        DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    presentation->championIndex = 2;
    presentation->spellKind = 2;
    presentation->spellPowerOrdinal = 3;
    commands->accepted = 1;
    commands->drawable = 1;
    commands->presentationKind = presentation->presentationKind;
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
}

int main(void)
{
    DM1_V1_LiveActionEffectPc34 effect;
    DM1_V1_ActionSpellHudPresentationReceiptPc34 presentation;
    DM1_V1_ActionSpellRenderCommandReceiptPc34 commands;
    DM1_V1_ActionSpellExecutionReceiptPc34 receipt;
    unsigned int originalFingerprint;

    setup_spell(&effect, &presentation, &commands);
    CHECK(dm1_v1_action_spell_execution_receipt_build_pc34(
              &effect, &presentation, &commands, &receipt));
    CHECK(receipt.accepted && receipt.readyForPresentation);
    CHECK(receipt.sourceEffectKind == DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34 &&
          receipt.presentationKind ==
              DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34);
    CHECK(receipt.championIndex == 2 && receipt.spellKind == 2 &&
          receipt.spellPowerOrdinal == 3 && receipt.sourceTick == 77 &&
          receipt.serial == 9);
    CHECK(receipt.commandCount == 4 && receipt.sourceOwnedCommandCount == 4 &&
          receipt.commandFingerprint != 0);
    originalFingerprint = receipt.commandFingerprint;
    commands.commands[1].zoneId = 246;
    CHECK(dm1_v1_action_spell_execution_receipt_build_pc34(
              &effect, &presentation, &commands, &receipt));
    CHECK(receipt.commandFingerprint != originalFingerprint);

    setup_spell(&effect, &presentation, &commands);
    effect.combatOutcome = 1;
    CHECK(!dm1_v1_action_spell_execution_receipt_build_pc34(
              &effect, &presentation, &commands, &receipt));
    setup_spell(&effect, &presentation, &commands);
    commands.sourceOwnedCommandCount = 3;
    CHECK(!dm1_v1_action_spell_execution_receipt_build_pc34(
              &effect, &presentation, &commands, &receipt));
    setup_spell(&effect, &presentation, &commands);
    presentation.suppressSyntheticFallback = 0;
    CHECK(!dm1_v1_action_spell_execution_receipt_build_pc34(
              &effect, &presentation, &commands, &receipt));

    printf("%s\n", failures ? "failed" : "ok: action/spell execution receipt");
    return failures ? 1 : 0;
}
