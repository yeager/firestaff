#include "csb_v1_f0326_f0345_core_ui_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed, failed;
#define CHECK(c, m) do { if (c) { ++passed; printf("  PASS: %s\n", m); } else { ++failed; printf("  FAIL: %s\n", m); } } while (0)
static void write_u16(uint8_t *bytes, int offset, uint16_t value) { bytes[offset] = (uint8_t)value; bytes[offset + 1] = (uint8_t)(value >> 8); }
static void fixture(CSB_V1_RuntimeProfile *profile, CSB_V1_DungeonData *dungeon, CSB_V1_CSBGraphicsStartupPackage *package, uint8_t raw[160])
{
    memset(profile, 0, sizeof(*profile)); memset(dungeon, 0, sizeof(*dungeon)); memset(package, 0, sizeof(*package)); memset(raw, 0, 160);
    dungeon->raw_data = raw; dungeon->raw_size = 160; dungeon->square_bytes = 1;
    dungeon->thing_data_bases[5] = 96; dungeon->thing_type_counts[5] = 1;
    write_u16(raw, 96, 0xfffeu); write_u16(raw, 98, 7);
    profile->dungeon_handle = dungeon; profile->graphics_asset.path = "/pc34/CSBgraphics.dat";
    profile->party_state_valid = 1; profile->champion_count = 1; profile->party_state.ChampionCount = 1; profile->party_state.ImportedFromDM1 = 1;
    package->valid = 1; package->hud_ready = 1; package->palette_material_complete = 1;
    package->palette_source.valid = 1; package->palette_source.source_kind = CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    snprintf(package->palette_source.source_path, sizeof(package->palette_source.source_path), "%s", "/pc34/CSBgraphics.dat");
    snprintf(package->palette_source.source_md5, sizeof(package->palette_source.source_md5), "%s", "0123456789abcdef0123456789abcdef");
    package->assets[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].present = 1;
    package->assets[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].entry_index = 17;
    package->assets[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].decompressed_size = 1;
    package->image_sources[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].valid = 1;
    package->image_sources[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].source_kind = CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    package->image_sources[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].entry_span.entry_index = 17;
    package->image_sources[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].entry_span.compressed_size = 1;
    package->image_sources[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].entry_span.decompressed_size = 1;
    snprintf(package->image_sources[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].source_path, CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP, "%s", "/pc34/CSBgraphics.dat");
    snprintf(package->image_sources[CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY].source_md5, CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP, "%s", "0123456789abcdef0123456789abcdef");
}
int main(void)
{
    CSB_V1_RuntimeProfile profile; CSB_V1_DungeonData dungeon; CSB_V1_CSBGraphicsStartupPackage package;
    CSB_V1_F0326F0345CoreUiReceiptPc34 receipt; uint8_t raw[160]; uint8_t before[160];
    fixture(&profile, &dungeon, &package, raw); memcpy(before, raw, sizeof(raw));
    CHECK(csb_v1_f0326_f0345_core_ui_receipt_pc34(&profile, &package, (uint16_t)(5u << 10), &receipt) == 1 && receipt.valid && receipt.source_bound_mask == CSB_V1_F0326_F0345_SOURCE_MASK && receipt.projectile_runtime_owner_required && receipt.inventory_runtime_owner_required, "F0326-F0345 contract requires loaded PC34 Thing and panel material");
    CHECK(memcmp(before, raw, sizeof(raw)) == 0 && profile.timeline_queue.eventCount == 0, "receipt creates no projectile, event, mutation, or render action");
    package.palette_source.source_path[0] = '\0';
    CHECK(csb_v1_f0326_f0345_core_ui_receipt_pc34(&profile, &package, (uint16_t)(5u << 10), &receipt) == 0, "missing panel source fails closed");
    printf("PASSED: %d\nFAILED: %d\n", passed, failed); return failed ? 1 : 0;
}
