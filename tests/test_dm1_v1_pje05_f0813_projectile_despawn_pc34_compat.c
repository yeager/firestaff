/*
 * test_dm1_v1_pje05_f0813_projectile_despawn_pc34_compat.c
 *
 * Source-locked to ReDMCSB PROJEXPL.C F0214_PROJECTILE_DeleteEvent
 * and BUG0_16: per-dungeon projectile list capacity (676 in DM
 * Atari ST 1.0a, 690 in CSB) replaced with hard cap to prevent
 * list-overfill crash.
 *
 * PJE-05 (DM1 V1 functional-divergence-report.md):
 *   "BUG0_16 projectile list capacity (676..690 slots) is
 *    replaced with hard cap.  Firestaff hard-caps at
 *    PROJECTILE_LIST_CAPACITY (no comment confirms the value;
 *    check needed)."
 *
 * Verification 2026-06-15: PROJECTILE_LIST_CAPACITY = 60
 * (memory_projectile_pc34_compat.h).  This is a bounded
 * approximation of the original 676/690-slot list; the new
 * V2 path uses a fixed 60-slot ring.  This test pins:
 *  T1  PROJECTILE_LIST_CAPACITY == 60 (bounded V2 cap)
 *  T2  F0813 despawn marks slot as empty (slotIndex = -1)
 *  T3  F0813 decrements list->count
 *  T4  F0813 NULL list returns 0
 *  T5  F0813 out-of-range slotIndex returns 0
 *  T6  F0813 already-empty slot is a no-op (returns 0)
 *  T7  Initial list->count is 0; after spawn = 1; after despawn = 0
 *  T8  Multiple spawns + multiple despawns -> count tracks
 *      correctly (no off-by-one, no underflow)
 *
 * Source-locked to ReDMCSB PROJEXPL.C F0214 and BUG0_16.
 */

#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

/* Helper: synthesize a "spawned" projectile at slotIndex. */
static void spawn(struct ProjectileList_Compat* list, int slotIndex, int subtype) {
    memset(&list->entries[slotIndex], 0, sizeof(list->entries[slotIndex]));
    list->entries[slotIndex].slotIndex = slotIndex;
    list->entries[slotIndex].projectileSubtype = subtype;
    list->entries[slotIndex].reserved3 = 1; /* non-zero = "in use" */
    list->count++;
}

int main(void) {
    struct ProjectileList_Compat list;

    /* T1: PROJECTILE_LIST_CAPACITY == 60 (bounded V2 cap). */
    CHECK(PROJECTILE_LIST_CAPACITY == 60,
          "T1: PROJECTILE_LIST_CAPACITY = 60 (bounded V2 cap, BUG0_16 defensive)");

    /* T2-T3: Spawn a projectile, then despawn it. */
    memset(&list, 0, sizeof(list));
    spawn(&list, 0, PROJECTILE_SUBTYPE_FIREBALL);
    CHECK(list.count == 1, "T7a: count = 1 after spawn");
    int rc = F0813_PROJECTILE_Despawn_Compat(&list, 0);
    CHECK(rc == 1, "T2: F0813 returns 1 on success");
    CHECK(list.entries[0].slotIndex == -1,
          "T2: slot marked as empty (slotIndex = -1)");
    CHECK(list.count == 0, "T3: count decremented to 0");

    /* T4: NULL list. */
    CHECK(F0813_PROJECTILE_Despawn_Compat(NULL, 0) == 0,
          "T4: NULL list returns 0");

    /* T5: Out-of-range slotIndex. */
    memset(&list, 0, sizeof(list));
    CHECK(F0813_PROJECTILE_Despawn_Compat(&list, -1) == 0,
          "T5: -1 returns 0 (out-of-range)");
    CHECK(F0813_PROJECTILE_Despawn_Compat(&list, PROJECTILE_LIST_CAPACITY) == 0,
          "T5: CAPACITY returns 0 (out-of-range)");
    CHECK(F0813_PROJECTILE_Despawn_Compat(&list, 100) == 0,
          "T5: 100 returns 0 (out-of-range)");

    /* T6: Already-empty slot. */
    memset(&list, 0, sizeof(list));
    CHECK(F0813_PROJECTILE_Despawn_Compat(&list, 0) == 0,
          "T6: despawning an empty slot returns 0 (no-op)");

    /* T7: Count tracking through spawn + despawn cycle. */
    memset(&list, 0, sizeof(list));
    CHECK(list.count == 0, "T7: initial count = 0");
    spawn(&list, 0, PROJECTILE_SUBTYPE_FIREBALL);
    CHECK(list.count == 1, "T7: count = 1 after spawn");
    F0813_PROJECTILE_Despawn_Compat(&list, 0);
    CHECK(list.count == 0, "T7: count = 0 after despawn");

    /* T8: Multiple spawns + multiple despawns. */
    memset(&list, 0, sizeof(list));
    spawn(&list, 0, PROJECTILE_SUBTYPE_FIREBALL);
    spawn(&list, 1, PROJECTILE_SUBTYPE_SLIME);
    spawn(&list, 2, PROJECTILE_SUBTYPE_LIGHTNING_BOLT);
    CHECK(list.count == 3, "T8: count = 3 after 3 spawns");
    F0813_PROJECTILE_Despawn_Compat(&list, 1);
    CHECK(list.count == 2, "T8: count = 2 after despawn 1");
    F0813_PROJECTILE_Despawn_Compat(&list, 0);
    CHECK(list.count == 1, "T8: count = 1 after despawn 0");
    F0813_PROJECTILE_Despawn_Compat(&list, 2);
    CHECK(list.count == 0, "T8: count = 0 after despawn 2");

    /* T8b: F0813 with already-empty slot does NOT underflow. */
    F0813_PROJECTILE_Despawn_Compat(&list, 0); /* already empty */
    CHECK(list.count == 0, "T8: count stays at 0 (no underflow)");

    printf("PASS: PJE-05 BUG0_16 projectile cap + F0813 despawn invariants (8 scenarios)\n");
    return 0;
}
