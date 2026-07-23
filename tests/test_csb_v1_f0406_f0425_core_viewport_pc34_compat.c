#include "csb_v1_f0406_f0425_core_viewport_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("PASS: %s\n", message); } \
    else { ++failed; printf("FAIL: %s\n", message); } \
} while (0)

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_CSBGraphicsStartupPackage package;
    CSB_V1_F0406F0425CoreViewportReceiptPc34 receipt;
    uint8_t raw[80];
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY;

    memset(&profile, 0, sizeof(profile));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&package, 0, sizeof(package));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = sizeof(raw);
    dungeon.square_bytes = 1;
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.raw_map_data_base = 0;
    profile.dungeon_handle = &dungeon;
    profile.party_state_valid = 1;
    profile.graphics_asset.path = "/pc34/CSBgraphics.dat";

    package.valid = 1;
    package.hud_ready = 1;
    package.palette_material_complete = 1;
    package.palette_source.valid = 1;
    package.palette_source.source_kind =
        CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    snprintf(package.palette_source.source_path,
             sizeof(package.palette_source.source_path), "%s",
             profile.graphics_asset.path);
    package.assets[role].present = 1;
    package.assets[role].entry_index = 17;
    package.assets[role].decompressed_size = 1;
    package.image_sources[role].valid = 1;
    package.image_sources[role].source_kind =
        CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    package.image_sources[role].entry_span.entry_index = 17;
    package.image_sources[role].entry_span.compressed_size = 1;
    package.image_sources[role].entry_span.decompressed_size = 1;
    snprintf(package.image_sources[role].source_path,
             CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP, "%s",
             profile.graphics_asset.path);

    CHECK(csb_v1_f0406_f0425_core_viewport_receipt_pc34(
              &profile, &package, &receipt) &&
              receipt.source_bound_mask == CSB_V1_F0406_F0425_SOURCE_MASK &&
              receipt.menu_action_owner_required && receipt.save_owner_required &&
              receipt.dialog_owner_required,
          "F0406-F0425 authentic receipt retains unproven owners");

    package.palette_source.source_path[0] = '\0';
    CHECK(!csb_v1_f0406_f0425_core_viewport_receipt_pc34(
              &profile, &package, &receipt),
          "foreign or missing graphics provenance fails closed");

    printf("%d/%d\n", passed, passed + failed);
    return failed ? 1 : 0;
}
