/* Authentic DM2 inventory hand-field source/material receipt coverage.
 *
 * This reads GRAPHICS.DAT in place.  It enumerates the complete source
 * hand-action orientation matrix and, where supported, DRAW_ITEM_SURVEY's
 * static frame. It may consume authenticated receipts into a local test
 * surface; it does not exercise the production viewport or presented frame.
 * No path can use host icons, procedural panels, cached replacements or
 * synthetic game data.
 *
 * Source: SKProject SKWIN/c_gui_draw.cpp:2072-2106 (DRAW_ITEM_SURVEY) and
 * 2341-2386 (DRAW_HAND_ACTION_ICONS).
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_hand_action_gdat.h"
#include "dm2_v1_inventory_panel.h"
#include "asset_find_by_hash.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int load_graphics_data(uint8_t **graphics, size_t *graphics_size)
{
    const char *file = getenv("FIRESTAFF_DM2_GRAPHICS_FILE");
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");

    if (file && file[0]) {
        FILE *fp = fopen(file, "rb");
        long length;
        if (!fp || fseek(fp, 0, SEEK_END) != 0 ||
            (length = ftell(fp)) <= 0 || fseek(fp, 0, SEEK_SET) != 0) {
            if (fp) fclose(fp);
            return 0;
        }
        *graphics = (uint8_t *)malloc((size_t)length);
        if (!*graphics || fread(*graphics, 1u, (size_t)length, fp) !=
                (size_t)length) {
            free(*graphics);
            *graphics = NULL;
            fclose(fp);
            return 0;
        }
        fclose(fp);
        *graphics_size = (size_t)length;
        return 1;
    }

    if (!archive || !archive[0] || !graphics || !graphics_size) return 0;
    *graphics = NULL;
    *graphics_size = 0u;
    return firestaff_zip_extract_by_suffix(archive, "data/graphics.dat",
                                            graphics, graphics_size) == 0 &&
           *graphics && *graphics_size;
}

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_BootProfile amiga_profile;
    int graphics_borrowed = 0;
    DM2_V1_AssetLoader loader;
    uint8_t target[640 * 400];
    unsigned int hand_count = 0u;
    unsigned int decoded_count = 0u;
    unsigned int receipt_count = 0u;
    unsigned int consumed_count = 0u;
    unsigned int failure_count = 0u;
    uint32_t identity_hash = 2166136261u;
    uint32_t raw4_hash = 2166136261u;
    uint32_t decoded_hash = 2166136261u;
    uint8_t admitted_gdat_version = 0u;
    const char *survey_frame_status = "PASS";
    const char *survey_frame_reason = "verified";
    const char *material_test_status = "source-receipt-pixel-consumer";
    const uint8_t *raw4 = NULL;
    size_t raw4_size = 0u;
    const char *expected_graphics_md5 = getenv("FIRESTAFF_DM2_GRAPHICS_MD5");

    const char *amiga_root = getenv("FIRESTAFF_DM2_AMIGA_ROOT");
    if ((!getenv("FIRESTAFF_DM2_DOS_ARCHIVE") ||
         !getenv("FIRESTAFF_DM2_DOS_ARCHIVE")[0]) &&
        (!getenv("FIRESTAFF_DM2_GRAPHICS_FILE") ||
         !getenv("FIRESTAFF_DM2_GRAPHICS_FILE")[0]) &&
        (!amiga_root || !amiga_root[0])) {
        puts("SKIP: set a DM2 authentic graphics file, archive, or Amiga root");
        return 77;
    }
    if (!expected_graphics_md5 || strlen(expected_graphics_md5) != 32u) {
        puts("SKIP: set the expected edition GRAPHICS.DAT MD5");
        return 77;
    }
    if (amiga_root && amiga_root[0]) {
        dm2_v1_boot_profile_init(&amiga_profile);
        if (dm2_v1_boot_scan_assets(&amiga_profile, amiga_root) != 0 ||
            !amiga_profile.assets_verified ||
            amiga_profile.platform != DM2_PLATFORM_AMIGA_EN ||
            !amiga_profile.graphics_mem || !amiga_profile.graphics_mem_size) {
            fputs("FAIL: authentic DM2 Amiga GRAPHICS.DAT was not admitted\n",
                  stderr);
            dm2_v1_boot_cleanup(&amiga_profile);
            return 1;
        }
        graphics = amiga_profile.graphics_mem;
        graphics_size = amiga_profile.graphics_mem_size;
        graphics_borrowed = 1;
    } else if (!load_graphics_data(&graphics, &graphics_size)) {
        fputs("FAIL: selected authentic DM2 GRAPHICS.DAT is unreadable\n", stderr);
        return 1;
    }
    if (!asset_memory_matches_md5(graphics, graphics_size,
                                  expected_graphics_md5)) {
        fputs("FAIL: GRAPHICS.DAT does not match the selected DM2 edition\n",
              stderr);
        if (graphics_borrowed) dm2_v1_boot_cleanup(&amiga_profile);
        else free(graphics);
        return 1;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !dm2_v1_asset_loader_verify(&loader)) {
        fputs("FAIL: authentic DM2 GRAPHICS.DAT was not admitted\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        if (graphics_borrowed) dm2_v1_boot_cleanup(&amiga_profile);
        else free(graphics);
        return 1;
    }
    admitted_gdat_version = loader.gdat_version;
    raw4 = dm2_v1_asset_load_typed_sized(
        &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw4_size);
    if (!raw4 || raw4_size == 0u) {
        fputs("FAIL: canonical HUD RAW4 table was not admitted\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        if (graphics_borrowed) dm2_v1_boot_cleanup(&amiga_profile);
        else free(graphics);
        return 1;
    }
    for (size_t i = 0u; i < raw4_size; ++i) {
        raw4_hash ^= raw4[i];
        raw4_hash *= 16777619u;
    }
    if (raw4_hash == 0u) {
        fputs("FAIL: canonical HUD RAW4 identity was empty\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        if (!graphics_borrowed) free(graphics);
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
                    DM2_V1_HandActionInput input;
                    DM2_V1_HandActionGdatRoute route;
                    int image_width = 0;
                    int image_height = 0;
                    DM2_ImageFormat image_format = DM2_IMG_FMT_UNKNOWN;
                    uint8_t *image_pixels;
                    memset(&input, 0, sizeof(input));
                    input.possession_index = possession;
                    input.left_or_right = side;
                    input.player_position = position;
                    input.party_direction = direction;
                    image_pixels = dm2_v1_hand_action_gdat_load_image(
                        &loader, &input, &route, &image_width, &image_height,
                        &image_format);
                    if (!image_pixels ||
                        route.category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
                        route.subcategory != 4u ||
                        route.entry != (uint8_t)(2u + possession * 2u + side) ||
                        route.rectno != (uint8_t)((possession ? 0x46u : 0x4au) +
                            ((position + 4u - direction) & 3u)) ||
                        image_format == DM2_IMG_FMT_UNKNOWN || image_width <= 0 ||
                        image_height <= 0 || image_width > 320 ||
                        image_height > 200) {
                        dm2_v1_asset_free_pixels(image_pixels);
                        ++failure_count;
                        continue;
                    }
                    decoded_hash = hash_bytes(decoded_hash, image_pixels,
                        (size_t)image_width * (size_t)image_height);
                    ++decoded_count;
                    if (!dm2_v1_inventory_panel_hand_slot_backdrop_receipt(
                            &loader, possession, side, position, direction,
                            &receipt) && loader.gdat_version !=
                                4u) {
                        dm2_v1_asset_free_pixels(image_pixels);
                        ++failure_count;
                        continue;
                    }
                    if (receipt.valid && (
                        receipt.category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
                        receipt.index != 4u || receipt.image_field < 2u ||
                        receipt.image_field > 5u || !receipt.raw_hash ||
                        !receipt.decoded_hash || !receipt.palette_hash ||
                        !receipt.identity_hash ||
                        (receipt.format != DM2_IMG_FMT_IMG3 &&
                         receipt.format != DM2_IMG_FMT_U4) ||
                        receipt.decoded_width == 0u || receipt.decoded_height == 0u ||
                        receipt.decoded_width > 320u || receipt.decoded_height > 200u)) {
                        dm2_v1_asset_free_pixels(image_pixels);
                        ++failure_count;
                        continue;
                    }
                    if (receipt.valid) ++receipt_count;
                    /* SKProject SkWinCore.cpp::DRAW_HAND_ACTION_ICONS calls
                     * DRAW_ICON_PICT_ENTRY, whose QUERY_BLIT_RECT path uses
                     * the image dimensions and the original RAW4 table.  The
                     * receipt must therefore resolve to an in-frame,
                     * unscaled source destination before it can reach M11. */
                    memset(&raw4_destination, 0, sizeof(raw4_destination));
                    if (!dm2_v1_gdat_door_overlay_query_raw4_destination_rect(
                            &loader, receipt.valid ? receipt.expanded_rect_index :
                                route.rectno,
                            receipt.valid ? receipt.decoded_width :
                                (uint16_t)image_width,
                            receipt.valid ? receipt.decoded_height :
                                (uint16_t)image_height,
                            &raw4_destination) ||
                        raw4_destination.x < 0 || raw4_destination.y < 0 ||
                        raw4_destination.w != (receipt.valid ?
                            (int)receipt.decoded_width : image_width) ||
                        raw4_destination.h != (receipt.valid ?
                            (int)receipt.decoded_height : image_height) ||
                        raw4_destination.x + raw4_destination.w > 640 ||
                        raw4_destination.y + raw4_destination.h > 400) {
                        dm2_v1_asset_free_pixels(image_pixels);
                        ++failure_count;
                        continue;
                    }
                    dm2_v1_asset_free_pixels(image_pixels);
                    if (!receipt.valid) {
                        ++hand_count;
                        identity_hash ^= raw4_hash + route.entry + route.rectno;
                        identity_hash *= 16777619u;
                        continue;
                    }
                    blit.rect_number = receipt.expanded_rect_index;
                    blit.destination_x = raw4_destination.x;
                    blit.destination_y = raw4_destination.y;
                    blit.width = receipt.decoded_width;
                    blit.height = receipt.decoded_height;
                    blit.transparent_index = UINT8_MAX;
                    surface.pixels = target;
                    surface.width = 640;
                    surface.height = 400;
                    surface.stride = 640;
                    if (!dm2_v1_inventory_panel_consume_hand_slot_backdrop(
                            &loader, &receipt, &blit, &surface, &consumed) ||
                        !consumed.valid || consumed.drawn_pixel_count !=
                            (uint32_t)receipt.decoded_width * receipt.decoded_height ||
                        consumed.transparent_pixel_count != 0u ||
                        !consumed.identity_hash) {
                        ++failure_count;
                        continue;
                    }
                    ++consumed_count;
                    ++hand_count;
                    identity_hash ^= receipt.identity_hash;
                    identity_hash *= 16777619u;
                }
            }
        }
    }
    if (loader.gdat_version != 4u) {
        DM2_V1_InventoryPanelSurveyFrameReceipt receipt;
        memset(&receipt, 0, sizeof(receipt));
        if (!dm2_v1_inventory_panel_survey_frame_receipt(&loader, &receipt) ||
            !receipt.valid || receipt.category != DM2_GDAT_CATEGORY_INTERFACE_CHARSHEET ||
            receipt.index != 0u || receipt.image_field != 1u ||
            receipt.expanded_rect_index != DM2_V1_INVENTORY_SURVEY_PREVIEW_RECT ||
            !receipt.raw_hash || !receipt.decoded_hash || !receipt.palette_hash ||
            !receipt.identity_hash ||
            (receipt.format != DM2_IMG_FMT_IMG3 &&
             receipt.format != DM2_IMG_FMT_U4) ||
            receipt.decoded_width == 0u || receipt.decoded_height == 0u ||
            receipt.decoded_width > 320u || receipt.decoded_height > 200u) {
            survey_frame_status = "FAIL";
            survey_frame_reason = "source-receipt-or-field-check";
            ++failure_count;
        } else {
            DM2_V1_GdatRaw4BlitPlacement placement;
            memset(&placement, 0, sizeof(placement));
            if (!dm2_v1_gdat_door_overlay_query_raw4_blit_placement(
                    &loader, receipt.expanded_rect_index,
                    receipt.decoded_width, receipt.decoded_height,
                    &placement) || placement.source_x < 0 ||
                placement.source_y < 0 || placement.destination.x < 0 ||
                placement.destination.y < 0 || placement.destination.w <= 0 ||
                placement.destination.h <= 0 ||
                placement.destination.x + placement.destination.w > 640 ||
                placement.destination.y + placement.destination.h > 400 ||
                placement.source_x + placement.destination.w >
                    (int)receipt.decoded_width ||
                placement.source_y + placement.destination.h >
                    (int)receipt.decoded_height) {
                survey_frame_status = "FAIL";
                survey_frame_reason = "raw4-placement-check";
                ++failure_count;
            } else {
                survey_frame_status =
                    placement.source_x != 0 || placement.source_y != 0 ||
                    placement.destination.w != (int)receipt.decoded_width ||
                    placement.destination.h != (int)receipt.decoded_height
                        ? "source-receipt-clipped" : "source-receipt-unclipped";
                survey_frame_reason = "production-renderer-not-exercised";
                identity_hash = hash_bytes(identity_hash,
                    (const uint8_t *)&receipt.identity_hash,
                    sizeof(receipt.identity_hash));
            }
        }
    } else {
        survey_frame_status = "SKIP";
        survey_frame_reason = "gdat-v4-survey-frame-unmapped";
    }
    if (admitted_gdat_version == 4u) {
        if (receipt_count == 0u && consumed_count == 0u) {
            material_test_status = "decode-only-no-source-receipt-api";
        } else if (receipt_count == 64u && consumed_count == 64u) {
            material_test_status = "source-receipt-pixel-consumer";
        } else {
            ++failure_count;
            material_test_status = "partial-source-receipts";
        }
    } else if (receipt_count != 64u || consumed_count != 64u) {
        ++failure_count;
    }
    printf("platform=%s gdat-v%u hand-routes=%u decoded=%u receipts=%u consumed-on-test-surface=%u raw4-hash=%08x decoded-hash=%08x survey-frame=%s survey-reason=%s material-test=%s identity-hash=%08x\n",
           getenv("FIRESTAFF_DM2_PLATFORM") ? getenv("FIRESTAFF_DM2_PLATFORM") : "unspecified",
           loader.gdat_version, hand_count, decoded_count, receipt_count,
           consumed_count, raw4_hash, decoded_hash, survey_frame_status,
           survey_frame_reason,
           material_test_status,
           identity_hash);
    dm2_v1_asset_loader_free(&loader);
    if (graphics_borrowed) dm2_v1_boot_cleanup(&amiga_profile);
    else free(graphics);
    if (failure_count != 0u || hand_count != 64u || decoded_count != 64u ||
        decoded_hash == 0u || identity_hash == 0u ||
        (admitted_gdat_version != 4u &&
         (receipt_count != 64u || consumed_count != 64u)) ||
        (admitted_gdat_version == 4u &&
         !((receipt_count == 0u && consumed_count == 0u) ||
           (receipt_count == 64u && consumed_count == 64u)))) {
        fputs("FAIL: a DM2 inventory HUD route lacked exact GDAT material\n", stderr);
        return 1;
    }
    puts("PASS: all authentic DM2 hand routes decode; source receipts and test-surface consumption are reported separately");
    return 0;
}
