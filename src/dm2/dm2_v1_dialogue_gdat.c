#include "dm2_v1_dialogue_gdat.h"

#include <string.h>

static uint32_t dm2_dialogue_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static uint32_t dm2_dialogue_hash_bytes(uint32_t hash, const uint8_t *data,
                                        size_t size)
{
    size_t i;
    for (i = 0; i < size; ++i) hash = dm2_dialogue_hash_step(hash, data[i]);
    return hash;
}

int dm2_v1_dialogue_gdat_receipt(const DM2_V1_AssetLoader *loader,
                                 uint8_t graphicsset,
                                 uint8_t shell_field,
                                 DM2_V1_DialogueGdatReceipt *out)
{
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !loader->loaded ||
        shell_field < DM2_V1_DIALOGUE_SHELL_FIELD_MIN ||
        shell_field > DM2_V1_DIALOGUE_SHELL_FIELD_MAX ||
        !dm2_v1_asset_load_image_metadata(
            loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, shell_field,
            &out->shell_metadata) ||
        !dm2_v1_asset_load_image_metadata(
            loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
            DM2_V1_DIALOGUE_GLYPH_FIELD, &out->glyph_metadata) ||
        out->shell_metadata.bits_per_pixel != 4u ||
        out->glyph_metadata.bits_per_pixel != 4u ||
        !dm2_v1_asset_load_image_local_palette(
            loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, shell_field,
            out->shell_palette, &out->shell_palette_hash) ||
        !dm2_v1_asset_load_image_local_palette(
            loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
            DM2_V1_DIALOGUE_GLYPH_FIELD, out->glyph_palette,
            &out->glyph_palette_hash)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }

    /* c_gui_vp.cpp uses QUERY_GDAT_SUMMARY_IMAGE(8, MapGraphicsStyle,
     * shellField), then QUERY_GDAT_IMAGE_ENTRY_BUFF/LOCALPAL(8,
     * MapGraphicsStyle, 3) while composing message glyphs. */
    hash = dm2_dialogue_hash_step(hash, graphicsset);
    hash = dm2_dialogue_hash_step(hash, shell_field);
    hash = dm2_dialogue_hash_step(hash, out->shell_metadata.metadata_hash);
    hash = dm2_dialogue_hash_step(hash, out->glyph_metadata.metadata_hash);
    hash = dm2_dialogue_hash_step(hash, out->shell_palette_hash);
    hash = dm2_dialogue_hash_step(hash, out->glyph_palette_hash);
    out->graphicsset = graphicsset;
    out->shell_field = shell_field;
    out->receipt_hash = hash ? hash : 1u;
    out->valid = 1;
    return 1;
}

int dm2_v1_dialogue_box_gdat_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DialogueBoxGdatReceipt *out)
{
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !loader->loaded ||
        !dm2_v1_asset_load_image_metadata(
            loader, DM2_GDAT_CATEGORY_DIALOG_BOXES,
            DM2_V1_DIALOGUE_BOX_INDEX, DM2_V1_DIALOGUE_BOX_FIELD,
            &out->metadata) ||
        out->metadata.bits_per_pixel != 4u ||
        !dm2_v1_asset_load_image_local_palette(
            loader, DM2_GDAT_CATEGORY_DIALOG_BOXES,
            DM2_V1_DIALOGUE_BOX_INDEX, DM2_V1_DIALOGUE_BOX_FIELD,
            out->palette, &out->palette_hash)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }

    hash = dm2_dialogue_hash_step(hash, DM2_GDAT_CATEGORY_DIALOG_BOXES);
    hash = dm2_dialogue_hash_step(hash, DM2_V1_DIALOGUE_BOX_INDEX);
    hash = dm2_dialogue_hash_step(hash, DM2_V1_DIALOGUE_BOX_FIELD);
    hash = dm2_dialogue_hash_step(hash, out->metadata.metadata_hash);
    hash = dm2_dialogue_hash_step(hash, out->palette_hash);
    out->receipt_hash = hash ? hash : 1u;
    out->valid = 1;
    return 1;
}

int dm2_v1_dialogue_box_draw_plan(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DialogueBoxDrawPlan *out)
{
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!dm2_v1_dialogue_box_gdat_receipt(loader, &out->material)) {
        return 0;
    }

    /* skproject/SKWINSPX/src/v5/uidialog.cpp::DM2_dialog_2066_3820:
     * QUERY_EXPANDED_RECT(453), QUERY_GDAT_IMAGE_ENTRY_BUFF(26, 0x81, 0),
     * local palette, yellow text at rect.y + 4, optional orange clear. */
    out->gdat_category = DM2_GDAT_CATEGORY_DIALOG_BOXES;
    out->gdat_index = DM2_V1_DIALOGUE_BOX_INDEX;
    out->gdat_field = DM2_V1_DIALOGUE_BOX_FIELD;
    out->expanded_rect_index = DM2_V1_DIALOGUE_BOX_RECT_INDEX;
    out->text_y_offset = DM2_V1_DIALOGUE_BOX_TEXT_Y_OFFSET;
    out->text_palette_slot = DM2_V1_DIALOGUE_BOX_TEXT_PALETTE_SLOT;
    out->highlight_palette_slot = DM2_V1_DIALOGUE_BOX_HIGHLIGHT_PALETTE_SLOT;
    out->optional_highlight_clear = 1;
    hash = dm2_dialogue_hash_step(hash, (uint32_t)out->gdat_category);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)out->gdat_index);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)out->gdat_field);
    hash = dm2_dialogue_hash_step(hash, out->expanded_rect_index);
    hash = dm2_dialogue_hash_step(hash, out->text_y_offset);
    hash = dm2_dialogue_hash_step(hash, out->text_palette_slot);
    hash = dm2_dialogue_hash_step(hash, out->highlight_palette_slot);
    hash = dm2_dialogue_hash_step(hash, out->material.receipt_hash);
    out->plan_hash = hash ? hash : 1u;
    out->valid = 1;
    return 1;
}

int dm2_v1_dialogue_open_panel_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DialogueOpenPanelReceipt *out)
{
    uint32_t hash = 2166136261u;
    unsigned int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!dm2_v1_dialogue_box_gdat_receipt(loader, &out->material)) return 0;

    /* ReDMCSB is not applicable: this is skproject/SKULLWIN/c_dialog.cpp
     * lines 352-415. QUERY_GDAT_TEXT(0x1a, 0x81, 0/1) supplies the two
     * button labels before the source panel is blitted at rect 4. */
    for (i = 0; i < DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_COUNT; ++i) {
        out->text[i] = dm2_v1_asset_load_text_sized(
            loader, DM2_GDAT_CATEGORY_DIALOG_BOXES,
            DM2_V1_DIALOGUE_BOX_INDEX, (uint8_t)i, &out->text_size[i]);
        if (!out->text[i] || out->text_size[i] == 0u ||
            out->text[i][0] == '\0') {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        out->text_hash[i] = dm2_dialogue_hash_bytes(
            2166136261u, out->text[i], out->text_size[i]);
        if (out->text_hash[i] == 0u) out->text_hash[i] = 1u;
    }

    out->panel_rect_index = DM2_V1_DIALOGUE_OPEN_PANEL_RECT_INDEX;
    out->version_rect_index = DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_RECT;
    out->primary_button_rect_index = DM2_V1_DIALOGUE_OPEN_PANEL_PRIMARY_RECT;
    out->secondary_button_rect_index = DM2_V1_DIALOGUE_OPEN_PANEL_SECONDARY_RECT;
    out->save_list_rect_index = DM2_V1_DIALOGUE_OPEN_PANEL_SAVE_LIST_RECT;
    out->version_palette_slot = DM2_V1_DIALOGUE_OPEN_PANEL_VERSION_PALETTE;
    out->button_palette_slot = DM2_V1_DIALOGUE_OPEN_PANEL_BUTTON_PALETTE;
    out->save_slot_count = 10u;
    out->fade_when_dialog2 = 1;
    hash = dm2_dialogue_hash_step(hash, out->material.receipt_hash);
    for (i = 0; i < DM2_V1_DIALOGUE_OPEN_PANEL_TEXT_COUNT; ++i)
        hash = dm2_dialogue_hash_step(hash, out->text_hash[i]);
    hash = dm2_dialogue_hash_step(hash, out->panel_rect_index);
    hash = dm2_dialogue_hash_step(hash, out->version_rect_index);
    hash = dm2_dialogue_hash_step(hash, out->primary_button_rect_index);
    hash = dm2_dialogue_hash_step(hash, out->secondary_button_rect_index);
    hash = dm2_dialogue_hash_step(hash, out->save_list_rect_index);
    hash = dm2_dialogue_hash_step(hash, out->version_palette_slot);
    hash = dm2_dialogue_hash_step(hash, out->button_palette_slot);
    hash = dm2_dialogue_hash_step(hash, out->save_slot_count);
    out->receipt_hash = hash ? hash : 1u;
    out->valid = 1;
    return 1;
}
