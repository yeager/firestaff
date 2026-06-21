/**
 * test_dm1_v1_projectile_list_overflow_pc34_compat.c
 *
 * Data-free regression gate for the F0810 projectile list capacity
 * hard-cap (PJE-05 in docs/dm1-v1-functional-divergence-report.md).
 *
 * ReDMCSB PROJEXPL.C:F0212_PROJECTILE_Create line ~61 documents
 * BUG0_16: when no projectile thing remains (60 maximum), the
 * thrown/shot/launched object can be orphaned. Firestaff keeps a
 * bounded synthetic list and rejects overflow at PROJECTILE_LIST_CAPACITY.
 *
 * PJE-05 (audit, v2.7.x) makes the divergence observable: F0810
 * emits a one-shot warning to stderr when the cap is hit and
 * returns 0. This gate pins the cap-rejection behaviour
 * (list.count stays at the cap), the empty-slot reuse contract,
 * invalid-input rejection, and the no-event/no-slot overflow contract.
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

static void make_sentinel_event(struct TimelineEvent_Compat* ev) {
    memset(ev, 0xA5, sizeof(*ev));
}

static int event_is_sentinel(const struct TimelineEvent_Compat* ev) {
    struct TimelineEvent_Compat sentinel;
    make_sentinel_event(&sentinel);
    return memcmp(ev, &sentinel, sizeof(*ev)) == 0;
}

static int fill_to_capacity(struct ProjectileList_Compat* list,
                            struct TimelineEvent_Compat* ev,
                            int* slot) {
    int tick;
    memset(list, 0, sizeof(*list));
    for (tick = 0; tick < PROJECTILE_LIST_CAPACITY; ++tick) {
        struct ProjectileCreateInput_Compat in = make_throw_input(tick);
        int rc = F0810_PROJECTILE_Create_Compat(&in, list, slot, ev);
        CHECK(rc == 1, "F0810 accepts inserts up to capacity");
        CHECK(*slot == tick, "F0810 fills the first empty projectile slot");
        CHECK(list->entries[tick].reserved3 == 1,
              "accepted projectile slot is marked occupied");
        CHECK(list->entries[tick].slotIndex == tick,
              "accepted projectile slot stores its index");
        CHECK(ev->kind == TIMELINE_EVENT_PROJECTILE_MOVE,
              "accepted projectile schedules a projectile move event");
        CHECK(ev->fireAtTick == (uint32_t)(tick + 1),
              "accepted projectile schedules first move at currentTick+1");
        CHECK(ev->aux0 == tick,
              "accepted projectile event points back to the slot index");
        if (rc != 1 || *slot != tick) return 0;
    }
    return 1;
}

static void test_cap_reached(void) {
    /* Fill the projectile list to capacity; one extra insert
     * must be rejected (BUG0_16 PJE-05 defensive behaviour). */
    struct ProjectileList_Compat list;
    struct TimelineEvent_Compat ev;
    int slot = -1;

    CHECK(PROJECTILE_LIST_CAPACITY == 60,
          "PROJECTILE_LIST_CAPACITY stays at the bounded BUG0_16 gate size");
    if (!fill_to_capacity(&list, &ev, &slot)) return;

    CHECK(list.count == PROJECTILE_LIST_CAPACITY,
          "list.count equals capacity after fills");

    /* The (capacity+1)-th insert must be rejected. */
    {
        struct ProjectileCreateInput_Compat over = make_throw_input(
            PROJECTILE_LIST_CAPACITY);
        slot = 1234;
        make_sentinel_event(&ev);
        int rc = F0810_PROJECTILE_Create_Compat(&over, &list, &slot, &ev);
        CHECK(rc == 0, "F0810 rejects insert past capacity (BUG0_16 cap)");
        CHECK(list.count == PROJECTILE_LIST_CAPACITY,
              "list.count stays at capacity after overflow rejection");
        CHECK(slot == 1234,
              "overflow rejection does not publish a projectile slot");
        CHECK(event_is_sentinel(&ev),
              "overflow rejection does not publish a first-move event");
    }
}

static void test_despawn_releases_slot(void) {
    /* Despawning the most-recently-added projectile must reopen a
     * slot so a fresh insert succeeds. Pairs with the cap test. */
    struct ProjectileList_Compat list;
    struct TimelineEvent_Compat ev;
    int slot = -1;

    if (!fill_to_capacity(&list, &ev, &slot)) return;

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
        CHECK(slot == 0, "F0810 reuses the first freed projectile slot");
        CHECK(list.count == PROJECTILE_LIST_CAPACITY,
              "list.count returns to capacity after refill");
        CHECK(ev.aux0 == 0, "refill event points at the reused slot");
    }
}

static void test_repeated_overflow_does_not_grow(void) {
    /* Sustained overflow attempts must not corrupt the list. */
    struct ProjectileList_Compat list;
    struct TimelineEvent_Compat ev;
    int slot = -1;
    int i;

    if (!fill_to_capacity(&list, &ev, &slot)) return;

    /* Hammer 5 more inserts, all should be rejected without growth. */
    for (i = 0; i < 5; ++i) {
        struct ProjectileCreateInput_Compat in = make_throw_input(
            PROJECTILE_LIST_CAPACITY + i);
        slot = 2000 + i;
        make_sentinel_event(&ev);
        int rc = F0810_PROJECTILE_Create_Compat(&in, &list, &slot, &ev);
        CHECK(rc == 0, "F0810 rejects sustained overflow attempts");
        CHECK(slot == 2000 + i,
              "sustained overflow does not publish a projectile slot");
        CHECK(event_is_sentinel(&ev),
              "sustained overflow does not publish first-move events");
    }
    CHECK(list.count == PROJECTILE_LIST_CAPACITY,
          "list.count stays at capacity under sustained overflow");
}

static void test_invalid_inputs_do_not_grow(void) {
    struct ProjectileList_Compat list;
    struct TimelineEvent_Compat ev;
    struct ProjectileCreateInput_Compat in = make_throw_input(7);
    int slot = 9;

    memset(&list, 0, sizeof(list));
    make_sentinel_event(&ev);

    CHECK(F0810_PROJECTILE_Create_Compat(NULL, &list, &slot, &ev) == 0,
          "F0810 rejects NULL input");
    CHECK(F0810_PROJECTILE_Create_Compat(&in, NULL, &slot, &ev) == 0,
          "F0810 rejects NULL projectile list");
    CHECK(F0810_PROJECTILE_Create_Compat(&in, &list, NULL, &ev) == 0,
          "F0810 rejects NULL slot output");
    CHECK(F0810_PROJECTILE_Create_Compat(&in, &list, &slot, NULL) == 0,
          "F0810 rejects NULL event output");

    in.direction = 4;
    CHECK(F0810_PROJECTILE_Create_Compat(&in, &list, &slot, &ev) == 0,
          "F0810 rejects invalid direction");
    in = make_throw_input(7);
    in.cell = 4;
    CHECK(F0810_PROJECTILE_Create_Compat(&in, &list, &slot, &ev) == 0,
          "F0810 rejects invalid cell");

    CHECK(list.count == 0, "invalid F0810 inputs do not grow list.count");
    CHECK(event_is_sentinel(&ev),
          "invalid F0810 inputs do not publish first-move events");
}

int main(void) {
    printf("[dm1_v1_projectile_list_overflow_pc34_compat]\n");
    test_cap_reached();
    test_despawn_releases_slot();
    test_repeated_overflow_does_not_grow();
    test_invalid_inputs_do_not_grow();
    printf("  %d passed, %d failed\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
