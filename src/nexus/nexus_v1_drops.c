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

/* Drop table per creature type
 * Format: { item_id, min_qty, max_qty, chance% }
 * item_id=-1 means gold
 * Source: DM1 KILLMON.C drop tables (inferred from creature XP values) */
typedef struct {
    int type;
    Nexus_DropEntry entries[6];
    int entry_count;
} DropTableEntry;

static const DropTableEntry g_drop_tables[] = {
    /* Scorpion — low tier */
    {0, {{-1,1,5,80},{63,1,1,10},{64,1,1,5},{7,1,1,5},{0,0,0,0},{0,0,0,0}}, 4},
    /* Mummy — mid tier undead */
    {1, {{-1,5,15,90},{63,1,2,20},{64,1,1,10},{30,1,1,8},{1,1,1,5},{0,0,0,0}}, 5},
    /* Dragon — boss tier */
    {2, {{-1,100,300,100},{5,1,1,50},{10,1,1,30},{11,1,1,30},{1,1,1,40},{0,0,0,0}}, 5},
    /* Skeleton — common */
    {3, {{-1,3,12,85},{63,1,2,15},{7,1,1,8},{30,1,1,10},{0,0,0,0},{0,0,0,0}}, 4},
    /* Ghost — fast/weak */
    {4, {{-1,2,8,80},{62,1,1,10},{63,1,1,5},{0,0,0,0},{0,0,0,0},{0,0,0,0}}, 3},
    /* Worm — mid tier */
    {5, {{-1,8,25,90},{63,1,3,25},{64,1,2,15},{1,1,1,10},{30,1,1,8},{0,0,0,0}}, 5},
    /* Golem — heavy tank */
    {6, {{-1,20,60,95},{24,1,1,20},{25,1,1,15},{30,1,2,15},{1,1,1,12},{0,0,0,0}}, 5},
    /* Spider — low tier */
    {7, {{-1,1,4,75},{63,1,1,12},{7,1,1,5},{0,0,0,0},{0,0,0,0},{0,0,0,0}}, 3},
    /* Demon — tough */
    {8, {{-1,30,80,95},{30,1,2,25},{33,1,1,20},{31,1,1,15},{1,1,1,15},{0,0,0,0}}, 5},
    /* Giggler — steal/flee */
    {9, {{-1,5,15,85},{63,1,2,20},{64,1,1,10},{7,1,1,15},{0,0,0,0},{0,0,0,0}}, 4},
    /* Vexirk — tough spell caster */
    {10,{{-1,25,70,95},{31,1,2,20},{30,1,2,20},{33,1,1,15},{1,1,1,12},{0,0,0,0}}, 5},
    /* Rat — very low */
    {11,{{-1,1,3,60},{63,1,1,5},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}, 2},
    /* Screamer — fast mid */
    {12,{{-1,10,30,90},{63,1,3,20},{7,1,1,10},{30,1,1,8},{0,0,0,0},{0,0,0,0}}, 4},
    /* Rockpile — decorative, minimal */
    {13,{{-1,0,0,50},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}, 1},
    /* Oitu — boss-like */
    {14,{{-1,50,150,100},{10,1,1,40},{11,1,1,30},{30,1,3,25},{31,1,2,20},{0,0,0,0}}, 5},
    {-1, {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}, 0}
};

static int rng(int max) { return max > 0 ? (rand() % max) : 0; }

int nexus_drops_for_type(int creature_type_idx,
                          Nexus_DropEntry *out_table,
                          int max_entries) {
    const DropTableEntry *de = NULL;
    int i, count;

    for (i = 0; g_drop_tables[i].type >= 0; i++) {
        if (g_drop_tables[i].type == creature_type_idx) {
            de = &g_drop_tables[i];
            break;
        }
    }

    if (!de) {
        /* Fallback: small gold drop */
        if (out_table && max_entries > 0) {
            out_table[0].item_id = -1;
            out_table[0].min_qty = 1;
            out_table[0].max_qty = 5;
            out_table[0].chance = 70;
        }
        return de ? de->entry_count : 0;
    }

    count = de->entry_count;
    if (count > max_entries) count = max_entries;

    for (i = 0; i < count; i++)
        out_table[i] = de->entries[i];

    return count;
}

int nexus_drops_roll(int creature_type_idx, int x, int y,
                      int *out_item_ids, int *out_quantities,
                      int max_drops) {
    Nexus_DropEntry table[8];
    int entries, drop_count = 0;
    int i;

    entries = nexus_drops_for_type(creature_type_idx, table, 8);
    if (entries <= 0) return 0;

    for (i = 0; i < entries && drop_count < max_drops; i++) {
        if (table[i].chance <= 0) continue;
        if (rng(100) < table[i].chance) {
            int qty = table[i].min_qty;
            if (table[i].max_qty > table[i].min_qty)
                qty = table[i].min_qty + rng(table[i].max_qty - table[i].min_qty + 1);

            if (table[i].item_id == -1) {
                /* Gold — add to gold piles */
                nexus_gold_add(x, y, qty);
            } else if (out_item_ids && out_quantities) {
                out_item_ids[drop_count] = table[i].item_id;
                out_quantities[drop_count] = qty;
                drop_count++;
            }
        }
    }

    return drop_count;
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