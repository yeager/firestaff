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
#include <string.h>

/* skproject/SKULLWIN/c_random.cpp:5-31 — RANDOM_MAGIC LCG.
 * c_randomdata::init sets random = 0; DM2_RAND returns
 * (state * 0xbb40e62d + 11) >> 8; DM2_RAND16(n) is DM2_RAND() % n. */
#define DM2_V1_DROPS_RANDOM_MAGIC 0xbb40e62du

void dm2_v1_drops_rng_init(DM2_V1_DropRng *rng) {
    if (!rng) return;
    rng->random = 0u;
}

static uint32_t dm2_v1_drops_rand(DM2_V1_DropRng *rng) {
    uint32_t value = rng->random * DM2_V1_DROPS_RANDOM_MAGIC + 11u;
    rng->random = value;
    return value >> 8;
}

uint16_t dm2_v1_drops_rand16(DM2_V1_DropRng *rng, uint16_t n) {
    if (!rng) return 0u;
    if (n == 0u) return 0u;
    return (uint16_t)(dm2_v1_drops_rand(rng) % (uint32_t)n);
}

uint16_t dm2_v1_drops_randbit(DM2_V1_DropRng *rng) {
    if (!rng) return 0u;
    return (uint16_t)(dm2_v1_drops_rand(rng) & 1u);
}

uint16_t dm2_v1_drops_randdir(DM2_V1_DropRng *rng) {
    if (!rng) return 0u;
    return (uint16_t)(dm2_v1_drops_rand(rng) & 3u);
}

uint32_t dm2_v1_drops_rand24(DM2_V1_DropRng *rng) {
    if (!rng) return 0u;
    return dm2_v1_drops_rand(rng);
}

/* dm2_v1_drops_resolve_source_slots — source-ordered slot resolution.
 * Source: skproject/SKWINSPX/src/v4/skcrture.cpp:2084-2118
 * (DROP_CREATURE_POSSESSION, dropMode == CREATURE_GENERATED_DROPS),
 * skproject/SKWINSPX/src/v0/skdefine.h:898 (CREATURE_STAT_DROP_FIRST),
 * skproject/SKULLWIN/c_random.cpp:23-31 (DM2_RAND16). */
int dm2_v1_drops_resolve_source_slots(
    const uint16_t slot_words[DM2_DROP_SLOT_COUNT],
    DM2_V1_DropRng *rng,
    DM2_V1_DropSlotReceipt out_receipts[DM2_DROP_SLOT_COUNT],
    int *out_total) {
    int admitted = 0;
    int total = 0;

    if (!slot_words || !rng) return 0;
    if (out_total) *out_total = 0;

    for (int slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot) {
        DM2_V1_DropSlotReceipt r;
        uint16_t word = slot_words[slot];

        r.field = DM2_DROP_SLOT_FIRST + slot;
        r.word = word;
        r.admitted = 0;
        r.item_id = 0;
        r.base_count = 0;
        r.extra_range = 0;
        r.extra_roll = -1;
        r.final_count = 0;

        /* Source order: slots 0x0A..0x14 ascending, word 0 skipped via
         * the source's `continue`. */
        if (word != 0) {
            r.admitted = 1;
            r.base_count = (int)(word & 0x0fu) + 1;
            r.extra_range = (int)((word & 0x0070u) >> 4);
            r.final_count = r.base_count;
            if (r.extra_range != 0) {
                r.extra_roll = (int)dm2_v1_drops_rand16(
                    rng, (uint16_t)(r.extra_range + 1));
                r.final_count += r.extra_roll;
            }
            r.item_id = (int)(word >> 7);
            ++admitted;
            total += r.final_count;
        }
        if (out_receipts) out_receipts[slot] = r;
    }

    if (out_total) *out_total = total;
    return admitted;
}

/* dm2_v1_drops_resolve_gdat_creature_drops — real-data drop route.
 * Source: skproject/SKWINSPX/src/v4/skcrture.cpp:2092-2100
 * (DROP_CREATURE_POSSESSION reads CREATURES[type] word fields
 * CREATURE_STAT_DROP_FIRST..LAST 0x0A..0x14 directly, without the AI-row
 * indirection), skproject/SKULLWIN/c_random.cpp:23-31 (DM2_RAND16).
 * Absent optional word fields read as 0, exactly the loader's treatment of
 * absent optional fields; the source `continue`s on word 0. */
int dm2_v1_drops_resolve_gdat_creature_drops(
    const DM2_V1_AssetLoader *loader,
    int creature_type,
    DM2_V1_DropRng *rng,
    DM2_V1_DropSlotReceipt out_receipts[DM2_DROP_SLOT_COUNT],
    DM2_V1_DropGdatReceipt *out_receipt) {
    DM2_V1_DropGdatReceipt receipt;
    uint16_t words[DM2_DROP_SLOT_COUNT];
    int admitted;
    int total = 0;

    memset(&receipt, 0, sizeof(receipt));
    memset(words, 0, sizeof(words));
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (creature_type >= 0 && creature_type <= 0xff)
        receipt.creature_type = (uint8_t)creature_type;

    if (!loader || !loader->loaded) {
        receipt.rejected_no_loader = 1u;
        receipt.valid = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (creature_type < 0 || creature_type > 0xff) {
        receipt.rejected_type_out_of_range = 1u;
        receipt.valid = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    for (int slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot) {
        uint16_t word = 0u;
        if (dm2_v1_asset_load_word_value(
                loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
                DM2_DROP_SLOT_FIRST + slot, &word)) {
            words[slot] = word;
            ++receipt.words_present;
        }
    }

    admitted = dm2_v1_drops_resolve_source_slots(words, rng, out_receipts,
                                                 &total);
    receipt.admitted = (uint8_t)admitted;
    receipt.total_count = total;
    if (out_receipts) {
        for (int slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot) {
            if (out_receipts[slot].admitted) {
                receipt.first_item_id = out_receipts[slot].item_id;
                break;
            }
        }
    } else if (admitted > 0) {
        for (int slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot) {
            if (words[slot] != 0u) {
                receipt.first_item_id = (int)(words[slot] >> 7);
                break;
            }
        }
    }
    receipt.valid = 1u;
    if (out_receipt) *out_receipt = receipt;
    return admitted;
}

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
        "Source: skproject/SKWINSPX/src/v4/skcrture.cpp:2084-2118 (DROP_CREATURE_POSSESSION)\n"
        "Source: skproject/SKWINSPX/src/v0/skdefine.h:898 (CREATURE_STAT_DROP_FIRST=0x0A)\n"
        "Source: skproject/SKULLWIN/c_random.cpp:13-31 (DM2_RAND/DM2_RAND16 LCG)\n"
        "Source: docs/dm2_dungeon_design.md (11 drop slots, DropTableSeed RNG)\n"
        "Source: docs/dm2_characters.md (Thorn Demon worm food drops)\n"
        "DM1 comparison: single drop slot per creature\n"
        "DM2 comparison: 11 drop slots via GDAT category 0x0A, DropTableSeed per-creature RNG\n"
        "DM2 special: Thorn Demon (AI 19) drops sellable \"steak\" (DM2_DROP_THORN_DEMON_WORM_FOOD)\n";
}
