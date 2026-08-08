#include <stdio.h>
#include <string.h>
#include "nexus_v1_shop.h"

static int g_fail;
static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_fail;
    }
}

int main(void) {
    Nexus_ShopManager manager;
    Nexus_V1_Champion champion;
    int index, stock_idx;

    nexus_v1_shop_manager_init(&manager);
    expect(manager.count == 0 && !nexus_v1_shop_is_open(&manager),
           "shop manager starts closed and empty");

    /* Registration works */
    index = nexus_v1_shop_register(&manager, 42, "Armory");
    expect(index == 0 && manager.count == 1,
           "shop registration succeeds and increments count");
    expect(nexus_v1_shop_find_by_id(&manager, 42) == 0,
           "shop lookup finds registered shop");

    /* Stock addition works */
    stock_idx = nexus_v1_shop_add_stock(&manager, index, 0x9C, 500, 3);
    expect(stock_idx == 0,
           "shop stock addition succeeds");

    /* Open works */
    expect(nexus_v1_shop_open(&manager, index),
           "shop open succeeds");
    expect(nexus_v1_shop_is_open(&manager),
           "shop is open after open call");

    /* Buy works when champion has gold and inventory space */
    memset(&champion, 0, sizeof(champion));
    champion.gold = 1000;
    expect(nexus_v1_shop_buy(&manager, &champion, 0),
           "buy succeeds with sufficient gold");
    expect(champion.gold == 500,
           "gold deducted after purchase");

    /* Sell works when champion has an item */
    champion.inventory[1] = 0x9C;
    expect(nexus_v1_shop_sell(&manager, &champion, 1),
           "sell succeeds with item in slot");
    expect(champion.inventory[1] == 0,
           "inventory slot cleared after sell");

    /* Close works */
    nexus_v1_shop_close(&manager);
    expect(!nexus_v1_shop_is_open(&manager),
           "shop closed after close call");

    /* Buy/sell fail when shop is closed */
    expect(!nexus_v1_shop_buy(&manager, &champion, 0),
           "buy fails when shop is closed");
    expect(!nexus_v1_shop_sell(&manager, &champion, 0),
           "sell fails when shop is closed");

    if (g_fail) {
        fprintf(stderr, "test_nexus_v1_shop_manager: %d failure(s)\n", g_fail);
        return 1;
    }
    puts("ok: Nexus shop manager verified");
    return 0;
}
