/*
 * CSB V1 ReDMCSB F0213/F0220 explosion runtime boundary.
 *
 * Source:
 *   - ReDMCSB PROJEXPL.C F0213_EXPLOSION_Create
 *   - ReDMCSB PROJEXPL.C F0220_EXPLOSION_ProcessEvent25_Explosion
 *
 * This is a bounded runtime-symbol test over the existing source-shaped
 * explosion lifecycle helpers. It creates no fallback visuals, no synthetic
 * dungeon payload, and no title/viewport route; it only proves that the CSB
 * runtime names and consumes the same create/advance contract.
 */

#include "csb_v1_runtime_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass;
static int g_fail;

#define CHECK(cond, msg) do {                                      \
    if (cond) {                                                    \
        ++g_pass;                                                  \
        printf("  PASS: %s\n", msg);                              \
    } else {                                                       \
        ++g_fail;                                                  \
        printf("  FAIL: %s\n", msg);                              \
    }                                                              \
} while (0)

static void test_f0213_f0220_smoke_create_and_advance(void)
{
    struct ExplosionList_Compat list;
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_event;
    struct ExplosionInstance_Compat next_state;
    struct ExplosionTickResult_Compat tick;
    struct CellContentDigest_Compat digest;
    struct RngState_Compat rng;
    const char *source_evidence;
    int slot = -1;

    printf("-- F0213/F0220 smoke lifecycle --\n");

    source_evidence = csb_v1_runtime_source_evidence();
    CHECK(source_evidence &&
              strstr(source_evidence,
                     "PROJEXPL.C: F0213_EXPLOSION_Create") &&
              strstr(source_evidence,
                     "PROJEXPL.C: F0220_EXPLOSION_ProcessEvent25_Explosion"),
          "CSB runtime source evidence names F0213 and F0220");

    memset(&list, 0, sizeof(list));
    memset(&input, 0, sizeof(input));
    input.explosionType = C040_EXPLOSION_SMOKE;
    input.attack = 96;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 1;
    input.cell = EXPLOSION_CELL_CENTERED;
    input.centered = 1;
    input.currentTick = 0;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;

    CHECK(F0821_EXPLOSION_Create_Compat(
              &input, &list, &slot, &first_event) == 1 &&
              slot == 0 &&
              list.count == 1 &&
              list.entries[0].reserved0 == 1 &&
              list.entries[0].explosionType == C040_EXPLOSION_SMOKE &&
              list.entries[0].attack == 96 &&
              first_event.kind == TIMELINE_EVENT_EXPLOSION_ADVANCE &&
              first_event.fireAtTick > 0u,
          "F0213 creates one live explosion and its first C25 event");

    memset(&digest, 0, sizeof(digest));
    digest.sourceMapIndex = input.mapIndex;
    digest.sourceMapX = input.mapX;
    digest.sourceMapY = input.mapY;
    F0730_COMBAT_RngInit_Compat(&rng, 0x12345678u);
    CHECK(F0822_EXPLOSION_Advance_Compat(
              &list.entries[0], &digest, first_event.fireAtTick,
              &rng, &next_state, &tick) == 1 &&
              tick.resultKind == EXPLOSION_RESULT_ADVANCED_FRAME &&
              tick.despawn == 0 &&
              next_state.currentFrame == 1 &&
              next_state.attack == 56 &&
              tick.outNextTick.kind == TIMELINE_EVENT_EXPLOSION_ADVANCE &&
              tick.outNextTick.fireAtTick > first_event.fireAtTick,
          "F0220 advances persistent smoke and schedules the next C25 event");

    F0730_COMBAT_RngInit_Compat(&rng, 0x12345678u);
    CHECK(F0822_EXPLOSION_Advance_Compat(
              &next_state, &digest, tick.outNextTick.fireAtTick,
              &rng, &next_state, &tick) == 1 &&
              tick.resultKind == EXPLOSION_RESULT_ADVANCED_FRAME &&
              tick.despawn == 0 &&
              next_state.currentFrame == 2 &&
              next_state.attack == 16,
          "F0220 preserves the live slot across the second smoke frame");

    F0730_COMBAT_RngInit_Compat(&rng, 0x12345678u);
    CHECK(F0822_EXPLOSION_Advance_Compat(
              &next_state, &digest, tick.outNextTick.fireAtTick,
              &rng, &next_state, &tick) == 1 &&
              tick.despawn == 1,
          "F0220 despawns the depleted smoke without a stale event");
}

int main(void)
{
    printf("=== CSB V1 F0213/F0220 explosion runtime ===\n\n");
    test_f0213_f0220_smoke_create_and_advance();
    printf("\nSummary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
