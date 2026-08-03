
#include <stdio.h>
#include <string.h>
#include "nexus_v1_shop.h"

static int g_fail;
static void expect(int c, const char *m) {
    if (!c) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; }
}

int main(void) {
    /* Test 1: init */
    {
        Nexus_ShopManager mgr;
        nexus_v1_shop_manager_init(&mgr);
        expect(mgr.count == 0, "init count 0");
        expect(!nexus_v1_shop_is_open(&mgr), "not open");
    }

    /* Test 2: register shop */
    {
        Nexus_ShopManager mgr;
        int idx;
        nexus_v1_shop_manager_init(&mgr);
        idx = nexus_v1_shop_register(&mgr, 42, "Armory");
        expect(idx == 0, "first shop idx 0");
        expect(mgr.count == 1, "count 1");
        expect(strcmp(mgr.shops[0].name, "Armory") == 0, "shop name");
    }

    /* Test 3: add stock */
    {
        Nexus_ShopManager mgr;
        int si;
        nexus_v1_shop_manager_init(&mgr);
        si = nexus_v1_shop_register(&mgr, 1, "Items");
        nexus_v1_shop_add_stock(&mgr, si, 0x9C, 500, 3);
        nexus_v1_shop_add_stock(&mgr, si, 0x9D, 600, 1);
        expect(mgr.shops[si].stock_count == 2, "2 stock entries");
        expect(mgr.shops[si].stock[0].price == 500, "first price");
    }

    /* Test 4: open and close */
    {
        Nexus_ShopManager mgr;
        int si;
        nexus_v1_shop_manager_init(&mgr);
        si = nexus_v1_shop_register(&mgr, 1, "S");
        expect(nexus_v1_shop_open(&mgr, si), "open ok");
        expect(nexus_v1_shop_is_open(&mgr), "is open");
        nexus_v1_shop_close(&mgr);
        expect(!nexus_v1_shop_is_open(&mgr), "closed");
    }

    /* Test 5: buy item */
    {
        Nexus_ShopManager mgr;
        Nexus_V1_Champion ch;
        int si;
        nexus_v1_shop_manager_init(&mgr);
        si = nexus_v1_shop_register(&mgr, 1, "S");
        nexus_v1_shop_add_stock(&mgr, si, 0xAA, 100, 2);
        nexus_v1_shop_open(&mgr, si);

        memset(&ch, 0, sizeof(ch));
        memset(ch.inventory, 0xFF, sizeof(ch.inventory));
        ch.gold = 500;
        expect(nexus_v1_shop_buy(&mgr, &ch, 0), "buy ok");
        expect(ch.inventory[0] == 0xAA, "item in inventory");
        expect(mgr.shops[si].stock[0].stock == 1, "stock decremented");
        expect(ch.gold == 400, "gold deducted");
    }

    /* Test 6: buy when out of stock */
    {
        Nexus_ShopManager mgr;
        Nexus_V1_Champion ch;
        int si;
        nexus_v1_shop_manager_init(&mgr);
        si = nexus_v1_shop_register(&mgr, 1, "S");
        nexus_v1_shop_add_stock(&mgr, si, 0xBB, 50, 0);
        nexus_v1_shop_open(&mgr, si);
        memset(&ch, 0, sizeof(ch));
        memset(ch.inventory, 0xFF, sizeof(ch.inventory));
        expect(!nexus_v1_shop_buy(&mgr, &ch, 0), "cannot buy out of stock");
    }

    /* Test 7: buy when inventory full */
    {
        Nexus_ShopManager mgr;
        Nexus_V1_Champion ch;
        int si;
        nexus_v1_shop_manager_init(&mgr);
        si = nexus_v1_shop_register(&mgr, 1, "S");
        nexus_v1_shop_add_stock(&mgr, si, 0xCC, 100, 5);
        nexus_v1_shop_open(&mgr, si);
        memset(&ch, 0, sizeof(ch));
        memset(ch.inventory, 0x01, sizeof(ch.inventory));
        ch.gold = 500;
        expect(!nexus_v1_shop_buy(&mgr, &ch, 0), "cannot buy full inv");
    }

    /* Test 8: cannot buy without gold */
    {
        Nexus_ShopManager mgr;
        Nexus_V1_Champion ch;
        int si;
        nexus_v1_shop_manager_init(&mgr);
        si = nexus_v1_shop_register(&mgr, 1, "S");
        nexus_v1_shop_add_stock(&mgr, si, 0xAA, 100, 2);
        nexus_v1_shop_open(&mgr, si);
        memset(&ch, 0, sizeof(ch));
        memset(ch.inventory, 0xFF, sizeof(ch.inventory));
        ch.gold = 50;
        expect(!nexus_v1_shop_buy(&mgr, &ch, 0), "cannot buy no gold");
    }

    /* Test 9: sell item */
    {
        Nexus_ShopManager mgr;
        Nexus_V1_Champion ch;
        int si;
        nexus_v1_shop_manager_init(&mgr);
        si = nexus_v1_shop_register(&mgr, 1, "S");
        nexus_v1_shop_open(&mgr, si);
        memset(&ch, 0, sizeof(ch));
        memset(ch.inventory, 0xFF, sizeof(ch.inventory));
        ch.inventory[0] = 0x9C;
        ch.gold = 0;
        expect(nexus_v1_shop_sell(&mgr, &ch, 0), "sell ok");
        expect(ch.inventory[0] == 0xFF, "slot cleared");
        expect(ch.gold == 250, "gold received");
    }

    /* Test 10: find by id */
    {
        Nexus_ShopManager mgr;
        nexus_v1_shop_manager_init(&mgr);
        nexus_v1_shop_register(&mgr, 10, "A");
        nexus_v1_shop_register(&mgr, 20, "B");
        expect(nexus_v1_shop_find_by_id(&mgr, 20) == 1, "find shop B");
        expect(nexus_v1_shop_find_by_id(&mgr, 99) == -1, "not found");
    }

    /* Test 11: NULL safety */
    {
        nexus_v1_shop_manager_init(NULL);
        expect(nexus_v1_shop_register(NULL, 0, NULL) == -1, "NULL register");
        expect(nexus_v1_shop_add_stock(NULL, 0, 0, 0, 0) == -1, "NULL add");
        expect(!nexus_v1_shop_open(NULL, 0), "NULL open");
        nexus_v1_shop_close(NULL);
        expect(!nexus_v1_shop_is_open(NULL), "NULL is_open");
        expect(!nexus_v1_shop_buy(NULL, NULL, 0), "NULL buy");
        expect(nexus_v1_shop_find_by_id(NULL, 0) == -1, "NULL find");
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus shop manager verified\n");
    return 0;
}
