/*
 * dm2_v1_ccm_dispatch_pc34_compat.c — DM2-005 source-exact CCM b_1a
 * dispatch matrix.
 *
 * The group table below re-encodes the ordered compare chain of
 * skproject/SKULLWIN/c_creature.cpp:2930-3212 (DM2_PROCEED_CCM) one
 * branch at a time; the comments keep the source skip-flag names so the
 * mapping can be audited against the decompilation directly.
 *
 *   b_1a < 0x17:
 *     < 0x09: < 0x05: >= 0x01 -> (> 0x02 ? CCM03 : skip00387)
 *             == 0x05 -> JUMPS; 0x06/0x07 -> CCM06; 0x08 -> skip00388
 *     == 0x09 -> skip00387
 *     < 0x0C: 0x0A -> STEAL_FROM_CHAMPION; 0x0B -> CCM0B
 *     0x0C/0x0D -> CCM0C
 *     < 0x13: 0x0E/0x0F -> SHOOT_ITEM; 0x10-0x12 -> (no branch)
 *     == 0x13 -> KILL_ON_TIMER_POSITION; 0x14 -> (no branch)
 *     >= 0x15 -> ROTATES_TARGET_CREATURE (0x15/0x16)
 *   == 0x17 -> PLACE_MERCHANDISE
 *   > 0x17:
 *     < 0x2B: < 0x1A: 0x18 -> TAKE_MERCHANDISE; 0x19 -> skip00386
 *             == 0x1A -> skip00389
 *             < 0x27: 0x26 -> skip00388; 0x1B-0x25 -> (no branch)
 *             0x27/0x28 -> CAST_SPELL; 0x29/0x2A -> skip00386
 *     0x2B/0x2C -> skip00389
 *     < 0x35: 0x2D/0x2E -> skip00386; 0x2F-0x31 -> ACTIVATES_WALL;
 *             0x32-0x34 -> (no branch)
 *     0x35-0x3A -> USES_LADDER_HOLE
 *     0x3B/0x3C -> TRANSFORM
 *     0x3D-0x40 -> EXPLODE_OR_SUMMON
 *     == 0x55 -> DM2_1B7D5
 *   skip00386 -> PUTS_DOWN_ITEM; skip00387 -> WALK_NOW;
 *   skip00388 -> ATTACKS_PARTY; skip00389 -> TAKES_ITEM.
 *
 * Source: skproject/SKULLWIN/c_creature.cpp:2930-3212 (DM2_PROCEED_CCM)
 *         skproject/SKULLWIN/mdata.c:1615-1639       (table1d613a)
 */

#include "dm2_v1_ccm_dispatch_pc34_compat.h"

/* table1d613a, bound verbatim from skproject/SKULLWIN/mdata.c:1615-1639.
 * 86 entries indexed by the b_1a command byte; bit fields consumed at
 * c_creature.cpp:3196 (& 3), c_creature.cpp:1739/2874 (& 4),
 * c_ai.cpp:5432 (& 4), c_ai.cpp:5949 (& 0x10), c_move.cpp:695 (& 4). */
static const uint8_t s_timing_table[DM2_V1_CCM_TIMING_TABLE_COUNT] = {
    0x08, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x10, 0x10,
    0x11, 0x14, 0x11, 0x08,
    0x08, 0x08, 0x12, 0x12,
    0x08, 0x10, 0x08, 0x10,
    0x10, 0x10, 0x10, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x11, 0x12,
    0x12, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x10, 0x10,
    0x10, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x10,
    0x10, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x10
};

DM2_V1_CcmSourceHandler dm2_v1_ccm_dispatch_source_group(uint8_t b_1a)
{
    if (b_1a < 0x17) {
        if (b_1a < 0x09) {
            if (b_1a < 0x05) {
                if (b_1a >= 0x01) {
                    if (b_1a > 0x02) return DM2_V1_CCM_SRC_CCM03;
                    return DM2_V1_CCM_SRC_WALK_NOW; /* skip00387 */
                }
                return DM2_V1_CCM_SRC_NONE; /* 0x00: no branch taken */
            }
            if (b_1a <= 0x05) return DM2_V1_CCM_SRC_JUMPS;
            if (b_1a <= 0x07) return DM2_V1_CCM_SRC_CCM06;
            return DM2_V1_CCM_SRC_ATTACKS_PARTY; /* 0x08: skip00388 */
        }
        if (b_1a <= 0x09) return DM2_V1_CCM_SRC_WALK_NOW; /* skip00387 */
        if (b_1a < 0x0c) {
            if (b_1a <= 0x0a) return DM2_V1_CCM_SRC_STEAL_FROM_CHAMPION;
            return DM2_V1_CCM_SRC_CCM0B;
        }
        if (b_1a <= 0x0d) return DM2_V1_CCM_SRC_CCM0C;
        if (b_1a < 0x13) {
            if (b_1a <= 0x0f) return DM2_V1_CCM_SRC_SHOOT_ITEM;
            return DM2_V1_CCM_SRC_NONE; /* 0x10-0x12: no branch taken */
        }
        if (b_1a <= 0x13) return DM2_V1_CCM_SRC_KILL_ON_TIMER_POSITION;
        if (b_1a >= 0x15) return DM2_V1_CCM_SRC_ROTATES_TARGET_CREATURE;
        return DM2_V1_CCM_SRC_NONE; /* 0x14: no branch taken */
    }
    if (b_1a <= 0x17) return DM2_V1_CCM_SRC_PLACE_MERCHANDISE;
    if (b_1a < 0x2b) {
        if (b_1a < 0x1a) {
            if (b_1a <= 0x18) return DM2_V1_CCM_SRC_TAKE_MERCHANDISE;
            return DM2_V1_CCM_SRC_PUTS_DOWN_ITEM; /* 0x19: skip00386 */
        }
        if (b_1a <= 0x1a) return DM2_V1_CCM_SRC_TAKES_ITEM; /* skip00389 */
        if (b_1a < 0x27) {
            if (b_1a == 0x26) return DM2_V1_CCM_SRC_ATTACKS_PARTY;
            return DM2_V1_CCM_SRC_NONE; /* 0x1B-0x25: no branch taken */
        }
        if (b_1a <= 0x28) return DM2_V1_CCM_SRC_CAST_SPELL;
        return DM2_V1_CCM_SRC_PUTS_DOWN_ITEM; /* 0x29/0x2A: skip00386 */
    }
    if (b_1a <= 0x2c) return DM2_V1_CCM_SRC_TAKES_ITEM; /* skip00389 */
    if (b_1a < 0x35) {
        if (b_1a <= 0x2e) return DM2_V1_CCM_SRC_PUTS_DOWN_ITEM;
        if (b_1a <= 0x31) return DM2_V1_CCM_SRC_ACTIVATES_WALL;
        return DM2_V1_CCM_SRC_NONE; /* 0x32-0x34: no branch taken */
    }
    if (b_1a <= 0x3a) return DM2_V1_CCM_SRC_USES_LADDER_HOLE;
    if (b_1a < 0x3d) return DM2_V1_CCM_SRC_TRANSFORM;
    if (b_1a <= 0x40) return DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON;
    if (b_1a == 0x55) return DM2_V1_CCM_SRC_1B7D5;
    return DM2_V1_CCM_SRC_NONE;
}

const char *dm2_v1_ccm_dispatch_group_name(DM2_V1_CcmSourceHandler group)
{
    switch (group) {
        case DM2_V1_CCM_SRC_NONE: return "NONE";
        case DM2_V1_CCM_SRC_WALK_NOW: return "WALK_NOW";
        case DM2_V1_CCM_SRC_CCM03: return "CCM03";
        case DM2_V1_CCM_SRC_JUMPS: return "JUMPS";
        case DM2_V1_CCM_SRC_CCM06: return "CCM06";
        case DM2_V1_CCM_SRC_ATTACKS_PARTY: return "ATTACKS_PARTY";
        case DM2_V1_CCM_SRC_STEAL_FROM_CHAMPION: return "STEAL_FROM_CHAMPION";
        case DM2_V1_CCM_SRC_CCM0B: return "CCM0B";
        case DM2_V1_CCM_SRC_CCM0C: return "CCM0C";
        case DM2_V1_CCM_SRC_SHOOT_ITEM: return "SHOOT_ITEM";
        case DM2_V1_CCM_SRC_KILL_ON_TIMER_POSITION:
            return "KILL_ON_TIMER_POSITION";
        case DM2_V1_CCM_SRC_ROTATES_TARGET_CREATURE:
            return "ROTATES_TARGET_CREATURE";
        case DM2_V1_CCM_SRC_PLACE_MERCHANDISE: return "PLACE_MERCHANDISE";
        case DM2_V1_CCM_SRC_TAKE_MERCHANDISE: return "TAKE_MERCHANDISE";
        case DM2_V1_CCM_SRC_PUTS_DOWN_ITEM: return "PUTS_DOWN_ITEM";
        case DM2_V1_CCM_SRC_TAKES_ITEM: return "TAKES_ITEM";
        case DM2_V1_CCM_SRC_CAST_SPELL: return "CAST_SPELL";
        case DM2_V1_CCM_SRC_ACTIVATES_WALL: return "ACTIVATES_WALL";
        case DM2_V1_CCM_SRC_USES_LADDER_HOLE: return "USES_LADDER_HOLE";
        case DM2_V1_CCM_SRC_TRANSFORM: return "TRANSFORM";
        case DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON: return "EXPLODE_OR_SUMMON";
        case DM2_V1_CCM_SRC_1B7D5: return "DM2_1B7D5";
        default: return "?";
    }
}

uint8_t dm2_v1_ccm_dispatch_timing_flags(uint8_t b_1a, int *out_proven)
{
    if (b_1a >= DM2_V1_CCM_TIMING_TABLE_COUNT) {
        if (out_proven) *out_proven = 0;
        return 0;
    }
    if (out_proven) *out_proven = 1;
    return s_timing_table[b_1a];
}

int dm2_v1_ccm_dispatch_writes_gametick(uint8_t b_1a)
{
    int proven = 0;
    uint8_t flags = dm2_v1_ccm_dispatch_timing_flags(b_1a, &proven);
    /* c_creature.cpp:3196-3205 — writeback only when (flags & 3) != 0. */
    return proven && (flags & 0x03u) != 0u;
}

const char *dm2_v1_ccm_dispatch_source_evidence(void)
{
    return
        "DM2-005 CCM b_1a dispatch matrix — skproject source-lock\n"
        "Source: skproject/SKULLWIN/c_creature.cpp:2930-3212 (DM2_PROCEED_CCM)\n"
        "Source: skproject/SKULLWIN/c_creature.cpp:3194-3206 (gametick writeback)\n"
        "Source: skproject/SKULLWIN/mdata.c:1615-1639 (table1d613a, 86 bytes)\n"
        "Source: skproject/SKULLWIN/mdata.h:30 (table1d613a extern)\n"
        "Source: skproject/SKULLWIN/c_ai.cpp:5432/5949 (timing bits 0x04/0x10)\n"
        "Source: skproject/SKULLWIN/c_move.cpp:695 (timing bit 0x04)\n";
}
