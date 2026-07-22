#include "dm1_v1_action_spell_m11_consumption_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void
set_lifecycle(DM1_V1_ActionSpellHostRouteLifecycleReceiptPc34 *lifecycle)
{
    memset(lifecycle, 0, sizeof(*lifecycle));
    lifecycle->accepted = 1; lifecycle->hostImageRouteCurrent = 1;
    lifecycle->suppressSyntheticFallback = 1; lifecycle->frameTick = 901;
    lifecycle->sourceTick = 101; lifecycle->serial = 10;
    lifecycle->commandFingerprint = 0x41u; lifecycle->orderingFingerprint = 0x42u;
    lifecycle->lifecycleGeneration = 13;
}

int
main(void)
{
    DM1_V1_ActionSpellHostRouteLifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellM11ConsumptionReceiptPc34 consumption;

    set_lifecycle(&lifecycle);
    lifecycle.hostImageRouteKind = DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_ACTION_PC34;
    lifecycle.sourceGraphicId = 10; lifecycle.sourceZoneId = 11;
    lifecycle.renderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 };
    CHECK(dm1_v1_action_spell_m11_consumption_build_pc34(&lifecycle, &consumption));
    CHECK(consumption.accepted && consumption.m11ConsumptionReady &&
          consumption.originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34 &&
          consumption.originalGraphicId == 10 && consumption.originalZoneId == 11);

    lifecycle.hostImageRouteKind = DM1_V1_ACTION_SPELL_HOST_IMAGE_ROUTE_SPELL_PC34;
    lifecycle.sourceGraphicId = 9; lifecycle.sourceZoneId = 13;
    lifecycle.renderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    CHECK(dm1_v1_action_spell_m11_consumption_build_pc34(&lifecycle, &consumption));
    CHECK(consumption.originalRouteKind == DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34 &&
          consumption.originalGraphicId == 9 && consumption.originalZoneId == 13);

    lifecycle.hostImageRouteCurrent = 0;
    CHECK(!dm1_v1_action_spell_m11_consumption_build_pc34(&lifecycle, &consumption));
    lifecycle.hostImageRouteCurrent = 1;
    lifecycle.sourceZoneId = 11;
    CHECK(!dm1_v1_action_spell_m11_consumption_build_pc34(&lifecycle, &consumption));
    lifecycle.sourceZoneId = 13;
    lifecycle.renderRect.h = 26;
    CHECK(!dm1_v1_action_spell_m11_consumption_build_pc34(&lifecycle, &consumption));

    printf("%s\n", failures ? "failed" : "ok: action/spell M11 consumption proof");
    return failures ? 1 : 0;
}
