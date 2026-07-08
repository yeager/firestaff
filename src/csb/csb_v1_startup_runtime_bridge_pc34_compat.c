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
