
#ifndef NEXUS_V1_SHOP_H
#define NEXUS_V1_SHOP_H
#include <stddef.h>
#include <stdint.h>

/* Nexus shop item price receipt from DM.BIN at 0x037210.
 * 8 entries: (item_id BE16, price BE16).
 * Item 0x009E appears twice (650 and 820 — likely different shops or tiers).
 * Source: DM.BIN yam\item.c 0x037210, Saturn binary. */

#define NEXUS_SHOP_ITEM_COUNT 8

typedef struct {
    uint16_t item_id;
    uint16_t price;
} Nexus_ShopEntry;

typedef struct Nexus_ShopManager Nexus_ShopManager;

/* Retrieve the static DM.BIN price receipt for diagnostics only. Returns
 * NEXUS_SHOP_ITEM_COUNT; this does not admit a live shop catalog. */
int nexus_v1_shop_table(const Nexus_ShopEntry **out);

/* Look up a diagnostic DM.BIN price receipt. Returns the price or -1 if not
 * found. If the item appears multiple times, returns the first match. */
int nexus_v1_shop_price(uint16_t item_id);

/* Bind the retail DM.BIN yam\\item.c price rows at 0x037210.  The source
 * hash gate belongs to the caller; unverified or truncated bytes are never
 * accepted as a live shop catalog. */
int nexus_v1_shop_bind_dm_bin(Nexus_ShopManager *mgr,
                              const uint8_t *dm_bin,
                              size_t dm_bin_size,
                              int source_hash_verified);

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

struct Nexus_ShopManager {
    Nexus_ShopInstance shops[NEXUS_MAX_SHOPS];
    int count;
    int active_shop;
    int selected_item;
    int open;
    Nexus_ShopEntry catalog[NEXUS_SHOP_ITEM_COUNT];
    int catalog_count;
    int catalog_source_bound;
};

void nexus_v1_shop_manager_init(Nexus_ShopManager *mgr);

/* Runtime shop admission is blocked until the Saturn shop-object consumer
 * and stock/open dispatch are authenticated. */
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

#define NEXUS_SHOP_SELL_RATIO 2

int nexus_v1_shop_sell(Nexus_ShopManager *mgr,
    Nexus_V1_Champion *seller, int inventory_slot);

#endif
