/*
 * test_dm2_v1_spell_rune_lookup_pc34_compat.c — DM2-007 source rune-key
 * lookup and failure classification gate.
 *
 * Source-locked coverage of:
 *   skproject/SKULLWIN/c_events.cpp:2211-2264
 *     DM2_FIND_SPELL_BY_RUNES: query key packing (rune[0]<<24 ... max 4
 *     runes, zero-terminated), reverse table scan, 24-bit masked compare
 *     when the record key top byte is zero, full 32-bit compare when
 *     non-zero (exact-power lock).
 *   skproject/SKULLWIN/c_events.cpp:2282-2289
 *     mana = ((w6 >> 10) & 0x3f) * (cast_power + 0x12) / 0x18.
 *   skproject/SKULLWIN/c_events.cpp:2687-2733
 *     DM2_PROCEED_SPELL_FAILURE classes 0x10/0x20/0x30.
 *   skproject/SKULLWIN/c_events.cpp:2738-2786
 *     DM2_TRY_CAST_SPELL rune-clear / panel redraw rule.
 */

#include "dm2_v1_spell.h"

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

#define REC_KEY(r1, r2, r3) \
    ((uint32_t)(r1) << 16 | (uint32_t)(r2) << 8 | (uint32_t)(r3))

int main(void)
{
    /* 1. Query key packing follows the source loop. */
    {
        const uint8_t two[4] = { 0x61, 0x07, 0x00, 0x00 };
        const uint8_t three[4] = { 0x63, 0x0E, 0x08, 0x00 };
        const uint8_t four[6] = { 0x60, 0x01, 0x02, 0x03, 0x04, 0x00 };
        const uint8_t five_plus[7] = { 0x60, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00 };
        const uint8_t single[2] = { 0x60, 0x00 };

        CHECK(dm2_v1_spell_pack_query_key(two) == 0x61070000u,
              "two-rune key");
        CHECK(dm2_v1_spell_pack_query_key(three) == 0x630E0800u,
              "three-rune key");
        CHECK(dm2_v1_spell_pack_query_key(four) == 0x60010203u,
              "four-rune key");
        CHECK(dm2_v1_spell_pack_query_key(five_plus) == 0x60010203u,
              "packing stops after four runes");
        CHECK(dm2_v1_spell_pack_query_key(single) == 0u,
              "single-rune tail rejected like source NULL");
        CHECK(dm2_v1_spell_pack_query_key(NULL) == 0u, "NULL guard");
    }

    /* 2. Lookup: masked compare for top-byte-zero records strips the
     *    query power rune; exact-power locks need a full match. */
    {
        DM2_V1_SpellRecord records[3];
        const uint8_t cast_a[4] = { 0x61, 0x07, 0x0B, 0x00 }; /* power 0x61 */
        const uint8_t cast_b[4] = { 0x66, 0x07, 0x0B, 0x00 }; /* power 0x66 */
        const uint8_t locked[4] = { 0x63, 0x0E, 0x08, 0x00 };
        const uint8_t locked_lo[4] = { 0x60, 0x0E, 0x08, 0x00 };
        const uint8_t unknown[4] = { 0x61, 0x01, 0x01, 0x00 };

        memset(records, 0, sizeof(records));
        records[0].key = REC_KEY(0x07, 0x0B, 0x00);        /* any power */
        records[1].key = 0x63000000u | REC_KEY(0x0E, 0x08, 0x00); /* lock 0x63 */
        records[2].key = REC_KEY(0x07, 0x0B, 0x00);        /* duplicate key */

        /* Reverse scan: the later duplicate wins (source scans last→first). */
        CHECK(dm2_v1_spell_find_by_runes(records, 3, cast_a) == 2,
              "reverse scan prefers later entry");
        /* Masked compare ignores the query power rune. */
        CHECK(dm2_v1_spell_find_by_runes(records, 3, cast_b) == 2,
              "masked compare strips power rune");
        /* Exact-power lock: 0x63 matches, 0x60 does not. */
        CHECK(dm2_v1_spell_find_by_runes(records, 3, locked) == 1,
              "exact-power lock match");
        CHECK(dm2_v1_spell_find_by_runes(records, 3, locked_lo) == -1,
              "exact-power lock rejects other power");
        CHECK(dm2_v1_spell_find_by_runes(records, 3, unknown) == -1,
              "unknown combination miss");
        CHECK(dm2_v1_spell_find_by_runes(NULL, 3, cast_a) == -1 &&
                  dm2_v1_spell_find_by_runes(records, 0, cast_a) == -1,
              "NULL/empty table fail-closed");
    }

    /* 3. Mana cost formula (c_events.cpp:2282-2289). */
    {
        DM2_V1_SpellRecord rec;
        memset(&rec, 0, sizeof(rec));
        rec.w6 = (uint16_t)(2u << 10); /* power factor 2 */
        CHECK(dm2_v1_spell_record_mana_cost(&rec, 6) ==
                  2 * (6 + 0x12) / 0x18,
              "mana formula factor 2 power 6");
        CHECK(dm2_v1_spell_record_mana_cost(&rec, 0) ==
                  2 * 0x12 / 0x18,
              "mana formula power 0 truncates");
        rec.w6 = 0;
        CHECK(dm2_v1_spell_record_mana_cost(&rec, 6) == 0,
              "factor 0 costs 0");
        CHECK(dm2_v1_spell_record_mana_cost(NULL, 6) == -1,
              "NULL record fail-closed");
    }

    /* 4. Failure classification (c_events.cpp:2687-2733). */
    {
        DM2_V1_SpellFailureReceipt r;

        CHECK(dm2_v1_spell_proceed_failure(0x10, &r) == 0x10 && r.handled &&
                  r.status == -5 && r.status_written &&
                  r.glob_var == 0x45 && r.clears_runes,
              "class 0x10 low!=3 → status -5, glob 0x45");
        CHECK(dm2_v1_spell_proceed_failure(0x13, &r) == 0x13 && r.handled &&
                  r.status == -4 && r.glob_var == 0x45,
              "class 0x10 low==3 → status -4");
        CHECK(dm2_v1_spell_proceed_failure(0x20, &r) == 0x20 && r.handled &&
                  r.status == -3 && r.glob_var == 0x46 && r.clears_runes,
              "class 0x20 unknown runes → status -3, glob 0x46");
        CHECK(dm2_v1_spell_proceed_failure(0x30, &r) == 0x30 && r.handled &&
                  r.flask_pic_drawn && r.glob_var == 0x44 &&
                  !r.clears_runes && !r.status_written,
              "class 0x30 flask → pic, glob 0x44, runes kept");
        CHECK(dm2_v1_spell_proceed_failure(0x40, &r) == 0x40 && !r.handled &&
                  r.glob_var == -1,
              "class 0x40 unchanged, no side effect");
        CHECK(dm2_v1_spell_proceed_failure(0x00, &r) == 0x00 && !r.handled,
              "class 0x00 unchanged");
        CHECK(dm2_v1_spell_proceed_failure(0x10, NULL) == 0x10,
              "NULL receipt tolerated");
        CHECK(r.glob_update_bound == 0 || !r.handled,
              "glob var update receipted pending, never simulated");
    }

    /* 5. Fixed table retains mechanics but has no invented display text. */
    CHECK(dm2_v1_spell_count() == DM2_MAX_SPELL_ORIGINAL,
          "fixed table still 34");
    CHECK(dm2_v1_spell_name(0) == NULL, "fixed spell table has no text owner");

    if (g_failures == 0) {
        printf("PASS: dm2_v1_spell_rune_lookup_pc34_compat\n");
        return 0;
    }
    fprintf(stderr, "FAILURES: %d\n", g_failures);
    return 1;
}
