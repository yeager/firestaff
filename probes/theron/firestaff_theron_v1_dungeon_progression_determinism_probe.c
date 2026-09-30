/*
 * firestaff_theron_v1_dungeon_progression_determinism_probe.c
 * =============================================================
 *
 * Theron V1 dungeon-progression determinism probe (Tier 4 #20 polish).
 *
 * Verifies fail-closed progression projection in
 * theron_v1_dungeon_progression.c. Stage focus and provisional host item bits
 * cannot manufacture original T900 completion. Only the six captured source
 * campaign bits can restore completed dungeons; the final-stage consumer is
 * still unbound.
 *
 * Source-lock:
 *   - THQUEST.ASM T080 (between-dungeon save/load)
 *   - ReDMCSB analogue siblings GROUP.C / CLIKMENU.C
 *   - src/theron/theron_v1_dungeon_progression.c
 *
 * Coverage includes:
 *   1. Init: dungeon 1 AVAILABLE, others LOCKED, quest_complete=0.
 *   2. Init NULL-safety.
 *   3. An unverified advance does not mutate completion state.
 *   4. Six source-proven completion bits unlock dungeon 7.
 *   5. A host-supplied final bit cannot complete the campaign.
 *   6. Advance NULL-safety.
 *   7. theron_v1_dungeon_next invalid input returns INVALID.
 *   8. Only the first six source-proven dungeons project COMPLETE.
 *   9. Final state remains quest-incomplete without original evidence.
 *  10. Reset (init) restores dungeon 1 AVAILABLE.
 *  11. Determinism: identical source masks produce identical state hashes.
 *
 * Run:
 *   ./build/firestaff_theron_v1_dungeon_progression_determinism_probe
 *
 * Pass: all fail-closed and deterministic invariants.
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
        CHECK(prog.current_dungeon == THERON_DUNGEON_1_AKUTUBA,
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

    /* 3. Advance 1 marks 1 COMPLETE, unlocks 2..6, keeps 7 LOCKED. */
    {
        Theron_DungeonProgression prog;
        int middle_available = 0;
        theron_v1_dungeon_progression_init(&prog);
        theron_v1_dungeon_progression_apply_campaign_completion(&prog, 0x01u);
        Theron_DungeonID next = theron_v1_dungeon_advance(&prog);
        CHECK(next == THERON_DUNGEON_2_DRATOR,
              "advance 1: focus returns dungeon 2");
        CHECK(prog.dungeon_states[0] == THERON_DUNGEON_STATE_COMPLETE,
              "advance 1: dungeon 1 marked COMPLETE");
        for (int i = 1; i <= 5; ++i) {
            if (prog.dungeon_states[i] == THERON_DUNGEON_STATE_AVAILABLE) {
                ++middle_available;
            }
        }
        CHECK(middle_available == 5,
              "advance 1: dungeons 2..6 marked AVAILABLE");
        CHECK(prog.dungeon_states[6] == THERON_DUNGEON_STATE_LOCKED,
              "advance 1: dungeon 7 remains LOCKED");
    }

    /* 4. After six completed dungeons, dungeon 7 becomes AVAILABLE. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        uint8_t campaign_mask = 0u;
        for (int i = 0; i < THERON_DUNGEON_COUNT - 1; ++i) {
            Theron_DungeonID current = prog.current_dungeon;
            campaign_mask |= (uint8_t)(1u << (current - 1));
            theron_v1_dungeon_progression_apply_campaign_completion(
                &prog, campaign_mask);
            theron_v1_dungeon_advance(&prog);
        }
        CHECK(prog.dungeon_states[6] == THERON_DUNGEON_STATE_AVAILABLE,
              "after six advances: dungeon 7 AVAILABLE");
        CHECK(prog.current_dungeon == THERON_DUNGEON_7_DEMON,
              "after six advances: focus moves to dungeon 7");
    }

    /* 5. Exhausting choices does not authenticate final-stage completion. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        uint8_t campaign_mask = 0u;
        for (int i = 0; i < THERON_DUNGEON_COUNT - 1; ++i) {
            Theron_DungeonID current = prog.current_dungeon;
            campaign_mask |= (uint8_t)(1u << (current - 1));
            theron_v1_dungeon_progression_apply_campaign_completion(
                &prog, campaign_mask);
            theron_v1_dungeon_advance(&prog);
        }
        theron_v1_dungeon_progression_apply_campaign_completion(
            &prog, 0x7fu);
        Theron_DungeonID next = theron_v1_dungeon_advance(&prog);
        CHECK(next == THERON_DUNGEON_INVALID,
              "unverified final advance returns INVALID sentinel");
        CHECK(!theron_v1_quest_complete(&prog),
              "unverified final advance leaves quest incomplete");
    }

    /* 6. Advance NULL-safety. */
    {
        Theron_DungeonID next = theron_v1_dungeon_advance(NULL);
        CHECK(next == THERON_DUNGEON_INVALID,
              "advance(NULL) returns INVALID");
    }

    /* 7. theron_v1_dungeon_next invalid input returns INVALID. */
    {
        Theron_DungeonID next = theron_v1_dungeon_next(0);   /* below range */
        CHECK(next == THERON_DUNGEON_INVALID,
              "dungeon_next(0) -> INVALID (below range)");
        next = theron_v1_dungeon_next(THERON_DUNGEON_COUNT);  /* above range */
        CHECK(next == THERON_DUNGEON_INVALID,
              "dungeon_next(COUNT) -> INVALID (above range)");
        next = theron_v1_dungeon_next(THERON_DUNGEON_3_FORMIC);
        CHECK(next == THERON_DUNGEON_4_SARMON,
              "dungeon_next(3) -> 4");
    }

    /* 7. Full 1..7 progression reaches COMPLETE for all 7. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        uint8_t campaign_mask = 0u;
        for (int i = 0; i < THERON_DUNGEON_COUNT - 1; ++i) {
            Theron_DungeonID current = prog.current_dungeon;
            campaign_mask |= (uint8_t)(1u << (current - 1));
            theron_v1_dungeon_progression_apply_campaign_completion(
                &prog, campaign_mask);
            theron_v1_dungeon_advance(&prog);
        }
        int complete_count = 0;
        for (int i = 0; i < THERON_DUNGEON_COUNT; ++i) {
            if (prog.dungeon_states[i] == THERON_DUNGEON_STATE_COMPLETE) {
                ++complete_count;
            }
        }
        CHECK(complete_count == THERON_DUNGEON_COUNT - 1,
              "only six source-proven dungeons project COMPLETE");
    }

    /* 8. Final state after full progression. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        uint8_t campaign_mask = 0u;
        for (int i = 0; i < THERON_DUNGEON_COUNT - 1; ++i) {
            Theron_DungeonID current = prog.current_dungeon;
            campaign_mask |= (uint8_t)(1u << (current - 1));
            theron_v1_dungeon_progression_apply_campaign_completion(
                &prog, campaign_mask);
            theron_v1_dungeon_advance(&prog);
        }
        CHECK(!theron_v1_quest_complete(&prog),
              "six advances do not claim campaign completion");
    }

    /* 9. Reset (init) after full progression restores dungeon 1 AVAILABLE. */
    {
        Theron_DungeonProgression prog;
        theron_v1_dungeon_progression_init(&prog);
        uint8_t campaign_mask = 0u;
        for (int i = 0; i < THERON_DUNGEON_COUNT - 1; ++i) {
            Theron_DungeonID current = prog.current_dungeon;
            campaign_mask |= (uint8_t)(1u << (current - 1));
            theron_v1_dungeon_progression_apply_campaign_completion(
                &prog, campaign_mask);
            theron_v1_dungeon_advance(&prog);
        }
        /* Re-init clears the source-projected campaign state. */
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
            uint8_t campaign_mask = 0u;
            for (int i = 0; i < THERON_DUNGEON_COUNT - 1; ++i) {
                Theron_DungeonID current = prog.current_dungeon;
                campaign_mask |= (uint8_t)(1u << (current - 1));
                theron_v1_dungeon_progression_apply_campaign_completion(
                    &prog, campaign_mask);
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
