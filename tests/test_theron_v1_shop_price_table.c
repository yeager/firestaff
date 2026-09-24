/*
 * Theron V1 shop price-table regression.
 *
 * Test-only price rows exercise parser bounds and purchase-state atomicity;
 * they are not Theron game data and are never admitted to production. When
 * authenticated local media is available, the buyer is initialized from its
 * real regional roster record rather than fixture champion defaults.
 */

#include "theron_v1_shop.h"
#include "theron_v1_champions.h"
#include "theron_v1_track02.h"
#include "theron_v1_dungeon_handoff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static void expect_status(Theron_ShopStatus got,
                          Theron_ShopStatus expected,
                          const char *message) {
    if (got != expected) {
        fprintf(stderr,
                "FAIL: %s (got %s, expected %s)\n",
                message,
                theron_v1_shop_status_name(got),
                theron_v1_shop_status_name(expected));
        ++g_failures;
    }
}

static void test_parse_fixture(void) {
    static const uint8_t fixture[] = {
        THERON_ITEM_POTION, 0x19, 0x00, 0x02,
        THERON_ITEM_KEY,    0xf4, 0x01, 0x01,
        THERON_ITEM_ARMOR,  0x20, 0x03, 0x00
    };
    Theron_ShopPriceTable table;

    expect_status(theron_v1_shop_parse_price_table(&table,
                                                   fixture,
                                                   sizeof(fixture)),
                  THERON_SHOP_OK,
                  "valid fixture parses");
    expect_true(table.count == 3u, "fixture entry count");
    expect_true(table.entries[0].item_id == THERON_ITEM_POTION,
                "first item id");
    expect_true(table.entries[0].price == 25u, "little-endian potion price");
    expect_true(table.entries[0].stock == 2u, "potion stock");
    expect_true(table.entries[1].price == 500u, "little-endian key price");
    expect_true(table.entries[2].stock == 0u, "zero stock is preserved");
}

static void test_parse_rejects_bad_tables(void) {
    uint8_t too_many[(THERON_SHOP_MAX_ENTRIES + 1u) * THERON_SHOP_PRICE_ROW_SIZE];
    static const uint8_t truncated[] = {
        THERON_ITEM_POTION, 0x19, 0x00
    };
    static const uint8_t zero_price[] = {
        THERON_ITEM_POTION, 0x00, 0x00, 0x01
    };
    static const uint8_t quest_item[] = {
        THERON_ITEM_QUEST_1, 0x19, 0x00, 0x01
    };
    Theron_ShopPriceTable table;
    size_t i;

    for (i = 0; i < sizeof(too_many); i += THERON_SHOP_PRICE_ROW_SIZE) {
        too_many[i + 0u] = THERON_ITEM_FOOD;
        too_many[i + 1u] = 0x01;
        too_many[i + 2u] = 0x00;
        too_many[i + 3u] = 0x01;
    }

    expect_status(theron_v1_shop_parse_price_table(NULL,
                                                   truncated,
                                                   sizeof(truncated)),
                  THERON_SHOP_BAD_INPUT,
                  "NULL output is rejected");
    expect_status(theron_v1_shop_parse_price_table(&table,
                                                   truncated,
                                                   sizeof(truncated)),
                  THERON_SHOP_BAD_TABLE_SIZE,
                  "partial row is rejected");
    expect_status(theron_v1_shop_parse_price_table(&table,
                                                   too_many,
                                                   sizeof(too_many)),
                  THERON_SHOP_TOO_MANY_ENTRIES,
                  "oversized price table is rejected");
    expect_status(theron_v1_shop_parse_price_table(&table,
                                                   zero_price,
                                                   sizeof(zero_price)),
                  THERON_SHOP_BAD_ITEM,
                  "zero-price item is rejected");
    expect_true(table.count == 0u, "failed parse clears table");
    expect_status(theron_v1_shop_parse_price_table(&table,
                                                   quest_item,
                                                   sizeof(quest_item)),
                  THERON_SHOP_BAD_ITEM,
                  "quest item is not buyable");
}

static void test_purchase_state_is_atomic(void) {
    static const uint8_t fixture[] = {
        THERON_ITEM_POTION, 0x19, 0x00, 0x02,
        THERON_ITEM_KEY,    0xf4, 0x01, 0x01,
        THERON_ITEM_ARMOR,  0x20, 0x03, 0x00
    };
    Theron_ShopPriceTable table;
    Theron_V1_Party party;
    uint8_t *track02_data = NULL;
    size_t track02_size = 0u;
    char track02_path[1024];
    char md5[33];
    const char *home = getenv("HOME");
    uint32_t gold_before;
    uint8_t stock_before;
    int i;

    if (!home || !home[0] ||
        snprintf(track02_path, sizeof(track02_path),
                 "%s/.firestaff/data/theron/TQUS02.bin", home) >=
            (int)sizeof(track02_path)) {
        puts("SKIP: authenticated Theron roster media unavailable; "
             "synthetic price rows are not used as game data");
        return;
    }
    {
        FILE *file = fopen(track02_path, "rb");
        long file_size;
        if (!file || fseek(file, 0, SEEK_END) != 0 ||
            (file_size = ftell(file)) <= 0 ||
            fseek(file, 0, SEEK_SET) != 0 ||
            !(track02_data = (uint8_t *)malloc((size_t)file_size)) ||
            fread(track02_data, 1u, (size_t)file_size, file) !=
                (size_t)file_size) {
            if (file) fclose(file);
            free(track02_data);
            puts("SKIP: authenticated Theron roster media unreadable; "
                 "synthetic price rows are not used as game data");
            return;
        }
        fclose(file);
        track02_size = (size_t)file_size;
    }
    snprintf(md5, sizeof(md5), "%s", THERON_TRACK02_MD5_US_BIN);
    if (!theron_v1_track02_raw_bytes_match_md5(
            track02_data, track02_size, md5)) {
        free(track02_data);
        puts("SKIP: Track 02 roster source is not the authenticated US BIN");
        return;
    }

    theron_v1_party_init(&party, 1);
    expect_true(theron_v1_party_init_theron_from_track02(
                    &party, track02_data, track02_size, md5),
                "buyer initializes from authenticated Theron roster");
    free(track02_data);
    if (party.champion_count != 1) return;
    party.gold = 100u;

    expect_status(theron_v1_shop_parse_price_table(&table,
                                                   fixture,
                                                   sizeof(fixture)),
                  THERON_SHOP_OK,
                  "purchase fixture parses");
    expect_status(theron_v1_shop_purchase_item(&party,
                                               &table,
                                               THERON_ITEM_POTION,
                                               THERON_CHAMPION_SLOT_THERON),
                  THERON_SHOP_OK,
                  "potion purchase succeeds");
    expect_true(party.gold == 75u, "gold deducted exactly once");
    expect_true(table.entries[0].stock == 1u, "stock decremented exactly once");
    expect_true(party.champions[0].inventory[0] == THERON_ITEM_POTION,
                "purchased item placed in first empty slot");
    expect_true(party.champions[0].load == 1, "load recalculated after purchase");

    gold_before = party.gold;
    stock_before = table.entries[1].stock;
    expect_status(theron_v1_shop_purchase_item(&party,
                                               &table,
                                               THERON_ITEM_KEY,
                                               THERON_CHAMPION_SLOT_THERON),
                  THERON_SHOP_NOT_ENOUGH_GOLD,
                  "expensive item rejects when gold is short");
    expect_true(party.gold == gold_before, "failed purchase keeps gold");
    expect_true(table.entries[1].stock == stock_before, "failed purchase keeps stock");

    gold_before = party.gold;
    stock_before = table.entries[2].stock;
    expect_status(theron_v1_shop_purchase_item(&party,
                                               &table,
                                               THERON_ITEM_ARMOR,
                                               THERON_CHAMPION_SLOT_THERON),
                  THERON_SHOP_OUT_OF_STOCK,
                  "zero-stock item rejects");
    expect_true(party.gold == gold_before, "out-of-stock keeps gold");
    expect_true(table.entries[2].stock == stock_before, "out-of-stock keeps stock");

    party.gold = 1000u;
    table.entries[0].stock = 2u;
    for (i = 0; i < THERON_INVENTORY_SLOTS; ++i) {
        party.champions[0].inventory[i] = THERON_ITEM_FOOD;
    }
    theron_v1_party_recalculate_loads(&party);
    gold_before = party.gold;
    stock_before = table.entries[0].stock;
    expect_status(theron_v1_shop_purchase_item(&party,
                                               &table,
                                               THERON_ITEM_POTION,
                                               THERON_CHAMPION_SLOT_THERON),
                  THERON_SHOP_INVENTORY_FULL,
                  "full inventory rejects purchase");
    expect_true(party.gold == gold_before, "full inventory keeps gold");
    expect_true(table.entries[0].stock == stock_before, "full inventory keeps stock");
}

int main(void) {
    const char *evidence = theron_v1_shop_source_evidence();

    test_parse_fixture();
    test_parse_rejects_bad_tables();
    test_purchase_state_is_atomic();

    expect_true(evidence != NULL && strstr(evidence, "THQUEST.ASM T560") != NULL,
                "source evidence cites Theron item table");
    expect_true(evidence != NULL && strstr(evidence, "ReDMCSB") != NULL,
                "source evidence records ReDMCSB audit boundary");

    if (g_failures != 0) {
        fprintf(stderr,
                "Theron V1 shop price-table regression FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: Theron V1 shop price-table parsing and purchase guards");
    return 0;
}
