/* dm2_v1_drops.c — DM2 V1 Drop System
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, SKWin.GDAT2.InternalCodes.txt
 * skproject/SKWIN/SkGlobal.h:636 (EXTENDED_GDAT_CATEGORIES)
 * docs/dm2_dungeon_design.md
 *
 * DM2: 11 drop slots per creature (GDAT 0x0A-0x14). DropTableSeed RNG.
 * DM1: 1 drop slot per creature.
 * Thorn Demon drops sellable worm food (steak).
 */

#include "dm2_v1_drops.h"
#include <stdlib.h>

/* dm2_v1_drops_generate — generate drop from creature drop table
 * Source: SKWin.GDAT2.InternalCodes.txt, docs/dm2_dungeon_design.md
 *
 * Drop determination:
 *   1. For each slot (0-10), check random flags in GDAT entry
 *   2. Roll against drop probability using DropTableSeed as RNG seed
 *   3. If successful, resolve item_id + count
 *   4. Return first successful drop (or all if merge mode)
 *
 * Real implementation: DM2 parses GDAT CREATURE category 0x0A sub-entries
 *   at offsets 0x0A-0x14. Each sub-entry: item_id(16) + count(8) + flags(8)
 *   flags encode: probability weight, random count range, required flag bits.
 *
 * Stub: simple RNG. Real implementation reads GDAT slot data. */
int dm2_v1_drops_generate(const DM2_V1_DropTable *table, uint32_t rng_state,
    DM2_DropEntry *out_drop) {
    if (!table || !out_drop) return 0;
    (void)rng_state;

    /* Stub: iterate drop slots, pick first non-empty.
     * Real: check slot random flags against RNG roll. */
    for (int i = 0; i < DM2_DROP_SLOT_COUNT; i++) {
        const DM2_DropEntry *slot = &table->slots[i];
        if (slot->item_id != 0) {
            *out_drop = *slot;
            return 1;  /* one drop returned */
        }
    }
    return 0;  /* no drop */
}

const char *dm2_v1_drops_source_evidence(void) {
    return
        "DM2 V1 Drop System — Phase 6 source-lock\n"
        "ReDMCSB: SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: SKWin.GDAT2.InternalCodes.txt (11 drop slots 0x0A-0x14 per creature)\n"
        "Source: skproject/SKWIN/SkGlobal.h:636 (EXTENDED_GDAT_CATEGORIES, CREATURE_AI_TAB_SIZE=64)\n"
        "Source: docs/dm2_dungeon_design.md (11 drop slots, DropTableSeed RNG)\n"
        "Source: docs/dm2_characters.md (Thorn Demon worm food drops)\n"
        "DM1 comparison: single drop slot per creature\n"
        "DM2 comparison: 11 drop slots via GDAT category 0x0A, DropTableSeed per-creature RNG\n"
        "DM2 special: Thorn Demon (AI 19) drops sellable \"steak\" (DM2_DROP_THORN_DEMON_WORM_FOOD)\n";
}