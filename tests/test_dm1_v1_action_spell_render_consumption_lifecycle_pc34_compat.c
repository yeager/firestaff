#include "dm1_v1_action_spell_render_consumption_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static DM1_V1_ActionSpellRenderConsumptionReceiptPc34
receipt(unsigned int frameTick, int graphicId, int zoneId)
{
    DM1_V1_ActionSpellRenderConsumptionReceiptPc34 value;
    memset(&value, 0, sizeof(value));
    value.accepted = 1; value.renderReadyForHost = 1;
    value.clearCount = 1; value.sourceGraphicId = graphicId; value.sourceZoneId = zoneId;
    value.suppressSyntheticFallback = 1;
    value.renderRect = graphicId == 10
        ? (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 }
        : (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    value.frameTick = frameTick; value.sourceTick = frameTick - 800;
    value.serial = frameTick - 891; value.commandFingerprint = frameTick + 0x10u;
    value.orderingFingerprint = frameTick + 0x20u; value.lifecycleGeneration = frameTick - 888;
    return value;
}

int
main(void)
{
    DM1_V1_ActionSpellRenderConsumptionLifecycleStatePc34 state;
    DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellRenderConsumptionReceiptPc34 first = receipt(900, 9, 13);
    DM1_V1_ActionSpellRenderConsumptionReceiptPc34 next = receipt(901, 10, 11);

    memset(&state, 0, sizeof(state));
    CHECK(dm1_v1_action_spell_render_consumption_lifecycle_apply_pc34(
              &state, &first, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.hostConsumptionCurrent &&
          !lifecycle.retirePreviousHostConsumption && !lifecycle.alreadyCurrent);
    CHECK(dm1_v1_action_spell_render_consumption_lifecycle_apply_pc34(
              &state, &next, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.hostConsumptionCurrent &&
          lifecycle.retirePreviousHostConsumption && lifecycle.sourceGraphicId == 10);
    CHECK(dm1_v1_action_spell_render_consumption_lifecycle_apply_pc34(
              &state, &next, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.alreadyCurrent && !lifecycle.retirePreviousHostConsumption);

    CHECK(!dm1_v1_action_spell_render_consumption_lifecycle_apply_pc34(
              &state, &first, &lifecycle));
    next.commandFingerprint++;
    CHECK(!dm1_v1_action_spell_render_consumption_lifecycle_apply_pc34(
              &state, &next, &lifecycle));
    next.commandFingerprint--;
    next.suppressSyntheticFallback = 0;
    CHECK(!dm1_v1_action_spell_render_consumption_lifecycle_apply_pc34(
              &state, &next, &lifecycle));

    printf("%s\n", failures ? "failed" : "ok: action/spell render consumption lifecycle");
    return failures ? 1 : 0;
}
