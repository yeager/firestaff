#include "nexus_v1_drops.h"
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

int nexus_drops_for_type(int creature_type_idx,
                          Nexus_DropEntry *out_table,
                          int max_entries) {
    /* No Nexus drop table has been extracted or source-locked.  The old
     * implementation copied inferred DM1 tables and even fabricated a
     * small-gold fallback for unknown actors.  Keep the API bounded but do
     * not manufacture Nexus inventory or gold state. */
    (void)creature_type_idx;
    (void)out_table;
    (void)max_entries;
    return 0;
}

int nexus_drops_roll(int creature_type_idx, int x, int y,
                      int *out_item_ids, int *out_quantities,
                      int max_drops) {
    (void)creature_type_idx;
    (void)x;
    (void)y;
    (void)out_item_ids;
    (void)out_quantities;
    (void)max_drops;
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════
 * Gold pile management
 * Source: DM1 gold pile system.
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
