#include "nexus_v1_creatures.h"

#include <stdio.h>

static int g_fail;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        g_fail++; \
    } \
} while (0)

static void fill_active_slots(Nexus_V1_CreatureManager *mgr) {
    int i;
    for (i = 1; i < NEXUS_MAX_ACTIVE_CREATURES; ++i) {
        int slot = nexus_v1_creature_spawn(mgr, 0, i, i, i);
        CHECK(slot == i, "valid DGN actor fixture fills the next active slot");
    }
}

int main(void) {
    Nexus_V1_CreatureManager mgr;
    int slot;

    nexus_v1_creatures_init(&mgr);
    CHECK(mgr.type_count > 0, "fixture has at least one Nexus actor type");

    slot = nexus_v1_creature_spawn(&mgr, 0, -4, 70, -1);
    CHECK(slot == 0, "malformed DGN actor coordinates still spawn in slot 0");
    CHECK(mgr.active[0].x == 0, "negative actor x clamps to DGN grid minimum");
    CHECK(mgr.active[0].y == NEXUS_MAX_MAP_SIZE - 1,
          "oversized actor y clamps to DGN grid maximum");
    CHECK(mgr.active[0].facing == 3, "negative actor facing normalizes");

    slot = nexus_v1_creature_spawn(&mgr, mgr.type_count, 1, 1, 0);
    CHECK(slot == -1, "actor type ref equal to type_count is rejected");
    CHECK(mgr.active_count == 1, "rejected actor type does not consume a slot");

    slot = nexus_v1_creature_spawn(&mgr, -1, 1, 1, 0);
    CHECK(slot == -1, "negative actor type ref is rejected");
    CHECK(mgr.active_count == 1, "negative actor type does not consume a slot");

    fill_active_slots(&mgr);
    CHECK(mgr.active_count == NEXUS_MAX_ACTIVE_CREATURES,
          "actor pool reaches fixed active slot capacity");

    slot = nexus_v1_creature_spawn(&mgr, 0, 2, 2, 0);
    CHECK(slot == -1, "actor slot overflow is gracefully rejected");
    CHECK(mgr.active_count == NEXUS_MAX_ACTIVE_CREATURES,
          "overflow actor does not advance active_count");

    if (g_fail != 0) {
        printf("Nexus V1 DGN actor slot bounds regression: %d failure(s)\n", g_fail);
        return 1;
    }

    printf("Nexus V1 DGN actor slot bounds regression passed\n");
    return 0;
}
