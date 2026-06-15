#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory_combat_pc34_compat.h"
#include "memory_runtime_dynamics_pc34_compat.h"

static int expect_int(const char* label, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s: got %d expected %d\n",
                label, actual, expected);
        return 0;
    }
    return 1;
}

static int expect_u32(const char* label, uint32_t actual, uint32_t expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s: got %u expected %u\n",
                label, actual, expected);
        return 0;
    }
    return 1;
}

static int test_deterministic_csb_generator_state(void)
{
    struct GeneratorContext_Compat ctx;
    struct GeneratorResult_Compat out;
    struct RngState_Compat rng;
    int ok = 1;

    memset(&ctx, 0, sizeof(ctx));
    memset(&out, 0, sizeof(out));

    ctx.sensorIndex = 17;
    ctx.mapIndex = 3;
    ctx.mapX = 12;
    ctx.mapY = 9;
    ctx.creatureType = 1;
    ctx.creatureCountRaw = 4 | 0x08;
    ctx.randomizeCount = 1;
    ctx.healthMultiplier = 0;
    ctx.ticksRaw = 130;
    ctx.onceOnly = 0;
    ctx.audible = 1;
    ctx.mapDifficulty = 4;
    ctx.isOnPartyMap = 1;
    ctx.currentActiveGroupCount = 3;
    ctx.maxActiveGroupCount = 60;

    /* ReDMCSB source-lock: TIMELINE.C F0245 lines 964-999 decodes
     * C006 group-generator count/multiplier/direction/re-enable, then
     * GROUP.C F0185 lines 525-541 consumes one random start cell and
     * descending creature indices for cell packing and HP jitter. */
    ok &= expect_int("rng init",
                     F0730_COMBAT_RngInit_Compat(&rng, 0x0BADF00Du), 1);
    ok &= expect_int("generator handles deterministic state",
                     F0860_RUNTIME_HandleGroupGenerator_Compat(
                         &ctx, &rng, 4000u, &out), 1);

    ok &= expect_int("spawned", out.spawned, 1);
    ok &= expect_int("creature type", out.spawnedCreatureType, 1);
    ok &= expect_int("randomized 0-based creature count",
                     out.spawnedCreatureCount, 1);
    ok &= expect_int("spawn direction", out.spawnedDirection, 1);
    ok &= expect_int("map difficulty health multiplier",
                     out.spawnedHealthMultiplier, 4);
    ok &= expect_int("sensor disabled for cooldown", out.sensorDisabled, 1);
    ok &= expect_int("re-enable scheduled", out.reEnableScheduled, 1);
    ok &= expect_u32("extended re-enable tick", out.reEnableAtTick, 4256u);
    ok &= expect_int("audible generator requests sound", out.soundRequested, 1);
    ok &= expect_int("suppression reason", out.suppressionReason,
                     GENERATOR_SUPPRESSION_NONE);
    ok &= expect_int("rng call count", out.rngCallCount, 5);
    ok &= expect_int("source-style packed cells", out.spawnedGroupCells, 0x06);
    ok &= expect_int("health[0]", out.spawnedGroupHealth[0], 446);
    ok &= expect_int("health[1]", out.spawnedGroupHealth[1], 441);
    ok &= expect_int("health[2] unused", out.spawnedGroupHealth[2], 0);
    ok &= expect_int("health[3] unused", out.spawnedGroupHealth[3], 0);

    ok &= expect_int("re-enable event kind", out.reEnableEvent.kind,
                     TIMELINE_EVENT_GROUP_GENERATOR);
    ok &= expect_u32("re-enable event tick", out.reEnableEvent.fireAtTick,
                     4256u);
    ok &= expect_int("re-enable event map", out.reEnableEvent.mapIndex, 3);
    ok &= expect_int("re-enable event x", out.reEnableEvent.mapX, 12);
    ok &= expect_int("re-enable event y", out.reEnableEvent.mapY, 9);
    ok &= expect_int("re-enable aux0", out.reEnableEvent.aux0,
                     GENERATOR_EVENT_AUX0_REENABLE);
    ok &= expect_int("re-enable aux1 carries sensor index",
                     out.reEnableEvent.aux1, 17);
    ok &= expect_u32("rng seed after generator sequence", rng.seed,
                     0xA8CD720Eu);

    return ok ? 0 : 1;
}

int main(void)
{
    int rc = test_deterministic_csb_generator_state();
    if (rc == 0) {
        printf("CSB V1 monster generator deterministic gate passed\n");
    }
    return rc;
}
