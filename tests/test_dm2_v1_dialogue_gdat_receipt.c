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
    uint8_t raw[240];
    uint32_t offsets[5] = { 0u, 40u, 80u, 120u, 160u };
    uint32_t sizes[5] = { 40u, 40u, 40u, 40u, 40u };
    DM2_V1_GdatEntry entries[6];
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
    memcpy(raw + offsets[3], "LOAD", 5u);
    entries[3].cls1 = DM2_GDAT_CATEGORY_DIALOG_BOXES;
    entries[3].cls2 = DM2_V1_DIALOGUE_BOX_INDEX;
    entries[3].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
    entries[3].cls4 = 0u;
    entries[3].data_index = 3u;
    memcpy(raw + offsets[4], "CANCEL", 7u);
    entries[4].cls1 = DM2_GDAT_CATEGORY_DIALOG_BOXES;
    entries[4].cls2 = DM2_V1_DIALOGUE_BOX_INDEX;
    entries[4].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
    entries[4].cls4 = 1u;
    entries[4].data_index = 4u;
    /* c_gdatfile.cpp captures this source word before QUERY_GDAT_TEXT.
     * The value zero deliberately selects the unencrypted form of the two
     * literal labels above; absence of this entry must block the panel. */
    entries[5].cls1 = 0u;
    entries[5].cls2 = 0u;
    entries[5].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[5].cls4 = 0u;
    entries[5].data_index = 0u;
    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 6u;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 5u;
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
    {
        DM2_V1_DialogueOpenPanelReceipt open_panel;
        DM2_V1_DialogueSaveInputState input_state;
        DM2_V1_DialogueSaveInputReceipt input_receipt;
        check(dm2_v1_dialogue_open_panel_receipt(&loader, &open_panel) &&
                  open_panel.valid &&
                  open_panel.text_size[0] == 40u &&
                  open_panel.text_size[1] == 40u &&
                  !memcmp(open_panel.text[0], "LOAD", 5u) &&
                  !memcmp(open_panel.text[1], "CANCEL", 7u) &&
                  open_panel.panel_rect_index == 4u &&
                  open_panel.version_rect_index == 450u &&
                  open_panel.primary_button_rect_index == 466u &&
                  open_panel.secondary_button_rect_index == 467u &&
                  open_panel.save_list_rect_index == 451u &&
                  open_panel.version_palette_slot == 12u &&
                  open_panel.button_palette_slot == 11u &&
                  open_panel.save_slot_count == 10u &&
                  open_panel.fade_when_dialog2 && open_panel.receipt_hash != 0u,
              "open panel binds original GDAT labels, palette slots, and raw4 rect IDs");
        check(dm2_v1_dialogue_save_input_init(&open_panel, 2,
                                              (const uint8_t *)"save", 4u,
                                              &input_state) &&
                  input_state.valid && input_state.selected_slot == 2 &&
                  input_state.text_length == 4u &&
                  !memcmp(input_state.text, "save", 5u),
              "save dialogue initializes only from authenticated source panel and save header");
        check(dm2_v1_dialogue_save_input_apply(
                  &open_panel, &input_state,
                  DM2_V1_DIALOGUE_SAVE_EVENT_EDIT, -1, 'x',
                  &input_receipt) && input_receipt.valid &&
                  input_receipt.redraw && input_state.editing &&
                  !memcmp(input_state.text, "saveX", 6u),
              "save dialogue uppercases the original decoded input byte");
        check(dm2_v1_dialogue_save_input_apply(
                  &open_panel, &input_state,
                  DM2_V1_DIALOGUE_SAVE_EVENT_SELECT_SLOT, 10, 0u,
                  &input_receipt) && input_receipt.redraw &&
                  input_state.selected_slot == 10 && !input_state.editing &&
                  !dm2_v1_dialogue_save_input_apply(
                      &open_panel, &input_state,
                      DM2_V1_DIALOGUE_SAVE_EVENT_EDIT, -1, 'a',
                      &input_receipt),
              "save dialogue retains source sentinel slot and blocks its edit path");
        check(dm2_v1_dialogue_save_input_apply(
                  &open_panel, &input_state,
                  DM2_V1_DIALOGUE_SAVE_EVENT_CANCEL, -1, 0u,
                  &input_receipt) && input_receipt.close_panel &&
                  input_receipt.cancelled && input_receipt.accepted_slot == -1,
              "save dialogue cancel follows source event zero");
        raw[offsets[4]] = '\0';
        check(!dm2_v1_dialogue_open_panel_receipt(&loader, &open_panel),
              "open panel rejects an empty original secondary label");
        raw[offsets[4]] = 'C';
        entries[5].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
        check(!dm2_v1_dialogue_open_panel_receipt(&loader, &open_panel),
              "open panel rejects labels without the source text-transform word");
        entries[5].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    }
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
