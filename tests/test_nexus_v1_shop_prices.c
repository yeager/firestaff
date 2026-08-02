
#include <stdio.h>
#include "nexus_v1_shop.h"

int main(void) {
    const Nexus_ShopEntry *entries;
    int count, fail = 0;

    count = nexus_v1_shop_table(&entries);
    if (count != NEXUS_SHOP_ITEM_COUNT) {
        fprintf(stderr, "FAIL: expected %d entries, got %d\n", NEXUS_SHOP_ITEM_COUNT, count);
        return 1;
    }

    /* Verify first and last entries */
    if (entries[0].item_id != 0x009C || entries[0].price != 500) {
        fprintf(stderr, "FAIL: entry 0 mismatch\n"); fail++;
    }
    if (entries[7].item_id != 0x00A2 || entries[7].price != 1400) {
        fprintf(stderr, "FAIL: entry 7 mismatch\n"); fail++;
    }

    /* Verify duplicate item (0x009E appears at indices 2 and 3) */
    if (entries[2].item_id != 0x009E || entries[2].price != 650 ||
        entries[3].item_id != 0x009E || entries[3].price != 820) {
        fprintf(stderr, "FAIL: duplicate 0x009E mismatch\n"); fail++;
    }

    /* Verify price lookup */
    if (nexus_v1_shop_price(0x00A1) != 990) {
        fprintf(stderr, "FAIL: price lookup 0x00A1\n"); fail++;
    }
    if (nexus_v1_shop_price(0x0001) != -1) {
        fprintf(stderr, "FAIL: unknown item should return -1\n"); fail++;
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus shop price table verified (%d entries from DM.BIN 0x037210)\n", count);
    return 0;
}
