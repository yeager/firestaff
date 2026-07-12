#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int failures;

static void expect_true(int condition, const char *label)
{
    ++checks;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

static void set_entry(DM2_V1_GdatEntry *entry, int type, int field,
                      uint16_t value)
{
    memset(entry, 0, sizeof(*entry));
    entry->cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
    entry->cls2 = 0x2a;
    entry->cls3 = (uint8_t)type;
    entry->cls4 = (uint8_t)field;
    entry->data_index = value;
}

int main(void)
{
    DM2_V1_G1Map5TextRuntimeReceipt texts;
    DM2_V1_G1TextWallGfxRuntimeReceipt receipt;
    DM2_V1_GdatEntry entries[5];
    DM2_V1_AssetLoader loader;
    uint32_t offsets[1] = { 0 };
    uint32_t sizes[1] = { 1 };
    uint8_t data[1] = { 0 };

    memset(&texts, 0, sizeof(texts));
    memset(&loader, 0, sizeof(loader));
    texts.committed = 1;
    texts.incomplete_world = 1;
    texts.map = 5;
    texts.text_root_count = 2;
    texts.texts[0].object_id = 0x8800;
    texts.texts[0].mode = 1;
    texts.texts[0].text_index = 0x012a;
    texts.texts[1].object_id = 0x0801;
    texts.texts[1].mode = 0;
    texts.texts[1].text_index = 0x0033;
    set_entry(&entries[0], DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 0x04, 9);
    set_entry(&entries[1], DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 0x05, 12);
    set_entry(&entries[2], DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 0x07, 1);
    set_entry(&entries[3], DM2_GDAT_ENTRY_TYPE_WORD_VALUE, 0x0a, 3);
    set_entry(&entries[4], DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET, 0xfd, 0xfe02);
    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 5;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 1;
    loader.data = data;
    loader.data_size = sizeof(data);

    expect_true(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
                    &texts, &loader, &receipt) == 1 && receipt.valid,
                "source DB2 text receipt reaches GDAT material consumer");
    expect_true(receipt.material_count == 1 &&
                    receipt.materials[0].object_id == 0x8800 &&
                    receipt.materials[0].wall_gfx_index == 0x2a &&
                    receipt.materials[0].position == 12 &&
                    receipt.materials[0].image_offset == 0xfe02,
                "TextMode one uses TextIndex low byte and original ornament fields");

    entries[4].cls4 = 0xfc;
    expect_true(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
                    &texts, &loader, &receipt) == 0 && !receipt.valid,
                "missing original ornament field blocks the full consumer");
    entries[4].cls4 = 0xfd;
    texts.texts[0].mode = 2;
    expect_true(dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
                    &texts, &loader, &receipt) == 1 && receipt.valid &&
                    receipt.material_count == 0,
                "non-ornamental DB2 text roots do not invent a GDAT material");

    printf("DM2 G1 DB2 wall-gfx runtime: %d/%d checks passed\n",
           checks - failures, checks);
    return failures ? 1 : 0;
}
