#ifndef FIRESTAFF_DM2_V1_CCM_DISPATCH_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CCM_DISPATCH_PC34_COMPAT_H

/*
 * dm2_v1_ccm_dispatch_pc34_compat.h — DM2-005 source-exact CCM b_1a
 * dispatch matrix.
 *
 * skproject/SKULLWIN/c_creature.cpp:2930-3212 DM2_PROCEED_CCM reads the
 * creature command byte at offset 0x1a and routes it through an ordered
 * compare chain.  This module binds that chain verbatim: every possible
 * b_1a byte (0x00-0xFF) maps to exactly one source handler group, or to
 * NONE when the chain admits no handler (RG3L stays 0 and the creature
 * keeps its current state).
 *
 * The post-dispatch timing writeback is bound from the same function
 * (c_creature.cpp:3194-3206): when the dispatched handler returns 0 and
 * table1d613a[b_1a] & 3 != 0, the low gametick byte is written to the
 * creature byte at offset 0x04.  table1d613a itself is bound verbatim
 * from skproject/SKULLWIN/mdata.c:1615-1639 (86 entries, 0x00-0x55).
 *
 * Source: skproject/SKULLWIN/c_creature.cpp:2930-3212 (DM2_PROCEED_CCM)
 *         skproject/SKULLWIN/mdata.c:1615-1639       (table1d613a)
 *         skproject/SKULLWIN/mdata.h:30              (table1d613a extern)
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM2_V1_CCM_SRC_NONE = 0,             /* chain admits no handler (RG3L = 0) */
    DM2_V1_CCM_SRC_WALK_NOW,             /* 0x01,0x02,0x09 (skip00387) */
    DM2_V1_CCM_SRC_CCM03,                /* 0x03,0x04 */
    DM2_V1_CCM_SRC_JUMPS,                /* 0x05 DM2_CREATURE_JUMPS */
    DM2_V1_CCM_SRC_CCM06,                /* 0x06,0x07 */
    DM2_V1_CCM_SRC_ATTACKS_PARTY,        /* 0x08,0x26 (skip00388) */
    DM2_V1_CCM_SRC_STEAL_FROM_CHAMPION,  /* 0x0A */
    DM2_V1_CCM_SRC_CCM0B,                /* 0x0B */
    DM2_V1_CCM_SRC_CCM0C,                /* 0x0C,0x0D */
    DM2_V1_CCM_SRC_SHOOT_ITEM,           /* 0x0E,0x0F */
    DM2_V1_CCM_SRC_KILL_ON_TIMER_POSITION, /* 0x13 */
    DM2_V1_CCM_SRC_ROTATES_TARGET_CREATURE, /* 0x15,0x16 */
    DM2_V1_CCM_SRC_PLACE_MERCHANDISE,    /* 0x17 */
    DM2_V1_CCM_SRC_TAKE_MERCHANDISE,     /* 0x18 */
    DM2_V1_CCM_SRC_PUTS_DOWN_ITEM,       /* 0x19,0x29,0x2A,0x2D,0x2E (skip00386) */
    DM2_V1_CCM_SRC_TAKES_ITEM,           /* 0x1A,0x2B,0x2C (skip00389) */
    DM2_V1_CCM_SRC_CAST_SPELL,           /* 0x27,0x28 */
    DM2_V1_CCM_SRC_ACTIVATES_WALL,       /* 0x2F-0x31 */
    DM2_V1_CCM_SRC_USES_LADDER_HOLE,     /* 0x35-0x3A */
    DM2_V1_CCM_SRC_TRANSFORM,            /* 0x3B,0x3C */
    DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON,    /* 0x3D-0x40 */
    DM2_V1_CCM_SRC_1B7D5,                /* 0x55 DM2_1B7D5 */
    DM2_V1_CCM_SRC_HANDLER_COUNT
} DM2_V1_CcmSourceHandler;

/* table1d613a span: proven bytes cover b_1a 0x00-0x55. */
#define DM2_V1_CCM_TIMING_TABLE_COUNT 86

/* Source handler group for a b_1a command byte.  Returns
 * DM2_V1_CCM_SRC_NONE for every byte the compare chain skips. */
DM2_V1_CcmSourceHandler dm2_v1_ccm_dispatch_source_group(uint8_t b_1a);

/* Stable name for a source handler group ("NONE", "WALK_NOW", ...). */
const char *dm2_v1_ccm_dispatch_group_name(DM2_V1_CcmSourceHandler group);

/* table1d613a lookup.  Returns 0 for b_1a outside the proven 86-entry
 * span and sets *out_proven (when non-NULL) to 0; the source reads the
 * table unchecked, so Firestaff stays fail-closed beyond the proven
 * span instead of guessing adjacent data. */
uint8_t dm2_v1_ccm_dispatch_timing_flags(uint8_t b_1a, int *out_proven);

/* c_creature.cpp:3194-3206 — post-dispatch gametick writeback gate:
 * true when table1d613a[b_1a] & 3 != 0 (proven span only). */
int dm2_v1_ccm_dispatch_writes_gametick(uint8_t b_1a);

const char *dm2_v1_ccm_dispatch_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CCM_DISPATCH_PC34_COMPAT_H */
