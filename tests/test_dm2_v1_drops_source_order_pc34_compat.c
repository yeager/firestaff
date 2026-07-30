/*
 * test_dm2_v1_drops_source_order_pc34_compat.c — DM2-006 source-ordered
 * creature drop resolution gate.
 *
 * Source-locked coverage of:
 *   skproject/SKWINSPX/src/v4/skcrture.cpp:2084-2118
 *     DROP_CREATURE_POSSESSION (CREATURE_GENERATED_DROPS): slots
 *     0x0A..0x14 ascending, word 0 skipped, base=(w&15)+1,
 *     extra=(w&0x70)>>4, count += RAND16(extra+1) when extra != 0,
 *     item = w >> 7.
 *   skproject/SKWINSPX/src/v0/skdefine.h:898 (CREATURE_STAT_DROP_FIRST)
 *   skproject/SKULLWIN/c_random.cpp:13-31 (DM2_RAND/DM2_RAND16 LCG)
 *
 * Plus the death-path binding: a creature type with imported GDAT drop
 * words dies through dm2_v1_creature_death_check and the observer
 * receipts the source-ordered resolution; the data-free Thorn Demon
 * fallback is unchanged.
 */

#include "dm2_v1_creature.h"
#include "dm2_v1_drops.h"


#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

/* Independent LCG replica (c_random.cpp:13-31) for expected rolls. */
static uint32_t g_rep_state;

static uint32_t replica_rand(void)
{
    uint32_t v = g_rep_state * 0xbb40e62du + 11u;
    g_rep_state = v;
    return v >> 8;
}

static uint16_t replica_rand16(uint16_t n)
{
    return n == 0 ? 0 : (uint16_t)(replica_rand() % n);
}

/* Cavern Bat AI index (dm2_v1_creature.c g_ai_names[23]); no public
 * constant exists, so the gate keeps the source index literal. */
#define TEST_AI_CAVERN_BAT 23

int main(void)
{
    /* 1. Zero words skip every slot (source `continue`). */
    {
        uint16_t words[DM2_DROP_SLOT_COUNT] = {0};
        DM2_V1_DropRng rng;
        DM2_V1_DropSlotReceipt rc[DM2_DROP_SLOT_COUNT];
        int total = -1;
        dm2_v1_drops_rng_init(&rng);
        int admitted =
            dm2_v1_drops_resolve_source_slots(words, &rng, rc, &total);
        CHECK(admitted == 0 && total == 0, "all-zero table admits none");
        CHECK(rc[0].admitted == 0 && rc[0].field == DM2_DROP_SLOT_FIRST,
              "slot 0 receipt field");
        CHECK(rc[DM2_DROP_SLOT_COUNT - 1].field == DM2_DROP_SLOT_LAST,
              "last receipt field 0x14");
    }

    /* 2. Base-only word: no RNG consumed, count = (w&15)+1, id = w>>7. */
    {
        uint16_t words[DM2_DROP_SLOT_COUNT] = {0};
        DM2_V1_DropRng rng;
        DM2_V1_DropSlotReceipt rc[DM2_DROP_SLOT_COUNT];
        int total = 0;
        words[0] = (uint16_t)((5 << 7) | 2); /* item 5, base 3, no extra */
        dm2_v1_drops_rng_init(&rng);
        int admitted =
            dm2_v1_drops_resolve_source_slots(words, &rng, rc, &total);
        CHECK(admitted == 1 && total == 3, "base-only total 3");
        CHECK(rc[0].item_id == 5 && rc[0].base_count == 3 &&
                  rc[0].extra_roll == -1 && rc[0].final_count == 3,
              "base-only receipt");
        CHECK(rng.random == 0u, "base-only consumes no RNG");
    }

    /* 3. Extra roll consumes exactly one RAND16 per extra slot, in slot
     *    order, matching the independent replica. */
    {
        uint16_t words[DM2_DROP_SLOT_COUNT] = {0};
        DM2_V1_DropRng rng;
        DM2_V1_DropSlotReceipt rc[DM2_DROP_SLOT_COUNT];
        int total = 0;
        words[2] = (uint16_t)((9 << 7) | (3 << 4) | 0);  /* item 9, base 1, extra range 3 */
        words[5] = (uint16_t)((4 << 7) | (1 << 4) | 4);  /* item 4, base 5, extra range 1 */
        g_rep_state = 0;
        int expect_roll_a = replica_rand16(4);
        int expect_roll_b = replica_rand16(2);
        dm2_v1_drops_rng_init(&rng);
        int admitted =
            dm2_v1_drops_resolve_source_slots(words, &rng, rc, &total);
        CHECK(admitted == 2, "two admitted slots");
        CHECK(rc[2].extra_roll == expect_roll_a &&
                  rc[2].final_count == 1 + expect_roll_a,
              "slot 2 roll matches replica");
        CHECK(rc[5].extra_roll == expect_roll_b &&
                  rc[5].final_count == 5 + expect_roll_b,
              "slot 5 roll matches replica");
        CHECK(total == (1 + expect_roll_a) + (5 + expect_roll_b),
              "total accumulates in slot order");
        CHECK(rng.random == g_rep_state, "RNG stream consumed identically");
    }

    /* 4. NULL guards are fail-closed. */
    CHECK(dm2_v1_drops_resolve_source_slots(NULL, NULL, NULL, NULL) == 0,
          "NULL table rejected");

    /* 5. Death path: imported drop words drive the observer through the
     *    source-ordered resolution. */
    {
        uint16_t words[DM2_DROP_SLOT_COUNT] = {0};
        words[0] = (uint16_t)((7 << 7) | 0); /* item 7, count 1, no RNG */
        dm2_v1_creature_test_reset_instances();
        dm2_v1_creature_reset_death_observer();
        dm2_v1_creature_test_clear_ai_overrides();
        dm2_v1_creature_drop_rng_reset();
        dm2_v1_creature_test_set_drop_slots(TEST_AI_CAVERN_BAT, words);
        CHECK(dm2_v1_creature_drop_slots_loaded(TEST_AI_CAVERN_BAT) == 1,
              "drop slots bound");
        CHECK(dm2_v1_creature_drop_slot_word(TEST_AI_CAVERN_BAT, 0) ==
                  words[0],
              "slot word readable");

        int slot = dm2_v1_creature_spawn(TEST_AI_CAVERN_BAT, 3, 4, 0, 1, 0);
        CHECK(slot >= 0, "spawn");
        dm2_v1_creature_deal_damage(slot, 9999);
        dm2_v1_creature_tick();

        DM2_V1_CreatureDeathDropObserver obs;
        CHECK(dm2_v1_creature_last_death_drop(&obs) == 1, "death observed");
        CHECK(obs.source_ordered == 1, "source-ordered resolution ran");
        CHECK(obs.source_slots_admitted == 1, "one admitted slot");
        CHECK(obs.source_total_items == 1, "one item");
        CHECK(obs.dropped == 1 && obs.item_id == 7 && obs.count == 1,
              "observer carries first admitted item");
        dm2_v1_creature_test_clear_ai_overrides();
    }

    /* 6. No GDAT words means no generated drops.  DROP_CREATURE_POSSESSION
     *    has no creature-specific fallback. */
    {
        dm2_v1_creature_test_reset_instances();
        dm2_v1_creature_reset_death_observer();
        dm2_v1_creature_test_clear_ai_overrides();

        int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 5, 10, 0, 1, 0);
        CHECK(slot >= 0, "thorn demon spawn");
        dm2_v1_creature_deal_damage(slot, 9999);
        dm2_v1_creature_tick();

        DM2_V1_CreatureDeathDropObserver obs;
        CHECK(dm2_v1_creature_last_death_drop(&obs) == 1, "death observed");
        CHECK(obs.source_ordered == 0, "no imported source drop words");
        CHECK(obs.dropped == 0 && obs.item_id == 0 && obs.count == 0,
              "unbound drop data is rejected");
    }

    /* 7. Source evidence strings bound. */
    CHECK(strstr(dm2_v1_drops_source_evidence(), "skcrture.cpp:2084-2118") !=
              NULL,
          "drops evidence cites DROP_CREATURE_POSSESSION");

    if (g_failures == 0) {
        printf("PASS: dm2_v1_drops_source_order_pc34_compat\n");
        return 0;
    }
    fprintf(stderr, "FAILURES: %d\n", g_failures);
    return 1;
}
