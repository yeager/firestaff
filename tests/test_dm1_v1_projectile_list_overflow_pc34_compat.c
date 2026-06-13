/**
 * test_dm1_v1_projectile_list_overflow_pc34_compat.c
 *
 * Regression gate for the F0810 projectile list capacity hard-cap
 * (PJE-05 in docs/dm1-v1-functional-divergence-report.md).
 *
 * ReDMCSB PROJEXPL.C:F0220_EXPLOSION_ProcessEvents50To51 can
 * overfill the per-dungeon projectile list (676 in DM Atari ST 1.0a,
 * 690 in CSB). Original can crash; Firestaff hard-caps at
 * PROJECTILE_LIST_CAPACITY and silently drops the overflow.
 *
 * PJE-05 (audit, v2.7.x) makes the divergence observable: F0810
 * emits a one-shot warning to stderr when the cap is hit and
 * returns 0. This gate pins both the cap-rejection behaviour
 * (list.count stays at the cap) and the API contract.
 */
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL: %s\n", msg); \
    } \
} while (0)

static struct ProjectileCreateInput_Compat make_throw_input(int tick) {
    struct ProjectileCreateInput_Compat in;
    memset(&in, 0, sizeof(in));
    in.category = PROJECTILE_CATEGORY_KINETIC;
    in.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    in.ownerKind = PROJECTILE_OWNER_CHAMPION;
    in.ownerIndex = 0;
    in.mapIndex = 0;
    in.mapX = 0;
    in.mapY = 0;
    in.cell = 0;
    in.direction = 0;
    in.kineticEnergy = 40;
    in.attack = 24;
    in.stepEnergy = 4;
    in.firstMoveGraceFlag = 0;
    in.currentTick = tick;
    in.potionPower = 0;
    in.poisonAttack = 0;
    in.attackTypeCode = COMBAT_ATTACK_NORMAL;
    return in;
}

static void test_cap_reached(void) {
    /* Fill the projectile list to capacity; one extra insert
     * must be rejected (BUG0_16 PJE-05 defensive behaviour). */
    struct ProjectileList_Compat list;
    struct TimelineEvent_Compat ev;
    int slot = -1;
    int tick;

    memset(&list, 0, sizeof(list));

    for (tick = 0; tick < PROJECTILE_LIST_CAPACITY; ++tick) {
        struct ProjectileCreateInput_Compat in = make_throw_input(tick);
        int rc = F0810_PROJECTILE_Create_Compat(&in, &list, &slot, &ev);
        CHECK(rc == 1, "F0810 accepts inserts up to capacity");
        if (rc != 1) return;
    }

    CHECK(list.count == PROJECTILE_LIST_CAPACITY,
          "list.count equals capacity after fills");

    /* The (capacity+1)-th insert must be rejected. */
    {
        struct ProjectileCreateInput_Compat over = make_throw_input(
            PROJECTILE_LIST_CAPACITY);
        int rc = F0810_PROJECTILE_Create_Compat(&over, &list, &slot, &ev);
        CHECK(rc == 0, "F0810 rejects insert past capacity (BUG0_16 cap)");
        CHECK(list.count == PROJECTILE_LIST_CAPACITY,
              "list.count stays at capacity after overflow rejection");
    }
}

static void test_despawn_releases_slot(void) {
    /* Despawning the most-recently-added projectile must reopen a
     * slot so a fresh insert succeeds. Pairs with the cap test. */
    struct ProjectileList_Compat list;
    struct TimelineEvent_Compat ev;
    int slot = -1;
    int tick;

    memset(&list, 0, sizeof(list));

    for (tick = 0; tick < PROJECTILE_LIST_CAPACITY; ++tick) {
        struct ProjectileCreateInput_Compat in = make_throw_input(tick);
        if (F0810_PROJECTILE_Create_Compat(&in, &list, &slot, &ev) != 1) {
            CHECK(0, "F0810 should succeed during fill");
            return;
        }
    }

    /* Despawn one to free a slot. */
    CHECK(F0813_PROJECTILE_Despawn_Compat(&list, 0) == 1,
          "F0813 despawn frees a slot");
    CHECK(list.count == PROJECTILE_LIST_CAPACITY - 1,
          "list.count decrements on despawn");

    {
        struct ProjectileCreateInput_Compat in = make_throw_input(
            PROJECTILE_LIST_CAPACITY);
        int rc = F0810_PROJECTILE_Create_Compat(&in, &list, &slot, &ev);
        CHECK(rc == 1, "F0810 re-accepts after despawn frees a slot");
        CHECK(list.count == PROJECTILE_LIST_CAPACITY,
              "list.count returns to capacity after refill");
    }
}

static void test_repeated_overflow_does_not_grow(void) {
    /* Sustained overflow attempts must not corrupt the list. */
    struct ProjectileList_Compat list;
    struct TimelineEvent_Compat ev;
    int slot = -1;
    int i;

    memset(&list, 0, sizeof(list));

    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        struct ProjectileCreateInput_Compat in = make_throw_input(i);
        if (F0810_PROJECTILE_Create_Compat(&in, &list, &slot, &ev) != 1) {
            CHECK(0, "fill before overflow should succeed");
            return;
        }
    }

    /* Hammer 5 more inserts, all should be rejected without growth. */
    for (i = 0; i < 5; ++i) {
        struct ProjectileCreateInput_Compat in = make_throw_input(
            PROJECTILE_LIST_CAPACITY + i);
        int rc = F0810_PROJECTILE_Create_Compat(&in, &list, &slot, &ev);
        CHECK(rc == 0, "F0810 rejects sustained overflow attempts");
    }
    CHECK(list.count == PROJECTILE_LIST_CAPACITY,
          "list.count stays at capacity under sustained overflow");
}

int main(void) {
    printf("[dm1_v1_projectile_list_overflow_pc34_compat]\n");
    test_cap_reached();
    test_despawn_releases_slot();
    test_repeated_overflow_does_not_grow();
    printf("  %d passed, %d failed\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
