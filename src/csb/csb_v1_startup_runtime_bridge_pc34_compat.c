#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_boot.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

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

int csb_v1_runtime_util_apply_point_with_state_from_boot_profile_facts_pc34(
    int selected_action_index,
    int imported_champion_count,
    const void *boot_profile,
    int x,
    int y,
    int import_available,
    int credits_active,
    int opening_active,
    int preview_active,
    CSB_V1_UtilApplyReceipt *out_receipt,
    CSB_V1_UtilStateReceipt *out_state_receipt)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return csb_v1_util_flow_apply_point_with_state_from_runtime_profile_facts(
        selected_action_index,
        imported_champion_count,
        profile ? &profile->runtime : NULL,
        x,
        y,
        import_available,
        credits_active,
        opening_active,
        preview_active,
        out_receipt,
        out_state_receipt);
}

int csb_v1_runtime_util_apply_firestaff_input_with_state_from_boot_profile_facts_pc34(
    int selected_action_index,
    int imported_champion_count,
    const void *boot_profile,
    int menu_input,
    int import_available,
    int credits_active,
    int opening_active,
    int preview_active,
    CSB_V1_UtilApplyReceipt *out_receipt,
    CSB_V1_UtilStateReceipt *out_state_receipt)
{
    const CSB_V1_BootProfile *profile =
        (const CSB_V1_BootProfile *)boot_profile;

    return csb_v1_util_flow_apply_firestaff_input_with_state_from_runtime_profile_facts(
        selected_action_index,
        imported_champion_count,
        profile ? &profile->runtime : NULL,
        menu_input,
        import_available,
        credits_active,
        opening_active,
        preview_active,
        out_receipt,
        out_state_receipt);
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
