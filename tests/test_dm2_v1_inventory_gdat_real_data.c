/* Canonical PC English inventory HUD material proof.
 *
 * This reads GRAPHICS.DAT in place.  It enumerates the complete source
 * hand-action orientation matrix and the DRAW_ITEM_SURVEY static frame;
 * neither path can use a host icon, procedural panel, cached replacement or
 * extracted game-data file.
 *
 * Source: SKProject SKWIN/c_gui_draw.cpp:2072-2106 (DRAW_ITEM_SURVEY) and
 * 2341-2386 (DRAW_HAND_ACTION_ICONS).
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_inventory_panel.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int load_canonical_graphics(uint8_t **graphics, size_t *graphics_size)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");

    if (!archive || !archive[0] || !graphics || !graphics_size) return 0;
    *graphics = NULL;
    *graphics_size = 0u;
    return firestaff_zip_extract_by_suffix(archive, "data/graphics.dat",
                                            graphics, graphics_size) == 0 &&
           *graphics && *graphics_size;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    uint8_t target[320 * 200];
    unsigned int hand_count = 0u;
    unsigned int failure_count = 0u;
    uint32_t identity_hash = 2166136261u;
    uint32_t raw4_hash = 2166136261u;
    const uint8_t *raw4 = NULL;
    size_t raw4_size = 0u;

    if (!getenv("FIRESTAFF_DM2_DOS_ARCHIVE") ||
        !getenv("FIRESTAFF_DM2_DOS_ARCHIVE")[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not set");
        return 0;
    }
    if (!load_canonical_graphics(&graphics, &graphics_size)) {
        fputs("FAIL: selected canonical DM2 GRAPHICS.DAT is unreadable\n", stderr);
        return 1;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_asset_loader_verify(&loader)) {
        fputs("FAIL: canonical DM2 GRAPHICS.DAT was not admitted\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    raw4 = dm2_v1_asset_load_typed_sized(
        &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw4_size);
    if (!raw4 || raw4_size == 0u) {
        fputs("FAIL: canonical HUD RAW4 table was not admitted\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    for (size_t i = 0u; i < raw4_size; ++i) {
        raw4_hash ^= raw4[i];
        raw4_hash *= 16777619u;
    }
    if (raw4_hash == 0u) {
        fputs("FAIL: canonical HUD RAW4 identity was empty\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }

    for (uint8_t possession = 0u; possession < 2u; ++possession) {
        for (uint8_t side = 0u; side < 2u; ++side) {
            for (uint8_t position = 0u; position < 4u; ++position) {
                for (uint8_t direction = 0u; direction < 4u; ++direction) {
                    DM2_V1_InventoryPanelHandSlotBackdropReceipt receipt;
                    DM2_V1_InventoryPanelHudBlit blit;
                    DM2_V1_InventoryPanelHudSurface surface;
                    DM2_V1_InventoryPanelHudConsumptionReceipt consumed;
                    DM2_V1_ViewportRect raw4_destination;

                    memset(&receipt, 0, sizeof(receipt));
                    memset(&blit, 0, sizeof(blit));
                    memset(&surface, 0, sizeof(surface));
                    memset(&consumed, 0, sizeof(consumed));
                    memset(target, 0, sizeof(target));
                    if (!dm2_v1_inventory_panel_hand_slot_backdrop_receipt(
                            &loader, possession, side, position, direction,
                            &receipt) || !receipt.valid ||
                        receipt.category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
                        receipt.index != 4u || receipt.image_field < 2u ||
                        receipt.image_field > 5u || !receipt.raw_hash ||
                        !receipt.decoded_hash || !receipt.palette_hash ||
                        !receipt.identity_hash || receipt.format != DM2_IMG_FMT_IMG3 ||
                        receipt.decoded_width == 0u || receipt.decoded_height == 0u ||
                        receipt.decoded_width > 320u || receipt.decoded_height > 200u) {
                        ++failure_count;
                        continue;
                    }
                    /* SKProject SkWinCore.cpp::DRAW_HAND_ACTION_ICONS calls
                     * DRAW_ICON_PICT_ENTRY, whose QUERY_BLIT_RECT path uses
                     * the image dimensions and the original RAW4 table.  The
                     * receipt must therefore resolve to an in-frame,
                     * unscaled source destination before it can reach M11. */
                    memset(&raw4_destination, 0, sizeof(raw4_destination));
                    if (!dm2_v1_gdat_door_overlay_query_raw4_destination_rect(
                            &loader, receipt.expanded_rect_index,
                            receipt.decoded_width, receipt.decoded_height,
                            &raw4_destination) ||
                        raw4_destination.x < 0 || raw4_destination.y < 0 ||
                        raw4_destination.w != (int)receipt.decoded_width ||
                        raw4_destination.h != (int)receipt.decoded_height ||
                        raw4_destination.x + raw4_destination.w > 320 ||
                        raw4_destination.y + raw4_destination.h > 200) {
                        ++failure_count;
                        continue;
                    }
                    blit.rect_number = receipt.expanded_rect_index;
                    blit.width = receipt.decoded_width;
                    blit.height = receipt.decoded_height;
                    blit.transparent_index = UINT8_MAX;
                    surface.pixels = target;
                    surface.width = 320;
                    surface.height = 200;
                    surface.stride = 320;
                    if (!dm2_v1_inventory_panel_consume_hand_slot_backdrop(
                            &loader, &receipt, &blit, &surface, &consumed) ||
                        !consumed.valid || consumed.drawn_pixel_count !=
                            (uint32_t)receipt.decoded_width * receipt.decoded_height ||
                        consumed.transparent_pixel_count != 0u ||
                        !consumed.identity_hash) {
                        ++failure_count;
                        continue;
                    }
                    ++hand_count;
                    identity_hash ^= receipt.identity_hash;
                    identity_hash *= 16777619u;
                }
            }
        }
    }
    {
        DM2_V1_InventoryPanelSurveyFrameReceipt receipt;
        DM2_V1_InventoryPanelHudBlit blit;
        DM2_V1_InventoryPanelHudSurface surface;
        DM2_V1_InventoryPanelHudConsumptionReceipt consumed;

        memset(&receipt, 0, sizeof(receipt));
        memset(&blit, 0, sizeof(blit));
        memset(&surface, 0, sizeof(surface));
        memset(&consumed, 0, sizeof(consumed));
        memset(target, 0, sizeof(target));
        if (!dm2_v1_inventory_panel_survey_frame_receipt(&loader, &receipt) ||
            !receipt.valid || receipt.category != DM2_GDAT_CATEGORY_INTERFACE_CHARSHEET ||
            receipt.index != 0u || receipt.image_field != 1u ||
            receipt.expanded_rect_index != DM2_V1_INVENTORY_SURVEY_PREVIEW_RECT ||
            !receipt.raw_hash || !receipt.decoded_hash || !receipt.palette_hash ||
            !receipt.identity_hash || receipt.format != DM2_IMG_FMT_IMG3 ||
            receipt.decoded_width == 0u || receipt.decoded_height == 0u ||
            receipt.decoded_width > 320u || receipt.decoded_height > 200u) {
            ++failure_count;
        } else {
            blit.rect_number = receipt.expanded_rect_index;
            blit.width = receipt.decoded_width;
            blit.height = receipt.decoded_height;
            blit.transparent_index = UINT8_MAX;
            surface.pixels = target;
            surface.width = 320;
            surface.height = 200;
            surface.stride = 320;
            if (!dm2_v1_inventory_panel_consume_survey_frame(
                    &loader, &receipt, &blit, &surface, &consumed) ||
                !consumed.valid || consumed.drawn_pixel_count !=
                    (uint32_t)receipt.decoded_width * receipt.decoded_height ||
                consumed.transparent_pixel_count != 0u || !consumed.identity_hash) {
                ++failure_count;
            } else {
                identity_hash ^= receipt.identity_hash;
                identity_hash *= 16777619u;
            }
        }
    }
    printf("hand-routes=%u raw4-hash=%08x survey-frame=%s identity-hash=%08x\n",
           hand_count, raw4_hash, failure_count == 0u ? "PASS" : "FAIL",
           identity_hash);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (failure_count != 0u || hand_count != 64u || identity_hash == 0u) {
        fputs("FAIL: a DM2 inventory HUD route lacked exact GDAT material\n", stderr);
        return 1;
    }
    puts("PASS: every inventory HUD receipt consumes original GDAT pixels");
    return 0;
}
