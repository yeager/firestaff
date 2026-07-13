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

static uint32_t dm2_dialogue_save_input_hash(
    const DM2_V1_DialogueOpenPanelReceipt *panel,
    const DM2_V1_DialogueSaveInputState *state)
{
    uint32_t hash = 2166136261u;
    if (!panel || !state) return 0u;
    hash = dm2_dialogue_hash_step(hash, panel->receipt_hash);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)state->selected_slot);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)state->editing);
    hash = dm2_dialogue_hash_step(hash, state->text_length);
    hash = dm2_dialogue_hash_bytes(hash, state->text, state->text_length);
    return hash ? hash : 1u;
}

int dm2_v1_dialogue_save_input_init(
    const DM2_V1_DialogueOpenPanelReceipt *panel,
    int selected_slot,
    const uint8_t *initial_name,
    size_t initial_name_size,
    DM2_V1_DialogueSaveInputState *out_state)
{
    size_t copy_size;

    if (!out_state) return 0;
    memset(out_state, 0, sizeof(*out_state));
    if (!panel || !panel->valid || selected_slot < 0 ||
        selected_slot > (int)panel->save_slot_count ||
        (!initial_name && initial_name_size != 0u)) {
        return 0;
    }
    /* DM2_dialog_2066_33e7 accepts at most 0x1f typed bytes and always
     * maintains a trailing NUL.  A long save header is rejected rather than
     * silently truncated into a different source-visible save name. */
    if (initial_name_size > 31u) return 0;
    copy_size = initial_name_size;
    if (copy_size != 0u && memchr(initial_name, '\0', copy_size) != NULL) {
        copy_size = (size_t)((const uint8_t *)memchr(initial_name, '\0', copy_size) -
                             initial_name);
    }
    memcpy(out_state->text, initial_name, copy_size);
    out_state->text[copy_size] = '\0';
    out_state->selected_slot = selected_slot;
    out_state->text_length = (uint8_t)copy_size;
    out_state->valid = 1;
    out_state->state_hash = dm2_dialogue_save_input_hash(panel, out_state);
    return out_state->state_hash != 0u;
}

int dm2_v1_dialogue_save_input_apply(
    const DM2_V1_DialogueOpenPanelReceipt *panel,
    DM2_V1_DialogueSaveInputState *state,
    DM2_V1_DialogueSaveEvent event,
    int source_slot,
    uint8_t text_key,
    DM2_V1_DialogueSaveInputReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!panel || !panel->valid || !state || !state->valid ||
        state->selected_slot < 0 ||
        state->selected_slot > (int)panel->save_slot_count ||
        state->text_length > 31u || state->text[state->text_length] != '\0') {
        return 0;
    }
    hash = dm2_dialogue_hash_step(hash, panel->receipt_hash);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)event);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)source_slot);

    switch (event) {
        case DM2_V1_DIALOGUE_SAVE_EVENT_CANCEL:
            out_receipt->close_panel = 1;
            out_receipt->cancelled = 1;
            out_receipt->accepted_slot = -1;
            break;
        case DM2_V1_DIALOGUE_SAVE_EVENT_ACCEPT:
            out_receipt->close_panel = 1;
            out_receipt->accepted_slot = state->selected_slot;
            break;
        case DM2_V1_DIALOGUE_SAVE_EVENT_SELECT_SLOT:
            if (source_slot < 0 || source_slot > (int)panel->save_slot_count)
                return 0;
            state->selected_slot = source_slot;
            state->editing = 0;
            out_receipt->redraw = 1;
            out_receipt->accepted_slot = -1;
            break;
        case DM2_V1_DIALOGUE_SAVE_EVENT_EDIT:
            if (state->selected_slot == (int)panel->save_slot_count) return 0;
            if (!state->editing) {
                state->editing = 1;
                out_receipt->redraw = 1;
            }
            /* c_dialog.cpp uses the decoded keyboard byte: backspace removes
             * one byte, return ends edit, and a-z is uppercased. No other
             * control or Unicode conversion is inferred here. */
            if (text_key == 0x0eu) {
                if (state->text_length != 0u) --state->text_length;
                state->text[state->text_length] = '\0';
                out_receipt->redraw = 1;
            } else if (text_key == 0x1cu) {
                state->editing = 0;
                out_receipt->redraw = 1;
            } else if (text_key >= 'a' && text_key <= 'z' &&
                       state->text_length < 31u) {
                state->text[state->text_length++] = (uint8_t)(text_key - 0x20u);
                state->text[state->text_length] = '\0';
                out_receipt->redraw = 1;
            }
            out_receipt->accepted_slot = -1;
            break;
        default:
            return 0;
    }
    state->state_hash = dm2_dialogue_save_input_hash(panel, state);
    if (state->state_hash == 0u) return 0;
    hash = dm2_dialogue_hash_step(hash, state->state_hash);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)out_receipt->redraw);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)out_receipt->close_panel);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)out_receipt->cancelled);
    hash = dm2_dialogue_hash_step(hash, (uint32_t)out_receipt->accepted_slot);
    out_receipt->route_hash = hash ? hash : 1u;
    out_receipt->valid = 1;
    return 1;
}
