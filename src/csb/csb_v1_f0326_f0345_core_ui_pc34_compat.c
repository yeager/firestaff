#include "csb_v1_f0326_f0345_core_ui_pc34_compat.h"

#include <string.h>

static uint32_t fnv1a32(const uint8_t *bytes, int size)
{
    uint32_t value = 2166136261u;
    int i;
    for (i = 0; i < size; ++i) {
        value ^= bytes[i];
        value *= 16777619u;
    }
    return value;
}

static int material_ready(const CSB_V1_RuntimeProfile *profile,
                          const CSB_V1_CSBGraphicsStartupPackage *package)
{
    return profile && package && profile->party_state_valid &&
        profile->party_state.ImportedFromDM1 && profile->champion_count > 0 &&
        profile->champion_count == profile->party_state.ChampionCount &&
        profile->dungeon_handle && profile->dungeon_handle->raw_data &&
        profile->dungeon_handle->square_bytes == 1 && profile->graphics_asset.path &&
        package->hud_ready &&
        csb_v1_csbgraphics_startup_package_surface_draw_eligible(
            package, CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY) &&
        strcmp(profile->graphics_asset.path, package->palette_source.source_path) == 0;
}

int csb_v1_f0326_f0345_core_ui_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    uint16_t thing, CSB_V1_F0326F0345CoreUiReceiptPc34 *out_receipt)
{
    CSB_V1_F0326F0345CoreUiReceiptPc34 receipt;
    const uint8_t *record;
    int thing_type;
    int thing_index;
    int record_size;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out_receipt = receipt;
    if (!material_ready(profile, package)) return 0;
    record = csb_v1_dungeon_get_thing_record(profile->dungeon_handle, thing,
                                              &thing_type, &thing_index,
                                              &record_size);
    if (!record || record_size < 4 || thing_type <= CSB_V1_THING_TYPE_GROUP ||
        thing_type >= 14) return 0;
    receipt.valid = 1;
    receipt.thing = thing;
    receipt.thing_type = thing_type;
    receipt.thing_index = thing_index;
    receipt.thing_record_fnv1a = fnv1a32(record, record_size);
    receipt.source_bound_mask = CSB_V1_F0326_F0345_SOURCE_MASK;
    receipt.projectile_runtime_owner_required = 1;
    receipt.time_effect_runtime_owner_required = 1;
    receipt.inventory_runtime_owner_required = 1;
    receipt.panel_material_bound = 1;
    receipt.source_evidence =
        "ReDMCSB CHAMPION.C F0326-F0331 and INVNTORY.C F0332-F0345 raw PC34 admission; no runtime side effect";
    *out_receipt = receipt;
    return 1;
}
