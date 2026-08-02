
#ifndef NEXUS_V1_SHOP_H
#define NEXUS_V1_SHOP_H
#include <stdint.h>

/* Nexus shop item price table from DM.BIN at 0x037210.
 * 8 entries: (item_id BE16, price BE16).
 * Item 0x009E appears twice (650 and 820 — likely different shops or tiers).
 * Source: DM.BIN yam\item.c 0x037210, Saturn binary. */

#define NEXUS_SHOP_ITEM_COUNT 8

typedef struct {
    uint16_t item_id;
    uint16_t price;
} Nexus_ShopEntry;

/* Retrieve the shop price table.  Returns NEXUS_SHOP_ITEM_COUNT. */
int nexus_v1_shop_table(const Nexus_ShopEntry **out);

/* Look up the price for an item_id.  Returns the price or -1 if not found.
 * If the item appears multiple times, returns the first match. */
int nexus_v1_shop_price(uint16_t item_id);

#endif
