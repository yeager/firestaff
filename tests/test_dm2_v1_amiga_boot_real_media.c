/* Opt-in boot regression for DM2's original Amiga installer.
 *
 * The archive is read through its nested ZIP/ADF/LZX transport in memory.
 * No game member is extracted, copied or materialized on disk. */

#include "dm2_v1_boot.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_gdat_wall_m11_command.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_AMIGA_ROOT");
    DM2_V1_BootProfile profile;
    DM2_V1_AssetLoader graphics_loader;
    DM2_V1_InterfacePalette interface_palette;
    DM2_V1_InterfaceHudLayout hud_layout;
    DM2_V1_GdatImageMetadata floor_metadata;
    DM2_V1_GdatImageMetadata ceiling_metadata;
    DM2_ImageFormat status_pane_format = DM2_IMG_FMT_UNKNOWN;
    uint8_t *status_pane_pixels = NULL;
    int status_pane_width = 0;
    int status_pane_height = 0;
    DM2_V1_GdatSceneM11CommandPlan scene_plan;
    DM2_V1_GdatSceneQueryBlitRectReceipt rect_receipt;
    DM2_V1_BootExpandedRectReceipt rect7_receipt;
    DM2_V1_GdatWallM11CommandPlan wall_plan;
    int floor_metadata_ok;
    int ceiling_metadata_ok;

    if (!root || root[0] == '\0') {
        puts("SKIP: FIRESTAFF_DM2_AMIGA_ROOT is not set");
        return 77;
    }
    dm2_v1_boot_profile_init(&profile);
    expect(dm2_v1_boot_scan_assets(&profile, root) == 0,
           "the original Amiga installer provides a complete DM2 hash pair");
    expect(profile.assets_verified &&
               profile.platform == DM2_PLATFORM_AMIGA_EN &&
               strcmp(profile.version_id, "amiga-en") == 0,
           "boot admits only the verified Amiga release identity");
    expect(profile.graphics_mem && profile.graphics_mem_size == 3493879u &&
               profile.dungeon_mem && profile.dungeon_mem_size == 39411u,
           "boot retains authenticated GRAPHICS.DAT and DUNGEON.DAT in RAM");
    memset(&graphics_loader, 0, sizeof(graphics_loader));
    memset(&floor_metadata, 0, sizeof(floor_metadata));
    memset(&ceiling_metadata, 0, sizeof(ceiling_metadata));
    memset(&hud_layout, 0, sizeof(hud_layout));
    memset(&scene_plan, 0, sizeof(scene_plan));
    memset(&rect_receipt, 0, sizeof(rect_receipt));
    memset(&wall_plan, 0, sizeof(wall_plan));
    expect(dm2_v1_asset_loader_init(&graphics_loader, profile.graphics_mem,
                                    profile.graphics_mem_size) == 0,
           "the authenticated Amiga GRAPHICS.DAT opens through the native GDAT loader");
    if (graphics_loader.loaded) {
        int interface_palette_entry_count = 0;
        int legacy_palette16_entry_count = 0;
        for (uint16_t i = 0; i < graphics_loader.entry_count; ++i) {
            const DM2_V1_GdatEntry *entry = &graphics_loader.entries[i];
            if (entry->cls1 == DM2_GDAT_CATEGORY_INTERFACE_GENERAL &&
                entry->cls2 == 0) {
                if (entry->cls3 == DM2_GDAT_ENTRY_TYPE_PAL_IRGB &&
                    entry->cls4 == 0) {
                    ++interface_palette_entry_count;
                }
                if (entry->cls3 == DM2_GDAT_ENTRY_TYPE_PAL_16) {
                    ++legacy_palette16_entry_count;
                }
            }
        }
        expect(interface_palette_entry_count == 1 &&
                   legacy_palette16_entry_count == 0,
               "the authenticated Amiga GDAT exposes its native 16-colour interface palette");
        expect(graphics_loader.gdat_version == 5u && graphics_loader.big_endian,
               "the original Amiga scene is admitted as big-endian GDAT v5");
        status_pane_pixels = dm2_v1_asset_load_image_field(
            &graphics_loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 2, 0,
            &status_pane_width, &status_pane_height, &status_pane_format);
        expect(status_pane_pixels && status_pane_width == 77 &&
                   status_pane_height == 37 &&
                   status_pane_format == DM2_IMG_FMT_IMG3,
               "the original Amiga 3-stat pane composes its IMG3 overlay with the tracked underlay");
        dm2_v1_asset_free_pixels(status_pane_pixels);
        status_pane_pixels = NULL;
        floor_metadata_ok = dm2_v1_asset_load_image_metadata(
                   &graphics_loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 2,
                   DM2_GDAT_GFXSET_FLOOR, &floor_metadata);
        expect(floor_metadata_ok &&
                   floor_metadata.width == 224u && floor_metadata.height == 78u &&
                   floor_metadata.bits_per_pixel == 4u &&
                   floor_metadata.metadata_hash != 0u,
               "Amiga GRAPHICSSET 2 floor metadata uses its C4 header");
        ceiling_metadata_ok = dm2_v1_asset_load_image_metadata(
                   &graphics_loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 2,
                   DM2_GDAT_GFXSET_CEIL, &ceiling_metadata);
        expect(ceiling_metadata_ok &&
                   ceiling_metadata.width == 224u && ceiling_metadata.height == 33u &&
                   ceiling_metadata.bits_per_pixel == 4u &&
                   ceiling_metadata.metadata_hash != 0u,
               "Amiga GRAPHICSSET 2 ceiling metadata uses its C4 header");
        expect(dm2_v1_gdat_scene_query_blit_rect_receipt(
                   &graphics_loader, &rect_receipt) && rect_receipt.valid &&
                   rect_receipt.table_hash != 0u &&
                   rect_receipt.floor_row_hash != 0u &&
                   rect_receipt.ceiling_row_hash != 0u,
               "the source Amiga QUERY_BLIT_RECT rows are available");
        expect(dm2_v1_gdat_scene_m11_command_plan_build(
                   &graphics_loader, 2u, &scene_plan) && scene_plan.valid &&
                   scene_plan.command_hash != 0u &&
                   scene_plan.commands[0].width == floor_metadata.width &&
                   scene_plan.commands[0].height == floor_metadata.height &&
                   scene_plan.commands[1].width == ceiling_metadata.width &&
                   scene_plan.commands[1].height == ceiling_metadata.height,
               "the real Amiga GRAPHICSSET 2 scene builds a complete plane plan");
        expect(scene_plan.commands[0].format == DM2_IMG_FMT_IMG3 &&
                   scene_plan.commands[1].format == DM2_IMG_FMT_IMG3 &&
                   scene_plan.commands[0].palette_hash != 0u &&
                   scene_plan.commands[1].palette_hash != 0u,
               "the Amiga floor and ceiling retain their decoded IMG3 local palettes");
        {
            int physical_palette_indices = 1;
            for (int color = 0; color < 16; ++color) {
                if (scene_plan.commands[0].palette16[color] != (uint8_t)color ||
                    scene_plan.commands[1].palette16[color] != (uint8_t)color) {
                    physical_palette_indices = 0;
                    break;
                }
            }
            expect(physical_palette_indices,
                   "Amiga C4-decoded scene indices bind directly to its physical 16-colour palette");
        }
        expect(dm2_v1_gdat_scene_m11_command_plan_draw_order_valid(
                   &scene_plan),
               "the Amiga source RECT_700/RECT_701 draw order binds to the scene plan");
        expect(dm2_v1_gdat_wall_m11_command_plan_build(
                   &graphics_loader, 2u, &wall_plan) && wall_plan.valid &&
                   wall_plan.command_count > 0u && wall_plan.command_hash != 0u,
               "the real Amiga GRAPHICSSET 2 wall images build an M11 plan");
    }
    dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
    dm2_v1_gdat_wall_m11_command_plan_free(&wall_plan);
    dm2_v1_asset_loader_free(&graphics_loader);
    expect(profile.music_map_verified && profile.music_map_size == 176u,
           "boot admits the original Amiga CD.DAT map in RAM");
    expect(profile.amiga_animation_media_verified &&
               profile.amiga_swsh_bytes && profile.amiga_swsh_byte_count == 28364u &&
               profile.amiga_titl_bytes && profile.amiga_titl_byte_count == 590134u &&
               profile.amiga_enda_bytes && profile.amiga_enda_byte_count == 650116u &&
               profile.amiga_swsh_stream.valid &&
               profile.amiga_titl_stream.valid && profile.amiga_enda_stream.valid,
           "boot retains the authenticated Amiga startup animations in RAM");
    expect(strstr(profile.graphics_path, "::DM2_archive.LZX/GRAPHICS.DAT") != NULL &&
               strstr(profile.dungeon_path, "::DM2_archive.LZX/DUNGEON.DAT") != NULL,
           "boot records nested media provenance instead of a cache path");
    expect(strcmp(profile.asset_root, root) == 0 &&
               strstr(profile.asset_root, "::") == NULL,
           "boot retains the selected outer archive as the runtime media owner");
    expect(dm2_v1_boot_enter_game(&profile) == 0,
           "the admitted original Amiga buffers complete DM2 boot");
    expect(dm2_v1_boot_interface_hud_layout(&profile, &hud_layout) &&
               hud_layout.valid && hud_layout.table_hash != 0u &&
               hud_layout.portrait_valid_mask == 0x0fu &&
               hud_layout.name_valid_mask == 0x0fu &&
               hud_layout.status_valid_mask[0] == 0x07u &&
               hud_layout.status_valid_mask[1] == 0x07u &&
               hud_layout.status_valid_mask[2] == 0x07u &&
               hud_layout.status_valid_mask[3] == 0x07u,
           "the authentic Amiga RAW4 table supplies all four HUD slot layouts");

    memset(&interface_palette, 0, sizeof(interface_palette));
    expect(dm2_v1_boot_interface_palette(&profile, &interface_palette) &&
               interface_palette.hash != 0u,
           "the native Amiga interface palette binds without a PC palette-table fallback");
    memset(&rect7_receipt, 0, sizeof(rect7_receipt));
    expect(dm2_v1_boot_query_expanded_rect_receipt(&profile, 7u,
                                                   &rect7_receipt) &&
               rect7_receipt.valid && rect7_receipt.rect_id == 7u &&
               rect7_receipt.rect.x == 0 && rect7_receipt.rect.y == 40 &&
               rect7_receipt.rect.w == 224 && rect7_receipt.rect.h == 136 &&
               rect7_receipt.raw4_bytes && rect7_receipt.raw4_byte_count &&
               rect7_receipt.receipt_hash,
           "Amiga runtime clips its 224x136 room viewport through authenticated RECT_7");
    dm2_v1_boot_cleanup(&profile);
    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM2 Amiga boot remains memory-owned from original installer media");
    return 0;
}
