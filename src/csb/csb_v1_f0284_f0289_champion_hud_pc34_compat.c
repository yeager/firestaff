#include "csb_v1_f0284_f0289_champion_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int pc34_champion_and_panel_ready(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsStartupPackage *package)
{
    if (!profile || !package || !profile->party_state_valid ||
        !profile->party_state.ImportedFromDM1 || !profile->dungeon_handle ||
        !profile->dungeon_handle->raw_data ||
        profile->dungeon_handle->square_bytes != 1 ||
        !profile->graphics_asset.path || !package->hud_ready ||
        !csb_v1_csbgraphics_startup_package_surface_draw_eligible(
            package, CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY) ||
        strcmp(profile->graphics_asset.path,
               package->palette_source.source_path) != 0) {
        return 0;
    }
    return profile->party_state.ChampionCount >= 1 &&
        profile->party_state.ChampionCount <= CSB_V1_MAX_CHAMPIONS &&
        profile->champion_count == profile->party_state.ChampionCount &&
        profile->party_dir >= 0 && profile->party_dir < 4 &&
        profile->party_state.PartyDirection == profile->party_dir;
}

static int ceil_bar_height(int current, int maximum, int clamp_to_maximum)
{
    int value;

    if (current <= 0) return 0;
    if (maximum <= 0) return -1;
    if (clamp_to_maximum && current > maximum) return 25;
    value = (current * 25 + maximum - 1) / maximum;
    return value > 25 ? 25 : value;
}

static int padded_decimal(int value, char out[5])
{
    if (value < 0 || value > 9999) return 0;
    (void)snprintf(out, 5u, "%3d", value);
    return 1;
}

int csb_v1_f0284_f0289_champion_hud_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    int champion_index, int requested_direction, int cell_query,
    int value_y, int value_current, int value_maximum,
    CSB_V1_F0284F0289ChampionHudReceiptPc34 *out_receipt)
{
    CSB_V1_F0284F0289ChampionHudReceiptPc34 receipt;
    const CSB_V1_Champion *champion;
    int index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.index_in_cell = -1;
    *out_receipt = receipt;
    if (!pc34_champion_and_panel_ready(profile, package) ||
        champion_index < 0 || champion_index >= profile->champion_count ||
        requested_direction < 0 || requested_direction > 3 ||
        cell_query < 0 || cell_query > 3 || value_y < 0 || value_y >= 200 ||
        value_maximum <= 0 || !padded_decimal(value_current, receipt.value_current_text) ||
        !padded_decimal(value_maximum, receipt.value_maximum_text)) {
        return 0;
    }
    champion = &profile->party_state.Champions[champion_index];
    if (champion->Fingerprint == 0 || champion->Cell > 3 ||
        champion->Direction > 3 || champion->MaximumHealth <= 0 ||
        champion->MaximumStamina <= 0 || champion->MaximumMana < 0) {
        return 0;
    }
    receipt.bar_heights[0] = ceil_bar_height(champion->CurrentHealth,
                                               champion->MaximumHealth, 0);
    receipt.bar_heights[1] = ceil_bar_height(champion->CurrentStamina,
                                               champion->MaximumStamina, 0);
    receipt.bar_heights[2] = ceil_bar_height(champion->CurrentMana,
                                               champion->MaximumMana, 1);
    if (receipt.bar_heights[0] < 0 || receipt.bar_heights[1] < 0 ||
        receipt.bar_heights[2] < 0) return 0;
    for (index = 0; index < profile->champion_count; ++index) {
        const CSB_V1_Champion *candidate = &profile->party_state.Champions[index];
        if (candidate->Fingerprint == 0 || candidate->Cell > 3 ||
            candidate->Direction > 3) return 0;
        if (candidate->Cell == cell_query && candidate->CurrentHealth > 0) {
            receipt.index_in_cell = index;
            break;
        }
    }
    receipt.valid = 1;
    receipt.champion_index = champion_index;
    receipt.party_direction_before = profile->party_dir;
    receipt.requested_direction = requested_direction;
    receipt.direction_delta = (requested_direction - profile->party_dir + 4) & 3;
    receipt.cell_query = cell_query;
    receipt.f0286_runtime_owner_required = 1;
    receipt.value_y = value_y;
    receipt.value_current = value_current;
    receipt.value_maximum = value_maximum;
    receipt.source_evidence =
        "ReDMCSB CHAMPION.C F0284-F0289: PC34 champion/panel receipt; "
        "F0286 ordered-cell owner and all mutation/draw calls excluded";
    *out_receipt = receipt;
    return 1;
}
