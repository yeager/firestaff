#include "dm1_v1_action_spell_m11_consumption_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static DM1_V1_ActionSpellM11ConsumptionReceiptPc34
receipt(unsigned int frameTick, int routeKind)
{
    DM1_V1_ActionSpellM11ConsumptionReceiptPc34 value;
    memset(&value, 0, sizeof(value));
    value.accepted = 1; value.m11ConsumptionReady = 1;
    value.originalRouteKind = routeKind; value.suppressSyntheticFallback = 1;
    if (routeKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34) {
        value.originalGraphicId = 10; value.originalZoneId = 11;
        value.originalRenderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 };
    } else {
        value.originalGraphicId = 9; value.originalZoneId = 13;
        value.originalRenderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    }
    value.frameTick = frameTick; value.sourceTick = frameTick - 800;
    value.serial = frameTick - 891; value.commandFingerprint = frameTick + 0x10u;
    value.orderingFingerprint = frameTick + 0x20u; value.lifecycleGeneration = frameTick - 888;
    return value;
}

int
main(void)
{
    DM1_V1_ActionSpellM11ConsumptionLifecycleStatePc34 state;
    DM1_V1_ActionSpellM11ConsumptionLifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellM11ConsumptionReceiptPc34 spell = receipt(
        900, DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34);
    DM1_V1_ActionSpellM11ConsumptionReceiptPc34 action = receipt(
        901, DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34);

    memset(&state, 0, sizeof(state));
    CHECK(dm1_v1_action_spell_m11_consumption_lifecycle_apply_pc34(
              &state, &spell, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.m11ConsumptionCurrent &&
          lifecycle.originalGraphicId == 9 && lifecycle.originalZoneId == 13);
    CHECK(dm1_v1_action_spell_m11_consumption_lifecycle_apply_pc34(
              &state, &action, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.retirePreviousM11Consumption &&
          lifecycle.originalGraphicId == 10 && lifecycle.originalZoneId == 11);
    CHECK(dm1_v1_action_spell_m11_consumption_lifecycle_apply_pc34(
              &state, &action, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.alreadyCurrent);

    CHECK(!dm1_v1_action_spell_m11_consumption_lifecycle_apply_pc34(
              &state, &spell, &lifecycle));
    action.originalZoneId = 13;
    CHECK(!dm1_v1_action_spell_m11_consumption_lifecycle_apply_pc34(
              &state, &action, &lifecycle));
    action.originalZoneId = 11;
    action.suppressSyntheticFallback = 0;
    CHECK(!dm1_v1_action_spell_m11_consumption_lifecycle_apply_pc34(
              &state, &action, &lifecycle));

    printf("%s\n", failures ? "failed" : "ok: action/spell M11 consumption lifecycle");
    return failures ? 1 : 0;
}
