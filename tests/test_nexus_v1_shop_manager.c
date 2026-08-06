#include <stdio.h>
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
    int index;

    nexus_v1_shop_manager_init(&manager);
    expect(manager.count == 0 && !nexus_v1_shop_is_open(&manager),
           "shop manager starts closed and empty");

    /* DM.BIN price rows are real metadata, but no retail Saturn shop-object
     * owner, stock chain or open/close dispatcher has been authenticated. */
    index = nexus_v1_shop_register(&manager, 42, "Armory");
    expect(index == -1 && manager.count == 0,
           "unproven shop registration is blocked");
    expect(nexus_v1_shop_add_stock(&manager, index, 0x9C, 500, 3) == -1,
           "unproven shop stock is blocked");
    expect(!nexus_v1_shop_open(&manager, index),
           "unproven shop open is blocked");
    expect(nexus_v1_shop_find_by_id(&manager, 42) == -1,
           "unproven shop lookup has no owner");

    champion = (Nexus_V1_Champion){0};
    expect(!nexus_v1_shop_buy(&manager, &champion, 0),
           "buy remains fail-closed");
    expect(!nexus_v1_shop_sell(&manager, &champion, 0),
           "sell remains fail-closed");
    nexus_v1_shop_close(&manager);
    expect(!nexus_v1_shop_is_open(&manager),
           "blocked shop remains closed");

    if (g_fail) {
        fprintf(stderr, "test_nexus_v1_shop_manager: %d failure(s)\n", g_fail);
        return 1;
    }
    puts("ok: Nexus shop provenance gate verified");
    return 0;
}
