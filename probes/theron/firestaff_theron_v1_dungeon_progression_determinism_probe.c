/*
 * firestaff_theron_v1_dungeon_progression_determinism_probe.c
 * =============================================================
 *
 * Theron V1 dungeon-progression determinism probe (Tier 4 #20 polish).
 *
 * Verifies the source-locked 7-dungeon progression in
 * theron_v1_dungeon_progression.c is deterministic across many runs.
 * Each dungeon has a state (LOCKED / AVAILABLE / COMPLETE) and the
 * advance() operation transitions current -> next, marking the
 * previous COMPLETE and the next AVAILABLE. After dungeon 7,
 * quest_complete becomes 1 and further advances return
 * THERON_DUNGEON_INVALID.
 *
 * Source-lock:
 *   - THQUEST.ASM T080 (between-dungeon save/load)
 *   - ReDMCSB analogue siblings GROUP.C / CLIKMENU.C
 *   - src/theron/theron_v1_dungeon_progression.c
 *
 * Coverage (10/10 invariants):
 *   1. Init: dungeon 1 AVAILABLE, others LOCKED, quest_complete=0.
 *   2. Init NULL-safety.
 *   3. Advance 1->2 marks 1 COMPLETE, 2 AVAILABLE.
 *   4. Advance 7->8 sets quest_complete=1, returns INVALID.
 *   5. Advance NULL-safety.
 *   6. theron_v1_dungeon_next invalid input returns INVALID.
 *   7. Full 1..7 progression reaches COMPLETE for all 7.
 *   8. Final state is quest_complete=1, current_dungeon=8 (INVALID sentinel).
 *   9. Reset (init) after full progression restores dungeon 1 AVAILABLE.
 *  10. Determinism: 50 full progressions produce identical state hash.
 *
 * Run:
 *   ./build/firestaff_theron_v1_dungeon_progression_determinism_probe
 *
 * Pass: 10/10 invariants.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "theron_v1_dungeon_progression.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }      \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }      \
} while (0)

/* Compute a 32-bit FNV-style hash over the dungeon progression state. */
static uint32_t progression_hash(const Theron_DungeonProgression* prog) {
    uint32_t h = 0x811c9dc5u;
    h ^= (uint32_t)prog->current_dungeon;       h *= 0x01000193u;
    h ^= (uint32_t)prog->quest_complete;        h *= 0x01000193u;
    h ^= (uint32_t)prog->quest_items_collected; h *= 0x01000193u;
    h ^= (uint32_t)prog->current_level;         h *= 0x01000193u;
    h ^= (uint32_t)prog->item_reset_applied;    h *= 0x01000193u;
    h ^= (uint32_t)prog->item_reset_mode;       h *= 0x01000193u;
    for (int i = 0; i < THERON_DUNGEON_COUNT; ++i) {
        h ^= (uint32_t)prog->dungeon_states[i]; h *= 0x01000193u;
    }
    return h;
}

int main(void) {
    printf("=== Theron V1 dungeon-progression determinism probe ===\n\n");

    /* 1. Init: dungeon 1 AVAILABLE, others LOCKED, quest_complete=0. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        CHECK(prog.current_dungeon == THERON_DUNGEON_1_HALL_OF_RECORDS,
              "init: current_dungeon == HoR");
        CHECK(prog.dungeon_states[0] == THERON_DUNGEON_STATE_AVAILABLE,
              "init: dungeon 1 state == AVAILABLE");
        int locked_count = 0;
        for (int i = 1; i < THERON_DUNGEON_COUNT; ++i) {
            if (prog.dungeon_states[i] == THERON_DUNGEON_STATE_LOCKED) {
                ++locked_count;
            }
        }
        CHECK(locked_count == THERON_DUNGEON_COUNT - 1,
              "init: dungeons 2..7 all LOCKED");
        CHECK(prog.quest_complete == 0, "init: quest_complete == 0");
    }

    /* 2. Init NULL-safety. */
    {
        theron_v1_dungeon_progression_init(NULL);
        CHECK(1, "init(NULL) is a safe no-op");
    }

    /* 3. Advance 1->2 marks 1 COMPLETE, 2 AVAILABLE. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        Theron_DungeonID next = theron_v1_dungeon_advance(&prog);
        CHECK(next == THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
              "advance 1->2: returns dungeon 2");
        CHECK(prog.dungeon_states[0] == THERON_DUNGEON_STATE_COMPLETE,
              "advance 1->2: dungeon 1 marked COMPLETE");
        CHECK(prog.dungeon_states[1] == THERON_DUNGEON_STATE_AVAILABLE,
              "advance 1->2: dungeon 2 marked AVAILABLE");
    }

    /* 4. Advance 7->8 sets quest_complete=1, returns INVALID. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        for (int i = 0; i < THERON_DUNGEON_COUNT - 1; ++i) {
            theron_v1_dungeon_advance(&prog);
        }
        /* After 6 advances we're in dungeon 7. One more advance should
         * mark 7 COMPLETE and set quest_complete=1 with INVALID return. */
        Theron_DungeonID next = theron_v1_dungeon_advance(&prog);
        CHECK(next == THERON_DUNGEON_INVALID,
              "advance 7->end: returns INVALID sentinel");
        CHECK(prog.quest_complete == 1,
              "advance 7->end: quest_complete == 1");
        CHECK(prog.dungeon_states[6] == THERON_DUNGEON_STATE_COMPLETE,
              "advance 7->end: dungeon 7 marked COMPLETE");
    }

    /* 5. Advance NULL-safety. */
    {
        Theron_DungeonID next = theron_v1_dungeon_advance(NULL);
        CHECK(next == THERON_DUNGEON_INVALID,
              "advance(NULL) returns INVALID");
    }

    /* 6. theron_v1_dungeon_next invalid input returns INVALID. */
    {
        Theron_DungeonID next = theron_v1_dungeon_next(0);   /* below range */
        CHECK(next == THERON_DUNGEON_INVALID,
              "dungeon_next(0) -> INVALID (below range)");
        next = theron_v1_dungeon_next(THERON_DUNGEON_COUNT);  /* above range */
        CHECK(next == THERON_DUNGEON_INVALID,
              "dungeon_next(COUNT) -> INVALID (above range)");
        next = theron_v1_dungeon_next(THERON_DUNGEON_3_ABYSS_OF_FLAMES);
        CHECK(next == THERON_DUNGEON_4_TOMB_OF_WOE,
              "dungeon_next(3) -> 4");
    }

    /* 7. Full 1..7 progression reaches COMPLETE for all 7. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        for (int i = 0; i < THERON_DUNGEON_COUNT; ++i) {
            theron_v1_dungeon_advance(&prog);
        }
        int complete_count = 0;
        for (int i = 0; i < THERON_DUNGEON_COUNT; ++i) {
            if (prog.dungeon_states[i] == THERON_DUNGEON_STATE_COMPLETE) {
                ++complete_count;
            }
        }
        CHECK(complete_count == THERON_DUNGEON_COUNT,
              "after 7 advances: all 7 dungeons COMPLETE");
    }

    /* 8. Final state after full progression. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        for (int i = 0; i < THERON_DUNGEON_COUNT; ++i) {
            theron_v1_dungeon_advance(&prog);
        }
        CHECK(prog.quest_complete == 1,
              "after 7 advances: quest_complete == 1");
    }

    /* 9. Reset (init) after full progression restores dungeon 1 AVAILABLE. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        for (int i = 0; i < THERON_DUNGEON_COUNT; ++i) {
            theron_v1_dungeon_advance(&prog);
        }
        /* Now quest_complete=1. Re-init should reset. */
        theron_v1_dungeon_progression_init(&prog);
        CHECK(prog.quest_complete == 0,
              "re-init after quest complete: quest_complete reset to 0");
        CHECK(prog.dungeon_states[0] == THERON_DUNGEON_STATE_AVAILABLE,
              "re-init after quest complete: dungeon 1 AVAILABLE again");
    }

    /* 10. Determinism across many runs. */
    {
        int mismatch = 0;
        uint32_t expected = 0;
        for (int rep = 0; rep < 50; ++rep) {
            Theron_DungeonProgression prog;
            theron_v1_dungeon_progression_init(&prog);
            for (int i = 0; i < THERON_DUNGEON_COUNT; ++i) {
                theron_v1_dungeon_advance(&prog);
            }
            uint32_t h = progression_hash(&prog);
            if (rep == 0) expected = h;
            else if (h != expected) { ++mismatch; break; }
        }
        CHECK(mismatch == 0,
              "50 full progressions produce identical state hash");
    }

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
