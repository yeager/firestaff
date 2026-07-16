#include "dm2_v1_spell.h"

#include <stdio.h>
#include <string.h>

static int g_checks;
static int g_failures;

static void expect_true(int condition, const char *label)
{
    ++g_checks;
    if (!condition) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

static void set_word(DM2_V1_GdatEntry *entry, int spell_index,
                     int field, uint16_t value)
{
    memset(entry, 0, sizeof(*entry));
    entry->cls1 = DM2_GDAT_CATEGORY_SPELL_DEF;
    entry->cls2 = (uint8_t)spell_index;
    entry->cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entry->cls4 = (uint8_t)field;
    entry->data_index = value;
}

int main(void)
{
    DM2_V1_GdatEntry entries[7];
    DM2_V1_AssetLoader loader;
    DM2_V1_GDATSpellReceipt receipt;
    const uint16_t fields[7] = { 7, 1, 2, 13, 15, 2, 4 };
    uint32_t raw_offsets[1] = { 0 };
    uint32_t raw_sizes[1] = { 1 };
    uint8_t raw_data[1] = { 0 };
    int i;

    memset(&loader, 0, sizeof(loader));
    for (i = 0; i < 7; ++i) set_word(&entries[i], 42, i + 1, fields[i]);
    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 7;
    loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes;
    loader.raw_data_count = 1;
    loader.data = raw_data;
    loader.data_size = sizeof(raw_data);

    expect_true(dm2_v1_spell_gdat_receipt(&loader, 42, &receipt) == 1,
                "complete source spell fields materialize");
    expect_true(receipt.valid && receipt.index == 42,
                "receipt identifies the source spell index");
    expect_true(receipt.rune1 == 7 && receipt.rune2 == 1 &&
                    receipt.rune3 == 2 && receipt.symbols == 0x070102u,
                "receipt preserves MkssymVal rune packing");
    expect_true(receipt.difficulty == 13 && receipt.required_skill == 15 &&
                    receipt.spell_type == 2 && receipt.result == 4 &&
                    receipt.type_and_result == 0x42u,
                "receipt preserves type/result packing");

    entries[6].cls4 = 8;
    expect_true(dm2_v1_spell_gdat_receipt(&loader, 42, &receipt) == 0 &&
                    !receipt.valid,
                "missing source field rejects the whole receipt");
    entries[6].cls4 = 7;
    entries[0].data_index = 0;
    expect_true(dm2_v1_spell_gdat_receipt(&loader, 42, &receipt) == 0 &&
                    !receipt.valid,
                "zero first rune rejects an unused source row");
    entries[0].data_index = 7;
    entries[4].data_index = 256;
    expect_true(dm2_v1_spell_gdat_receipt(&loader, 42, &receipt) == 0 &&
                    !receipt.valid,
                "wide scalar rejects U8-only original fields");

    printf("DM2 GDAT spell receipt: %d/%d checks passed\n",
           g_checks - g_failures, g_checks);
    return g_failures ? 1 : 0;
}
