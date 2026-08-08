#include "nexus_v1_drops.h"
#include "nexus_v1_combat.h"
#include <string.h>

int nexus_drops_for_type(int creature_type_idx,
                          Nexus_DropEntry *out_table,
                          int max_entries) {
    (void)creature_type_idx;
    if (out_table && max_entries > 0) {
        memset(out_table, 0, (size_t)max_entries * sizeof(Nexus_DropEntry));
    }
    /* No retail Nexus creature-drop owner has been identified.  In
     * particular, do not reuse the old DM1-shaped gold formula here:
     * ITEM.IBS/DGN floor declarations are source-owned objects, not proof of
     * a death-drop table.  Keep the production boundary empty until a
     * Saturn action/event capture binds creature death to an item or gold
     * record. */
    return 0;
}

int nexus_drops_roll(int creature_type_idx, int x, int y,
                      int *out_item_ids, int *out_quantities,
                      int max_drops) {
    (void)creature_type_idx;
    (void)x; (void)y;
    (void)out_item_ids;
    (void)out_quantities;
    (void)max_drops;
    /* Fail closed: no guessed RNG, item id, quantity or gold amount may
     * become a runtime floor mutation before Saturn capture proves the
     * death/drop dispatch. */
    return 0;
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
