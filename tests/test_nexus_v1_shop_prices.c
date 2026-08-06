
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_v1_shop.h"

static int test_real_dm_bin(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[512];
    FILE *file;
    long size;
    uint8_t *data;
    Nexus_ShopManager manager;
    int i;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 0;
    }
    if (snprintf(path, sizeof(path), "%s/DM.BIN", root) >= (int)sizeof(path) ||
        !(file = fopen(path, "rb"))) {
        puts("SKIP: real Nexus DM.BIN is not mounted");
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 1;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 1;
    }
    fclose(file);
    nexus_v1_shop_manager_init(&manager);
    if (!nexus_v1_shop_bind_dm_bin(&manager, data, (size_t)size, 1) ||
        manager.catalog_count != NEXUS_SHOP_ITEM_COUNT ||
        !manager.catalog_source_bound) {
        free(data);
        fprintf(stderr, "FAIL: retail DM.BIN shop catalog did not bind\n");
        return 1;
    }
    for (i = 0; i < manager.catalog_count; ++i) {
        if (manager.catalog[i].item_id !=
                (uint16_t[]){0x009C, 0x009D, 0x009E, 0x009E,
                             0x009F, 0x00A0, 0x00A1, 0x00A2}[i] ||
            manager.catalog[i].price !=
                (uint16_t[]){500, 600, 650, 820, 550, 350, 990, 1400}[i]) {
            free(data);
            fprintf(stderr, "FAIL: retail DM.BIN shop row %d mismatch\n", i);
            return 1;
        }
    }
    free(data);
    puts("PASS: retail DM.BIN shop catalog bound at 0x037210");
    return 0;
}

int main(void) {
    const Nexus_ShopEntry *entries;
    int count, fail = 0;

    fail += test_real_dm_bin();

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
