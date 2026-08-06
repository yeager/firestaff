#include "dm2_v1_inventory_panel.h"
#include "dm2_v1_hand_action_gdat.h"

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

static uint32_t inventory_panel_hash_bytes(uint32_t hash,
                                           const uint8_t *bytes,
                                           size_t size)
{
    size_t i;
    if (!bytes) return 0u;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int inventory_panel_image_receipt(
    const DM2_V1_AssetLoader *loader, int category, int index, int field,
    uint16_t rect, DM2_V1_InventoryPanelHandSlotBackdropReceipt *out)
{
    DM2_V1_QueryGdatSummaryImageReceipt summary;
    DM2_V1_GdatGfxRawMaterialReceipt material;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || category < 0 || category > 0xff || index < 0 ||
        index > 0xff || field < 0 || field > 0xff || rect == 0u ||
        !dm2_v1_query_gdat_summary_image_receipt(loader, category, index,
                                                  field, &summary) ||
        !summary.accepted || summary.colors != 16u || !summary.palette_hash ||
        !dm2_v1_gdat_image_raw_material_receipt(loader, category, index,
                                                 field, &material) ||
        !material.accepted || !material.source_bytes ||
        !material.source_byte_count || !material.source_hash ||
        !material.receipt_hash) return 0;

    pixels = dm2_v1_asset_load_image_field(loader, category, index, field,
                                            &width, &height, &format);
    /* DRAW_ICON_PICT_ENTRY accepts both uncompressed IMG3/U4 and the real
     * PC corpus's compressed IMG3 form.  Both carry the same 16-entry local
     * palette; IMG9/global-palette pixels are not admitted by this route. */
    if (!pixels || width <= 0 || height <= 0 ||
        (format != DM2_IMG_FMT_U4 && format != DM2_IMG_FMT_IMG3) ||
        width > UINT16_MAX || height > UINT16_MAX) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    out->category = (uint8_t)category;
    out->index = (uint8_t)index;
    out->image_field = (uint8_t)field;
    out->expanded_rect_index = rect;
    out->source_raw_index = material.raw_index;
    out->decoded_width = (uint16_t)width;
    out->decoded_height = (uint16_t)height;
    out->format = format;
    memcpy(out->palette16, summary.palette16, sizeof(out->palette16));
    out->raw_hash = material.source_hash;
    out->decoded_hash = inventory_panel_hash_bytes(
        2166136261u, pixels, (size_t)width * (size_t)height);
    out->palette_hash = summary.palette_hash;
    dm2_v1_asset_free_pixels(pixels);
    if (!out->decoded_hash) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    hash = inventory_panel_hash_bytes(hash, (const uint8_t *)&material.receipt_hash,
                                      sizeof(material.receipt_hash));
    hash = inventory_panel_hash_bytes(hash, (const uint8_t *)&out->decoded_hash,
                                      sizeof(out->decoded_hash));
    hash = inventory_panel_hash_bytes(hash, (const uint8_t *)&out->palette_hash,
                                      sizeof(out->palette_hash));
    hash = inventory_panel_hash_bytes(hash, (const uint8_t *)&rect, sizeof(rect));
    out->identity_hash = hash ? hash : 1u;
    out->valid = 1;
    return 1;
}

static int inventory_panel_consume_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelHandSlotBackdropReceipt *receipt,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out)
{
    DM2_V1_InventoryPanelHandSlotBackdropReceipt current;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    uint32_t hash;
    int x;
    int y;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !receipt || !receipt->valid || !receipt->identity_hash ||
        !blit || !surface || !surface->pixels || surface->width <= 0 ||
        surface->height <= 0 || surface->stride < surface->width ||
        blit->rect_number != receipt->expanded_rect_index ||
        blit->width != receipt->decoded_width || blit->height != receipt->decoded_height ||
        blit->transparent_index != UINT8_MAX || blit->destination_x < 0 ||
        blit->destination_y < 0 || blit->destination_x > surface->width - blit->width ||
        blit->destination_y > surface->height - blit->height ||
        !inventory_panel_image_receipt(loader, receipt->category, receipt->index,
                                       receipt->image_field,
                                       receipt->expanded_rect_index, &current) ||
        current.identity_hash != receipt->identity_hash ||
        current.raw_hash != receipt->raw_hash ||
        current.decoded_hash != receipt->decoded_hash ||
        current.palette_hash != receipt->palette_hash ||
        current.source_raw_index != receipt->source_raw_index) return 0;

    pixels = dm2_v1_asset_load_image_field(loader, receipt->category,
                                            receipt->index, receipt->image_field,
                                            &width, &height, &format);
    if (!pixels || width != receipt->decoded_width || height != receipt->decoded_height ||
        format != receipt->format || inventory_panel_hash_bytes(
            2166136261u, pixels, (size_t)width * (size_t)height) != receipt->decoded_hash) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            uint8_t pixel = pixels[(size_t)y * (size_t)width + (size_t)x];
            surface->pixels[(blit->destination_y + y) * surface->stride +
                            blit->destination_x + x] = receipt->palette16[pixel];
            ++out->drawn_pixel_count;
        }
    }
    dm2_v1_asset_free_pixels(pixels);
    hash = inventory_panel_hash_bytes(2166136261u,
        (const uint8_t *)&receipt->identity_hash, sizeof(receipt->identity_hash));
    hash = inventory_panel_hash_bytes(hash, (const uint8_t *)&blit->rect_number,
                                      sizeof(blit->rect_number));
    out->identity_hash = hash ? hash : 1u;
    out->valid = 1;
    return 1;
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

int dm2_v1_inventory_panel_hand_slot_backdrop_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t possession_index,
    uint8_t left_or_right,
    uint8_t player_position,
    uint8_t party_direction,
    DM2_V1_InventoryPanelHandSlotBackdropReceipt *out_receipt)
{
    DM2_V1_HandActionInput input;
    DM2_V1_HandActionGdatRoute route;

    memset(&input, 0, sizeof(input));
    memset(&route, 0, sizeof(route));
    input.possession_index = possession_index;
    input.left_or_right = left_or_right;
    input.player_position = player_position;
    input.party_direction = party_direction;
    /* SKProject SKWIN c_gui_draw.cpp:2341-2386 derives this address and
     * destination from the hand/action and party orientation. */
    if (!dm2_v1_hand_action_gdat_route(&input, &route)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return inventory_panel_image_receipt(loader, route.category,
                                         route.subcategory, route.entry,
                                         route.rectno, out_receipt);
}

int dm2_v1_inventory_panel_survey_frame_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_InventoryPanelSurveyFrameReceipt *out_receipt)
{
    /* SKProject SKWIN c_gui_draw.cpp:2072-2106: DRAW_ITEM_SURVEY calls
     * DRAW_STATIC_PIC(7, 0, 1, RECT_1EE, NOALPHA) before names or icons. */
    return inventory_panel_image_receipt(loader,
        DM2_GDAT_CATEGORY_INTERFACE_CHARSHEET, 0, 1,
        DM2_V1_INVENTORY_SURVEY_PREVIEW_RECT, out_receipt);
}

int dm2_v1_inventory_panel_consume_hand_slot_backdrop(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelHandSlotBackdropReceipt *receipt,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_consumption)
{
    return inventory_panel_consume_receipt(loader, receipt, blit, surface,
                                           out_consumption);
}

int dm2_v1_inventory_panel_consume_survey_frame(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelSurveyFrameReceipt *receipt,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_consumption)
{
    return inventory_panel_consume_receipt(loader, receipt, blit, surface,
                                           out_consumption);
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
