#include "theron_v1_track02_trace_bundle_discovery.h"
#include <string.h>
int theron_v1_track02_trace_bundle_select(const Theron_V1Track02MednafenTraceConvertReceipt *c, unsigned int n, unsigned int v, const Theron_V1Track02LaunchTraceIdentityReceipt *id, const Theron_V1Track02CaptureTargetPlan *plan, Theron_V1Track02TraceBundleReceipt *out) {
 Theron_V1Track02TraceBundleReceipt r={0}; unsigned int i; uint32_t plan_identity; const Theron_V1Track02MednafenTraceConvertReceipt *one=0;
 if(!out)return 0; *out=r; r.virtual_candidate_count=v; plan_identity=theron_v1_track02_capture_target_plan_identity(plan); if(!c||!n||!id||!id->valid||!plan_identity)return 1;
 if(v){r.status=THERON_V1_TRACK02_TRACE_BUNDLE_REJECTED;*out=r;return 1;}
 for(i=0;i<n;i++) if(c[i].status==THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED && c[i].source_trace_md5_verified && c[i].huc6280_event_log_md5_verified && !c[i].emulator_launched && !c[i].media_copied && !c[i].synthetic_event_created){++r.direct_candidate_count;one=&c[i];}
 if(r.direct_candidate_count!=1u||strcmp(one->source_trace_md5,id->source_trace_md5)||strcmp(one->event_log_md5,id->event_log_md5)){r.status=THERON_V1_TRACK02_TRACE_BUNDLE_REJECTED;*out=r;return 1;}
 r.status=THERON_V1_TRACK02_TRACE_BUNDLE_READY;r.source_md5_verified=r.event_log_md5_verified=r.opaque_only=1;r.campaign_layout_epoch=id->campaign_layout_epoch;r.capture_target_plan_identity=plan_identity;r.trace=*one;*out=r;return 1;
}
