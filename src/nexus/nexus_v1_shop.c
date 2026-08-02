
#include "nexus_v1_shop.h"

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

int nexus_v1_shop_price(uint16_t item_id) {
    int i;
    for (i = 0; i < NEXUS_SHOP_ITEM_COUNT; i++) {
        if (g_shop_table[i].item_id == item_id)
            return (int)g_shop_table[i].price;
    }
    return -1;
}
