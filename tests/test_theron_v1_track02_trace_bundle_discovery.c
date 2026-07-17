#include "theron_v1_track02_trace_bundle_discovery.h"

#include <stdio.h>
#include <string.h>

static void plan_fixture(Theron_V1Track02CaptureTargetPlan *plan)
{
    size_t i;

    memset(plan, 0, sizeof(*plan));
    plan->valid = 1;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        plan->targets[i].route = (Theron_V1Track02CaptureTargetRoute)i;
        plan->targets[i].track02_variant = THERON_TRACK02_VARIANT_US_BIN;
        strcpy(plan->targets[i].track02_md5,
               "f23601102138f87c33025877767ebf76");
    }
}

int main(void)
{
    Theron_V1Track02MednafenTraceConvertReceipt candidates[2] = {{0}};
    Theron_V1Track02LaunchTraceIdentityReceipt identity = {0};
    Theron_V1Track02CaptureTargetPlan plan;
    Theron_V1Track02TraceBundleReceipt receipt;

    plan_fixture(&plan);
    identity.valid = 1;
    strcpy(identity.source_trace_md5, "11111111111111111111111111111111");
    strcpy(identity.event_log_md5, "22222222222222222222222222222222");
    candidates[0].status = THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED;
    candidates[0].source_trace_md5_verified = 1;
    candidates[0].huc6280_event_log_md5_verified = 1;
    strcpy(candidates[0].source_trace_md5, identity.source_trace_md5);
    strcpy(candidates[0].event_log_md5, identity.event_log_md5);
    if (!theron_v1_track02_trace_bundle_select(candidates, 1u, 0u, &identity,
                                                &plan, &receipt) ||
        receipt.status != THERON_V1_TRACK02_TRACE_BUNDLE_READY ||
        receipt.capture_target_plan_identity !=
            theron_v1_track02_capture_target_plan_identity(&plan)) return 1;
    plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .destination_identity++;
    if (receipt.capture_target_plan_identity ==
        theron_v1_track02_capture_target_plan_identity(&plan)) return 2;
    plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .destination_identity--;
    if (!theron_v1_track02_trace_bundle_select(candidates, 1u, 1u, &identity,
                                                &plan, &receipt) ||
        receipt.status != THERON_V1_TRACK02_TRACE_BUNDLE_REJECTED) return 3;
    candidates[1] = candidates[0];
    if (!theron_v1_track02_trace_bundle_select(candidates, 2u, 0u, &identity,
                                                &plan, &receipt) ||
        receipt.status != THERON_V1_TRACK02_TRACE_BUNDLE_REJECTED) return 4;
    puts("test_theron_v1_track02_trace_bundle_discovery: PASS");
    return 0;
}
