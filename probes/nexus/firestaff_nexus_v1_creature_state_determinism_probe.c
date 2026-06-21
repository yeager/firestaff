/*
 * firestaff_nexus_v1_creature_state_determinism_probe.c
 * ========================================================
 *
 * Nexus V1 creature-AI state-machine determinism probe (Tier 4 #19 polish).
 *
 * Verifies the source-locked creature-state transitions in
 * nexus_v1_creatures.c (DM1 F0209_GROUP_ProcessEvents29to41 timeline) are
 * deterministic across many invocations:
 *
 *   - distance<=1 -> state=3 (attack range)
 *   - distance<=3 -> state=2 (chase)
 *   - else         -> state=1 (patrol)
 *   - state=2 (chase) triggers movement on ai_timer % (6 - speed) == 0
 *
 * Also verifies:
 *   - Alert-all sweep moves every active creature into state=2 (chase)
 *   - Dead creatures are skipped (state frozen)
 *   - Hash of (state, x, y, facing) is stable across re-runs
 *
 * Source-lock: src/nexus/nexus_v1_creatures.c (DM1 F0209 + CREATURE.C
 * movement timing), include/nexus_v1_creatures.h state enum.
 *
 * Run:
 *   ./build/firestaff_nexus_v1_creature_state_determinism_probe
 *
 * Pass: 8/8 invariants (boundary cases + repetition determinism).
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "nexus_v1_creatures.h"
#include "nexus_v1_dungeon.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }      \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }      \
} while (0)

/* Empty passable map. nexus_get_square returns squares[y][x] & 0x1F,
 * so 0 = wall (impassable) and 1 = floor (passable). The chase AI
 * refuses to step into a wall, so we need a fully-floor map for the
 * movement determinism check. */
static uint8_t g_empty_map[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];

static void clear_map(void) {
    /* Fill with 0x01 (floor) everywhere. */
    for (size_t y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (size_t x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            g_empty_map[y][x] = 0x01;
        }
    }
}

/* Spawn one creature at (cx,cy) with a basic type at index 0. */
static int spawn_basic(Nexus_V1_CreatureManager* mgr, int cx, int cy) {
    /* Register a minimal type so the manager can run ticks. */
    if (mgr->type_count == 0) {
        strncpy(mgr->types[0].name, "Test", 31);
        strncpy(mgr->types[0].model_file, "TEST.MNS", 31);
        mgr->types[0].health = 100;
        mgr->types[0].attack = 5;
        mgr->types[0].defense = 2;
        mgr->types[0].speed = 3;
        mgr->types[0].experience_value = 10;
        mgr->types[0].model_index = 0;
        mgr->type_count = 1;
    }
    return nexus_v1_creature_spawn(mgr, 0, cx, cy, /*dir=*/0);
}

/* Compute a 32-bit hash over active creature state. */
static uint32_t state_hash(const Nexus_V1_CreatureManager* mgr) {
    uint32_t h = 0x811c9dc5u;
    for (int i = 0; i < mgr->active_count; ++i) {
        const Nexus_Creature* c = &mgr->active[i];
        uint32_t v = (uint32_t)c->alive * 0x10000u
                   ^ (uint32_t)c->state * 0x1000u
                   ^ (uint32_t)c->x    * 0x100u
                   ^ (uint32_t)c->y    * 0x10u
                   ^ (uint32_t)c->facing
                   ^ (uint32_t)c->ai_timer;
        h ^= v;
        h *= 0x01000193u;
    }
    return h;
}

int main(void) {
    printf("=== Nexus V1 creature-state determinism probe ===\n\n");
    clear_map();

    /* 1. Idle creature far from party -> state=1 (patrol). */
    {
        Nexus_V1_CreatureManager mgr;
        memset(&mgr, 0, sizeof(mgr));
        spawn_basic(&mgr, 10, 10);
        nexus_v1_creatures_tick(&mgr, /*party=*/0, /*party=*/0,
                                g_empty_map, /*map=*/0);
        CHECK(mgr.active[0].state == 1,
              "distance>=4 -> state=1 (patrol)");
    }

    /* 2. distance=3 -> state=2 (chase, not attack). */
    {
        Nexus_V1_CreatureManager mgr;
        memset(&mgr, 0, sizeof(mgr));
        spawn_basic(&mgr, 3, 0);  /* party at (0,0), dist=3 */
        nexus_v1_creatures_tick(&mgr, 0, 0, g_empty_map, 0);
        CHECK(mgr.active[0].state == 2,
              "distance=3 -> state=2 (chase boundary)");
    }

    /* 3. distance=1 -> state=3 (attack range). */
    {
        Nexus_V1_CreatureManager mgr;
        memset(&mgr, 0, sizeof(mgr));
        spawn_basic(&mgr, 1, 0);
        nexus_v1_creatures_tick(&mgr, 0, 0, g_empty_map, 0);
        CHECK(mgr.active[0].state == 3,
              "distance=1 -> state=3 (attack range)");
    }

    /* 4. Chase triggers movement on speed-based interval.
     *    Start at (3,0) with party at (0,0) -> dist=3 -> chase. */
    {
        Nexus_V1_CreatureManager mgr;
        memset(&mgr, 0, sizeof(mgr));
        /* speed=3 -> movement every (6-3)=3 ticks. */
        spawn_basic(&mgr, 3, 0);
        /* First tick sets state=2 (chase); movement triggers on
         * ai_timer % 3 == 0, i.e. on ticks 0 (after the first
         * increment), 3, 6. */
        for (int i = 0; i < 6; ++i) {
            nexus_v1_creatures_tick(&mgr, 0, 0, g_empty_map, 0);
        }
        CHECK(mgr.active[0].x < 3,
              "speed=3 chase triggers movement (x decremented from 3)");
        CHECK(mgr.active[0].state == 2,
              "speed=3 chase stays in state=2 after movement");
    }

    /* 5. Alert-all sweep forces every active creature to state=2. */
    {
        Nexus_V1_CreatureManager mgr;
        memset(&mgr, 0, sizeof(mgr));
        spawn_basic(&mgr, 50, 50); /* far away, would normally patrol */
        spawn_basic(&mgr, 51, 50);
        nexus_v1_creatures_alert_all(&mgr, /*level=*/0);
        CHECK(mgr.active[0].state == 2 && mgr.active[1].state == 2,
              "alert_all forces every creature to chase (state=2)");
    }

    /* 6. Dead creature is skipped (state frozen). */
    {
        Nexus_V1_CreatureManager mgr;
        memset(&mgr, 0, sizeof(mgr));
        spawn_basic(&mgr, 0, 0);  /* distance=0 -> state=3 */
        mgr.active[0].alive = 0;
        mgr.active[0].state = 1;  /* pre-set to patrol */
        nexus_v1_creatures_tick(&mgr, 0, 0, g_empty_map, 0);
        CHECK(mgr.active[0].state == 1,
              "dead creature is skipped (state frozen at 1)");
    }

    /* 7. Determinism across many ticks. */
    {
        int mismatch = 0;
        uint32_t expected = 0;
        for (int rep = 0; rep < 5; ++rep) {
            Nexus_V1_CreatureManager mgr;
            memset(&mgr, 0, sizeof(mgr));
            spawn_basic(&mgr, 10, 10);
            spawn_basic(&mgr, 5, 5);
            spawn_basic(&mgr, 2, 2);
            for (int i = 0; i < 20; ++i) {
                nexus_v1_creatures_tick(&mgr, 7, 7, g_empty_map, 0);
            }
            uint32_t h = state_hash(&mgr);
            if (rep == 0) expected = h;
            else if (h != expected) { ++mismatch; break; }
        }
        CHECK(mismatch == 0,
              "20-tick evolution is deterministic across 5 fresh runs");
    }

    /* 8. NULL-safety. */
    {
        nexus_v1_creatures_tick(NULL, 0, 0, g_empty_map, 0);
        CHECK(1, "nexus_v1_creatures_tick(NULL, ...) is a no-op");
    }

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
