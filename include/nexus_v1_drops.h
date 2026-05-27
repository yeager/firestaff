#ifndef NEXUS_V1_DROPS_H
#define NEXUS_V1_DROPS_H

#include <stdint.h>

/* Nexus V1 creature drops — gold and item drops on creature death.
 * DM1-compatible drop tables per creature type.
 * Source: DM1 creature drop tables, nexus_v1_combat.c,
 * docs/nexus_combat_creatures.md (gold/XP on kill). */

#define NEXUS_MAX_DROPS 256

/* Drop entry */
typedef struct {
    int item_id;   /* -1 = gold */
    int min_qty;
    int max_qty;
    int chance;    /* 0-100 percentage */
} Nexus_DropEntry;

/* Floor drop record */
typedef struct {
    int item_id;
    int quantity;
    int x, y;
    int on_ground;
} Nexus_FloorDrop;

/* Gold pile on floor */
typedef struct {
    int x, y;
    int amount;
} Nexus_GoldPile;

#define NEXUS_MAX_GOLD_PILES 64

/* Get drop table for a creature type index.
 * Returns number of entries; fills table. */
int nexus_drops_for_type(int creature_type_idx,
                          Nexus_DropEntry *out_table,
                          int max_entries);

/* Roll drops for a killed creature at (x,y).
 * Returns number of drop events generated. */
int nexus_drops_roll(int creature_type_idx, int x, int y,
                      int *out_item_ids, int *out_quantities,
                      int max_drops);

/* Gold management */
void nexus_gold_init(void);
int nexus_gold_add(int x, int y, int amount);
int nexus_gold_pickup(int *out_amount);
int nexus_gold_at(int x, int y);

/* Simple loot table builder */
int nexus_build_loot_table(Nexus_DropEntry *table, int max,
                            const uint8_t *item_ids,
                            const uint8_t *min_q,
                            const uint8_t *max_q,
                            const uint8_t *chances,
                            int count);

#endif /* NEXUS_V1_DROPS_H */