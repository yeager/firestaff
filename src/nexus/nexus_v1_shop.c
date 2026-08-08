
#include "nexus_v1_shop.h"
/* Shop-object manager study. The retail DM.BIN catalog lives in
 * nexus_v1_shop_catalog.c; this file remains fixture-only until Saturn shop
 * ownership and buy/sell dispatch are captured. */

int nexus_v1_shop_register(Nexus_ShopManager *mgr,
    int shop_id, const char *name) {
    int idx;
    if (!mgr || mgr->count >= NEXUS_MAX_SHOPS) return -1;
    idx = mgr->count++;
    mgr->shops[idx].shop_id = shop_id;
    mgr->shops[idx].stock_count = 0;
    if (name) {
        int i;
        for (i = 0; i < 31 && name[i]; i++)
            mgr->shops[idx].name[i] = name[i];
        mgr->shops[idx].name[i] = '\0';
    } else {
        mgr->shops[idx].name[0] = '\0';
    }
    return idx;
}

int nexus_v1_shop_add_stock(Nexus_ShopManager *mgr,
    int shop_idx, int item_id, int price, int stock) {
    Nexus_ShopInstance *s;
    int si;
    if (!mgr || shop_idx < 0 || shop_idx >= mgr->count) return -1;
    s = &mgr->shops[shop_idx];
    if (s->stock_count >= NEXUS_MAX_SHOP_STOCK) return -1;
    si = s->stock_count++;
    s->stock[si].item_id = item_id;
    s->stock[si].price = price;
    s->stock[si].stock = stock;
    return si;
}

int nexus_v1_shop_open(Nexus_ShopManager *mgr, int shop_idx) {
    if (!mgr || shop_idx < 0 || shop_idx >= mgr->count) return 0;
    mgr->active_shop = shop_idx;
    mgr->open = 1;
    mgr->selected_item = -1;
    return 1;
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
    if (e->stock <= 0) return 0;
    if (buyer->gold < e->price) return 0;
    for (slot = 0; slot < 30; slot++) {
        if (buyer->inventory[slot] == 0 || buyer->inventory[slot] == 0xFF) {
            buyer->gold -= e->price;
            e->stock--;
            buyer->inventory[slot] = (uint8_t)(e->item_id & 0xFF);
            return 1;
        }
    }
    return 0;
}

int nexus_v1_shop_find_by_id(const Nexus_ShopManager *mgr, int shop_id) {
    int i;
    if (!mgr) return -1;
    for (i = 0; i < mgr->count; i++) {
        if (mgr->shops[i].shop_id == shop_id) return i;
    }
    return -1;
}

int nexus_v1_shop_sell(Nexus_ShopManager *mgr,
    Nexus_V1_Champion *seller, int inventory_slot) {
    int item_id, price;
    if (!mgr || !seller || !mgr->open) return 0;
    if (inventory_slot < 0 || inventory_slot >= NEXUS_INVENTORY_SLOTS) return 0;
    item_id = seller->inventory[inventory_slot];
    if (item_id == 0 || item_id == 0xFF) return 0;
    price = nexus_v1_shop_price((uint16_t)item_id);
    if (price < 0) price = 10;
    price /= NEXUS_SHOP_SELL_RATIO;
    if (price < 1) price = 1;
    seller->gold += price;
    seller->inventory[inventory_slot] = 0;
    return 1;
}
