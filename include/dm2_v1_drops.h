#ifndef FIRESTAFF_DM2_V1_DROPS_H
#define FIRESTAFF_DM2_V1_DROPS_H
#include <stdint.h>

#include "dm2_v1_asset_loader.h"

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

/* ── Source-ordered slot resolution (DM2-006) ──────────────────────────
 * skproject/SKWINSPX/src/v4/skcrture.cpp:2084-2118 DROP_CREATURE_POSSESSION
 * resolves the 11 creature GDAT word fields CREATURE_STAT_DROP_FIRST
 * (0x0A) .. CREATURE_STAT_DROP_LAST (0x14) in ascending slot order:
 *
 *   bp06 = QUERY_GDAT_CREATURE_WORD_VALUE(type, slot);
 *   if (bp06 == 0) continue;
 *   bp0c = (bp06 & 15) + 1;               // base item count
 *   bp0e = (bp06 & 0x0070) >> 4;          // additional random range
 *   if (bp0e != 0) bp0c += RAND16(bp0e + 1);
 *   bp06 >>= 7;                           // item record type
 *   while (bp0c-- != 0) { ALLOC_NEW_DBITEM(bp06); ... }
 *
 * The per-item direction draw (RAND01 near the party cell, RAND02
 * otherwise) happens only after ALLOC_NEW_DBITEM succeeds; Firestaff has
 * no bound DB item allocator yet, so this receipt layer resolves counts
 * and item ids and consumes exactly the count-roll RNG the source
 * consumes, while item-record creation stays fail-closed. */

typedef struct {
    int field;            /* GDAT field 0x0A..0x14 */
    int admitted;         /* 1 when the slot word is non-zero */
    uint16_t word;        /* raw slot word */
    int item_id;          /* word >> 7 */
    int base_count;       /* (word & 15) + 1 */
    int extra_range;      /* (word & 0x70) >> 4 */
    int extra_roll;       /* RAND16(extra_range + 1), -1 when not drawn */
    int final_count;      /* base_count + extra_roll (or base_count) */
} DM2_V1_DropSlotReceipt;

/* DM2-006 RNG stream.  skproject/SKULLWIN/c_random.cpp:13-31 DM2_RAND /
 * DM2_RAND16 is a self-contained LCG (state * 0xbb40e62d + 11, >> 8);
 * the drop module keeps its own copy so this header does not drag the
 * whole skproject-core translation unit into every drops consumer. */
typedef struct {
    uint32_t random;      /* c_randomdata::random; init state 0 */
} DM2_V1_DropRng;

void dm2_v1_drops_rng_init(DM2_V1_DropRng *rng);

/* Source LCG draws (c_random.cpp:13-47): DM2_RAND16(n) is
 * CUTX16(DM2_RAND()) % n, DM2_RANDBIT (was RAND01) is DM2_RAND() & 1,
 * DM2_RANDDIR (was RAND02) is DM2_RAND() & 3.  Each draw advances the
 * stream exactly once.  dm2_v1_drops_rand16 applies the modulo to the
 * full 24-bit draw, which is identical to the source's CUTX16 form
 * whenever n divides 2^16 (all current drops uses: 2 and 256);
 * consumers with other moduli must build on the raw 24-bit draw from
 * dm2_v1_drops_rand24 and apply CUTX16 first. */
uint16_t dm2_v1_drops_rand16(DM2_V1_DropRng *rng, uint16_t n);
uint16_t dm2_v1_drops_randbit(DM2_V1_DropRng *rng);
uint16_t dm2_v1_drops_randdir(DM2_V1_DropRng *rng);

/* Raw 24-bit DM2_RAND draw (c_random.cpp:13-21): advances the stream
 * once and returns (state * 0xbb40e62d + 11) >> 8. */
uint32_t dm2_v1_drops_rand24(DM2_V1_DropRng *rng);

/* dm2_v1_drops_resolve_source_slots — resolve all 11 slots in source
 * order against the source LCG (c_random.cpp DM2_RAND16).
 * Returns the number of admitted (non-zero) slots; *out_total receives
 * the summed final_count.  out_receipts may be NULL. */
int dm2_v1_drops_resolve_source_slots(
    const uint16_t slot_words[DM2_DROP_SLOT_COUNT],
    DM2_V1_DropRng *rng,
    DM2_V1_DropSlotReceipt out_receipts[DM2_DROP_SLOT_COUNT],
    int *out_total);

/* ── Real-data GDAT drop route (Lane E, cycle 16) ─────────────────────────
 * skproject/SKWINSPX/src/v4/skcrture.cpp:2092-2100 reads the creature's drop
 * words straight from GDAT CREATURES word fields 0x0A..0x14; absent optional
 * fields read as 0 and the source `continue`s on word 0.  This route pulls
 * the eleven words from a verified GRAPHICS.DAT loader and resolves them in
 * source order (DROP_CREATURE_POSSESSION), so drop tables are driven by real
 * GDAT evidence wherever it exists.  Fail-closed: without a loaded GDAT
 * session, or for an out-of-range creature type, nothing is resolved. */

typedef struct {
    uint8_t valid;
    uint8_t rejected_no_loader;
    uint8_t rejected_type_out_of_range;
    uint8_t creature_type;
    uint8_t words_present;   /* slots whose GDAT word field exists */
    uint8_t admitted;        /* slots with a non-zero drop word */
    int first_item_id;       /* word >> 7 of the first admitted slot, else 0 */
    int total_count;         /* summed final_count over admitted slots */
} DM2_V1_DropGdatReceipt;

/* Resolve a creature's drop table from a verified GDAT loader in source
 * order.  rng drives the per-slot extra-count RAND16 draws (caller owns the
 * stream; DropTableSeed-style seeding happens before the call).  Returns the
 * number of admitted slots; out_receipts may be NULL. */
int dm2_v1_drops_resolve_gdat_creature_drops(
    const DM2_V1_AssetLoader *loader,
    int creature_type,
    DM2_V1_DropRng *rng,
    DM2_V1_DropSlotReceipt out_receipts[DM2_DROP_SLOT_COUNT],
    DM2_V1_DropGdatReceipt *out_receipt);

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
