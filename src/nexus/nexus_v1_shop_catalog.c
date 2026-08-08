#include "nexus_v1_shop.h"

#include <string.h>

#define NEXUS_SHOP_DM_BIN_OFFSET 0x37210U
#define NEXUS_SHOP_ROW_BYTES 4U

/* DM.BIN yam\\item.c, retail Saturn offset 0x037210.  This is source
 * metadata only: shop-object ownership and buy/sell dispatch remain gated. */
static const Nexus_ShopEntry g_shop_table[NEXUS_SHOP_ITEM_COUNT] = {
    {0x009C, 500}, {0x009D, 600}, {0x009E, 650}, {0x009E, 820},
    {0x009F, 550}, {0x00A0, 350}, {0x00A1, 990}, {0x00A2, 1400}
};

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int nexus_v1_shop_table(const Nexus_ShopEntry **out)
{
    if (out) *out = g_shop_table;
    return NEXUS_SHOP_ITEM_COUNT;
}

void nexus_v1_shop_manager_init(Nexus_ShopManager *mgr)
{
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
    mgr->active_shop = -1;
    mgr->selected_item = -1;
}

int nexus_v1_shop_bind_dm_bin(Nexus_ShopManager *mgr,
                              const uint8_t *dm_bin, size_t dm_bin_size,
                              int source_hash_verified)
{
    size_t i;
    size_t terminator;
    if (!mgr || !dm_bin || !source_hash_verified ||
        dm_bin_size < NEXUS_SHOP_DM_BIN_OFFSET +
                       NEXUS_SHOP_ITEM_COUNT * NEXUS_SHOP_ROW_BYTES + 2U)
        return 0;
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

int nexus_v1_shop_price(uint16_t item_id)
{
    int i;
    for (i = 0; i < NEXUS_SHOP_ITEM_COUNT; ++i)
        if (g_shop_table[i].item_id == item_id)
            return (int)g_shop_table[i].price;
    return -1;
}
