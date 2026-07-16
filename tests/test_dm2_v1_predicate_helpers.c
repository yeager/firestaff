#include "dm2_v1_predicate_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char* label) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        failures++;
    }
}

static uint8_t tile(int type) {
    return (uint8_t)((type & 7) << 5);
}

static void test_tile_passage(void) {
    DM2_V1_PredicateReceipt receipt;

    expect_true(dm2_v1_IS_TILE_PASSAGE(tile(1), 0, 0, &receipt) == 1,
                "ordinary floor is passable");
    expect_true(receipt.valid && receipt.result == 1 &&
                    strcmp(receipt.symbol, "IS_TILE_PASSAGE") == 0,
                "tile passage receipt names source symbol");
    expect_true(dm2_v1_IS_TILE_PASSAGE(tile(0), 0, 0, &receipt) == 0,
                "wall blocks passage");
    expect_true(dm2_v1_IS_TILE_PASSAGE(tile(7), 0, 0, &receipt) == 0,
                "map exit blocks passage");
    expect_true(dm2_v1_IS_TILE_PASSAGE(tile(5), 1, 1, &receipt) == 0,
                "active teleporter record blocks passage");
    expect_true(dm2_v1_IS_TILE_PASSAGE(tile(5), 1, 0, &receipt) == 1,
                "inactive teleporter allows passage");
}

static void test_dbspec_word_value(void) {
    DM2_V1_PredicateReceipt receipt;
    size_t count = ((size_t)2 * 256U + 3U) * 256U + 5U + 1U;
    uint16_t* values = (uint16_t*)calloc(count, sizeof(uint16_t));
    expect_true(values != NULL, "allocated dbspec fixture");
    values[((size_t)2 * 256U + 3U) * 256U + 5U] = 0xbeefU;

    expect_true(dm2_v1_QUERY_GDAT_DBSPEC_WORD_VALUE(
                    7, 2, 3, 5, values, count, &receipt) == 0xbeefU,
                "dbspec word returns indexed value");
    expect_true(receipt.valid && strcmp(receipt.symbol,
                                        "QUERY_GDAT_DBSPEC_WORD_VALUE") == 0,
                "dbspec receipt names source symbol");
    expect_true(dm2_v1_QUERY_GDAT_DBSPEC_WORD_VALUE(
                    0xffffU, 2, 3, 5, values, count, &receipt) == 0,
                "object null returns zero");
    expect_true(receipt.valid && !receipt.blocked,
                "object null is valid zero path");
    expect_true(dm2_v1_QUERY_GDAT_DBSPEC_WORD_VALUE(
                    7, 2, 3, 6, values, count, &receipt) == 0,
                "missing dbspec returns zero");
    expect_true(receipt.blocked, "missing dbspec is blocked receipt");
    free(values);
}

static void test_player_ability(void) {
    DM2_V1_PlayerAbility abilities[3] = {
        {5, 30, 0},
        {100, 140, 8},
        {230, 240, 0}
    };
    DM2_V1_PredicateReceipt receipt;

    expect_true(dm2_v1_GET_PLAYER_ABILITY(
                    abilities, 3, 0, 0, 0, 0, 0, &receipt) == 10,
                "ability current clamps low to 10");
    expect_true(dm2_v1_GET_PLAYER_ABILITY(
                    abilities, 3, 1, 1, 80, 3, 99, &receipt) == 148,
                "max ability ignores enchantment and adds enhanced");
    expect_true(dm2_v1_GET_PLAYER_ABILITY(
                    abilities, 3, 1, 0, 100, 3, 7, &receipt) == 119,
                "current matching aura applies deterministic enchantment");
    expect_true(dm2_v1_GET_PLAYER_ABILITY(
                    abilities, 3, 2, 0, 0, 0, 0, &receipt) == 220,
                "ability clamps high to 220");
    expect_true(receipt.valid && strcmp(receipt.symbol,
                                        "GET_PLAYER_ABILITY") == 0,
                "ability receipt names source symbol");
    expect_true(dm2_v1_GET_PLAYER_ABILITY(
                    abilities, 3, 9, 0, 0, 0, 0, &receipt) == 0 &&
                    receipt.blocked,
                "ability invalid index blocks");
}

static void test_bar_colors(void) {
    uint16_t charsheet[8] = {0};
    uint16_t general[8] = {0};
    DM2_V1_PredicateReceipt receipt;
    charsheet[6] = 12;
    general[4] = 9;

    expect_true(dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
                    6, 3, charsheet, 8, &receipt) == 268,
                "food/water bar color adds 256 to GDAT word");
    expect_true(receipt.valid && strcmp(receipt.symbol,
                                        "QUERY_FOOD_WATER_BAR_COLOR") == 0,
                "food/water receipt names source symbol");
    expect_true(dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
                    9, 3, charsheet, 8, &receipt) == 3,
                "food/water missing word returns default");
    expect_true(dm2_v1_QUERY_3STAT_BAR_COLOR(
                    4, 5, general, 8, &receipt) == 9,
                "3stat bar color returns GDAT word directly");
    expect_true(receipt.valid && strcmp(receipt.symbol,
                                        "QUERY_3STAT_BAR_COLOR") == 0,
                "3stat receipt names source symbol");
    expect_true(dm2_v1_QUERY_3STAT_BAR_COLOR(
                    4, 5, NULL, 0, &receipt) == 5,
                "3stat missing table returns default");
}

int main(void) {
    test_tile_passage();
    test_dbspec_word_value();
    test_player_ability();
    test_bar_colors();
    expect_true(strstr(dm2_v1_predicate_helpers_source_evidence(),
                       "SKWIN/SkWinCore.cpp") != NULL,
                "source evidence names skproject");
    if (failures) {
        return 1;
    }
    puts("DM2 predicate helpers: ok");
    return 0;
}
