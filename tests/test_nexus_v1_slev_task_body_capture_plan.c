#include "nexus_v1_slev_task_body_capture_plan.h"

#include <stdio.h>
#include <string.h>

static Nexus_V1_LevelScriptCaptureTargetReceipt targets[16];
static Nexus_V1_LevelScriptTraceAdmissionReceipt traces[16];
static Nexus_V1_SlevDispatchEvidenceReceipt evidence[16];

static void set_fixture(Nexus_V1_SlevTaskBodyCapturePlanInput *input)
{
    uint32_t i;
    memset(input, 0, sizeof(*input));
    memset(targets, 0, sizeof(targets));
    memset(traces, 0, sizeof(traces));
    memset(evidence, 0, sizeof(evidence));
    for (i = 0; i < 16; ++i) {
        targets[i].valid = targets[i].original_saturn_execution_required =
            targets[i].no_dispatch_only = 1;
        targets[i].level_index = (int)i; targets[i].source_fnv1a64 = 0x1000U + i;
        targets[i].task_header_size = 36; targets[i].primary_literal_address = 1;
        targets[i].auxiliary_literal_address = 2;
        traces[i].status = NEXUS_V1_SLEV_TRACE_ADMITTED_OPAQUE;
        traces[i].level_index = (int)i; traces[i].capture_target_bound =
            traces[i].mednafen_debugger_provenance = traces[i].original_saturn_execution_claimed =
            traces[i].trace_sha256_present = traces[i].raw_trace_bytes_bound = traces[i].trace_chain_complete =
            traces[i].blocks_real_script_dispatch = 1;
        traces[i].source_fnv1a64 = targets[i].source_fnv1a64;
        traces[i].raw_trace_fnv1a64 = 0x2000U + i; traces[i].raw_trace_byte_count = 100 + i;
        traces[i].entry_pc = 0x10000U + i; traces[i].task_body_pc = 0x11000U + i;
        traces[i].task_body_opcode = 0x2000U + i; traces[i].callback_or_write_pc = 0x12000U + i;
        traces[i].callback_or_write_is_write = (int)(i & 1U);
        evidence[i].status = NEXUS_V1_SLEV_DISPATCH_EVIDENCE_OBSERVED;
        evidence[i].level_index = (int)i; evidence[i].raw_trace_bound = evidence[i].entry_observed =
            evidence[i].task_body_observed = evidence[i].callback_or_write_observed =
            evidence[i].observation_order_proven = evidence[i].primary_literal_observed =
            evidence[i].auxiliary_literal_observed = evidence[i].literal_observation_proven =
            evidence[i].blocks_real_script_dispatch = 1;
        evidence[i].raw_trace_fnv1a64 = traces[i].raw_trace_fnv1a64;
        evidence[i].raw_trace_byte_count = traces[i].raw_trace_byte_count;
        input->rows[i].target = &targets[i]; input->rows[i].trace = &traces[i];
        input->rows[i].evidence = &evidence[i];
        input->rows[i].location.entry_pc = traces[i].entry_pc;
        input->rows[i].location.task_body_pc = traces[i].task_body_pc;
        input->rows[i].location.task_body_opcode = traces[i].task_body_opcode;
        input->rows[i].location.callback_or_write_pc = traces[i].callback_or_write_pc;
        input->rows[i].location.callback_or_write_is_write = traces[i].callback_or_write_is_write;
        input->rows[i].location.raw_trace_fnv1a64 = traces[i].raw_trace_fnv1a64;
        input->rows[i].location.raw_trace_byte_count = traces[i].raw_trace_byte_count;
        input->rows[i].location.task_body_opcode_known_opaque = 1;
        input->rows[i].location.callback_owner_known_opaque = 1;
    }
}

static int rejects(Nexus_V1_SlevTaskBodyCapturePlanInput *input)
{
    Nexus_V1_SlevTaskBodyCapturePlan plan;
    return nexus_v1_slev_task_body_capture_plan_build(input, &plan) || plan.valid ||
        !plan.no_dispatch_only || plan.fallback_script_permitted;
}

int main(void)
{
    Nexus_V1_SlevTaskBodyCapturePlanInput input;
    Nexus_V1_SlevTaskBodyCapturePlan plan;
    set_fixture(&input);
    if (!nexus_v1_slev_task_body_capture_plan_build(&input, &plan) || !plan.valid ||
        !plan.targets[15].valid || plan.targets[15].level_index != 15 ||
        !plan.targets[0].source_order_required || !plan.targets[0].no_dispatch_only ||
        plan.targets[0].fallback_script_permitted) return 1;
    input.rows[3].location.task_body_opcode_known_opaque = 0;
    if (rejects(&input)) return 1;
    set_fixture(&input); input.rows[3].location.callback_owner_known_opaque = 0;
    if (rejects(&input)) return 1;
    set_fixture(&input); evidence[3].observation_order_proven = 0;
    if (rejects(&input)) return 1;
    set_fixture(&input); input.rows[3].location.raw_trace_fnv1a64++;
    if (rejects(&input)) return 1;
    set_fixture(&input); traces[3].task_body_pc++;
    if (rejects(&input)) return 1;
    set_fixture(&input); targets[3].level_index = 4;
    if (rejects(&input)) return 1;
    puts("slev task body capture plan: PASS");
    return 0;
}
