/* Opt-in boot regression for DM2's original Amiga installer.
 *
 * The archive is read through its nested ZIP/ADF/LZX transport in memory.
 * No game member is extracted, copied or materialized on disk. */

#include "dm2_v1_boot.h"
#include "dm2_v1_asset_loader.h"

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

    if (!root || root[0] == '\0') {
        puts("SKIP: FIRESTAFF_DM2_AMIGA_ROOT is not set");
        return 0;
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
    }
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
    memset(&interface_palette, 0, sizeof(interface_palette));
    expect(dm2_v1_boot_interface_palette(&profile, &interface_palette) &&
               interface_palette.hash != 0u,
           "the native Amiga interface palette binds without a PC palette-table fallback");
    dm2_v1_boot_cleanup(&profile);
    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM2 Amiga boot remains memory-owned from original installer media");
    return 0;
}
