#include "dm2_v1_inventory_panel.h"

#include <stdio.h>
#include <string.h>

static const char *const k_slot_labels[DM2_V1_INV_SLOT_COUNT] = {
    "ready_hand",
    "action_hand",
    "head",
    "torso",
    "legs",
    "feet",
    "pouch_2",
    "quiver_line2_1",
    "quiver_line1_2",
    "quiver_line2_2",
    "neck",
    "pouch_1",
    "quiver_line1_1",
    "backpack_line1_1",
    "backpack_line2_2",
    "backpack_line2_3",
    "backpack_line2_4",
    "backpack_line2_5",
    "backpack_line2_6",
    "backpack_line2_7",
    "backpack_line2_8",
    "backpack_line2_9",
    "backpack_line1_2",
    "backpack_line1_3",
    "backpack_line1_4",
    "backpack_line1_5",
    "backpack_line1_6",
    "backpack_line1_7",
    "backpack_line1_8",
    "backpack_line1_9"
};

static int slot_valid(int slot)
{
    return slot == DM2_V1_INV_SLOT_LEADER_HAND ||
           (slot >= 0 && slot < DM2_V1_INV_SLOT_COUNT);
}

static uint32_t inventory_panel_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static int inventory_item_gdat_category(uint8_t category)
{
    switch (category) {
        case DM2_GDAT_CATEGORY_WEAPONS:
        case DM2_GDAT_CATEGORY_CLOTHES:
        case DM2_GDAT_CATEGORY_SCROLLS:
        case DM2_GDAT_CATEGORY_POTIONS:
        case DM2_GDAT_CATEGORY_CONTAINERS:
        case DM2_GDAT_CATEGORY_MISCELLANEOUS:
            return 1;
        default:
            return 0;
    }
}

const char *dm2_v1_inventory_slot_label(int slot)
{
    if (slot == DM2_V1_INV_SLOT_LEADER_HAND) return "leader_hand";
    if (slot < 0 || slot >= DM2_V1_INV_SLOT_COUNT) return "invalid";
    return k_slot_labels[slot];
}

int dm2_v1_inventory_slot_is_equipment(int slot)
{
    switch (slot) {
        case DM2_V1_INV_SLOT_LEADER_HAND:
        case DM2_V1_INV_SLOT_READY_HAND:
        case DM2_V1_INV_SLOT_ACTION_HAND:
        case DM2_V1_INV_SLOT_HEAD:
        case DM2_V1_INV_SLOT_TORSO:
        case DM2_V1_INV_SLOT_LEGS:
        case DM2_V1_INV_SLOT_FEET:
        case DM2_V1_INV_SLOT_NECK:
            return 1;
        default:
            return 0;
    }
}

static const char *lookup_description(
    uint32_t object_id,
    const DM2_V1_InventoryPanelDescription *descriptions,
    size_t description_count)
{
    if (!descriptions) return NULL;
    for (size_t i = 0; i < description_count; ++i) {
        if (descriptions[i].object_id == object_id &&
            descriptions[i].description &&
            descriptions[i].description[0] != '\0') {
            return descriptions[i].description;
        }
    }
    return NULL;
}

int dm2_v1_inventory_panel_select_item(
    const DM2_ChampionRecord *champion,
    const DM2_LeaderPossession *leader_hand,
    int selected_slot,
    const DM2_DB_State *db,
    const DM2_V1_InventoryPanelDescription *descriptions,
    size_t description_count,
    DM2_V1_InventoryPanelItemView *out)
{
    uint32_t object_id;
    const char *desc;

    if (!out || !slot_valid(selected_slot)) return 0;
    if (selected_slot != DM2_V1_INV_SLOT_LEADER_HAND && !champion) return 0;

    memset(out, 0, sizeof(*out));
    out->selected_slot = selected_slot;

    object_id = (selected_slot == DM2_V1_INV_SLOT_LEADER_HAND)
        ? (leader_hand ? leader_hand->object : 0u)
        : champion->inventory[selected_slot];
    out->object_id = object_id;

    if (object_id == 0u) {
        snprintf(out->description, sizeof(out->description), "EMPTY");
        return 1;
    }

    out->has_object = 1;
    out->db_resolved = dm2_db_resolve(
        object_id, db, &out->db_pool, &out->db_index) ? 1 : 0;

    desc = lookup_description(object_id, descriptions, description_count);
    if (desc) {
        snprintf(out->description, sizeof(out->description), "%s", desc);
    } else if (out->db_resolved) {
        snprintf(out->description, sizeof(out->description),
                 "POOL %u INDEX %lu",
                 (unsigned)out->db_pool,
                 (unsigned long)out->db_index);
    } else {
        snprintf(out->description, sizeof(out->description), "UNRESOLVED");
    }

    return 1;
}

int dm2_v1_inventory_panel_gdat_material_receipt(
    const DM2_V1_AssetLoader *loader,
    uint32_t object_id,
    uint8_t gdat_category,
    uint8_t gdat_index,
    uint8_t image_field,
    DM2_V1_InventoryPanelGdatMaterialReceipt *out_receipt)
{
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    size_t pixel_count;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM_ICON 13478-13620 and
     * DRAW_ITEM_IN_HAND 15778-15812 first retain the source record's cls1,
     * cls2 and selected image field, then query that exact dtImage and its
     * QUERY_GDAT_IMAGE_LOCALPAL payload.  A host icon chooser would not
     * preserve this contract, so a caller must supply the source address. */
    if (!loader || object_id == 0u || !inventory_item_gdat_category(gdat_category) ||
        !dm2_v1_asset_load_image_metadata(loader, gdat_category, gdat_index,
                                          image_field,
                                          &out_receipt->image_metadata) ||
        out_receipt->image_metadata.bits_per_pixel != 4u ||
        !dm2_v1_asset_load_image_local_palette(loader, gdat_category,
                                               gdat_index, image_field,
                                               out_receipt->local_palette16,
                                               &out_receipt->local_palette_hash) ||
        out_receipt->local_palette_hash == 0u) {
        return 0;
    }

    pixels = dm2_v1_asset_load_image_field(loader, gdat_category, gdat_index,
                                            image_field, &width, &height,
                                            &format);
    if (!pixels || width <= 0 || height <= 0 ||
        width != (int)out_receipt->image_metadata.width ||
        height != (int)out_receipt->image_metadata.height ||
        (format != DM2_IMG_FMT_IMG3 && format != DM2_IMG_FMT_U4)) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count == 0u || pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    for (size_t i = 0u; i < pixel_count; ++i) {
        hash = inventory_panel_hash_step(hash, pixels[i]);
    }
    dm2_v1_asset_free_pixels(pixels);
    if (hash == 0u) return 0;

    out_receipt->object_id = object_id;
    out_receipt->gdat_category = gdat_category;
    out_receipt->gdat_index = gdat_index;
    out_receipt->image_field = image_field;
    out_receipt->decoded_width = (uint16_t)width;
    out_receipt->decoded_height = (uint16_t)height;
    out_receipt->decoded_format = format;
    out_receipt->decoded_pixel_count = (uint32_t)pixel_count;
    out_receipt->decoded_pixels_hash = hash;
    hash = inventory_panel_hash_step(hash,
                                     out_receipt->image_metadata.metadata_hash);
    hash = inventory_panel_hash_step(hash, out_receipt->local_palette_hash);
    hash = inventory_panel_hash_step(hash, object_id);
    hash = inventory_panel_hash_step(hash,
                                     ((uint32_t)gdat_category << 16) |
                                     ((uint32_t)gdat_index << 8) |
                                     image_field);
    if (hash == 0u) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->material_hash = hash;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_inventory_panel_hud_receipt(
    const DM2_V1_InventoryPanelItemView *item,
    const DM2_V1_InventoryPanelGdatMaterialReceipt *material,
    DM2_V1_InventoryPanelHudReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!item || !material || !item->has_object || item->object_id == 0u ||
        !material->valid || material->object_id != item->object_id ||
        material->material_hash == 0u) {
        return 0;
    }
    hash = inventory_panel_hash_step(hash, (uint32_t)item->selected_slot);
    hash = inventory_panel_hash_step(hash, item->object_id);
    hash = inventory_panel_hash_step(hash, item->db_pool);
    hash = inventory_panel_hash_step(hash, item->db_index);
    hash = inventory_panel_hash_step(hash, material->material_hash);
    if (hash == 0u) return 0;

    out_receipt->selected_slot = item->selected_slot;
    out_receipt->object_id = item->object_id;
    out_receipt->db_pool = item->db_pool;
    out_receipt->db_index = item->db_index;
    out_receipt->material = *material;
    out_receipt->receipt_hash = hash;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_inventory_panel_consume_hud_material(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelHudReceipt *hud_receipt,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_receipt)
{
    const DM2_V1_InventoryPanelGdatMaterialReceipt *material;
    uint8_t palette16[16];
    uint8_t *source_pixels;
    uint32_t palette_hash = 0u;
    uint32_t pixels_hash = 2166136261u;
    uint32_t blit_hash = 2166136261u;
    int source_width = 0;
    int source_height = 0;
    DM2_ImageFormat source_format = DM2_IMG_FMT_UNKNOWN;
    size_t source_pixel_count;
    size_t drawn_pixel_count = 0u;
    size_t transparent_pixel_count = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !hud_receipt || !hud_receipt->valid || !blit || !surface ||
        !surface->pixels || surface->width <= 0 || surface->height <= 0 ||
        surface->stride < surface->width || blit->width == 0u ||
        blit->height == 0u || blit->source_x < 0 || blit->source_y < 0 ||
        blit->destination_x < 0 || blit->destination_y < 0 ||
        blit->transparent_index != 12u) {
        return 0;
    }
    material = &hud_receipt->material;
    if (!material->valid || material->object_id != hud_receipt->object_id ||
        material->material_hash == 0u ||
        !dm2_v1_asset_load_image_local_palette(
            loader, material->gdat_category, material->gdat_index,
            material->image_field, palette16, &palette_hash) ||
        palette_hash == 0u || palette_hash != material->local_palette_hash ||
        memcmp(palette16, material->local_palette16, sizeof(palette16)) != 0) {
        return 0;
    }
    source_pixels = dm2_v1_asset_load_image_field(
        loader, material->gdat_category, material->gdat_index,
        material->image_field, &source_width, &source_height, &source_format);
    if (!source_pixels || source_width <= 0 || source_height <= 0 ||
        source_width != (int)material->decoded_width ||
        source_height != (int)material->decoded_height ||
        source_format != material->decoded_format ||
        (source_format != DM2_IMG_FMT_IMG3 && source_format != DM2_IMG_FMT_U4) ||
        blit->source_x + (int)blit->width > source_width ||
        blit->source_y + (int)blit->height > source_height ||
        blit->destination_x + (int)blit->width > surface->width ||
        blit->destination_y + (int)blit->height > surface->height) {
        dm2_v1_asset_free_pixels(source_pixels);
        return 0;
    }
    source_pixel_count = (size_t)source_width * (size_t)source_height;
    if (source_pixel_count == 0u || source_pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(source_pixels);
        return 0;
    }
    for (size_t i = 0u; i < source_pixel_count; ++i) {
        pixels_hash = inventory_panel_hash_step(pixels_hash, source_pixels[i]);
    }
    if (pixels_hash != material->decoded_pixels_hash) {
        dm2_v1_asset_free_pixels(source_pixels);
        return 0;
    }

    /* skproject DRAW_ITEM_ICON:13478-13620 calls DRAW_ICON_PICT_ENTRY,
     * which resolves QUERY_BLIT_RECT and passes the selected dtImage plus its
     * exact local palette to FIRE_BLIT_PICTURE with color key 12.  This is a
     * direct indexed blit only: no icon selection, scaling, or fallback. */
    for (uint16_t y = 0u; y < blit->height; ++y) {
        for (uint16_t x = 0u; x < blit->width; ++x) {
            uint8_t source = source_pixels[
                (size_t)(blit->source_y + (int)y) * (size_t)source_width +
                (size_t)(blit->source_x + (int)x)];
            if (source == blit->transparent_index) {
                ++transparent_pixel_count;
                continue;
            }
            surface->pixels[(size_t)(blit->destination_y + (int)y) *
                                (size_t)surface->stride +
                            (size_t)(blit->destination_x + (int)x)] =
                palette16[source & 0x0fu];
            blit_hash = inventory_panel_hash_step(blit_hash, source);
            blit_hash = inventory_panel_hash_step(blit_hash,
                                                  palette16[source & 0x0fu]);
            ++drawn_pixel_count;
        }
    }
    dm2_v1_asset_free_pixels(source_pixels);
    if (drawn_pixel_count == 0u || drawn_pixel_count > UINT32_MAX ||
        transparent_pixel_count > UINT32_MAX || blit_hash == 0u) {
        return 0;
    }
    out_receipt->rect_number = blit->rect_number;
    out_receipt->width = blit->width;
    out_receipt->height = blit->height;
    out_receipt->drawn_pixel_count = (uint32_t)drawn_pixel_count;
    out_receipt->transparent_pixel_count = (uint32_t)transparent_pixel_count;
    out_receipt->blit_hash = blit_hash;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_inventory_panel_survey_preview_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelItemView *item,
    uint8_t gdat_category,
    uint8_t gdat_index,
    DM2_V1_InventoryPanelSurveyPreviewReceipt *out_receipt)
{
    DM2_V1_InventoryPanelGdatMaterialReceipt material;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM_SURVEY:13962-14019 queries
     * the selected record's dtImage/0x11 only when it is loadable, then
     * draws that precise image at expanded rect 0x1ee with colour key 12.
     * There is no original substitute when field 0x11 is absent. */
    if (!item || !item->has_object || item->object_id == 0u ||
        !dm2_v1_inventory_panel_gdat_material_receipt(
            loader, item->object_id, gdat_category, gdat_index,
            DM2_V1_INVENTORY_SURVEY_PREVIEW_FIELD, &material) ||
        !dm2_v1_inventory_panel_hud_receipt(item, &material,
                                             &out_receipt->hud)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->expanded_rect_index = DM2_V1_INVENTORY_SURVEY_PREVIEW_RECT;
    out_receipt->transparent_index = DM2_V1_INVENTORY_SURVEY_TRANSPARENCY;
    hash = inventory_panel_hash_step(hash, out_receipt->expanded_rect_index);
    hash = inventory_panel_hash_step(hash, out_receipt->transparent_index);
    hash = inventory_panel_hash_step(hash, out_receipt->hud.receipt_hash);
    if (hash == 0u) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->receipt_hash = hash;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_inventory_panel_consume_survey_preview(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelSurveyPreviewReceipt *preview,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_receipt)
{
    if (!preview || !preview->valid || preview->receipt_hash == 0u || !blit ||
        blit->rect_number != preview->expanded_rect_index ||
        blit->transparent_index != preview->transparent_index) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_inventory_panel_consume_hud_material(
        loader, &preview->hud, blit, surface, out_receipt);
}

int dm2_v1_inventory_panel_hand_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelItemView *item,
    uint8_t gdat_category,
    uint8_t gdat_index,
    uint8_t image_field,
    DM2_V1_InventoryPanelHandReceipt *out_receipt)
{
    DM2_V1_InventoryPanelGdatMaterialReceipt material;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* skproject SKWIN/SkWinCore.cpp DRAW_ITEM_IN_HAND:15778-15812 obtains
     * the record-selected field, copies QUERY_GDAT_IMAGE_LOCALPAL into the
     * hand picture, and FIRE_BLIT_PICTUREs the whole image at origin with
     * color key -1. The caller therefore supplies the original selected
     * field; no default hand icon can substitute for an absent record. */
    if (!item || !item->has_object || item->object_id == 0u ||
        !dm2_v1_inventory_panel_gdat_material_receipt(
            loader, item->object_id, gdat_category, gdat_index, image_field,
            &material) ||
        !dm2_v1_inventory_panel_hud_receipt(item, &material,
                                             &out_receipt->hud)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->origin_width = material.decoded_width;
    out_receipt->origin_height = material.decoded_height;
    hash = inventory_panel_hash_step(hash, out_receipt->origin_width);
    hash = inventory_panel_hash_step(hash, out_receipt->origin_height);
    hash = inventory_panel_hash_step(hash, out_receipt->hud.receipt_hash);
    if (hash == 0u) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->receipt_hash = hash;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_inventory_panel_consume_hand_item(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelHandReceipt *hand,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_receipt)
{
    const DM2_V1_InventoryPanelGdatMaterialReceipt *material;
    uint8_t palette16[16];
    uint8_t *source_pixels;
    uint32_t palette_hash = 0u;
    uint32_t pixels_hash = 2166136261u;
    uint32_t blit_hash = 2166136261u;
    int source_width = 0;
    int source_height = 0;
    DM2_ImageFormat source_format = DM2_IMG_FMT_UNKNOWN;
    size_t pixel_count;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !hand || !hand->valid || hand->receipt_hash == 0u ||
        !surface || !surface->pixels || surface->width != hand->origin_width ||
        surface->height != hand->origin_height || surface->stride < surface->width) {
        return 0;
    }
    material = &hand->hud.material;
    if (!hand->hud.valid || !material->valid ||
        material->object_id != hand->hud.object_id ||
        !dm2_v1_asset_load_image_local_palette(
            loader, material->gdat_category, material->gdat_index,
            material->image_field, palette16, &palette_hash) ||
        palette_hash == 0u || palette_hash != material->local_palette_hash ||
        memcmp(palette16, material->local_palette16, sizeof(palette16)) != 0) {
        return 0;
    }
    source_pixels = dm2_v1_asset_load_image_field(
        loader, material->gdat_category, material->gdat_index,
        material->image_field, &source_width, &source_height, &source_format);
    if (!source_pixels || source_width != (int)hand->origin_width ||
        source_height != (int)hand->origin_height ||
        source_format != material->decoded_format ||
        (source_format != DM2_IMG_FMT_IMG3 && source_format != DM2_IMG_FMT_U4)) {
        dm2_v1_asset_free_pixels(source_pixels);
        return 0;
    }
    pixel_count = (size_t)source_width * (size_t)source_height;
    if (pixel_count == 0u || pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(source_pixels);
        return 0;
    }
    for (size_t i = 0u; i < pixel_count; ++i) {
        pixels_hash = inventory_panel_hash_step(pixels_hash, source_pixels[i]);
    }
    if (pixels_hash != material->decoded_pixels_hash) {
        dm2_v1_asset_free_pixels(source_pixels);
        return 0;
    }
    for (size_t i = 0u; i < pixel_count; ++i) {
        uint8_t source = source_pixels[i];
        uint8_t destination = palette16[source & 0x0fu];
        surface->pixels[(i / (size_t)source_width) * (size_t)surface->stride +
                        (i % (size_t)source_width)] = destination;
        blit_hash = inventory_panel_hash_step(blit_hash, source);
        blit_hash = inventory_panel_hash_step(blit_hash, destination);
    }
    dm2_v1_asset_free_pixels(source_pixels);
    if (blit_hash == 0u) return 0;
    out_receipt->width = hand->origin_width;
    out_receipt->height = hand->origin_height;
    out_receipt->drawn_pixel_count = (uint32_t)pixel_count;
    out_receipt->transparent_pixel_count = 0u;
    out_receipt->blit_hash = blit_hash;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_inventory_panel_hand_slot_backdrop_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t possession_index,
    uint8_t left_or_right,
    uint8_t champion_direction,
    uint8_t player_direction,
    DM2_V1_InventoryPanelHandSlotBackdropReceipt *out_receipt)
{
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    uint32_t pixels_hash = 2166136261u;
    uint32_t hash = 2166136261u;
    size_t pixel_count;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* skproject SKWIN/SkWinCore.cpp DRAW_HAND_ACTION_ICONS:7488-7560 selects
     * interface-general/4 field (possession * 2) + side + 2 and rect
     * (possession == 1 ? 0x46 : 0x4a) + ((championDir + 4 - playerDir) & 3).
     * DRAW_ICON_PICT_ENTRY:6901-6925 then uses that exact dtImage/local
     * palette. Neither route selects an alternate panel tile. */
    if (!loader || possession_index > 1u || left_or_right > 1u ||
        champion_direction > 3u || player_direction > 3u ||
        !dm2_v1_asset_load_image_metadata(
            loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0x04u,
            (uint8_t)(possession_index * 2u + left_or_right + 2u),
            &out_receipt->image_metadata) ||
        out_receipt->image_metadata.bits_per_pixel != 4u ||
        !dm2_v1_asset_load_image_local_palette(
            loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0x04u,
            (uint8_t)(possession_index * 2u + left_or_right + 2u),
            out_receipt->local_palette16, &out_receipt->local_palette_hash) ||
        out_receipt->local_palette_hash == 0u) {
        return 0;
    }
    out_receipt->image_field =
        (uint8_t)(possession_index * 2u + left_or_right + 2u);
    pixels = dm2_v1_asset_load_image_field(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0x04u,
        out_receipt->image_field, &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 ||
        width != (int)out_receipt->image_metadata.width ||
        height != (int)out_receipt->image_metadata.height ||
        (format != DM2_IMG_FMT_IMG3 && format != DM2_IMG_FMT_U4)) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count == 0u || pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    for (size_t i = 0u; i < pixel_count; ++i) {
        pixels_hash = inventory_panel_hash_step(pixels_hash, pixels[i]);
    }
    dm2_v1_asset_free_pixels(pixels);
    if (pixels_hash == 0u) return 0;
    out_receipt->possession_index = possession_index;
    out_receipt->left_or_right = left_or_right;
    out_receipt->expanded_rect_index = (uint16_t)(
        (possession_index == 1u ? 0x46u : 0x4au) +
        ((champion_direction + 4u - player_direction) & 3u));
    out_receipt->decoded_width = (uint16_t)width;
    out_receipt->decoded_height = (uint16_t)height;
    out_receipt->decoded_format = format;
    out_receipt->decoded_pixels_hash = pixels_hash;
    hash = inventory_panel_hash_step(hash, out_receipt->expanded_rect_index);
    hash = inventory_panel_hash_step(hash, out_receipt->image_field);
    hash = inventory_panel_hash_step(hash, out_receipt->image_metadata.metadata_hash);
    hash = inventory_panel_hash_step(hash, out_receipt->local_palette_hash);
    hash = inventory_panel_hash_step(hash, pixels_hash);
    if (hash == 0u) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->receipt_hash = hash;
    out_receipt->valid = 1;
    return 1;
}

int dm2_v1_inventory_panel_consume_hand_slot_backdrop(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelHandSlotBackdropReceipt *backdrop,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_receipt)
{
    uint8_t palette16[16];
    uint8_t *source_pixels;
    uint32_t palette_hash = 0u;
    uint32_t pixels_hash = 2166136261u;
    uint32_t blit_hash = 2166136261u;
    int source_width = 0;
    int source_height = 0;
    DM2_ImageFormat source_format = DM2_IMG_FMT_UNKNOWN;
    size_t pixel_count;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !backdrop || !backdrop->valid ||
        backdrop->receipt_hash == 0u || !blit || !surface || !surface->pixels ||
        surface->width <= 0 || surface->height <= 0 ||
        surface->stride < surface->width ||
        blit->rect_number != backdrop->expanded_rect_index ||
        blit->transparent_index != UINT8_MAX || blit->source_x < 0 ||
        blit->source_y < 0 || blit->destination_x < 0 ||
        blit->destination_y < 0 || blit->width == 0u || blit->height == 0u) {
        return 0;
    }
    if (!dm2_v1_asset_load_image_local_palette(
            loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0x04u,
            backdrop->image_field, palette16, &palette_hash) ||
        palette_hash == 0u || palette_hash != backdrop->local_palette_hash ||
        memcmp(palette16, backdrop->local_palette16, sizeof(palette16)) != 0) {
        return 0;
    }
    source_pixels = dm2_v1_asset_load_image_field(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0x04u,
        backdrop->image_field, &source_width, &source_height, &source_format);
    if (!source_pixels || source_width != (int)backdrop->decoded_width ||
        source_height != (int)backdrop->decoded_height ||
        source_format != backdrop->decoded_format ||
        (source_format != DM2_IMG_FMT_IMG3 && source_format != DM2_IMG_FMT_U4) ||
        blit->source_x + (int)blit->width > source_width ||
        blit->source_y + (int)blit->height > source_height ||
        blit->destination_x + (int)blit->width > surface->width ||
        blit->destination_y + (int)blit->height > surface->height) {
        dm2_v1_asset_free_pixels(source_pixels);
        return 0;
    }
    pixel_count = (size_t)source_width * (size_t)source_height;
    if (pixel_count == 0u || pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(source_pixels);
        return 0;
    }
    for (size_t i = 0u; i < pixel_count; ++i) {
        pixels_hash = inventory_panel_hash_step(pixels_hash, source_pixels[i]);
    }
    if (pixels_hash != backdrop->decoded_pixels_hash) {
        dm2_v1_asset_free_pixels(source_pixels);
        return 0;
    }
    for (uint16_t y = 0u; y < blit->height; ++y) {
        for (uint16_t x = 0u; x < blit->width; ++x) {
            uint8_t source = source_pixels[
                (size_t)(blit->source_y + (int)y) * (size_t)source_width +
                (size_t)(blit->source_x + (int)x)];
            uint8_t destination = palette16[source & 0x0fu];
            surface->pixels[(size_t)(blit->destination_y + (int)y) *
                                (size_t)surface->stride +
                            (size_t)(blit->destination_x + (int)x)] = destination;
            blit_hash = inventory_panel_hash_step(blit_hash, source);
            blit_hash = inventory_panel_hash_step(blit_hash, destination);
        }
    }
    dm2_v1_asset_free_pixels(source_pixels);
    if (blit_hash == 0u) return 0;
    out_receipt->rect_number = blit->rect_number;
    out_receipt->width = blit->width;
    out_receipt->height = blit->height;
    out_receipt->drawn_pixel_count = (uint32_t)blit->width * (uint32_t)blit->height;
    out_receipt->transparent_pixel_count = 0u;
    out_receipt->blit_hash = blit_hash;
    out_receipt->valid = 1;
    return 1;
}

const char *dm2_v1_inventory_panel_source_evidence(void)
{
    return
        "ReDMCSB DEFS.H:779-810 slot indices and leader-hand sentinel\n"
        "ReDMCSB PANEL.C:1127-1200 object-description panel route\n"
        "ReDMCSB PANEL.C:1658-1692 action-hand item panel route\n"
        "ReDMCSB PANEL.C:2421-2423 inventory slot redraw loop\n"
        "ReDMCSB CHAMPION.C:250-268/270-282 leader-hand put/remove state\n"
        "ReDMCSB LOADSAVE.C:1535-1537/2744 leader-hand object persistence";
}
