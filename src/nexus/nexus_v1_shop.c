
#include "nexus_v1_shop.h"
/* Shop-object manager study. The retail DM.BIN catalog lives in
 * nexus_v1_shop_catalog.c; this file remains fixture-only until Saturn shop
 * ownership and buy/sell dispatch are captured. */

int nexus_v1_shop_register(Nexus_ShopManager *mgr,
    int shop_id, const char *name) {
    /* DM.BIN owns price rows only. Retail Nexus shop-object ownership,
     * stock records and Saturn open/close dispatch are not captured. */
    (void)mgr; (void)shop_id; (void)name;
    return -1;
}

int nexus_v1_shop_add_stock(Nexus_ShopManager *mgr,
    int shop_idx, int item_id, int price, int stock) {
    (void)mgr; (void)shop_idx; (void)item_id;
    (void)price; (void)stock;
    return -1;
}

int nexus_v1_shop_open(Nexus_ShopManager *mgr, int shop_idx) {
    (void)mgr; (void)shop_idx;
    return 0;
}

void nexus_v1_shop_close(Nexus_ShopManager *mgr) {
    if (!mgr) return;
    mgr->open = 0;
    mgr->active_shop = -1;
    mgr->selected_item = -1;
}

int nexus_v1_shop_is_open(const Nexus_ShopManager *mgr) {
    if (!mgr) return 0;
    return mgr->open;
}

int nexus_v1_shop_buy(Nexus_ShopManager *mgr,
    Nexus_V1_Champion *buyer, int entry_idx) {
    Nexus_ShopInstance *s;
    Nexus_ShopStockEntry *e;
    int slot;
    if (!mgr || !buyer || !mgr->open) return 0;
    if (mgr->active_shop < 0 || mgr->active_shop >= mgr->count) return 0;
    s = &mgr->shops[mgr->active_shop];
    if (entry_idx < 0 || entry_idx >= s->stock_count) return 0;
    e = &s->stock[entry_idx];
    (void)e;
    (void)slot;
    /* DM.BIN proves the eight price rows, not the Saturn shop object,
     * stock ownership, wallet write or ITEM.IBS action consumer.  The old
     * implementation mutated a host champion from an unbound test manager;
     * keep the source catalog readable but do not manufacture a purchase. */
    return 0;
}

int nexus_v1_shop_find_by_id(const Nexus_ShopManager *mgr, int shop_id) {
    (void)mgr; (void)shop_id;
    return -1;
}

int nexus_v1_shop_sell(Nexus_ShopManager *mgr,
    Nexus_V1_Champion *seller, int inventory_slot) {
    if (!mgr || !seller || !mgr->open) return 0;
    if (inventory_slot < 0 || inventory_slot >= NEXUS_INVENTORY_SLOTS) return 0;
    /* A sell-side price is not a proof of the Saturn event dispatcher or
     * inventory transaction.  Until that consumer is captured, no host
     * inventory/gold mutation is permitted. */
    return 0;
}
