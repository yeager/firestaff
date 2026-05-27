#ifndef FIRESTAFF_DM2_V1_DROPS_H
#define FIRESTAFF_DM2_V1_DROPS_H
#include <stdint.h>

/* DM2 V1 — Drop System
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, SKWin.GDAT2.InternalCodes.txt
 * skproject/SKWIN/SkGlobal.h:636 (EXTENDED_GDAT_CATEGORIES)
 * docs/dm2_dungeon_design.md
 *
 * DM2 extends creature drop tables from 1 slot (DM1) to 11 slots.
 * Each slot: item ID + count + random flags.
 * DropTableSeed in GDAT controls drop RNG per creature.
 *
 * DM1: single drop slot in creature record.
 * DM2: 11 drop slots (0x0A–0x14 = indices 10-20) via GDAT category 0x0A.
 */

/* ── Drop slot range ───────────────────────────────────────────────────
 * Source: SKWin.GDAT2.InternalCodes.txt, CREATURE section
 * 0x0A through 0x14 = 11 drop slots per creature */

#define DM2_DROP_SLOT_COUNT   11   /* per creature in DM2 */
#define DM2_DROP_SLOT_FIRST   10  /* GDAT offset 0x0A */
#define DM2_DROP_SLOT_LAST    20  /* GDAT offset 0x14 */

/* ── Drop table seed field (GDAT per-creature) ─────────────────────────
 * Source: docs/dm2_dungeon_design.md
 * DropTableSeed controls RNG for drop determination.
 * Per-slot random flags: varies by slot (specific encoding in GDAT). */

/* ── Drop resolution result ──────────────────────────────────────────── */

typedef struct {
    int item_id;          /* GDAT item ID or 0 if no drop */
    int count;            /* quantity */
    int random_flags;     /* random/variation encoding */
} DM2_DropEntry;

/* ── Drop table struct ──────────────────────────────────────────────────
 * Source: SKWin.GDAT2.InternalCodes.txt (11 slots per creature)
 * Extended GDAT category 0x0A CREATURE has 11 sub-slots (0x0A-0x14) */

typedef struct {
    DM2_DropEntry slots[DM2_DROP_SLOT_COUNT];
    uint16_t drop_seed;   /* DropTableSeed from GDAT */
    uint8_t  slot_count;  /* actual used slots (0-11) */
} DM2_V1_DropTable;

/* ── Special drops ────────────────────────────────────────────────────
 * Source: docs/dm2_characters.md (Thorn Demon worm food), SKWinCore.cpp */

#define DM2_DROP_THORN_DEMON_WORM_FOOD  1  /* sellable "steak" from Thorn Demon */

/* ── Public API ──────────────────────────────────────────────────────── */

int dm2_v1_drops_generate(const DM2_V1_DropTable *table, uint32_t rng_state,
    DM2_DropEntry *out_drop);
const char *dm2_v1_drops_source_evidence(void);

#endif /* FIRESTAFF_DM2_V1_DROPS_H */