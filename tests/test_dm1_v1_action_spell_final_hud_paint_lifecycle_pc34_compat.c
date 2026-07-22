#include "dm1_v1_action_spell_final_hud_paint_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static DM1_V1_ActionSpellFinalHudPaintReceiptPc34
paint(unsigned int frameTick, int presentationKind, unsigned int fingerprint)
{
    DM1_V1_ActionSpellFinalHudPaintReceiptPc34 value;
    memset(&value, 0, sizeof(value));
    value.accepted = 1;
    value.presentationKind = presentationKind;
    value.clearColor = 0;
    value.sourceGraphicId = presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 ? 10 : 9;
    value.sourceZoneId = presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 ? 11 : 13;
    value.clearBeforeRender = 1;
    value.suppressSyntheticFallback = 1;
    if (presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34) {
        value.clearRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 224, 77, 96, 45 };
        value.renderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 };
    } else {
        value.clearRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 224, 42, 96, 33 };
        value.renderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    }
    value.frameTick = frameTick;
    value.sourceTick = frameTick - 800;
    value.serial = frameTick - 891;
    value.commandFingerprint = fingerprint;
    value.orderingFingerprint = fingerprint + 1;
    value.lifecycleGeneration = frameTick - 888;
    return value;
}

int
main(void)
{
    DM1_V1_ActionSpellFinalHudPaintLifecycleStatePc34 state;
    DM1_V1_ActionSpellFinalHudPaintLifecycleReceiptPc34 receipt;
    DM1_V1_ActionSpellFinalHudPaintReceiptPc34 first = paint(
        900, DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34, 0x31u);
    DM1_V1_ActionSpellFinalHudPaintReceiptPc34 next = paint(
        901, DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34, 0x41u);

    memset(&state, 0, sizeof(state));
    CHECK(dm1_v1_action_spell_final_hud_paint_lifecycle_apply_pc34(
              &state, &first, &receipt));
    CHECK(receipt.accepted && !receipt.clearPreviousPaint &&
          receipt.clearCurrentPaint && receipt.renderCurrentPaint &&
          receipt.currentClearRect.y == 42);
    CHECK(dm1_v1_action_spell_final_hud_paint_lifecycle_apply_pc34(
              &state, &next, &receipt));
    CHECK(receipt.accepted && receipt.clearPreviousPaint &&
          receipt.previousClearRect.y == 42 && receipt.currentClearRect.y == 77 &&
          receipt.renderCurrentPaint && receipt.suppressSyntheticFallback);
    CHECK(dm1_v1_action_spell_final_hud_paint_lifecycle_apply_pc34(
              &state, &next, &receipt));
    CHECK(receipt.accepted && receipt.alreadyCurrent && !receipt.renderCurrentPaint);

    next.commandFingerprint++;
    CHECK(!dm1_v1_action_spell_final_hud_paint_lifecycle_apply_pc34(
              &state, &next, &receipt));
    next.commandFingerprint--;
    first.frameTick = 899;
    CHECK(!dm1_v1_action_spell_final_hud_paint_lifecycle_apply_pc34(
              &state, &first, &receipt));

    printf("%s\n", failures ? "failed" : "ok: action/spell final HUD paint lifecycle");
    return failures ? 1 : 0;
}
