#include "dm2_v1_extended_spells_definition.h"

#include <stdio.h>
#include <string.h>

static int g_checks;
static int g_failures;

static void expect_true(int condition, const char* label)
{
    ++g_checks;
    if (!condition) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

static void set_word(DM2_V1_GdatEntry* entry,
                     int spellIndex,
                     int field,
                     uint16_t value)
{
    memset(entry, 0, sizeof(*entry));
    entry->cls1 = DM2_GDAT_CATEGORY_SPELL_DEF;
    entry->cls2 = (uint8_t)spellIndex;
    entry->cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entry->cls4 = (uint8_t)field;
    entry->data_index = value;
}

static void set_text(DM2_V1_GdatEntry* entry,
                     int spellIndex,
                     uint16_t rawIndex)
{
    memset(entry, 0, sizeof(*entry));
    entry->cls1 = DM2_GDAT_CATEGORY_SPELL_DEF;
    entry->cls2 = (uint8_t)spellIndex;
    entry->cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
    entry->cls4 = 0x18u;
    entry->data_index = rawIndex;
}

int main(void)
{
    DM2_V1_GdatEntry entries[15];
    uint32_t rawOffsets[1] = {0u};
    uint32_t rawSizes[1] = {12u};
    const uint8_t rawData[] = "FUL IR TEST";
    uint16_t originalW6[DM2_V1_EXT_SPELLS_ORIGINAL_COUNT];
    DM2_V1_AssetLoader loader;
    DM2_V1_ExtendedSpellsReceipt receipt;
    int i;

    memset(entries, 0, sizeof(entries));
    memset(&loader, 0, sizeof(loader));
    for (i = 0; i < DM2_V1_EXT_SPELLS_ORIGINAL_COUNT; ++i) {
        originalW6[i] = (uint16_t)((i & 0x3f) << 4);
    }

    set_word(&entries[0], 3, 0x01, 7u);
    set_word(&entries[1], 3, 0x02, 1u);
    set_word(&entries[2], 3, 0x03, 2u);
    set_word(&entries[3], 3, 0x04, 13u);
    set_word(&entries[4], 3, 0x05, 15u);
    set_word(&entries[5], 3, 0x06, 2u);
    set_word(&entries[6], 3, 0x07, 68u);
    set_text(&entries[7], 3, 0u);
    set_word(&entries[8], 42, 0x01, 14u);
    set_word(&entries[9], 42, 0x02, 6u);
    set_word(&entries[10], 42, 0x03, 11u);
    set_word(&entries[11], 42, 0x04, 9u);
    set_word(&entries[12], 42, 0x05, 49u);
    set_word(&entries[13], 42, 0x06, 4u);
    set_word(&entries[14], 42, 0x07, 0x31u);

    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = (uint16_t)(sizeof(entries) / sizeof(entries[0]));
    loader.raw_offsets = rawOffsets;
    loader.raw_sizes = rawSizes;
    loader.raw_data_count = 1u;
    loader.data = rawData;
    loader.data_size = sizeof(rawData);

    expect_true(dm2_v1_extended_load_spells_definition(
                    &loader, 0, originalW6,
                    DM2_V1_EXT_SPELLS_ORIGINAL_COUNT, &receipt) == 0,
                "disabled extended mode returns skproject zero");
    expect_true(receipt.handled && !receipt.loaded && !receipt.failClosed,
                "disabled route is handled without mutation");

    expect_true(dm2_v1_extended_load_spells_definition(
                    &loader, 1, originalW6,
                    DM2_V1_EXT_SPELLS_ORIGINAL_COUNT, &receipt) == 1,
                "enabled extended mode loads real GDAT rows");
    expect_true(receipt.loaded && receipt.customLoadedCount == 2,
                "two nonzero rune1 rows loaded");
    expect_true(receipt.firstLoadedIndex == 3 && receipt.lastLoadedIndex == 42,
                "receipt preserves sparse GDAT spell indexes");
    expect_true(receipt.custom[3].dw0 == 0x00070102u &&
                    receipt.custom[3].difficulty == 13u &&
                    receipt.custom[3].requiredSkill == 15u,
                "MkssymVal and difficulty/skill match skproject packing");
    expect_true(receipt.custom[3].w6 == 0x0042u &&
                    receipt.custom[3].spellValue == 68u,
                "w6 masks result to six bits but spellValue preserves result");
    expect_true(receipt.custom[3].textLoaded &&
                    strcmp(receipt.custom[3].name, "FUL IR TEST") == 0,
                "QUERY_GDAT_TEXT field 0x18 is consumed when present");
    expect_true(receipt.custom[42].dw0 == 0x000e060bu &&
                    receipt.custom[42].w6 == 0x0314u &&
                    receipt.custom[42].spellValue == 0x31u,
                "second custom row preserves summon result");
    expect_true(receipt.originalAdaptedCount == 33 &&
                    receipt.originalSpellValue[0] == 0u &&
                    receipt.originalSpellValue[32] == 32u &&
                    receipt.originalSpellValue[33] == 0u,
                "original spell table adapts only MAXSPELL_ORIGINAL-1 rows");
    expect_true(receipt.tableHash != 0u, "receipt has stable nonzero hash");

    entries[13].data_index = 0x0104u;
    expect_true(dm2_v1_extended_load_spells_definition(
                    &loader, 1, originalW6,
                    DM2_V1_EXT_SPELLS_ORIGINAL_COUNT, &receipt) == 0,
                "wide U8 field fails closed");
    expect_true(receipt.failClosed && receipt.wideRequiredField,
                "wide U8 field is reported");
    entries[13].data_index = 4u;
    entries[14].cls4 = 0x08u;
    expect_true(dm2_v1_extended_load_spells_definition(
                    &loader, 1, originalW6,
                    DM2_V1_EXT_SPELLS_ORIGINAL_COUNT, &receipt) == 0,
                "missing required field fails closed");
    expect_true(receipt.failClosed && receipt.missingRequiredField,
                "missing field is reported");

    printf("DM2 extended spells definition: %d/%d checks passed\n",
           g_checks - g_failures, g_checks);
    return g_failures ? 1 : 0;
}
