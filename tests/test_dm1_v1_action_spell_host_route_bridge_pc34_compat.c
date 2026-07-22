#include "dm1_v1_action_spell_host_route_bridge_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void
set_lifecycle(DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *lifecycle)
{
    memset(lifecycle, 0, sizeof(*lifecycle));
    lifecycle->accepted = 1; lifecycle->hostConsumptionCurrent = 1;
    lifecycle->suppressSyntheticFallback = 1; lifecycle->frameTick = 901;
    lifecycle->sourceTick = 101; lifecycle->serial = 10;
    lifecycle->commandFingerprint = 0x41u; lifecycle->orderingFingerprint = 0x42u;
    lifecycle->lifecycleGeneration = 13;
}

int
main(void)
{
    DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellHostRouteBridgeReceiptPc34 bridge;

    set_lifecycle(&lifecycle);
    lifecycle.sourceGraphicId = 10; lifecycle.sourceZoneId = 11;
    lifecycle.renderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 };
    CHECK(dm1_v1_action_spell_host_route_bridge_build_pc34(&lifecycle, &bridge));
    CHECK(bridge.accepted && bridge.hostImageRouteActive &&
          bridge.hostImageRouteKind == DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_ACTION_PC34 &&
          bridge.sourceGraphicId == 10 && bridge.suppressSyntheticFallback);

    lifecycle.sourceGraphicId = 9; lifecycle.sourceZoneId = 13;
    lifecycle.renderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    CHECK(dm1_v1_action_spell_host_route_bridge_build_pc34(&lifecycle, &bridge));
    CHECK(bridge.hostImageRouteKind == DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_SPELL_PC34);

    lifecycle.hostConsumptionCurrent = 0;
    CHECK(!dm1_v1_action_spell_host_route_bridge_build_pc34(&lifecycle, &bridge));
    lifecycle.hostConsumptionCurrent = 1;
    lifecycle.renderRect.w = 88;
    CHECK(!dm1_v1_action_spell_host_route_bridge_build_pc34(&lifecycle, &bridge));
    lifecycle.renderRect.w = 87;
    lifecycle.sourceGraphicId = 999;
    CHECK(!dm1_v1_action_spell_host_route_bridge_build_pc34(&lifecycle, &bridge));

    printf("%s\n", failures ? "failed" : "ok: action/spell host route bridge");
    return failures ? 1 : 0;
}
