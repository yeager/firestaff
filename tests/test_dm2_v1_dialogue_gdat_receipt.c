#include "dm2_v1_dialogue_gdat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *name)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    } else {
        fprintf(stderr, "PASS: %s\n", name);
    }
}

int main(void)
{
    uint8_t raw[96];
    uint32_t offsets[2] = { 0u, 40u };
    uint32_t sizes[2] = { 40u, 40u };
    DM2_V1_GdatEntry entries[4];
    DM2_V1_AssetLoader loader;
    DM2_V1_DialogueGdatReceipt receipt;
    int i;

    memset(raw, 0, sizeof(raw));
    memset(entries, 0, sizeof(entries));
    memset(&loader, 0, sizeof(loader));
    /* Minimal real-shaped 4bpp IMG3 records: ten-byte header + 14 payload
     * bytes + the exact 16-byte local-palette tail. */
    for (i = 0; i < 2; ++i) {
        size_t at = offsets[i];
        raw[at + 0u] = (uint8_t)(32 + i);
        raw[at + 2u] = (uint8_t)(16 + i);
        raw[at + 4u] = 4u;
        for (int p = 0; p < 16; ++p) raw[at + 24u + (size_t)p] = (uint8_t)(p + i);
        entries[i].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
        entries[i].cls2 = 7u;
        entries[i].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
        entries[i].cls4 = i == 0 ? 0xfdu : DM2_V1_DIALOGUE_GLYPH_FIELD;
        entries[i].data_index = (uint16_t)i;
    }
    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 2u;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 2u;
    loader.data = raw;
    loader.data_size = sizeof(raw);

    check(dm2_v1_dialogue_gdat_receipt(&loader, 7u, 0xfdu, &receipt) &&
              receipt.valid && receipt.graphicsset == 7u &&
              receipt.shell_field == 0xfdu &&
              receipt.shell_metadata.width == 32u &&
              receipt.glyph_metadata.width == 33u &&
              receipt.shell_palette_hash != 0u &&
              receipt.glyph_palette_hash != 0u && receipt.receipt_hash != 0u,
          "dialogue receipt binds source shell, glyph and local palettes");
    check(!dm2_v1_dialogue_gdat_receipt(&loader, 7u, 0xfbu, &receipt),
          "dialogue receipt rejects a non-source shell field");
    entries[1].cls2 = 8u;
    check(!dm2_v1_dialogue_gdat_receipt(&loader, 7u, 0xfdu, &receipt),
          "dialogue receipt rejects a glyph from another graphics set");
    entries[1].cls2 = 7u;
    raw[offsets[1] + 4u] = 8u;
    check(!dm2_v1_dialogue_gdat_receipt(&loader, 7u, 0xfdu, &receipt),
          "dialogue receipt rejects a non-4bpp image without a source palette");

    fprintf(stderr, "DM2 dialogue GDAT receipt: %d failure(s)\n", failures);
    return failures != 0;
}
