#ifndef THERON_V1_TRACK02_TRACE_BUNDLE_DISCOVERY_H
#define THERON_V1_TRACK02_TRACE_BUNDLE_DISCOVERY_H

#include "theron_v1_track02_launch_trace_identity.h"
#include "theron_v1_track02_mednafen_trace_converter.h"
#include "theron_v1_track02_capture_target_plan.h"

typedef enum { THERON_V1_TRACK02_TRACE_BUNDLE_UNAVAILABLE = 0, THERON_V1_TRACK02_TRACE_BUNDLE_REJECTED, THERON_V1_TRACK02_TRACE_BUNDLE_READY } Theron_V1Track02TraceBundleStatus;
typedef struct {
    Theron_V1Track02TraceBundleStatus status;
    unsigned int direct_candidate_count;
    unsigned int virtual_candidate_count;
    int source_md5_verified;
    int event_log_md5_verified;
    int opaque_only;
    uint32_t campaign_layout_epoch;
    uint32_t capture_target_plan_identity;
    Theron_V1Track02MednafenTraceConvertReceipt trace;
} Theron_V1Track02TraceBundleReceipt;
int theron_v1_track02_trace_bundle_select(const Theron_V1Track02MednafenTraceConvertReceipt *candidates, unsigned int candidate_count, unsigned int virtual_candidate_count, const Theron_V1Track02LaunchTraceIdentityReceipt *identity, const Theron_V1Track02CaptureTargetPlan *plan, Theron_V1Track02TraceBundleReceipt *out);
#endif
