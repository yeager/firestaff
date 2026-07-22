#include "dm1_v1_action_spell_host_route_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static DM1_V1_ActionSpellHostRouteBridgeReceiptPc34
route(unsigned int frameTick, int kind)
{
    DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 value;
    memset(&value, 0, sizeof(value));
    value.accepted = 1; value.hostImageRouteActive = 1;
    value.hostImageRouteKind = kind; value.suppressSyntheticFallback = 1;
    if (kind == DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_ACTION_PC34) {
        value.sourceGraphicId = 10; value.sourceZoneId = 11;
        value.renderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 };
    } else {
        value.sourceGraphicId = 9; value.sourceZoneId = 13;
        value.renderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    }
    value.frameTick = frameTick; value.sourceTick = frameTick - 800;
    value.serial = frameTick - 891; value.commandFingerprint = frameTick + 0x10u;
    value.orderingFingerprint = frameTick + 0x20u; value.lifecycleGeneration = frameTick - 888;
    return value;
}

int
main(void)
{
    DM1_V1_ActionSpellHostRouteLifecycleStatePc34 state;
    DM1_V1_ActionSpellHostRouteLifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 spell = route(
        900, DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_SPELL_PC34);
    DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 action = route(
        901, DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_ACTION_PC34);

    memset(&state, 0, sizeof(state));
    CHECK(dm1_v1_action_spell_host_route_lifecycle_apply_pc34(
              &state, &spell, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.hostImageRouteCurrent &&
          !lifecycle.retirePreviousHostImageRoute &&
          lifecycle.hostImageRouteKind == DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_SPELL_PC34);
    CHECK(dm1_v1_action_spell_host_route_lifecycle_apply_pc34(
              &state, &action, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.retirePreviousHostImageRoute &&
          lifecycle.hostImageRouteKind == DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_ACTION_PC34);
    CHECK(dm1_v1_action_spell_host_route_lifecycle_apply_pc34(
              &state, &action, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.alreadyCurrent &&
          !lifecycle.retirePreviousHostImageRoute);

    CHECK(!dm1_v1_action_spell_host_route_lifecycle_apply_pc34(
              &state, &spell, &lifecycle));
    action.sourceZoneId = 13;
    CHECK(!dm1_v1_action_spell_host_route_lifecycle_apply_pc34(
              &state, &action, &lifecycle));
    action.sourceZoneId = 11;
    action.hostImageRouteActive = 0;
    CHECK(!dm1_v1_action_spell_host_route_lifecycle_apply_pc34(
              &state, &action, &lifecycle));

    printf("%s\n", failures ? "failed" : "ok: action/spell host route lifecycle");
    return failures ? 1 : 0;
}
