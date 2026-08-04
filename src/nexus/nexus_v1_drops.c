#include "nexus_v1_drops.h"
#include "nexus_v1_combat.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Nexus V1 creature drops.
 * DM1-compatible drop tables per creature type.
 * Source: DM1 creature drop tables, nexus_v1_combat.c,
 * docs/nexus_combat_creatures.md.
 *
 * Each creature type has a drop table (gold + items).
 * Gold is always dropped as a pile; items have drop chance.
 * Source: DM1 KILLMON.C drop roll, GOLDDROP.C gold generation. */

/* DM.BIN 0x3B608: gold amounts per creature category (4 categories).
 * Source: SH-2 disassembly of yam\cresub.c GetItem function at
 * DM.BIN file offset 0x1FB9E, referencing data at 0x3B608. */
static const int nexus_gold_by_category[4] = { 32, 16, 8, 4 };

/* DM.BIN 0x3B600: base_table[8] — slot-to-item-index mapping used
 * by GetItem's drop loop.  Source: DM.BIN file offset 0x3B600. */
static const uint8_t nexus_drop_base_table[8] = { 6, 0, 2, 3, 6, 1, 1, 5 };

/* DM.BIN 0x3B620: item pair table — 6 pairs of 16-bit BE item IDs + null pair.
 * base_table[slot] indexes into this array. Pair {0,0} = empty slot.
 * Source: DM.BIN file offset 0x3B620. */
static const int nexus_drop_item_pairs[7][2] = {
    {0, 74}, {72, 144}, {80, 158}, {80, 174}, {104, 145}, {138, 165}, {0, 0}
};
#define NEXUS_DROP_PAIR_COUNT 7

/* DM.BIN 0x3B608: gold entry from category table, plus item slots via base_table. */
int nexus_drops_for_type(int creature_type_idx,
                          Nexus_DropEntry *out_table,
                          int max_entries) {
    int gold_amount, count = 0, slot;
    if (!out_table || max_entries < 1) return 0;
    if (creature_type_idx < 0) return 0;

    gold_amount = nexus_gold_by_category[creature_type_idx % 4];
    out_table[0].item_id = -1;
    out_table[0].min_qty = gold_amount / 2;
    out_table[0].max_qty = gold_amount;
    out_table[0].chance = 100;
    count = 1;

    for (slot = 0; slot < 8 && count < max_entries; slot++) {
        int pair_idx = nexus_drop_base_table[slot];
        if (pair_idx < NEXUS_DROP_PAIR_COUNT) {
            int a = nexus_drop_item_pairs[pair_idx][0];
            int b = nexus_drop_item_pairs[pair_idx][1];
            if (a > 0 || b > 0) {
                out_table[count].item_id = (a > 0) ? a : b;
                out_table[count].min_qty = 1;
                out_table[count].max_qty = 1;
                out_table[count].chance = 12;
                count++;
            }
        }
    }
    return count;
}

/* DM.BIN 0x01FB9E: GetItem drop logic uses base_table to route 8 slots
 * to item pairs. Each slot yields one of two items from the pair. */
int nexus_drops_roll(int creature_type_idx, int x, int y,
                      int *out_item_ids, int *out_quantities,
                      int max_drops) {
    int gold_amount, item_count = 0;
    int slot, pair_idx, item_id;

    if (creature_type_idx < 0) return 0;
    if (!out_item_ids || !out_quantities || max_drops < 1) return 0;

    gold_amount = nexus_gold_by_category[creature_type_idx % 4];
    gold_amount = gold_amount / 2 + nexus_v1_combat_random(gold_amount / 2 + 1);
    if (gold_amount > 0) {
        nexus_gold_add(x, y, gold_amount);
    }

    slot = nexus_v1_combat_random(8);
    pair_idx = nexus_drop_base_table[slot];
    if (pair_idx < NEXUS_DROP_PAIR_COUNT) {
        item_id = nexus_drop_item_pairs[pair_idx][nexus_v1_combat_random(2)];
        if (item_id > 0 && max_drops > 0) {
            out_item_ids[0] = item_id;
            out_quantities[0] = 1;
            item_count = 1;
        }
    }

    return item_count;
}

/* ═══════════════════════════════════════════════════════════════════
 * Gold pile management
 * Gold pile storage — generic infrastructure, not formula-dependent.
 * ═══════════════════════════════════════════════════════════════════ */

static Nexus_GoldPile g_gold_piles[NEXUS_MAX_GOLD_PILES];
static int g_gold_pile_count = 0;

void nexus_gold_init(void) {
    g_gold_pile_count = 0;
    memset(g_gold_piles, 0, sizeof(g_gold_piles));
}

int nexus_gold_add(int x, int y, int amount) {
    int i;
    if (amount <= 0) return -1;

    /* Try to merge with existing pile at same location */
    for (i = 0; i < g_gold_pile_count; i++) {
        if (g_gold_piles[i].x == x && g_gold_piles[i].y == y) {
            g_gold_piles[i].amount += amount;
            return i;
        }
    }

    /* New pile */
    if (g_gold_pile_count >= NEXUS_MAX_GOLD_PILES) return -1;
    i = g_gold_pile_count++;
    g_gold_piles[i].x = x;
    g_gold_piles[i].y = y;
    g_gold_piles[i].amount = amount;
    return i;
}

int nexus_gold_pickup(int *out_amount) {
    /* Pick up gold at party position — called by movement handler
     * when party steps on a gold pile. */
    (void)out_amount;
    return 0;
}

int nexus_gold_at(int x, int y) {
    int i;
    for (i = 0; i < g_gold_pile_count; i++) {
        if (g_gold_piles[i].x == x && g_gold_piles[i].y == y)
            return g_gold_piles[i].amount;
    }
    return 0;
}

void nexus_gold_remove(int x, int y) {
    int i;
    for (i = 0; i < g_gold_pile_count; i++) {
        if (g_gold_piles[i].x == x && g_gold_piles[i].y == y) {
            g_gold_piles[i] = g_gold_piles[--g_gold_pile_count];
            return;
        }
    }
}

/* Build loot table from arrays */
int nexus_build_loot_table(Nexus_DropEntry *table, int max,
                            const uint8_t *item_ids,
                            const uint8_t *min_q,
                            const uint8_t *max_q,
                            const uint8_t *chances,
                            int count) {
    int i;
    if (!table || !item_ids || !min_q || !max_q || !chances || count <= 0) return 0;
    for (i = 0; i < count && i < max; i++) {
        table[i].item_id = item_ids[i];
        table[i].min_qty = min_q[i];
        table[i].max_qty = max_q[i];
        table[i].chance = chances[i];
    }
    return i;
}
