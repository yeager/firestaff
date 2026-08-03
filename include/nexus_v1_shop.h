
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

/* ── Shop runtime manager ──────────────────────────────────────────── */

#include "nexus_v1_champions.h"

#define NEXUS_MAX_SHOP_STOCK 16
#define NEXUS_MAX_SHOPS 8

typedef struct {
    int item_id;
    int price;
    int stock;
} Nexus_ShopStockEntry;

typedef struct {
    Nexus_ShopStockEntry stock[NEXUS_MAX_SHOP_STOCK];
    int stock_count;
    int shop_id;
    char name[32];
} Nexus_ShopInstance;

typedef struct {
    Nexus_ShopInstance shops[NEXUS_MAX_SHOPS];
    int count;
    int active_shop;
    int selected_item;
    int open;
} Nexus_ShopManager;

void nexus_v1_shop_manager_init(Nexus_ShopManager *mgr);

int nexus_v1_shop_register(Nexus_ShopManager *mgr,
    int shop_id, const char *name);

int nexus_v1_shop_add_stock(Nexus_ShopManager *mgr,
    int shop_idx, int item_id, int price, int stock);

int nexus_v1_shop_open(Nexus_ShopManager *mgr, int shop_idx);

void nexus_v1_shop_close(Nexus_ShopManager *mgr);

int nexus_v1_shop_is_open(const Nexus_ShopManager *mgr);

int nexus_v1_shop_buy(Nexus_ShopManager *mgr,
    Nexus_V1_Champion *buyer, int entry_idx);

int nexus_v1_shop_find_by_id(const Nexus_ShopManager *mgr, int shop_id);

#endif
