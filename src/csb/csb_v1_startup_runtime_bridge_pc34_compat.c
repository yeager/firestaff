#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_boot.h"
#include "csb_v1_startup_real_asset_receipt.h"
#include "csb_v1_startup_session_contract_pc34_compat.h"
#include "firestaff/csb/v1/startup_entrance_pointer_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

#include <string.h>

int csb_v1_runtime_startup_package_handoff_receipt_from_transition_pc34(
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_surface,
    const CSB_V1_StartupEntranceInputOutcome_PC34 *input_outcome,
    const CSB_V1_StartupRuntimeApplyReceipt_PC34 *runtime_apply,
    const CSB_V1_StartupCommandStateReceipt_PC34 *state,
    CSB_V1_RuntimeStartupPackageHandoffReceipt_PC34 *out_receipt)
{
    CSB_V1_RuntimeStartupPackageHandoffReceipt_PC34 receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!package_receipt || !host_surface || !input_outcome || !runtime_apply ||
        !state || !package_receipt->valid || !host_surface->valid) {
        return 0;
    }

    receipt.real_package_matched = package_receipt->real_package_matched &&
        package_receipt->c001_title_consumed &&
        package_receipt->c001_presents_consumed &&
        package_receipt->c001_chaos_consumed &&
        package_receipt->c001_strikes_back_consumed &&
        package_receipt->c017_hud_consumed &&
        package_receipt->c040_hud_consumed &&
        package_receipt->title_to_hud_same_session;
    receipt.same_session_generation =
        package_receipt->session_generation != 0u &&
        package_receipt->session_generation ==
            host_surface->frame.session_generation;
    receipt.no_legacy_wrappers = package_receipt->no_legacy_wrappers &&
        package_receipt->no_fallback_routes && host_surface->no_legacy_wrappers;
    receipt.no_synthetic_surface = host_surface->no_synthetic_surface;
    receipt.session_generation = package_receipt->session_generation;
    receipt.host_surface_hash = host_surface->host_surface_hash;
    receipt.real_asset_receipt_hash = package_receipt->real_asset_receipt_hash;
    receipt.consumed_surface_hash = package_receipt->consumed_surface_hash;

    if (input_outcome->result != CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 ||
        runtime_apply->result != CSB_V1_STARTUP_RUNTIME_APPLY_REDRAW_PC34) {
        return 0;
    }
    if (host_surface->host_surface ==
        CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34) {
        receipt.door_opening_transition = host_surface->door_opening_decision &&
            state->entrance_active && !state->entrance_dismissed &&
            state->opening_active && state->opening_step > 0;
    } else if (host_surface->host_surface ==
               CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34) {
        receipt.hud_runtime_transition = host_surface->runtime_hud_decision &&
            host_surface->uses_c017_inventory && host_surface->uses_c040_resurrect &&
            !state->entrance_active && state->entrance_dismissed &&
            !state->opening_active && state->pending_command == 0;
    } else {
        return 0;
    }
    receipt.input_runtime_transition_ready = receipt.door_opening_transition ||
        receipt.hud_runtime_transition;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; ENTRANCE.C F0806 lines "
        "850-903; CSBWin CSBCode.cpp OpenPrisonDoors handoff";
    receipt.valid = receipt.real_package_matched &&
        receipt.same_session_generation && receipt.no_legacy_wrappers &&
        receipt.no_synthetic_surface && receipt.input_runtime_transition_ready &&
        receipt.host_surface_hash != 0u &&
        receipt.real_asset_receipt_hash != 0u &&
        receipt.consumed_surface_hash != 0u;
    if (!receipt.valid) return 0;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_runtime_startup_title_package_handoff_receipt_pc34(
    const CSB_V1_StartupSessionPackageTitleReceipt_PC34 *title_receipt,
    const CSB_V1_RuntimeStartupPackageHandoffReceipt_PC34 *runtime_receipt,
    CSB_V1_RuntimeStartupTitlePackageHandoffReceipt_PC34 *out_receipt)
{
    CSB_V1_RuntimeStartupTitlePackageHandoffReceipt_PC34 receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!title_receipt || !runtime_receipt || !title_receipt->valid ||
        !runtime_receipt->valid) return 0;

    receipt.full_title_to_hud_package_bound =
        title_receipt->real_package_matched && title_receipt->c001_title_ready &&
        title_receipt->c001_presents_ready && title_receipt->c001_chaos_ready &&
        title_receipt->c001_strikes_back_ready &&
        title_receipt->title_to_hud_same_session &&
        runtime_receipt->real_package_matched &&
        runtime_receipt->input_runtime_transition_ready;
    receipt.same_session_generation = title_receipt->session_generation != 0u &&
        title_receipt->session_generation == runtime_receipt->session_generation;
    receipt.no_legacy_wrappers = title_receipt->no_legacy_wrappers &&
        title_receipt->no_fallback_routes && runtime_receipt->no_legacy_wrappers;
    receipt.no_synthetic_surface = runtime_receipt->no_synthetic_surface;
    receipt.session_generation = title_receipt->session_generation;
    receipt.host_surface_hash = runtime_receipt->host_surface_hash;
    receipt.real_asset_receipt_hash = title_receipt->real_asset_receipt_hash;
    receipt.consumed_surface_hash = title_receipt->consumed_surface_hash;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; ENTRANCE.C F0806 lines "
        "850-903";
    receipt.valid = receipt.full_title_to_hud_package_bound &&
        receipt.same_session_generation && receipt.no_legacy_wrappers &&
        receipt.no_synthetic_surface && receipt.host_surface_hash != 0u &&
        receipt.real_asset_receipt_hash != 0u &&
        receipt.consumed_surface_hash != 0u &&
        receipt.real_asset_receipt_hash == runtime_receipt->real_asset_receipt_hash &&
        receipt.consumed_surface_hash == runtime_receipt->consumed_surface_hash;
    if (!receipt.valid) return 0;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_runtime_startup_title_door_handoff_receipt_pc34(
    const CSB_V1_StartupSessionPackageTitleReceipt_PC34 *title_receipt,
    const CSB_V1_StartupSessionOpeningDoorReceipt_PC34 *opening_receipt,
    const CSB_V1_RuntimeStartupPackageHandoffReceipt_PC34 *runtime_receipt,
    CSB_V1_RuntimeStartupTitleDoorHandoffReceipt_PC34 *out_receipt)
{
    CSB_V1_RuntimeStartupTitleDoorHandoffReceipt_PC34 receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!title_receipt || !opening_receipt || !runtime_receipt ||
        !title_receipt->valid || !opening_receipt->valid ||
        !runtime_receipt->valid) return 0;

    receipt.full_title_to_opening_package_bound =
        title_receipt->real_package_matched && title_receipt->c001_title_ready &&
        title_receipt->c001_presents_ready && title_receipt->c001_chaos_ready &&
        title_receipt->c001_strikes_back_ready &&
        title_receipt->title_to_hud_same_session &&
        opening_receipt->real_package_matched &&
        opening_receipt->c004_entrance_ready &&
        opening_receipt->c002_left_door_ready &&
        opening_receipt->c003_right_door_ready &&
        opening_receipt->opening_to_title_same_session &&
        runtime_receipt->real_package_matched &&
        runtime_receipt->door_opening_transition &&
        runtime_receipt->input_runtime_transition_ready;
    receipt.same_session_generation = title_receipt->session_generation != 0u &&
        title_receipt->session_generation == opening_receipt->session_generation &&
        title_receipt->session_generation == runtime_receipt->session_generation;
    receipt.no_legacy_wrappers = title_receipt->no_legacy_wrappers &&
        title_receipt->no_fallback_routes && opening_receipt->no_legacy_wrappers &&
        opening_receipt->no_fallback_routes && runtime_receipt->no_legacy_wrappers;
    receipt.no_synthetic_surface = runtime_receipt->no_synthetic_surface;
    receipt.session_generation = title_receipt->session_generation;
    receipt.host_surface_hash = runtime_receipt->host_surface_hash;
    receipt.real_asset_receipt_hash = title_receipt->real_asset_receipt_hash;
    receipt.consumed_surface_hash = title_receipt->consumed_surface_hash;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; ENTRANCE.C F0806 lines "
        "775-826";
    receipt.valid = receipt.full_title_to_opening_package_bound &&
        receipt.same_session_generation && receipt.no_legacy_wrappers &&
        receipt.no_synthetic_surface && receipt.host_surface_hash != 0u &&
        receipt.real_asset_receipt_hash != 0u &&
        receipt.consumed_surface_hash != 0u &&
        receipt.real_asset_receipt_hash == opening_receipt->real_asset_receipt_hash &&
        receipt.real_asset_receipt_hash == runtime_receipt->real_asset_receipt_hash &&
        receipt.consumed_surface_hash == opening_receipt->consumed_surface_hash &&
        receipt.consumed_surface_hash == runtime_receipt->consumed_surface_hash;
    if (!receipt.valid) return 0;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_runtime_startup_title_opening_consumption_handoff_receipt_pc34(
    const CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34
        *consumption_receipt,
    const CSB_V1_RuntimeStartupPackageHandoffReceipt_PC34 *runtime_receipt,
    CSB_V1_RuntimeStartupTitleOpeningConsumptionHandoffReceipt_PC34 *out_receipt)
{
    CSB_V1_RuntimeStartupTitleOpeningConsumptionHandoffReceipt_PC34 receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!consumption_receipt || !runtime_receipt || !consumption_receipt->valid ||
        !runtime_receipt->valid) return 0;

    receipt.real_title_opening_consumption =
        consumption_receipt->real_package_matched &&
        consumption_receipt->presents_consumed &&
        consumption_receipt->chaos_consumed &&
        consumption_receipt->strikes_back_consumed &&
        consumption_receipt->c004_c002_c003_consumed &&
        runtime_receipt->real_package_matched &&
        runtime_receipt->door_opening_transition &&
        runtime_receipt->input_runtime_transition_ready;
    receipt.same_session_generation = consumption_receipt->session_generation != 0u &&
        consumption_receipt->session_generation == runtime_receipt->session_generation;
    receipt.no_legacy_wrappers = consumption_receipt->no_legacy_wrappers &&
        runtime_receipt->no_legacy_wrappers;
    receipt.no_synthetic_surface = consumption_receipt->no_synthetic_surface &&
        runtime_receipt->no_synthetic_surface;
    receipt.session_generation = consumption_receipt->session_generation;
    receipt.opening_host_surface_hash =
        consumption_receipt->opening_host_surface_hash;
    receipt.real_asset_receipt_hash = consumption_receipt->real_asset_receipt_hash;
    receipt.consumed_surface_hash = consumption_receipt->consumed_surface_hash;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; ENTRANCE.C F0806 lines "
        "775-826";
    receipt.valid = receipt.real_title_opening_consumption &&
        receipt.same_session_generation && receipt.no_legacy_wrappers &&
        receipt.no_synthetic_surface && receipt.opening_host_surface_hash != 0u &&
        receipt.real_asset_receipt_hash != 0u &&
        receipt.consumed_surface_hash != 0u &&
        receipt.opening_host_surface_hash == runtime_receipt->host_surface_hash &&
        receipt.real_asset_receipt_hash == runtime_receipt->real_asset_receipt_hash &&
        receipt.consumed_surface_hash == runtime_receipt->consumed_surface_hash;
    if (!receipt.valid) return 0;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_runtime_startup_hud_door_input_handoff_receipt_pc34(
    const CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34
        *package_receipt,
    const CSB_V1_RuntimeStartupPackageHandoffReceipt_PC34 *runtime_receipt,
    CSB_V1_RuntimeStartupHudDoorInputHandoffReceipt_PC34 *out_receipt)
{
    CSB_V1_RuntimeStartupHudDoorInputHandoffReceipt_PC34 receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!package_receipt || !runtime_receipt || !package_receipt->valid ||
        !runtime_receipt->valid) return 0;

    receipt.real_hud_door_input_consumption =
        package_receipt->real_package_matched &&
        package_receipt->c017_hud_consumed && package_receipt->c040_hud_consumed &&
        package_receipt->first_live_door_frame &&
        package_receipt->first_runtime_input &&
        runtime_receipt->real_package_matched &&
        runtime_receipt->hud_runtime_transition &&
        runtime_receipt->input_runtime_transition_ready;
    receipt.same_session_generation = package_receipt->session_generation != 0u &&
        package_receipt->session_generation == runtime_receipt->session_generation;
    receipt.no_legacy_wrappers = package_receipt->no_legacy_wrappers &&
        runtime_receipt->no_legacy_wrappers;
    receipt.no_synthetic_surface = package_receipt->no_synthetic_surface &&
        runtime_receipt->no_synthetic_surface;
    receipt.session_generation = package_receipt->session_generation;
    receipt.hud_host_surface_hash = package_receipt->hud_host_surface_hash;
    receipt.real_asset_receipt_hash = package_receipt->real_asset_receipt_hash;
    receipt.consumed_surface_hash = package_receipt->consumed_surface_hash;
    receipt.source_evidence =
        "ReDMCSB PANEL.C F0346/F0347; DUNGEON.C live door tick; "
        "COMMAND.C first input; CSBWin CSBCode.cpp OpenPrisonDoors";
    receipt.valid = receipt.real_hud_door_input_consumption &&
        receipt.same_session_generation && receipt.no_legacy_wrappers &&
        receipt.no_synthetic_surface && receipt.hud_host_surface_hash != 0u &&
        receipt.real_asset_receipt_hash != 0u &&
        receipt.consumed_surface_hash != 0u &&
        receipt.hud_host_surface_hash == runtime_receipt->host_surface_hash &&
        receipt.real_asset_receipt_hash == runtime_receipt->real_asset_receipt_hash &&
        receipt.consumed_surface_hash == runtime_receipt->consumed_surface_hash;
    if (!receipt.valid) return 0;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_runtime_apply_startup_sequence_plan_from_state_facts_with_receipts_pc34(
    CSB_V1_RuntimeProfile *profile,
    const struct CSB_V1_StartupRuntimePlan_PC34 *startup_plan,
    const char *resume_path,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_runtime_exec_receipt,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome,
    CSB_V1_StartupRuntimeApplyReceipt_PC34 *out_runtime_apply_receipt,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_state_receipt)
{
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 local_exec_receipt;
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *exec_receipt =
        out_runtime_exec_receipt ? out_runtime_exec_receipt
                                 : &local_exec_receipt;

    csb_v1_runtime_startup_runtime_plan_receipt_init_pc34(exec_receipt);
    if (out_runtime_apply_receipt) {
        csb_v1_startup_runtime_apply_receipt_init_pc34(
            out_runtime_apply_receipt);
    }
    if (out_state_receipt) {
        csb_v1_startup_command_state_receipt_init_pc34(out_state_receipt);
    }
    if (!profile || !startup_plan ||
        !csb_v1_runtime_apply_startup_sequence_plan_pc34(
            profile,
            startup_plan,
            resume_path,
            exec_receipt)) {
        return 0;
    }
    return csb_v1_startup_apply_runtime_plan_from_facts_with_receipts_pc34(
        title_active,
        title_frame,
        title_source_step,
        entrance_active,
        entrance_source_step,
        entrance_dismissed,
        credits_active,
        credits_remaining_ticks,
        opening_active,
        opening_delay_ticks,
        opening_step,
        pending_command,
        startup_plan,
        exec_receipt->resume_available,
        exec_receipt->resume_loaded,
        out_outcome,
        out_runtime_apply_receipt,
        out_state_receipt);
}

int csb_v1_runtime_apply_startup_sequence_plan_from_boot_profile_facts_with_receipts_pc34(
    void *boot_profile,
    const struct CSB_V1_StartupRuntimePlan_PC34 *startup_plan,
    const char *resume_path,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_runtime_exec_receipt,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome,
    CSB_V1_StartupRuntimeApplyReceipt_PC34 *out_runtime_apply_receipt,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_state_receipt)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    if (!profile) {
        if (out_runtime_exec_receipt) {
            csb_v1_runtime_startup_runtime_plan_receipt_init_pc34(
                out_runtime_exec_receipt);
        }
        if (out_runtime_apply_receipt) {
            csb_v1_startup_runtime_apply_receipt_init_pc34(
                out_runtime_apply_receipt);
        }
        if (out_state_receipt) {
            csb_v1_startup_command_state_receipt_init_pc34(
                out_state_receipt);
        }
        return 0;
    }

    return csb_v1_runtime_apply_startup_sequence_plan_from_state_facts_with_receipts_pc34(
        &profile->runtime,
        startup_plan,
        resume_path,
        title_active,
        title_frame,
        title_source_step,
        entrance_active,
        entrance_source_step,
        entrance_dismissed,
        credits_active,
        credits_remaining_ticks,
        opening_active,
        opening_delay_ticks,
        opening_step,
        pending_command,
        out_runtime_exec_receipt,
        out_outcome,
        out_runtime_apply_receipt,
        out_state_receipt);
}

int csb_v1_runtime_m11_mirror_receipt_from_boot_profile_pc34(
    const void *boot_profile,
    CSB_V1_RuntimeM11MirrorReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    if (!profile) {
        if (out_receipt) {
            csb_v1_runtime_m11_mirror_receipt_init_pc34(out_receipt);
        }
        return 0;
    }
    return csb_v1_runtime_m11_mirror_receipt_from_profile_pc34(
        &profile->runtime,
        out_receipt);
}

int csb_v1_runtime_util_render_plan_from_boot_profile_facts_pc34(
    int selected_action_index,
    int imported_champion_count,
    const void *boot_profile,
    const char *prompt_override,
    int preview_active,
    CSB_V1_UtilRenderPlan *out_plan)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return csb_v1_util_flow_render_plan_from_runtime_profile_facts(
        selected_action_index,
        imported_champion_count,
        profile ? &profile->runtime : NULL,
        prompt_override,
        preview_active,
        out_plan);
}

void csb_v1_runtime_util_startup_host_action_receipt_init_pc34(
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_util_flow_apply_receipt_init(&receipt->util_receipt);
    csb_v1_util_flow_state_receipt_init(&receipt->util_state_receipt);
    csb_v1_startup_entrance_host_action_receipt_init_pc34(
        &receipt->entrance_receipt);
}

int csb_v1_runtime_save_game_to_path_from_boot_profile_pc34(
    const void *boot_profile,
    const char *path,
    uint32_t *out_game_time)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;
    int result;

    if (out_game_time) {
        *out_game_time = 0U;
    }
    if (!profile || !path) {
        return -1;
    }
    result = csb_v1_runtime_save_game_to_path(&profile->runtime, path);
    if (out_game_time) {
        *out_game_time = profile->runtime.game_time;
    }
    return result;
}

int csb_v1_runtime_load_game_from_path_from_boot_profile_pc34(
    void *boot_profile,
    const char *path,
    uint32_t *out_game_time)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    int result;

    if (out_game_time) {
        *out_game_time = 0U;
    }
    if (!profile || !path) {
        return -1;
    }
    result = csb_v1_runtime_load_game_from_path(&profile->runtime, path);
    if (out_game_time) {
        *out_game_time = profile->runtime.game_time;
    }
    return result;
}

int csb_v1_runtime_tick_from_boot_profile_pc34(
    void *boot_profile,
    uint32_t *out_game_time)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    int result;

    if (out_game_time) {
        *out_game_time = 0U;
    }
    if (!profile) {
        return 0;
    }
    result = csb_v1_runtime_tick_v1(&profile->runtime);
    if (out_game_time) {
        *out_game_time = profile->runtime.game_time;
    }
    return result;
}

int csb_v1_runtime_object_icon_index_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_object_icon_index(&profile->runtime, thing)
                   : -1;
}

int csb_v1_runtime_object_action_set_index_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return profile
               ? csb_v1_runtime_object_action_set_index(&profile->runtime,
                                                        thing)
               : 0;
}

uint16_t csb_v1_runtime_object_allowed_slots_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_object_allowed_slots(&profile->runtime,
                                                         thing)
                   : 0u;
}

int csb_v1_runtime_object_name_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing,
    char *out,
    size_t out_size)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    if (!profile) {
        if (out && out_size > 0u) {
            out[0] = '\0';
        }
        return 0;
    }
    return csb_v1_runtime_object_name(&profile->runtime,
                                      thing,
                                      out,
                                      out_size);
}

int csb_v1_runtime_read_container_slots_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short container_thing,
    unsigned short out_slots[8])
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_read_container_slots(&profile->runtime,
                                                         container_thing,
                                                         out_slots)
                   : -1;
}

int csb_v1_runtime_write_container_slots_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short container_thing,
    const unsigned short slots[8])
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_write_container_slots(&profile->runtime,
                                                          container_thing,
                                                          slots)
                   : 0;
}

int csb_v1_runtime_set_thing_next_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short thing,
    unsigned short next_thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_set_thing_next(&profile->runtime,
                                                   thing,
                                                   next_thing)
                   : 0;
}

int csb_v1_runtime_write_inventory_slot_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int csb_slot,
    unsigned short thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;

    if (!profile) return 1;
    runtime = &profile->runtime;
    if (!runtime->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= runtime->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS ||
        csb_slot < 0 ||
        csb_slot >= CSB_V1_SLOT_COUNT) {
        return 0;
    }
    /* CSBWin Character.cpp::SetPossession runs EquipFilter once for the
     * removed RN (timer function 1) and once for the added RN (0), before it
     * writes the possession. Unsupported or non-authenticated DSA data is
     * deliberately not substituted; the established slot write still owns
     * its non-DSA runtime path. */
    (void)csb_v1_runtime_execute_csbwin_equip_filter(
        runtime, champion_index, csb_slot,
        runtime->party_state.Champions[champion_index].Slots[csb_slot], thing);
    runtime->party_state.Champions[champion_index].Slots[csb_slot] = thing;
    return 1;
}

int csb_v1_runtime_write_leader_hand_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;

    if (!profile) return 1;
    runtime = &profile->runtime;
    if (!runtime->party_state_valid) return 0;
    if (thing == 0xfffeu) thing = 0xffffu;
    runtime->party_state.LeaderHandThing = thing;
    if (runtime->csbwin_gameblock2_summary_valid) {
        runtime->csbwin_object_in_hand = thing;
    }
    return 1;
}

int csb_v1_runtime_throw_leader_hand_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    unsigned short leader_thing,
    unsigned short *out_restored_action_hand,
    int *out_projectile_slot)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;
    CSB_V1_Champion *champion;
    unsigned short saved_action_hand;

    if (out_restored_action_hand) *out_restored_action_hand = 0xffffu;
    if (!profile || leader_thing == 0xffffu || leader_thing == 0xfffeu) {
        return 0;
    }
    runtime = &profile->runtime;
    if (!runtime->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= runtime->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }

    champion = &runtime->party_state.Champions[champion_index];
    saved_action_hand = champion->Slots[CSB_V1_SLOT_ACTION_HAND];
    champion->Slots[CSB_V1_SLOT_ACTION_HAND] = leader_thing;
    if (!csb_v1_runtime_throw_action_hand(runtime,
                                          champion_index,
                                          out_projectile_slot)) {
        champion->Slots[CSB_V1_SLOT_ACTION_HAND] = saved_action_hand;
        if (out_restored_action_hand) {
            *out_restored_action_hand = saved_action_hand;
        }
        return 0;
    }
    champion->Slots[CSB_V1_SLOT_ACTION_HAND] = saved_action_hand;
    if (out_restored_action_hand) {
        *out_restored_action_hand = saved_action_hand;
    }
    return 1;
}

int csb_v1_runtime_write_champion_vitals_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int current_health,
    int current_stamina,
    int current_mana)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;
    CSB_V1_Champion *champion;

    if (!profile) return 1;
    runtime = &profile->runtime;
    if (!runtime->party_state_valid) return 0;
    if (champion_index < 0 ||
        champion_index >= runtime->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return 0;
    }
    champion = &runtime->party_state.Champions[champion_index];
    champion->CurrentHealth = (int16_t)current_health;
    champion->CurrentStamina = (int16_t)current_stamina;
    champion->CurrentMana = (int16_t)current_mana;
    return 1;
}

int csb_v1_runtime_throw_action_hand_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int *out_projectile_slot)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_throw_action_hand(&profile->runtime,
                                                      champion_index,
                                                      out_projectile_slot)
                   : 0;
}

int csb_v1_runtime_shoot_ready_hand_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int *out_projectile_slot)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_shoot_ready_hand(&profile->runtime,
                                                     champion_index,
                                                     out_projectile_slot)
                   : 0;
}

int csb_v1_runtime_refill_ready_hand_after_shoot_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int *out_source_slot,
    unsigned short *out_thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_refill_ready_hand_after_shoot(
                         &profile->runtime,
                         champion_index,
                         out_source_slot,
                         out_thing)
                   : 0;
}

int csb_v1_runtime_spawn_champion_projectile_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int action_index,
    int projectile_subtype,
    int projectile_category,
    int kinetic_energy,
    int attack,
    int attack_type_code,
    int step_energy,
    unsigned short associated_thing,
    int poison_attack,
    int potion_power,
    int *out_projectile_slot)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_spawn_champion_projectile(
                         &profile->runtime,
                         champion_index,
                         action_index,
                         projectile_subtype,
                         projectile_category,
                         kinetic_energy,
                         attack,
                         attack_type_code,
                         step_energy,
                         associated_thing,
                         poison_attack,
                         potion_power,
                         out_projectile_slot)
                   : 0;
}

int csb_v1_runtime_perform_melee_action_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int action_index)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;

    return profile ? csb_v1_runtime_perform_melee_action(&profile->runtime,
                                                         champion_index,
                                                         action_index,
                                                         NULL)
                   : 0;
}

int csb_v1_runtime_trigger_front_wall_ornament_click_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short leader_hand_thing,
    unsigned short *out_leader_hand_thing)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_RuntimeProfile *runtime;
    int dx = 0;
    int dy = 0;
    int queued;

    if (out_leader_hand_thing) *out_leader_hand_thing = leader_hand_thing;
    if (!profile) return 0;
    runtime = &profile->runtime;
    switch (runtime->party_dir & 3) {
        case 0: dy = -1; break;
        case 1: dx = 1; break;
        case 2: dy = 1; break;
        case 3: dx = -1; break;
        default: break;
    }
    runtime->party_state.LeaderHandThing = leader_hand_thing;
    queued = csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
        runtime,
        runtime->party_x + dx,
        runtime->party_y + dy,
        0);
    if (queued <= 0) return queued;
    if (out_leader_hand_thing) {
        *out_leader_hand_thing = runtime->party_state.LeaderHandThing;
    }
    return queued;
}
