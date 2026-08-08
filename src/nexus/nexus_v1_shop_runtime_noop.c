/* Capture-gated Nexus V1 shop-object production adapter.
 * DM.BIN price rows remain in shop_catalog; object buy/sell dispatch does not. */

#include "nexus_v1_shop.h"

int nexus_v1_shop_register(Nexus_ShopManager *manager, int shop_id,
                           const char *name)
{
    (void)manager; (void)shop_id; (void)name;
    return -1;
}

int nexus_v1_shop_add_stock(Nexus_ShopManager *manager, int shop_idx,
                            int item_id, int price, int stock)
{
    (void)manager; (void)shop_idx; (void)item_id; (void)price; (void)stock;
    return -1;
}

int nexus_v1_shop_open(Nexus_ShopManager *manager, int shop_idx)
{
    (void)manager; (void)shop_idx;
    return 0;
}

void nexus_v1_shop_close(Nexus_ShopManager *manager)
{
    (void)manager;
}

int nexus_v1_shop_is_open(const Nexus_ShopManager *manager)
{
    (void)manager;
    return 0;
}

int nexus_v1_shop_buy(Nexus_ShopManager *manager, Nexus_V1_Champion *buyer,
                      int entry_idx)
{
    (void)manager; (void)buyer; (void)entry_idx;
    return 0;
}

int nexus_v1_shop_find_by_id(const Nexus_ShopManager *manager, int shop_id)
{
    (void)manager; (void)shop_id;
    return -1;
}

int nexus_v1_shop_sell(Nexus_ShopManager *manager, Nexus_V1_Champion *seller,
                       int inventory_slot)
{
    (void)manager; (void)seller; (void)inventory_slot;
    return 0;
}
