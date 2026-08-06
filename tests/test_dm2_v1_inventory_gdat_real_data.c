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
#include "dm2_v1_inventory_panel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static int load_canonical_graphics(uint8_t **graphics, size_t *graphics_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    char path[2048];

    if (root && root[0]) {
        snprintf(path, sizeof(path), "%s/graphics.dat", root);
        if (read_file(path, graphics, graphics_size)) return 1;
    }
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm2/data/graphics.dat",
                 home);
        if (read_file(path, graphics, graphics_size)) return 1;
    }
    return 0;
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

    if (!load_canonical_graphics(&graphics, &graphics_size)) {
        puts("SKIP: no local canonical DM2 GRAPHICS.DAT");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_asset_loader_verify(&loader)) {
        fputs("FAIL: canonical DM2 GRAPHICS.DAT was not admitted\n", stderr);
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
    printf("hand-routes=%u survey-frame=%s identity-hash=%08x\n",
           hand_count, failure_count == 0u ? "PASS" : "FAIL", identity_hash);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (failure_count != 0u || hand_count != 64u || identity_hash == 0u) {
        fputs("FAIL: a DM2 inventory HUD route lacked exact GDAT material\n", stderr);
        return 1;
    }
    puts("PASS: every inventory HUD receipt consumes original GDAT pixels");
    return 0;
}
