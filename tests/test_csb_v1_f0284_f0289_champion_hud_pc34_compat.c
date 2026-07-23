#include "csb_v1_f0284_f0289_champion_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;
#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\n", message); } \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

static void fixture(CSB_V1_RuntimeProfile *profile, CSB_V1_DungeonData *dungeon,
                    CSB_V1_CSBGraphicsStartupPackage *package, uint8_t raw[32])
{
    CSB_V1_Champion *champion;
    memset(profile, 0, sizeof(*profile)); memset(dungeon, 0, sizeof(*dungeon));
    memset(package, 0, sizeof(*package)); memset(raw, 0, 32);
    dungeon->raw_data = raw; dungeon->raw_size = 32; dungeon->square_bytes = 1;
    profile->dungeon_handle = dungeon; profile->graphics_asset.path = "/pc34/CSBgraphics.dat";
    profile->party_state_valid = 1; profile->champion_count = 1; profile->party_dir = 1;
    profile->party_state.ImportedFromDM1 = 1; profile->party_state.ChampionCount = 1;
    profile->party_state.PartyDirection = 1;
    champion = &profile->party_state.Champions[0]; champion->Fingerprint = 1;
    champion->Cell = 2; champion->Direction = 1;
    champion->CurrentHealth = 31; champion->MaximumHealth = 100;
    champion->CurrentStamina = 1000; champion->MaximumStamina = 1000;
    champion->CurrentMana = 110; champion->MaximumMana = 100;
    package->valid = 1; package->hud_ready = 1; package->palette_material_complete = 1;
    package->palette_source.valid = 1;
    package->palette_source.source_kind = CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
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
    CSB_V1_RuntimeProfile profile; CSB_V1_DungeonData dungeon;
    CSB_V1_CSBGraphicsStartupPackage package; CSB_V1_F0284F0289ChampionHudReceiptPc34 receipt;
    uint8_t raw[32]; CSB_V1_PartyState before;
    fixture(&profile, &dungeon, &package, raw); before = profile.party_state;
    CHECK(csb_v1_f0284_f0289_champion_hud_receipt_pc34(&profile, &package, 0, 3, 2, 116, 31, 100, &receipt) == 1 && receipt.valid && receipt.direction_delta == 2 && receipt.index_in_cell == 0 && receipt.bar_heights[0] == 8 && receipt.bar_heights[1] == 25 && receipt.bar_heights[2] == 25 && strcmp(receipt.value_current_text, " 31") == 0, "F0284/F0285/F0287-F0289 source receipt is material-bound");
    CHECK(memcmp(&before, &profile.party_state, sizeof(before)) == 0 && receipt.f0286_runtime_owner_required, "receipt does not mutate party and leaves F0286 ordered-cell ownership external");
    package.palette_source.source_path[0] = '\0';
    CHECK(csb_v1_f0284_f0289_champion_hud_receipt_pc34(&profile, &package, 0, 3, 2, 116, 31, 100, &receipt) == 0, "missing authenticated panel material fails closed");
    printf("PASSED: %d\nFAILED: %d\n", passed, failed); return failed ? 1 : 0;
}
