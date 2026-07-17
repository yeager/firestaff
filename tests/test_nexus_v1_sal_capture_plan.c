#include "nexus_v1_sal_capture_plan.h"

#include <stdio.h>
#include <string.h>

static Nexus_V1_LevelSoundRouteReceipt routes[3];
static Nexus_V1_LevelSoundCaptureTargetReceipt targets[3];
static Nexus_V1_LevelSoundTraceAdmissionReceipt traces[3];
static Nexus_V1_SalDispatchEvidenceReceipt evidence[3];
static Nexus_V1_SalCapturePlanInputRow rows[3];

static void set_fixture(void)
{
    int i;
    memset(routes, 0, sizeof(routes)); memset(targets, 0, sizeof(targets));
    memset(traces, 0, sizeof(traces)); memset(evidence, 0, sizeof(evidence)); memset(rows, 0, sizeof(rows));
    for (i = 0; i < 3; ++i) {
        routes[i].status = NEXUS_V1_LEVEL_SOUND_ROUTE_BOUND_OPAQUE;
        routes[i].level_index = i == 2 ? 1 : 0; routes[i].raw_map_selector = 0x40 + i;
        routes[i].map_attribute = 2; routes[i].sal_offset = 0x100 + i; routes[i].sal_size = 0x20;
        routes[i].canonical_sal_source_verified = routes[i].canonical_map_source_verified =
            routes[i].canonical_sound_driver_source_verified = routes[i].map_window_unique_and_bounded =
            routes[i].blocks_real_sfx_playback = 1;
        targets[i].valid = targets[i].original_saturn_driver_capture_required = targets[i].no_playback_only = 1;
        targets[i].level_index = routes[i].level_index; targets[i].raw_map_selector = routes[i].raw_map_selector;
        targets[i].map_attribute = routes[i].map_attribute; targets[i].sal_offset = routes[i].sal_offset; targets[i].sal_size = routes[i].sal_size;
        targets[i].canonical_sal_fnv1a64 = 0x1000 + i; targets[i].canonical_map_fnv1a64 = 0x2000 + i;
        strcpy(targets[i].canonical_driver_md5, "9a2bfe6df8b4a69077054ca2dbf78cb4");
        traces[i].status = NEXUS_V1_SAL_TRACE_ADMITTED_OPAQUE; traces[i].level_index = targets[i].level_index;
        traces[i].raw_map_selector = targets[i].raw_map_selector; traces[i].map_attribute = targets[i].map_attribute;
        traces[i].sal_offset = targets[i].sal_offset; traces[i].sal_size = targets[i].sal_size;
        traces[i].canonical_sal_fnv1a64 = targets[i].canonical_sal_fnv1a64; traces[i].canonical_map_fnv1a64 = targets[i].canonical_map_fnv1a64;
        traces[i].capture_target_bound = traces[i].mednafen_debugger_provenance = traces[i].original_saturn_execution_claimed =
            traces[i].trace_sha256_present = traces[i].raw_trace_bytes_bound = traces[i].trace_chain_complete = traces[i].blocks_real_sfx_playback = 1;
        traces[i].raw_trace_fnv1a64 = 0x3000 + i; traces[i].raw_trace_byte_count = 100 + i;
        traces[i].selector_dispatch_pc = 1; traces[i].sal_read_pc = 2; traces[i].driver_output_pc = 3;
        evidence[i].status = NEXUS_V1_SAL_DISPATCH_EVIDENCE_OBSERVED; evidence[i].level_index = traces[i].level_index;
        evidence[i].raw_trace_bound = evidence[i].selector_dispatch_observed = evidence[i].sal_read_observed =
            evidence[i].driver_output_observed = evidence[i].observation_order_proven = evidence[i].blocks_real_sfx_playback = 1;
        evidence[i].raw_trace_fnv1a64 = traces[i].raw_trace_fnv1a64; evidence[i].raw_trace_byte_count = traces[i].raw_trace_byte_count;
        rows[i].route = &routes[i]; rows[i].target = &targets[i]; rows[i].trace = &traces[i]; rows[i].evidence = &evidence[i];
    }
}

static int rejects(void) { Nexus_V1_SalCapturePlan p; Nexus_V1_SalCapturePlanInput in = { rows, 3 }; return nexus_v1_sal_capture_plan_build(&in, &p) || p.valid || !p.no_playback_only || p.fallback_script_permitted; }
int main(void)
{
    Nexus_V1_SalCapturePlan plan; Nexus_V1_SalCapturePlanInput input = { rows, 3 };
    set_fixture();
    if (!nexus_v1_sal_capture_plan_build(&input, &plan) || !plan.valid || plan.target_count != 3 ||
        !plan.targets[2].no_playback_only || plan.targets[2].fallback_script_permitted) return 1;
    routes[1].map_window_unique_and_bounded = 0; if (rejects()) return 1;
    set_fixture(); targets[1].raw_map_selector = targets[0].raw_map_selector; if (rejects()) return 1;
    set_fixture(); traces[1].canonical_sal_fnv1a64++; if (rejects()) return 1;
    set_fixture(); evidence[1].observation_order_proven = 0; if (rejects()) return 1;
    set_fixture(); traces[1].sal_decode_proven = 1; if (rejects()) return 1;
    puts("sal capture plan: PASS"); return 0;
}
