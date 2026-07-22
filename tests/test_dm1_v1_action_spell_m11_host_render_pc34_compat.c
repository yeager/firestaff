#include "dm1_v1_action_spell_m11_host_render_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void
set_lifecycle(DM1_V1_ActionSpellM11ConsumptionLifecycleReceiptPc34 *lifecycle)
{
    memset(lifecycle, 0, sizeof(*lifecycle));
    lifecycle->accepted = 1; lifecycle->m11ConsumptionCurrent = 1;
    lifecycle->suppressSyntheticFallback = 1; lifecycle->frameTick = 901;
    lifecycle->sourceTick = 101; lifecycle->serial = 10;
    lifecycle->commandFingerprint = 0x41u; lifecycle->orderingFingerprint = 0x42u;
    lifecycle->lifecycleGeneration = 13;
}

int
main(void)
{
    DM1_V1_ActionSpellM11ConsumptionLifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellM11HostRenderReceiptPc34 render;

    set_lifecycle(&lifecycle);
    lifecycle.originalRouteKind = DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34;
    lifecycle.originalGraphicId = 10; lifecycle.originalZoneId = 11;
    lifecycle.originalRenderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 };
    CHECK(dm1_v1_action_spell_m11_host_render_build_pc34(&lifecycle, &render));
    CHECK(render.accepted && render.hostRenderReady && render.originalGraphicId == 10 &&
          render.originalZoneId == 11 && render.originalRenderRect.y == 77);

    lifecycle.originalRouteKind = DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34;
    lifecycle.originalGraphicId = 9; lifecycle.originalZoneId = 13;
    lifecycle.originalRenderRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    CHECK(dm1_v1_action_spell_m11_host_render_build_pc34(&lifecycle, &render));
    CHECK(render.originalGraphicId == 9 && render.originalZoneId == 13 &&
          render.originalRenderRect.y == 42);

    lifecycle.m11ConsumptionCurrent = 0;
    CHECK(!dm1_v1_action_spell_m11_host_render_build_pc34(&lifecycle, &render));
    lifecycle.m11ConsumptionCurrent = 1;
    lifecycle.originalZoneId = 11;
    CHECK(!dm1_v1_action_spell_m11_host_render_build_pc34(&lifecycle, &render));
    lifecycle.originalZoneId = 13;
    lifecycle.originalRenderRect.h = 26;
    CHECK(!dm1_v1_action_spell_m11_host_render_build_pc34(&lifecycle, &render));

    printf("%s\n", failures ? "failed" : "ok: action/spell M11 host render");
    return failures ? 1 : 0;
}
