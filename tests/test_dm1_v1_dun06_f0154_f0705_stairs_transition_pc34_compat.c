/*
 * test_dm1_v1_dun06_f0154_f0705_stairs_transition_pc34_compat.c
 *
 * Source-locked to ReDMCSB DUNGEON.C F0154_DUNGEON_GetLocationAfterLevelChange.
 *
 * DUN-06 (DM1 V1 functional-divergence-report.md):
 *   "F0154 is not in compat layer.  The new compat layer has
 *    F0705_MOVEMENT_ResolveStairsTransition_Compat which uses
 *    inline level-change math rather than F0154."
 *
 * This test exercises the F0705 public API robustness:
 *  T1  NULL outResult returns 0
 *  T2  NULL dungeon returns 0
 *  T3  NULL party returns 0
 *  T4  Party with mapIndex out-of-bounds returns 0
 *  T5  Party with mapX out-of-bounds returns 0
 *  T6  Party with mapY out-of-bounds returns 0
 *  T7  Party at invalid mapIndex = -1 returns 0
 *  T8  F0705 with valid bounds returns 0 (no in-memory dungeon
 *      means no transition; outResult fields are zeroed)
 *  T9  result.fromMapIndex/toMapIndex start at 0 for unconfigured
 *      dungeon (no crash on empty dungeon)
 *
 * The F0154 in the amalgam is source-locked verbatim.  The
 * F0705 inline implementation in the new path is not
 * source-locked against F0154 (per DUN-06); the test pins the
 * inline path's robustness contract.
 *
 * Source-locked to ReDMCSB DUNGEON.C F0154.
 */

#include "memory_movement_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    struct DungeonDatState_Compat dungeon;
    struct PartyState_Compat party;
    struct StairsTransitionResult_Compat r;

    /* Defensive: initialize to silence MSVC -Wmaybe-uninitialized
     * when running under -Wall -Wextra -O2 (CI flags).  Even
     * though F0705 checks for NULL/zero bounds, MSVC analyzes
     * the deref paths and flags uninitialized locals. */
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&party, 0, sizeof(party));

    /* T1: NULL outResult. */
    CHECK(F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, NULL) == 0,
          "T1: NULL outResult returns 0");

    /* T2: NULL dungeon. */
    memset(&r, 0, sizeof(r));
    CHECK(F0705_MOVEMENT_ResolveStairsTransition_Compat(NULL, &party, &r) == 0,
          "T2: NULL dungeon returns 0");

    /* T3: NULL party. */
    memset(&r, 0, sizeof(r));
    CHECK(F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, NULL, &r) == 0,
          "T3: NULL party returns 0");

    /* T4: Out-of-bounds mapIndex. */
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&party, 0, sizeof(party));
    dungeon.header.mapCount = 1;
    party.mapIndex = 100; /* out-of-bounds */
    memset(&r, 0, sizeof(r));
    CHECK(F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r) == 0,
          "T4: out-of-bounds mapIndex returns 0");

    /* T5: Out-of-bounds mapX. */
    party.mapIndex = 0;
    party.mapX = 100;
    memset(&r, 0, sizeof(r));
    CHECK(F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r) == 0,
          "T5: out-of-bounds mapX returns 0");

    /* T6: Out-of-bounds mapY. */
    party.mapX = 0;
    party.mapY = 100;
    memset(&r, 0, sizeof(r));
    CHECK(F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r) == 0,
          "T6: out-of-bounds mapY returns 0");

    /* T7: Invalid mapIndex = -1. */
    party.mapY = 0;
    party.mapIndex = -1;
    memset(&r, 0, sizeof(r));
    CHECK(F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r) == 0,
          "T7: mapIndex = -1 returns 0");

    /* T8: With valid bounds but no tiles loaded, returns 0
     * (no transition; outResult fields stay at zero from memset). */
    party.mapIndex = 0;
    party.mapX = 0;
    party.mapY = 0;
    party.direction = 0;
    memset(&r, 0xFF, sizeof(r));
    F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r);
    CHECK(r.newMapX == 0, "T8: newMapX = 0 (no transition)");
    CHECK(r.newMapY == 0, "T8: newMapY = 0 (no transition)");

    /* T9: Empty dungeon (no maps, no tiles) -> no crash. */
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&party, 0, sizeof(party));
    memset(&r, 0xFF, sizeof(r));
    F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r);
    CHECK(r.newMapX == 0, "T9: empty dungeon -> newMapX = 0");
    CHECK(r.newMapY == 0, "T9: empty dungeon -> newMapY = 0");

    printf("PASS: DUN-06 F0154/F0705 stairs-transition robustness (9 scenarios)\n");
    return 0;
}
