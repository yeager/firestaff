/*
 * test_dm2_v1_skcore2_gdat_word_helpers.c
 *
 * Focused SKWIN/SkWinCore2 GDAT word helper receipts.
 */

#include "dm2_v1_asset_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static void fixture_loader(DM2_V1_AssetLoader *loader,
                           uint32_t raw_offsets[1],
                           uint32_t raw_sizes[1],
                           DM2_V1_GdatEntry entries[5])
{
    memset(loader, 0, sizeof(*loader));
    memset(entries, 0, sizeof(DM2_V1_GdatEntry) * 5u);
    raw_offsets[0] = 0u;
    raw_sizes[0] = 1u;

    entries[0] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_POTIONS, 2u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x4du, 0u, 0u, 0x000du};
    entries[1] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_POTIONS, 2u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x05u, 0u, 0u, 0x0031u};
    entries[2] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_MISCELLANEOUS, 3u,
        DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 0x43u, 0u, 0u, 0x0042u};
    entries[3] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 4u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x20u, 0u, 0u, 0x0001u};
    entries[4] = (DM2_V1_GdatEntry){
        DM2_GDAT_CATEGORY_DOORS, 5u, DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
        0x20u, 0u, 0u, 0x0000u};

    loader->loaded = 1;
    loader->category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->raw_data_count = 1u;
    loader->entries = entries;
    loader->entry_count = 5u;
}

int main(void)
{
    DM2_V1_AssetLoader loader;
    uint32_t raw_offsets[1];
    uint32_t raw_sizes[1];
    DM2_V1_GdatEntry entries[5];
    DM2_V1_GdatWordQueryReceipt word;

    printf("DM2 SkWinCore2 GDAT word helper receipts\n");
    fixture_loader(&loader, raw_offsets, raw_sizes, entries);

    CHECK(dm2_v1_query_gdat_potion_spell_type_from_record_receipt(
              &loader, DM2_GDAT_CATEGORY_POTIONS, 2, &word) &&
              word.accepted && word.category == DM2_GDAT_CATEGORY_POTIONS &&
              word.field == 0x4du && word.value == 0x000du,
          "QUERY_GDAT_POTION_SPELL_TYPE_FROM_RECORD binds DBSPEC field 0x4d");
    CHECK(dm2_v1_query_gdat_potion_behaviour_from_record_receipt(
              &loader, DM2_GDAT_CATEGORY_POTIONS, 2, &word) &&
              word.accepted && word.field == 0x05u && word.value == 0x0031u,
          "QUERY_GDAT_POTION_BEHAVIOUR_FROM_RECORD binds DBSPEC field 0x05");
    CHECK(dm2_v1_query_gdat_water_value_from_record_receipt(
              &loader, DM2_GDAT_CATEGORY_MISCELLANEOUS, 3, &word) &&
              word.accepted && word.field == 0x43u && word.value == 0x0042u,
          "QUERY_GDAT_WATER_VALUE_FROM_RECORD binds DBSPEC field 0x43");
    CHECK(dm2_v1_query_gdat_door_is_mirrored_receipt(
              &loader, 4, &word) &&
              word.accepted && word.category == DM2_GDAT_CATEGORY_DOORS &&
              word.field == 0x20u && word.value == 1u,
          "QUERY_GDAT_DOOR_IS_MIRRORED binds DOORS field 0x20");
    CHECK(dm2_v1_query_gdat_door_is_mirrored_receipt(
              &loader, 5, &word) &&
              word.accepted && word.field == 0x20u && word.value == 0u,
          "QUERY_GDAT_DOOR_IS_MIRRORED preserves explicit zero mirror flag");
    CHECK(!dm2_v1_query_gdat_potion_spell_type_from_record_receipt(
              &loader, DM2_GDAT_CATEGORY_POTIONS, 9, &word),
          "SkWinCore2 GDAT word helpers reject absent source rows");

    printf("DM2 SkWinCore2 GDAT word helper receipts: %d passed, %d failed\n",
           passed, failed);
    return failed == 0 ? 0 : 1;
}
