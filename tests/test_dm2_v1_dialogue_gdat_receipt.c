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
    uint8_t raw[144];
    uint32_t offsets[3] = { 0u, 40u, 80u };
    uint32_t sizes[3] = { 40u, 40u, 40u };
    DM2_V1_GdatEntry entries[4];
    DM2_V1_AssetLoader loader;
    DM2_V1_DialogueGdatReceipt receipt;
    DM2_V1_DialogueBoxGdatReceipt box_receipt;
    DM2_V1_DialogueBoxDrawPlan box_plan;
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
    raw[80u] = 48u;
    raw[82u] = 24u;
    raw[84u] = 4u;
    for (i = 0; i < 16; ++i) raw[104u + (size_t)i] = (uint8_t)(0xa0 + i);
    entries[2].cls1 = DM2_GDAT_CATEGORY_DIALOG_BOXES;
    entries[2].cls2 = DM2_V1_DIALOGUE_BOX_INDEX;
    entries[2].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[2].cls4 = DM2_V1_DIALOGUE_BOX_FIELD;
    entries[2].data_index = 2u;
    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 3u;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 3u;
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
    check(dm2_v1_dialogue_box_gdat_receipt(&loader, &box_receipt) &&
              box_receipt.valid && box_receipt.metadata.width == 48u &&
              box_receipt.metadata.height == 24u &&
              box_receipt.palette_hash != 0u && box_receipt.receipt_hash != 0u,
          "save dialogue receipt binds skproject dialog-box image and palette");
    check(dm2_v1_dialogue_box_draw_plan(&loader, &box_plan) &&
              box_plan.valid &&
              box_plan.gdat_category == DM2_GDAT_CATEGORY_DIALOG_BOXES &&
              box_plan.gdat_index == DM2_V1_DIALOGUE_BOX_INDEX &&
              box_plan.gdat_field == DM2_V1_DIALOGUE_BOX_FIELD &&
              box_plan.expanded_rect_index == DM2_V1_DIALOGUE_BOX_RECT_INDEX &&
              box_plan.text_y_offset == DM2_V1_DIALOGUE_BOX_TEXT_Y_OFFSET &&
              box_plan.text_palette_slot == DM2_V1_DIALOGUE_BOX_TEXT_PALETTE_SLOT &&
              box_plan.highlight_palette_slot ==
                  DM2_V1_DIALOGUE_BOX_HIGHLIGHT_PALETTE_SLOT &&
              box_plan.optional_highlight_clear && box_plan.plan_hash != 0u,
          "save dialogue plan keeps skproject RECT_453, palette, and text semantics");
    entries[2].cls2 = 0x80u;
    check(!dm2_v1_dialogue_box_gdat_receipt(&loader, &box_receipt),
          "save dialogue receipt rejects a non-source dialog-box index");
    check(!dm2_v1_dialogue_box_draw_plan(&loader, &box_plan),
          "save dialogue plan rejects a non-source dialog-box index");
    entries[2].cls2 = DM2_V1_DIALOGUE_BOX_INDEX;
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
