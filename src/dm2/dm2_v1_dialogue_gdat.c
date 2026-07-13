#include "dm2_v1_dialogue_gdat.h"

#include <string.h>

static uint32_t dm2_dialogue_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
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
