#include "csb_v1_f0806_f0825_startup_source_admission_pc34_compat.h"

#include <string.h>

static int csb_v1_f0806_f0825_package_is_complete(
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package)
{
    return package && package->valid && package->real_package_matched &&
        package->c001_title_consumed && package->c001_presents_consumed &&
        package->c001_chaos_consumed && package->c001_strikes_back_consumed &&
        package->c002_left_door_consumed && package->c003_right_door_consumed &&
        package->c004_entrance_consumed && package->c017_hud_consumed &&
        package->c040_hud_consumed && package->title_to_entrance_same_session &&
        package->title_to_hud_same_session && package->no_legacy_wrappers &&
        package->no_fallback_routes && package->real_asset_receipt_hash != 0u;
}

static const char *csb_v1_f0806_f0825_evidence(uint16_t function_id)
{
    switch (function_id) {
    case 806u:
        return "ReDMCSB ENTRANCE.C F0806: C001-C005/C017/C040 startup session";
    case 807u:
        return "ReDMCSB ENTRANCE.C F0807: verified C004/C002/C003 door step";
    case 808u:
    case 809u:
    case 810u:
    case 811u:
    case 812u:
    case 813u:
    case 814u:
    case 815u:
        return "ReDMCSB IO.C F0808-F0815: no authenticated CSB disk or MIDI package route";
    case 816u:
    case 817u:
    case 818u:
    case 819u:
        return "ReDMCSB STRING/TEXT F0816-F0819: no authenticated CSB text/font package route";
    default:
        return NULL;
    }
}

int csb_v1_f0806_f0825_startup_source_admit_pc34(
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package,
    const CSB_V1_F0806_EntranceLoopReceipt_PC34 *entrance,
    const CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 *door,
    uint16_t function_id,
    CSB_V1_F0806F0825StartupSourceReceiptPc34 *out)
{
    CSB_V1_F0806F0825StartupSourceReceiptPc34 receipt;
    const char *evidence;

    if (!out) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;

    evidence = csb_v1_f0806_f0825_evidence(function_id);
    if (!evidence || !csb_v1_f0806_f0825_package_is_complete(package)) {
        return 0;
    }
    if (function_id == 806u) {
        if (!entrance || !entrance->valid || !entrance->opening_material_consumed ||
            !entrance->no_synthetic_input ||
            !entrance->no_synthetic_graphics_bytes ||
            !entrance->no_fallback_visuals ||
            entrance->real_asset_receipt_hash != package->real_asset_receipt_hash ||
            entrance->session_generation != package->session_generation) {
            return 0;
        }
    } else if (function_id == 807u) {
        if (!door || !door->valid || !door->animation_step_bound ||
            !door->target_screen_bound || !door->bitplanes_consumed ||
            !door->runtime_coupling_consumed || !door->draw_consumes_receipt_only ||
            !door->no_synthetic_visuals ||
            door->accepted_animation_step_index <
                CSB_V1_F0807_ENTRANCE_DOOR_STEP_FIRST_PC34 ||
            door->accepted_animation_step_index >
                CSB_V1_F0807_ENTRANCE_DOOR_STEP_LAST_PC34) {
            return 0;
        }
    } else {
        return 0;
    }

    receipt.valid = 1;
    receipt.function_id = function_id;
    receipt.authentic_package_consumed = 1;
    receipt.existing_runtime_owner_required = 1;
    receipt.runtime_execution_blocked = 1;
    receipt.source_tick = package->source_tick;
    receipt.session_generation = package->session_generation;
    receipt.package_receipt_hash = package->real_asset_receipt_hash;
    receipt.source_evidence = evidence;
    *out = receipt;
    return 1;
}
