/*
 * test_dm2_v1_ccm_dispatch_pc34_compat.c — DM2-005 source-exact CCM
 * b_1a dispatch matrix gate.
 *
 * Verifies the DM2_PROCEED_CCM compare-chain binding against skproject
 * anchors:
 *   c_creature.cpp:2930-3212  every b_1a byte maps to the source handler
 *                             group (or NONE where the chain skips)
 *   c_creature.cpp:3194-3206  gametick writeback gate (& 3)
 *   mdata.c:1615-1639         table1d613a bytes, 86-entry proven span
 *
 * The expected matrix below is an independent re-derivation of the
 * compare chain (byte ranges per handler), not a copy of the
 * implementation's switch.
 */

#include "dm2_v1_ccm_dispatch_pc34_compat.h"

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

/* Independent expected-group derivation from the source branch ranges. */
static DM2_V1_CcmSourceHandler expected_group(int b)
{
    if (b == 0x01 || b == 0x02 || b == 0x09) return DM2_V1_CCM_SRC_WALK_NOW;
    if (b == 0x03 || b == 0x04) return DM2_V1_CCM_SRC_CCM03;
    if (b == 0x05) return DM2_V1_CCM_SRC_JUMPS;
    if (b == 0x06 || b == 0x07) return DM2_V1_CCM_SRC_CCM06;
    if (b == 0x08 || b == 0x26) return DM2_V1_CCM_SRC_ATTACKS_PARTY;
    if (b == 0x0A) return DM2_V1_CCM_SRC_STEAL_FROM_CHAMPION;
    if (b == 0x0B) return DM2_V1_CCM_SRC_CCM0B;
    if (b == 0x0C || b == 0x0D) return DM2_V1_CCM_SRC_CCM0C;
    if (b == 0x0E || b == 0x0F) return DM2_V1_CCM_SRC_SHOOT_ITEM;
    if (b == 0x13) return DM2_V1_CCM_SRC_KILL_ON_TIMER_POSITION;
    if (b == 0x15 || b == 0x16) return DM2_V1_CCM_SRC_ROTATES_TARGET_CREATURE;
    if (b == 0x17) return DM2_V1_CCM_SRC_PLACE_MERCHANDISE;
    if (b == 0x18) return DM2_V1_CCM_SRC_TAKE_MERCHANDISE;
    if (b == 0x19 || b == 0x29 || b == 0x2A || b == 0x2D || b == 0x2E)
        return DM2_V1_CCM_SRC_PUTS_DOWN_ITEM;
    if (b == 0x1A || b == 0x2B || b == 0x2C) return DM2_V1_CCM_SRC_TAKES_ITEM;
    if (b == 0x27 || b == 0x28) return DM2_V1_CCM_SRC_CAST_SPELL;
    if (b >= 0x2F && b <= 0x31) return DM2_V1_CCM_SRC_ACTIVATES_WALL;
    if (b >= 0x35 && b <= 0x3A) return DM2_V1_CCM_SRC_USES_LADDER_HOLE;
    if (b == 0x3B || b == 0x3C) return DM2_V1_CCM_SRC_TRANSFORM;
    if (b >= 0x3D && b <= 0x40) return DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON;
    if (b == 0x55) return DM2_V1_CCM_SRC_1B7D5;
    return DM2_V1_CCM_SRC_NONE;
}

int main(void)
{
    /* 1. Full 256-byte matrix matches the independently derived chain. */
    for (int b = 0; b < 256; ++b) {
        DM2_V1_CcmSourceHandler got =
            dm2_v1_ccm_dispatch_source_group((uint8_t)b);
        if (got != expected_group(b)) {
            fprintf(stderr,
                    "FAIL: b_1a=0x%02X group %s expected %s\n", b,
                    dm2_v1_ccm_dispatch_group_name(got),
                    dm2_v1_ccm_dispatch_group_name(expected_group(b)));
            ++g_failures;
        }
    }

    /* 2. Group names are stable and non-NULL for every enum value. */
    for (int g = 0; g < DM2_V1_CCM_SRC_HANDLER_COUNT; ++g) {
        const char *name = dm2_v1_ccm_dispatch_group_name((DM2_V1_CcmSourceHandler)g);
        CHECK(name != NULL && name[0] != '\0' && name[0] != '?',
              "group name bound");
    }

    /* 3. table1d613a spot checks (mdata.c:1615-1639 verbatim bytes). */
    {
        static const struct {
            uint8_t b;
            uint8_t flags;
        } spots[] = {
            { 0x00, 0x08 }, { 0x01, 0x14 }, { 0x06, 0x10 }, { 0x08, 0x11 },
            { 0x0E, 0x12 }, { 0x15, 0x10 }, { 0x26, 0x11 }, { 0x27, 0x12 },
            { 0x28, 0x12 }, { 0x39, 0x14 }, { 0x54, 0x08 }, { 0x55, 0x10 },
        };
        for (size_t i = 0; i < sizeof(spots) / sizeof(spots[0]); ++i) {
            int proven = 0;
            uint8_t got =
                dm2_v1_ccm_dispatch_timing_flags(spots[i].b, &proven);
            CHECK(proven == 1, "timing byte inside proven span");
            if (got != spots[i].flags) {
                fprintf(stderr,
                        "FAIL: table1d613a[0x%02X]=0x%02X expected 0x%02X\n",
                        spots[i].b, got, spots[i].flags);
                ++g_failures;
            }
        }
    }

    /* 4. Bytes beyond the proven 86-entry span are fail-closed. */
    {
        int proven = 1;
        uint8_t got = dm2_v1_ccm_dispatch_timing_flags(0x56, &proven);
        CHECK(proven == 0 && got == 0, "0x56 outside proven span");
        got = dm2_v1_ccm_dispatch_timing_flags(0xFF, &proven);
        CHECK(proven == 0 && got == 0, "0xFF outside proven span");
    }

    /* 5. Gametick writeback gate follows (flags & 3) != 0, proven span
     *    only: 0x00 (0x08) no, 0x01 (0x14) no, 0x06 (0x10) no,
     *    0x0E (0x12) yes, 0x26 (0x11) yes, 0x2A (0x08) no,
     *    0x56 unproven no. */
    CHECK(dm2_v1_ccm_dispatch_writes_gametick(0x00) == 0,
          "0x00 no writeback");
    CHECK(dm2_v1_ccm_dispatch_writes_gametick(0x01) == 0,
          "0x01 no writeback");
    CHECK(dm2_v1_ccm_dispatch_writes_gametick(0x06) == 0,
          "0x06 no writeback");
    CHECK(dm2_v1_ccm_dispatch_writes_gametick(0x0E) == 1,
          "0x0E writeback");
    CHECK(dm2_v1_ccm_dispatch_writes_gametick(0x26) == 1,
          "0x26 writeback");
    CHECK(dm2_v1_ccm_dispatch_writes_gametick(0x2A) == 0,
          "0x2A no writeback");
    CHECK(dm2_v1_ccm_dispatch_writes_gametick(0x56) == 0,
          "unproven span never writes back");

    /* 6. Source evidence string is bound. */
    CHECK(dm2_v1_ccm_dispatch_source_evidence() != NULL &&
          strstr(dm2_v1_ccm_dispatch_source_evidence(),
                 "c_creature.cpp:2930-3212") != NULL,
          "source evidence cites DM2_PROCEED_CCM");

    if (g_failures == 0) {
        printf("PASS: dm2_v1_ccm_dispatch_pc34_compat\n");
        return 0;
    }
    fprintf(stderr, "FAILURES: %d\n", g_failures);
    return 1;
}
