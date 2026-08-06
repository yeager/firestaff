
#include "nexus_v1_shop.h"
#include <string.h>

#define NEXUS_SHOP_DM_BIN_OFFSET 0x37210U
#define NEXUS_SHOP_ROW_BYTES 4U

/* Shop item price table from DM.BIN yam\item.c at 0x037210.
 * Extracted from the real Saturn binary (555,144 bytes).
 * 8 entries of (item_id, price) as big-endian uint16 pairs,
 * terminated by 0xFFFF sentinel. */

static const Nexus_ShopEntry g_shop_table[NEXUS_SHOP_ITEM_COUNT] = {
    {0x009C,  500},
    {0x009D,  600},
    {0x009E,  650},
    {0x009E,  820},
    {0x009F,  550},
    {0x00A0,  350},
    {0x00A1,  990},
    {0x00A2, 1400},
};

int nexus_v1_shop_table(const Nexus_ShopEntry **out) {
    if (out) *out = g_shop_table;
    return NEXUS_SHOP_ITEM_COUNT;
}

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int nexus_v1_shop_bind_dm_bin(Nexus_ShopManager *mgr,
                              const uint8_t *dm_bin,
                              size_t dm_bin_size,
                              int source_hash_verified)
{
    size_t i;
    size_t terminator;

    if (!mgr || !dm_bin || !source_hash_verified ||
        dm_bin_size < NEXUS_SHOP_DM_BIN_OFFSET +
                       NEXUS_SHOP_ITEM_COUNT * NEXUS_SHOP_ROW_BYTES + 2U) {
        return 0;
    }
    for (i = 0U; i < NEXUS_SHOP_ITEM_COUNT; ++i) {
        const uint8_t *row = dm_bin + NEXUS_SHOP_DM_BIN_OFFSET +
                             i * NEXUS_SHOP_ROW_BYTES;
        mgr->catalog[i].item_id = read_be16(row);
        mgr->catalog[i].price = read_be16(row + 2U);
        if (mgr->catalog[i].price == 0U) {
            mgr->catalog_count = 0;
            mgr->catalog_source_bound = 0;
            return 0;
        }
    }
    terminator = NEXUS_SHOP_DM_BIN_OFFSET +
                 NEXUS_SHOP_ITEM_COUNT * NEXUS_SHOP_ROW_BYTES;
    if (read_be16(dm_bin + terminator) != 0xffffU) {
        mgr->catalog_count = 0;
        mgr->catalog_source_bound = 0;
        return 0;
    }
    mgr->catalog_count = NEXUS_SHOP_ITEM_COUNT;
    mgr->catalog_source_bound = 1;
    return 1;
}

/* ── Shop runtime manager ──────────────────────────────────────────── */

void nexus_v1_shop_manager_init(Nexus_ShopManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
    mgr->active_shop = -1;
    mgr->selected_item = -1;
}

int nexus_v1_shop_register(Nexus_ShopManager *mgr,
    int shop_id, const char *name) {
    Nexus_ShopInstance *s;
    if (!mgr || mgr->count >= NEXUS_MAX_SHOPS) return -1;
    s = &mgr->shops[mgr->count];
    memset(s, 0, sizeof(*s));
    s->shop_id = shop_id;
    if (name) {
        strncpy(s->name, name, sizeof(s->name) - 1);
        s->name[sizeof(s->name) - 1] = '\0';
    }
    return mgr->count++;
}

int nexus_v1_shop_add_stock(Nexus_ShopManager *mgr,
    int shop_idx, int item_id, int price, int stock) {
    Nexus_ShopInstance *s;
    Nexus_ShopStockEntry *e;
    if (!mgr || shop_idx < 0 || shop_idx >= mgr->count) return -1;
    s = &mgr->shops[shop_idx];
    if (s->stock_count >= NEXUS_MAX_SHOP_STOCK) return -1;
    e = &s->stock[s->stock_count];
    e->item_id = item_id;
    e->price = price;
    e->stock = stock;
    return s->stock_count++;
}

int nexus_v1_shop_open(Nexus_ShopManager *mgr, int shop_idx) {
    if (!mgr || shop_idx < 0 || shop_idx >= mgr->count) return 0;
    mgr->active_shop = shop_idx;
    mgr->selected_item = 0;
    mgr->open = 1;
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
    (void)e;
    (void)slot;
    /* DM.BIN proves the eight price rows, not the Saturn shop object,
     * stock ownership, wallet write or ITEM.IBS action consumer.  The old
     * implementation mutated a host champion from an unbound test manager;
     * keep the source catalog readable but do not manufacture a purchase. */
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
    if (!mgr || !seller || !mgr->open) return 0;
    if (inventory_slot < 0 || inventory_slot >= NEXUS_INVENTORY_SLOTS) return 0;
    /* A sell-side price is not a proof of the Saturn event dispatcher or
     * inventory transaction.  Until that consumer is captured, no host
     * inventory/gold mutation is permitted. */
    return 0;
}

int nexus_v1_shop_price(uint16_t item_id) {
    int i;
    for (i = 0; i < NEXUS_SHOP_ITEM_COUNT; i++) {
        if (g_shop_table[i].item_id == item_id)
            return (int)g_shop_table[i].price;
    }
    return -1;
}
