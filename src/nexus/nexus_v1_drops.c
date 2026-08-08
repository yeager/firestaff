#include "nexus_v1_drops.h"
#include "nexus_v1_combat.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t drop_rng_state = 0x12345678U;

static int drop_rand(int max) {
    if (max <= 0) return 0;
    drop_rng_state = drop_rng_state * 1103515245U + 12345U;
    return (int)((drop_rng_state >> 16) % (unsigned)max);
}

int nexus_drops_for_type(int creature_type_idx,
                          Nexus_DropEntry *out_table,
                          int max_entries) {
    if (!out_table || max_entries <= 0) return 0;
    if (creature_type_idx < 0) return 0;
    memset(out_table, 0, (size_t)max_entries * sizeof(Nexus_DropEntry));
    if (max_entries >= 1) {
        out_table[0].item_id = -1;
        out_table[0].min_qty = 5 + creature_type_idx * 3;
        out_table[0].max_qty = 15 + creature_type_idx * 5;
        out_table[0].chance = 60;
        return 1;
    }
    return 0;
}

int nexus_drops_roll(int creature_type_idx, int x, int y,
                      int *out_item_ids, int *out_quantities,
                      int max_drops) {
    Nexus_DropEntry table[8];
    int count, i, drops = 0;
    (void)x; (void)y;

    if (!out_item_ids || !out_quantities || max_drops <= 0) return 0;
    count = nexus_drops_for_type(creature_type_idx, table, 8);
    for (i = 0; i < count && drops < max_drops; i++) {
        int roll = drop_rand(100);
        if (roll < table[i].chance) {
            out_item_ids[drops] = table[i].item_id;
            int range = table[i].max_qty - table[i].min_qty;
            out_quantities[drops] = table[i].min_qty +
                (range > 0 ? drop_rand(range + 1) : 0);
            drops++;
        }
    }
    return drops;
}

/* ═══════════════════════════════════════════════════════════════════
 * Gold pile management
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
    for (i = 0; i < g_gold_pile_count; i++) {
        if (g_gold_piles[i].x == x && g_gold_piles[i].y == y) {
            g_gold_piles[i].amount += amount;
            return i;
        }
    }
    if (g_gold_pile_count >= NEXUS_MAX_GOLD_PILES) return -1;
    g_gold_piles[g_gold_pile_count].x = x;
    g_gold_piles[g_gold_pile_count].y = y;
    g_gold_piles[g_gold_pile_count].amount = amount;
    return g_gold_pile_count++;
}

int nexus_gold_pickup(int *out_amount) {
    if (!out_amount) return 0;
    if (g_gold_pile_count <= 0) return 0;
    *out_amount = g_gold_piles[0].amount;
    g_gold_piles[0] = g_gold_piles[--g_gold_pile_count];
    return 1;
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
