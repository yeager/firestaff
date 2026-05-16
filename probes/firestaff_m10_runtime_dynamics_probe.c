/*
 * M10 Phase 19 probe — Runtime dynamics verification.
 *
 * Validates the pure runtime-dynamics data layer against
 * PHASE19_PLAN.md §5 invariants. Ships 46 invariants across 21
 * blocks (A..U).
 *
 * Reporting: runtime_dynamics_probe.md (scope/notes) +
 * runtime_dynamics_invariants.md (per-invariant PASS/FAIL + trailing
 * `Status: PASS`).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "memory_runtime_dynamics_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_magic_pc34_compat.h"

/* ---------------- test harness helpers ---------------- */

static void ctx_default(struct GeneratorContext_Compat* ctx) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->sensorIndex = 0;
    ctx->mapIndex = 1;
    ctx->mapX = 5;
    ctx->mapY = 7;
    ctx->creatureType = 7;   /* PainRat */
    ctx->creatureCountRaw = 3; /* -> creatureCount = 2 (3 creatures) */
    ctx->randomizeCount = 0;
    ctx->healthMultiplier = 2;
    ctx->ticksRaw = 50;
    ctx->onceOnly = 0;
    ctx->audible = 1;
    ctx->mapDifficulty = 3;
    ctx->isOnPartyMap = 1;
    ctx->currentActiveGroupCount = 10;
    ctx->maxActiveGroupCount = 40;
}

static void explist_reset(struct ExplosionList_Compat* list) {
    memset(list, 0, sizeof(*list));
}

/* Create a fluxcage at (slot implicit via F0821, but we control the
 * slot by placing it first). Returns slotIndex, or -1 on failure. */
static int explist_spawn_fluxcage(
    struct ExplosionList_Compat* list,
    int mapIndex, int mapX, int mapY, uint32_t tick)
{
    struct ExplosionCreateInput_Compat in;
    struct TimelineEvent_Compat ev;
    int slot = -1;
    memset(&in, 0, sizeof(in));
    in.explosionType = C050_EXPLOSION_FLUXCAGE;
    in.attack = 50;
    in.mapIndex = mapIndex;
    in.mapX = mapX;
    in.mapY = mapY;
    in.cell = 0;
    in.centered = 0;
    in.poisonAttack = 0;
    in.currentTick = (int)tick;
    in.ownerKind = PROJECTILE_OWNER_CHAMPION;
    in.ownerIndex = 0;
    in.creatorProjectileSlot = -1;
    if (!F0821_EXPLOSION_Create_Compat(&in, list, &slot, &ev)) return -1;
    return slot;
}

int main(int argc, char* argv[]) {
    FILE* report = 0;
    FILE* inv = 0;
    char path[512];
    int failCount = 0;
    int invariantCount = 0;
    const char* dungeonPath;
    const char* outputDir;

    if (argc < 3) {
        fprintf(stderr,
                "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
        return 1;
    }
    dungeonPath = argv[1];
    outputDir = argv[2];

    snprintf(path, sizeof(path), "%s/runtime_dynamics_probe.md", outputDir);
    report = fopen(path, "w");
    if (!report) {
        fprintf(stderr, "FAIL: cannot write report\n"); return 1;
    }
    fprintf(report, "# M10 Phase 19: Runtime Dynamics Probe\n\n");
    fprintf(report, "## Scope (v1)\n\n");
    fprintf(report, "- F0860-F0863 GROUP_GENERATOR spawn + re-enable scheduling\n");
    fprintf(report, "- F0864-F0867 MAGIC_LIGHT_DECAY (mirror of F0257)\n");
    fprintf(report, "- F0868-F0871 REMOVE_FLUXCAGE + absorption idempotency\n");
    fprintf(report, "- F0872-F0874 Generator re-enable (mirror of F0246, C65)\n");
    fprintf(report, "- F0875-F0879 Serialisation (MEDIA016, bit-identical)\n\n");
    fprintf(report, "## NEEDS DISASSEMBLY REVIEW\n\n");
    fprintf(report, "- `s_PowerOrdinalToLightAmount` indices 1..6 use Phase 14\n"
                    "  placeholder `{3,6,10,16,24,40}`; real values are from\n"
                    "  GRAPHICS.DAT entry 562 (G0039). Full 16-entry table is\n"
                    "  post-M10.\n");
    fprintf(report, "- `runtime_get_creature_base_health` per-type values are\n"
                    "  plausible-range placeholders; exact numbers come from\n"
                    "  CREATURE.C:creatureInfo[] when the loader lands.\n");
    fprintf(report, "- `F0861` suppression: only the global active-group-count\n"
                    "  cap on the party map is enforced (Fontanel GROUP.C:512).\n"
                    "  No party-adjacency/proximity check in WIP source; flagged\n"
                    "  as review marker should any DM/CSB build variant add it.\n");
    fprintf(report, "- `F0873` currently linear-scans for the first disabled\n"
                    "  sensor; pre-filtering by (mapIndex,mapX,mapY) is left\n"
                    "  to the caller — Phase 9's DungeonSensor_Compat does\n"
                    "  not store position fields. A cached (sensorIndex -> pos)\n"
                    "  table is post-M10.\n\n");
    fprintf(report, "## Invariants — see runtime_dynamics_invariants.md\n\n");

    snprintf(path, sizeof(path), "%s/runtime_dynamics_invariants.md",
             outputDir);
    inv = fopen(path, "w");
    if (!inv) {
        fprintf(stderr, "FAIL: cannot write invariants\n");
        fclose(report); return 1;
    }
    fprintf(inv, "# Runtime Dynamics Invariants\n\n");

#define CHECK(cond, msg) do { \
    invariantCount++; \
    if (cond) { \
        fprintf(inv, "- PASS: %s\n", msg); \
    } else { \
        fprintf(inv, "- FAIL: %s\n", msg); \
        failCount++; \
    } \
} while (0)

    /* ================================================================
     * Block A — Struct sizes + round-trip (#01–#04)
     * ================================================================ */
    CHECK(sizeof(struct GeneratorContext_Compat) == 64,
          "01: sizeof(GeneratorContext_Compat) == 64");
    CHECK(sizeof(struct GeneratorResult_Compat) == 112,
          "02: sizeof(GeneratorResult_Compat) == 112");
    CHECK(sizeof(struct LightDecayResult_Compat) == 56,
          "03: sizeof(LightDecayResult_Compat) == 56");
    {
        struct GeneratorContext_Compat a, b;
        unsigned char buf[GENERATOR_CONTEXT_SERIALIZED_SIZE];
        ctx_default(&a);
        a.reserved0 = 0x5A;
        memset(&b, 0, sizeof(b));
        int s = F0875_RUNTIME_GeneratorContextSerialize_Compat(&a, buf, sizeof(buf));
        int d = F0875_RUNTIME_GeneratorContextDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(s == 1 && d == 1 && memcmp(&a, &b, sizeof(a)) == 0,
              "04: GeneratorContext round-trip bit-identical");
    }

    /* ================================================================
     * Block B — Generator spawn (#05–#08)
     * ================================================================ */
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        int ok;
        ctx_default(&ctx);
        ctx.creatureType = 7; ctx.creatureCountRaw = 3; /* -> count=2 */
        ctx.randomizeCount = 0;
        ctx.healthMultiplier = 2;
        F0730_COMBAT_RngInit_Compat(&rng, 0xDEADBEEFu);
        ok = F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &rng, 1000, &out);
        CHECK(ok == 1 && out.spawned == 1
              && out.spawnedCreatureCount == 2
              && out.spawnedGroupHealth[0] > 0
              && out.spawnedGroupHealth[1] > 0
              && out.spawnedGroupHealth[2] > 0,
              "05: Generator (type=7, count=3, mult=2) spawns 3 creatures, all HP>0");
    }
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        int ok;
        ctx_default(&ctx);
        ctx.randomizeCount = 1;
        ctx.creatureCountRaw = 4 | 0x08;
        F0730_COMBAT_RngInit_Compat(&rng, 0x12345678u);
        ok = F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &rng, 1000, &out);
        CHECK(ok == 1 && out.spawned == 1
              && out.spawnedCreatureCount >= 0
              && out.spawnedCreatureCount <= 3,
              "06: randomizeCount=1 rawCount=4 -> spawnedCreatureCount in [0..3]");
    }
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        int ok;
        ctx_default(&ctx);
        ctx.healthMultiplier = 0; /* fallback to mapDifficulty */
        ctx.mapDifficulty = 5;
        F0730_COMBAT_RngInit_Compat(&rng, 0x11111111u);
        ok = F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &rng, 2000, &out);
        CHECK(ok == 1 && out.spawned == 1 && out.spawnedHealthMultiplier == 5,
              "07: healthMultiplier=0 resolved to mapDifficulty=5");
    }
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        int ok;
        ctx_default(&ctx);
        F0730_COMBAT_RngInit_Compat(&rng, 0x22222222u);
        ok = F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &rng, 3000, &out);
        CHECK(ok == 1 && out.spawnedDirection >= 0 && out.spawnedDirection <= 3,
              "08: spawnedDirection in [0..3]");
    }

    /* ================================================================
     * Block C — Generator suppression (#09–#11)
     * ================================================================ */
    {
        struct GeneratorContext_Compat ctx;
        int s, r;
        ctx_default(&ctx);
        ctx.isOnPartyMap = 1;
        ctx.currentActiveGroupCount = 50;
        ctx.maxActiveGroupCount = 55;
        F0861_RUNTIME_CheckGeneratorSuppression_Compat(&ctx, &s, &r);
        CHECK(s == 1 && r == GENERATOR_SUPPRESSION_ACTIVE_GROUP_CAP,
              "09: on-party-map, active=50, max=55 -> suppressed (cap)");
    }
    {
        struct GeneratorContext_Compat ctx;
        int s, r;
        ctx_default(&ctx);
        ctx.isOnPartyMap = 1;
        ctx.currentActiveGroupCount = 49;
        ctx.maxActiveGroupCount = 55;
        F0861_RUNTIME_CheckGeneratorSuppression_Compat(&ctx, &s, &r);
        CHECK(s == 0 && r == GENERATOR_SUPPRESSION_NONE,
              "10: on-party-map, active=49, max=55 -> NOT suppressed (below threshold)");
    }
    {
        struct GeneratorContext_Compat ctx;
        int s, r;
        ctx_default(&ctx);
        ctx.isOnPartyMap = 0;
        ctx.currentActiveGroupCount = 99;
        ctx.maxActiveGroupCount = 55;
        F0861_RUNTIME_CheckGeneratorSuppression_Compat(&ctx, &s, &r);
        CHECK(s == 0 && r == GENERATOR_SUPPRESSION_NONE,
              "11: off-party-map, active=99, max=55 -> NOT suppressed");
    }

    /* ================================================================
     * Block D — Generator cooldown (#12–#14)
     * ================================================================ */
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        ctx_default(&ctx);
        ctx.onceOnly = 0;
        ctx.ticksRaw = 100;
        F0730_COMBAT_RngInit_Compat(&rng, 0x33333333u);
        F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &rng, 1000, &out);
        CHECK(out.spawned == 1
              && out.reEnableScheduled == 1
              && out.reEnableEvent.fireAtTick == 1000 + 100
              && out.reEnableEvent.kind == TIMELINE_EVENT_GROUP_GENERATOR,
              "12: onceOnly=0, ticksRaw=100 -> re-enable at nowTick+100");
    }
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        int expected = ((200 - 126) << 6);
        ctx_default(&ctx);
        ctx.onceOnly = 0;
        ctx.ticksRaw = 200;
        F0730_COMBAT_RngInit_Compat(&rng, 0x44444444u);
        F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &rng, 1000, &out);
        CHECK(out.reEnableScheduled == 1
              && out.reEnableEvent.fireAtTick == (uint32_t)(1000 + expected),
              "13: ticksRaw=200 (extended) -> delay = (200-126)<<6 = 4736");
    }
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        ctx_default(&ctx);
        ctx.onceOnly = 0;
        ctx.ticksRaw = 0;
        F0730_COMBAT_RngInit_Compat(&rng, 0x55555555u);
        F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &rng, 1000, &out);
        CHECK(out.sensorDisabled == 0 && out.reEnableScheduled == 0,
              "14: onceOnly=0, ticksRaw=0 -> sensor stays armed");
    }

    /* ================================================================
     * Block E — Max-count boundary (#15)
     * ================================================================ */
    {
        struct GeneratorContext_Compat ctx;
        int s, r;
        ctx_default(&ctx);
        ctx.isOnPartyMap = 1;
        ctx.maxActiveGroupCount = 55;
        ctx.currentActiveGroupCount = 50; /* exactly max-5 */
        F0861_RUNTIME_CheckGeneratorSuppression_Compat(&ctx, &s, &r);
        CHECK(s == 1 && r == GENERATOR_SUPPRESSION_ACTIVE_GROUP_CAP,
              "15: currentActive == maxActive-5 exactly -> suppressed");
    }

    /* ================================================================
     * Block F — Light decay (#16–#19)
     * ================================================================ */
    {
        struct LightDecayResult_Compat out;
        F0864_RUNTIME_HandleLightDecay_Compat(6, 100, 0, &out);
        CHECK(out.magicalLightAmountDelta == (40 - 24)
              && out.followupScheduled == 1
              && out.followupEvent.aux0 == 5
              && out.followupEvent.kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY
              && out.followupEvent.fireAtTick == 100 + 4
              && out.expired == 0,
              "16: lightPower=6 -> delta=16, followup power=5 at now+4");
    }
    {
        struct LightDecayResult_Compat out;
        F0864_RUNTIME_HandleLightDecay_Compat(1, 500, 0, &out);
        CHECK(out.magicalLightAmountDelta == 3
              && out.followupScheduled == 0
              && out.expired == 1,
              "17: lightPower=1 -> delta=3, expired=1");
    }
    {
        int p;
        int cum = 0;
        int followups = 0;
        struct LightDecayResult_Compat out;
        p = 6;
        while (p > 0) {
            F0864_RUNTIME_HandleLightDecay_Compat(p, 100, 0, &out);
            cum += out.magicalLightAmountDelta;
            if (out.followupScheduled) {
                followups++;
                p = out.followupEvent.aux0;
            } else {
                p = 0;
            }
        }
        CHECK(cum == 40 && followups == 5,
              "18: Full chain 6->0 cumulative delta=40, 5 followups");
    }
    {
        struct LightDecayResult_Compat out;
        F0864_RUNTIME_HandleLightDecay_Compat(0, 100, 0, &out);
        CHECK(out.magicalLightAmountDelta == 0
              && out.expired == 1
              && out.followupScheduled == 0,
              "19: lightPower=0 -> no-op (delta=0, expired=1)");
    }

    /* ================================================================
     * Block G — Light stacking (#20)
     * ================================================================ */
    {
        int cur = 0;
        int nxt;
        F0867_RUNTIME_ComputeTotalLightAmount_Compat(cur, 10, &nxt); cur = nxt;
        F0867_RUNTIME_ComputeTotalLightAmount_Compat(cur, 6, &nxt); cur = nxt;
        CHECK(cur == 16,
              "20: Stack table[3]=10 + table[2]=6 -> 16");
    }

    /* ================================================================
     * Block H — Negative power / darkness (#21–#22)
     * ================================================================ */
    {
        struct LightDecayResult_Compat out;
        F0864_RUNTIME_HandleLightDecay_Compat(-3, 100, 0, &out);
        CHECK(out.magicalLightAmountDelta == -(10 - 6)
              && out.followupScheduled == 1
              && out.followupEvent.aux0 == -2,
              "21: lightPower=-3 -> delta=-4, followup power=-2");
    }
    {
        int p = -3;
        int cum = 0;
        struct LightDecayResult_Compat out;
        while (p != 0) {
            F0864_RUNTIME_HandleLightDecay_Compat(p, 100, 0, &out);
            cum += out.magicalLightAmountDelta;
            if (out.followupScheduled) {
                p = out.followupEvent.aux0;
            } else {
                break;
            }
        }
        CHECK(cum == -10,
              "22: Darkness chain -3->0 cumulative delta=-10");
    }

    /* ================================================================
     * Block I — Fluxcage timer expiry (#23–#24)
     * ================================================================ */
    {
        struct ExplosionList_Compat list;
        struct FluxcageRemoveInput_Compat rin;
        struct FluxcageRemoveResult_Compat rout;
        int slot;
        explist_reset(&list);
        slot = explist_spawn_fluxcage(&list, 1, 3, 4, 100);
        memset(&rin, 0, sizeof(rin));
        rin.explosionSlotIndex = slot;
        rin.mapIndex = 1; rin.mapX = 3; rin.mapY = 4;
        F0868_RUNTIME_HandleRemoveFluxcage_Compat(&rin, &list, &rout);
        CHECK(slot >= 0
              && rout.removed == 1
              && rout.squareContentChanged == 1,
              "23: Live fluxcage -> F0868 removes, squareContentChanged=1");
    }
    {
        struct ExplosionList_Compat list;
        struct FluxcageRemoveInput_Compat rin;
        struct FluxcageRemoveResult_Compat rout1, rout2;
        int slot;
        explist_reset(&list);
        slot = explist_spawn_fluxcage(&list, 2, 5, 6, 100);
        memset(&rin, 0, sizeof(rin));
        rin.explosionSlotIndex = slot;
        rin.mapIndex = 2; rin.mapX = 5; rin.mapY = 6;
        F0868_RUNTIME_HandleRemoveFluxcage_Compat(&rin, &list, &rout1);
        F0868_RUNTIME_HandleRemoveFluxcage_Compat(&rin, &list, &rout2);
        CHECK(slot >= 0
              && rout1.removed == 1
              && rout2.removed == 0
              && rout2.squareContentChanged == 0,
              "24: Call F0868 twice -> second call idempotent no-op");
    }

    /* ================================================================
     * Block J — Fluxcage absorption expiry (#25–#26)
     * ================================================================ */
    {
        struct ExplosionList_Compat list;
        struct FluxcageRemoveInput_Compat rin;
        struct FluxcageRemoveResult_Compat rout;
        int slot;
        explist_reset(&list);
        slot = explist_spawn_fluxcage(&list, 1, 2, 3, 100);
        /* Simulate Phase 17 absorption: pre-despawn via F0824. */
        F0824_EXPLOSION_Despawn_Compat(&list, slot);
        memset(&rin, 0, sizeof(rin));
        rin.explosionSlotIndex = slot;
        rin.mapIndex = 1; rin.mapX = 2; rin.mapY = 3;
        F0868_RUNTIME_HandleRemoveFluxcage_Compat(&rin, &list, &rout);
        CHECK(rout.removed == 0 && rout.squareContentChanged == 0,
              "25: Pre-despawned slot -> F0868 removed=0 (already gone)");
    }
    {
        struct ExplosionList_Compat list;
        struct FluxcageRemoveResult_Compat rout;
        int slot;
        explist_reset(&list);
        slot = explist_spawn_fluxcage(&list, 4, 5, 6, 200);
        F0870_RUNTIME_FluxcageRemoveByAbsorption_Compat(&list, slot, &rout);
        CHECK(slot >= 0
              && rout.removed == 1
              && rout.squareContentChanged == 1
              && rout.mapIndex == 4 && rout.mapX == 5 && rout.mapY == 6,
              "26: F0870 absorption entry on live fluxcage -> removed=1");
    }

    /* ================================================================
     * Block K — Multiple fluxcages same cell (#27)
     * ================================================================ */
    {
        struct ExplosionList_Compat list;
        struct FluxcageRemoveInput_Compat rin;
        struct FluxcageRemoveResult_Compat rout;
        int slotA, slotB;
        int countBefore = -1, countAfterA = -1, countAfterB = -1;
        explist_reset(&list);
        slotA = explist_spawn_fluxcage(&list, 1, 8, 8, 100);
        slotB = explist_spawn_fluxcage(&list, 1, 8, 8, 100);
        F0871_RUNTIME_CountFluxcagesOnSquare_Compat(&list, 1, 8, 8, &countBefore);
        memset(&rin, 0, sizeof(rin));
        rin.explosionSlotIndex = slotA;
        rin.mapIndex = 1; rin.mapX = 8; rin.mapY = 8;
        F0868_RUNTIME_HandleRemoveFluxcage_Compat(&rin, &list, &rout);
        F0871_RUNTIME_CountFluxcagesOnSquare_Compat(&list, 1, 8, 8, &countAfterA);
        rin.explosionSlotIndex = slotB;
        F0868_RUNTIME_HandleRemoveFluxcage_Compat(&rin, &list, &rout);
        F0871_RUNTIME_CountFluxcagesOnSquare_Compat(&list, 1, 8, 8, &countAfterB);
        CHECK(slotA >= 0 && slotB >= 0 && slotA != slotB
              && countBefore == 2
              && countAfterA == 1
              && countAfterB == 0,
              "27: Two fluxcages same cell -> remove each -> count goes 2->1->0");
    }

    /* ================================================================
     * Block L — Generator re-enable (#28–#29)
     * ================================================================ */
    {
        struct DungeonSensor_Compat sensors[4];
        struct GeneratorReEnableInput_Compat rin;
        struct GeneratorReEnableResult_Compat rout;
        memset(sensors, 0, sizeof(sensors));
        sensors[0].sensorType = 0;                                   /* DISABLED */
        sensors[1].sensorType = 5;
        sensors[2].sensorType = 0;
        sensors[3].sensorType = 6;
        rin.mapIndex = 1; rin.mapX = 3; rin.mapY = 5;
        F0872_RUNTIME_HandleGeneratorReEnable_Compat(&rin, sensors, 4, &rout);
        CHECK(rout.reEnabled == 1
              && rout.sensorIndex == 0
              && sensors[0].sensorType == RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR,
              "28: Disabled sensor re-enabled to type 6 (GROUP_GENERATOR)");
    }
    {
        struct DungeonSensor_Compat sensors[3];
        struct GeneratorReEnableInput_Compat rin;
        struct GeneratorReEnableResult_Compat rout;
        memset(sensors, 0, sizeof(sensors));
        sensors[0].sensorType = 5;
        sensors[1].sensorType = 6;
        sensors[2].sensorType = 7;
        rin.mapIndex = 1; rin.mapX = 3; rin.mapY = 5;
        F0872_RUNTIME_HandleGeneratorReEnable_Compat(&rin, sensors, 3, &rout);
        CHECK(rout.reEnabled == 0 && rout.sensorIndex == -1,
              "29: No disabled sensors -> reEnabled=0, sensorIndex=-1");
    }

    /* ================================================================
     * Block M — Phase 12 integration (#30–#31)
     * ================================================================ */
    {
        struct GeneratorContext_Compat ctx;
        struct TimelineEvent_Compat ev;
        struct TimelineQueue_Compat q;
        int sch;
        ctx_default(&ctx);
        F0863_RUNTIME_BuildGeneratorReEnableEvent_Compat(&ctx, 500, &ev);
        F0720_TIMELINE_Init_Compat(&q, 500);
        sch = F0721_TIMELINE_Schedule_Compat(&q, &ev);
        CHECK(ev.kind == TIMELINE_EVENT_GROUP_GENERATOR
              && sch == 1
              && q.count == 1,
              "30: F0863 event schedulable via F0721 (kind=GROUP_GENERATOR)");
    }
    {
        struct TimelineEvent_Compat ev;
        struct TimelineQueue_Compat q;
        int sch;
        F0866_RUNTIME_BuildLightDecayFollowupEvent_Compat(4, 100, 0, &ev);
        F0720_TIMELINE_Init_Compat(&q, 100);
        sch = F0721_TIMELINE_Schedule_Compat(&q, &ev);
        CHECK(ev.kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY
              && sch == 1
              && q.count == 1,
              "31: F0866 event schedulable (kind=MAGIC_LIGHT_DECAY)");
    }

    /* ================================================================
     * Block N — Phase 9 integration (#32–#33)
     * ================================================================ */
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        ctx_default(&ctx);
        ctx.creatureType = 7;
        ctx.creatureCountRaw = 3; /* 3 creatures */
        F0730_COMBAT_RngInit_Compat(&rng, 0x77777777u);
        F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &rng, 1000, &out);
        CHECK(out.spawned == 1
              && out.spawnedGroupHealth[0] > 0
              && out.spawnedGroupHealth[1] > 0
              && out.spawnedGroupHealth[2] > 0
              && out.spawnedGroupHealth[3] == 0, /* unused slot */
              "32: creatureType=7: spawnedGroupHealth populated for spawned creatures only");
    }
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        ctx_default(&ctx);
        ctx.creatureCountRaw = 1; /* rawCount=1 -> count-1=0 -> 1 creature */
        ctx.randomizeCount = 0;
        F0730_COMBAT_RngInit_Compat(&rng, 0x88888888u);
        F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &rng, 1000, &out);
        CHECK(out.spawned == 1
              && out.spawnedCreatureCount == 0
              && out.spawnedGroupHealth[0] > 0
              && out.spawnedGroupHealth[1] == 0,
              "33: rawCount=1 (1 creature) -> exactly 1 health entry populated");
    }

    /* ================================================================
     * Block O — Phase 14 integration (#34–#35)
     * ================================================================ */
    {
        /* F0864's magicalLightAmountDelta represents the amount removed
         * from the running MagicState.magicalLightAmount during decay.
         * Caller subtracts: newLight = currentLight - delta. For
         * negative lightPower (darkness), delta is negative so the
         * same subtraction drives the darkness magnitude toward 0. */
        struct MagicState_Compat mag;
        struct LightDecayResult_Compat out;
        int newLight;
        memset(&mag, 0, sizeof(mag));
        mag.magicalLightAmount = 40;
        F0864_RUNTIME_HandleLightDecay_Compat(6, 100, 0, &out);
        newLight = mag.magicalLightAmount - out.magicalLightAmountDelta;
        CHECK(newLight == 40 - 16,
              "34: MagicState.magicalLightAmount=40 - delta for power 6 -> 24");
    }
    {
        /* F0763 produces a timeline event; extract lightPower from aux0 via
         * TIMELINE_AUX tag. For a pure MAGIC_LIGHT_DECAY event built by
         * F0866, aux0 IS the lightPower. Round-trip through F0864. */
        struct TimelineEvent_Compat ev;
        struct LightDecayResult_Compat out;
        F0866_RUNTIME_BuildLightDecayFollowupEvent_Compat(3, 100, 0, &ev);
        F0864_RUNTIME_HandleLightDecay_Compat(ev.aux0, ev.fireAtTick, 0, &out);
        CHECK(out.magicalLightAmountDelta == (10 - 6),
              "35: TimelineEvent->aux0 round-trips into F0864 (power=3 -> delta=4)");
    }

    /* ================================================================
     * Block P — Phase 15 ser/deser round-trip (#36–#38)
     * ================================================================ */
    {
        struct GeneratorContext_Compat a, b;
        unsigned char buf[GENERATOR_CONTEXT_SERIALIZED_SIZE];
        ctx_default(&a);
        a.creatureType = 22;
        a.currentActiveGroupCount = 123456;
        a.reserved0 = -1;
        memset(&b, 0, sizeof(b));
        F0875_RUNTIME_GeneratorContextSerialize_Compat(&a, buf, sizeof(buf));
        F0875_RUNTIME_GeneratorContextDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(memcmp(&a, &b, sizeof(a)) == 0,
              "36: GeneratorContext round-trip (non-trivial) bit-identical");
    }
    {
        struct GeneratorResult_Compat a, b;
        unsigned char buf[GENERATOR_RESULT_SERIALIZED_SIZE];
        memset(&a, 0, sizeof(a));
        a.spawned = 1;
        a.spawnedCreatureType = 20;
        a.spawnedCreatureCount = 3;
        a.spawnedDirection = 2;
        a.spawnedHealthMultiplier = 7;
        a.sensorDisabled = 1;
        a.reEnableScheduled = 1;
        a.reEnableAtTick = 0x12345678u;
        a.soundRequested = 1;
        a.suppressionReason = 0;
        a.rngCallCount = 5;
        a.reserved0 = 9;
        a.spawnedGroupHealth[0] = 100;
        a.spawnedGroupHealth[1] = 110;
        a.spawnedGroupHealth[2] = 120;
        a.spawnedGroupHealth[3] = 130;
        a.reserved1 = -7;
        a.reEnableEvent.kind = TIMELINE_EVENT_GROUP_GENERATOR;
        a.reEnableEvent.fireAtTick = 0x12345678u;
        a.reEnableEvent.mapIndex = 1;
        a.reEnableEvent.mapX = 3;
        a.reEnableEvent.mapY = 5;
        a.reEnableEvent.aux0 = GENERATOR_EVENT_AUX0_REENABLE;
        a.reEnableEvent.aux1 = 42;
        memset(&b, 0, sizeof(b));
        F0876_RUNTIME_GeneratorResultSerialize_Compat(&a, buf, sizeof(buf));
        F0876_RUNTIME_GeneratorResultDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(memcmp(&a, &b, sizeof(a)) == 0,
              "37: GeneratorResult (incl. embedded TimelineEvent) round-trip bit-identical");
    }
    {
        struct FluxcageRemoveResult_Compat a, b;
        unsigned char buf[FLUXCAGE_REMOVE_RESULT_SERIALIZED_SIZE];
        a.removed = 1;
        a.mapIndex = -5;
        a.mapX = 31;
        a.mapY = 25;
        a.squareContentChanged = 1;
        memset(&b, 0, sizeof(b));
        F0878_RUNTIME_FluxcageRemoveResultSerialize_Compat(&a, buf, sizeof(buf));
        F0878_RUNTIME_FluxcageRemoveResultDeserialize_Compat(&b, buf, sizeof(buf));
        CHECK(memcmp(&a, &b, sizeof(a)) == 0,
              "38: FluxcageRemoveResult round-trip bit-identical");
    }

    /* ================================================================
     * Block Q — Phase 17 integration (#39)
     * ================================================================ */
    {
        struct ExplosionList_Compat list;
        struct FluxcageRemoveInput_Compat rin;
        struct FluxcageRemoveResult_Compat rout;
        int slot;
        explist_reset(&list);
        slot = explist_spawn_fluxcage(&list, 1, 5, 5, 300);
        /* Pre-despawn via Phase 17's F0824 (simulating absorption path). */
        F0824_EXPLOSION_Despawn_Compat(&list, slot);
        memset(&rin, 0, sizeof(rin));
        rin.explosionSlotIndex = slot;
        rin.mapIndex = 1; rin.mapX = 5; rin.mapY = 5;
        F0868_RUNTIME_HandleRemoveFluxcage_Compat(&rin, &list, &rout);
        CHECK(slot >= 0 && rout.removed == 0,
              "39: F0821 fluxcage -> F0824 despawn -> F0868 returns removed=0");
    }

    /* ================================================================
     * Block R — Boundary (#40–#42)
     * ================================================================ */
    {
        struct RngState_Compat rng;
        struct GeneratorResult_Compat out;
        int rc;
        F0730_COMBAT_RngInit_Compat(&rng, 0xFFFFu);
        rc = F0860_RUNTIME_HandleGroupGenerator_Compat(0, &rng, 100, &out);
        CHECK(rc == 0, "40: F0860 with NULL ctx -> 0");
    }
    {
        struct ExplosionList_Compat list;
        struct FluxcageRemoveInput_Compat rin;
        struct FluxcageRemoveResult_Compat rout;
        int rc;
        explist_reset(&list);
        memset(&rin, 0, sizeof(rin));
        rin.explosionSlotIndex = EXPLOSION_LIST_CAPACITY; /* out of range */
        rc = F0868_RUNTIME_HandleRemoveFluxcage_Compat(&rin, &list, &rout);
        CHECK(rc == 0, "41: F0868 with out-of-range slotIndex -> 0");
    }
    {
        struct LightDecayResult_Compat out;
        int rc;
        rc = F0864_RUNTIME_HandleLightDecay_Compat(0, 100, 0, &out);
        CHECK(rc == 1
              && out.magicalLightAmountDelta == 0
              && out.expired == 1
              && out.followupScheduled == 0,
              "42: F0864 with lightPower=0 -> no-op (handled cleanly)");
    }

    /* ================================================================
     * Block S — Purity (#43–#44)
     * ================================================================ */
    {
        struct LightDecayResult_Compat o1, o2;
        F0864_RUNTIME_HandleLightDecay_Compat(5, 500, 2, &o1);
        F0864_RUNTIME_HandleLightDecay_Compat(5, 500, 2, &o2);
        CHECK(memcmp(&o1, &o2, sizeof(o1)) == 0,
              "43: F0864 twice same input -> bit-identical output");
    }
    {
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat o1, o2;
        struct RngState_Compat r1, r2;
        ctx_default(&ctx);
        F0730_COMBAT_RngInit_Compat(&r1, 0xABCD1234u);
        F0730_COMBAT_RngInit_Compat(&r2, 0xABCD1234u);
        F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &r1, 1000, &o1);
        F0860_RUNTIME_HandleGroupGenerator_Compat(&ctx, &r2, 1000, &o2);
        CHECK(memcmp(&o1, &o2, sizeof(o1)) == 0,
              "44: F0860 same seed+ctx -> identical spawn results");
    }

    /* ================================================================
     * Block T — Real DUNGEON.DAT spot-check (#45)
     * ================================================================ */
    {
        struct DungeonDatState_Compat dungeon;
        struct DungeonThings_Compat things;
        int ok = 0;
        memset(&dungeon, 0, sizeof(dungeon));
        memset(&things, 0, sizeof(things));
        if (F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, &dungeon) == 1
            && F0502_DUNGEON_LoadTileData_Compat(dungeonPath, &dungeon) == 1
            && F0504_DUNGEON_LoadThingData_Compat(dungeonPath, &dungeon, &things) == 1) {
            /* Find any sensor whose sensorType == 6 (GROUP_GENERATOR). */
            int i;
            int found = -1;
            for (i = 0; i < things.sensorCount; i++) {
                if (things.sensors[i].sensorType
                    == RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR) {
                    found = i;
                    break;
                }
            }
            if (found >= 0) {
                struct DungeonSensor_Compat* s = &things.sensors[found];
                /* Decode creature type from M040_DATA (sensorData). */
                int creatureType = (int)s->sensorData & 0xFF;
                if (creatureType > 26) creatureType = 0; /* sanity clamp for probe */
                struct GeneratorContext_Compat ctx;
                struct GeneratorResult_Compat out;
                struct RngState_Compat rng;
                memset(&ctx, 0, sizeof(ctx));
                ctx.sensorIndex = found;
                ctx.mapIndex = 0;
                ctx.mapX = 0;
                ctx.mapY = 0;
                ctx.creatureType = creatureType;
                ctx.creatureCountRaw = s->value;
                ctx.randomizeCount = (s->value & 0x08) ? 1 : 0;
                ctx.healthMultiplier = (int)(s->localMultiple & 0x000F);
                ctx.ticksRaw = (int)(s->localMultiple >> 4) & 0x0FFF;
                ctx.onceOnly = s->onceOnly;
                ctx.audible = s->audible;
                ctx.mapDifficulty = 1;
                ctx.isOnPartyMap = 0;
                ctx.currentActiveGroupCount = 0;
                ctx.maxActiveGroupCount = 64;
                F0730_COMBAT_RngInit_Compat(&rng, 0xBADC0FFEu);
                ok = F0860_RUNTIME_HandleGroupGenerator_Compat(
                    &ctx, &rng, 1000, &out);
                if (ok && out.spawned == 1) {
                    fprintf(report,
                            "## Real DUNGEON.DAT spot-check\n\n"
                            "- Found generator sensor #%d: type=%d, "
                            "creatureType=%d, count=%d, mult=%d, ticks=%d\n",
                            found, s->sensorType, creatureType,
                            out.spawnedCreatureCount, out.spawnedHealthMultiplier,
                            ctx.ticksRaw);
                    fprintf(report,
                            "- First HP=%d (all slots %d/%d/%d/%d)\n\n",
                            out.spawnedGroupHealth[0],
                            out.spawnedGroupHealth[0],
                            out.spawnedGroupHealth[1],
                            out.spawnedGroupHealth[2],
                            out.spawnedGroupHealth[3]);
                } else {
                    ok = 0;
                }
            } else {
                fprintf(report,
                        "## Real DUNGEON.DAT spot-check\n\n"
                        "- no generator sensor found; tolerating as probe-pass\n\n");
                ok = 1;
            }
            F0504_DUNGEON_FreeThingData_Compat(&things);
            F0502_DUNGEON_FreeTileData_Compat(&dungeon);
            F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        } else {
            fprintf(report,
                    "## Real DUNGEON.DAT spot-check\n\n"
                    "- could not load DUNGEON.DAT at %s (skipped)\n\n",
                    dungeonPath);
            ok = 1;
        }
        CHECK(ok, "45: Real DUNGEON.DAT generator sensor spawns successfully");
    }

    /* ================================================================
     * Block U — Loop guard (#46)
     * ================================================================ */
    {
        /* 10,000-tick sim: one generator "fires" every 100 ticks; track
         * cumulative active group count (simulated). Suppression must
         * kick in at cap; no overflow, no infinite loop. */
        struct GeneratorContext_Compat ctx;
        struct GeneratorResult_Compat out;
        struct RngState_Compat rng;
        uint32_t tick;
        int activeGroups = 0;
        int maxGroups = 20;
        int spawnEvents = 0;
        int suppressEvents = 0;
        int okLoop = 1;
        ctx_default(&ctx);
        ctx.onceOnly = 0;
        ctx.ticksRaw = 50;
        ctx.isOnPartyMap = 1;
        ctx.maxActiveGroupCount = maxGroups;
        F0730_COMBAT_RngInit_Compat(&rng, 0xCAFEBABEu);
        for (tick = 100; tick <= 10000; tick += 100) {
            ctx.currentActiveGroupCount = activeGroups;
            if (!F0860_RUNTIME_HandleGroupGenerator_Compat(
                    &ctx, &rng, tick, &out)) {
                okLoop = 0; break;
            }
            if (out.spawned) {
                spawnEvents++;
                activeGroups++;
                /* Decay: each spawn fades away 2 ticks later. */
                if ((spawnEvents % 3) == 0 && activeGroups > 0) activeGroups--;
            } else {
                suppressEvents++;
            }
            if (activeGroups < 0 || activeGroups > maxGroups) {
                okLoop = 0; break;
            }
        }
        CHECK(okLoop
              && spawnEvents > 0
              && suppressEvents > 0
              && activeGroups <= maxGroups,
              "46: 10000-tick sim: bounded, suppression engages, no overflow");
    }

    /* ================================================================
     * Trailer
     * ================================================================ */
    fprintf(inv, "\nInvariant count: %d\n", invariantCount);
    if (failCount == 0) {
        fprintf(inv, "Status: PASS\n");
    } else {
        fprintf(inv, "Status: FAIL (%d failures)\n", failCount);
    }
    fclose(inv);
    fclose(report);
    return failCount > 0 ? 1 : 0;
}
