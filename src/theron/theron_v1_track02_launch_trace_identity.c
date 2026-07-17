#include "theron_v1_track02_launch_trace_identity.h"

#include <stdio.h>
#include <string.h>

int theron_v1_track02_launch_trace_identity_bind(
    const Theron_V1Track02LiveLoaderRouteAdmissionReceipt *live,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    uint32_t epoch, Theron_V1Track02LaunchTraceIdentityReceipt *out) {
    Theron_V1Track02LaunchTraceIdentityReceipt r = {0};
    if (!out) return 0;
    *out = r;
    if (!live || !replay || !epoch || !live->valid || !live->dynamic_cd_read_ownership_consumed ||
        !live->huc6280_event_log_consumed || !live->manifest_bound ||
        !live->opaque_runtime_route_ready || !live->source_trace_md5[0] ||
        !live->huc6280_event_log_md5[0] || !replay->active ||
        !replay->direct_campaign_layout_consumed || !replay->dynamic_cd_read_records_consumed ||
        replay->campaign_layout_epoch != epoch || live->track02_variant != replay->track02_variant ||
        strcmp(live->track02_md5, replay->track02_md5) ||
        !replay->last_track02_record || !replay->last_raw_sector ||
        live->level_object_semantics_allowed || live->bitmap_palette_admission_allowed ||
        live->pixel_decode_allowed || live->dungeon_draw_allowed || live->fallback_visuals_allowed ||
        replay->level_object_semantics_allowed || replay->bitmap_palette_admission_allowed ||
        replay->pixel_decode_allowed || replay->dungeon_draw_allowed || replay->fallback_visuals_allowed) return 0;
    r.valid = r.direct_campaign_consumed = r.loader_trace_consumed = r.event_log_consumed = 1;
    r.track02_variant = live->track02_variant;
    snprintf(r.track02_md5, sizeof(r.track02_md5), "%s", live->track02_md5);
    r.campaign_layout_epoch = epoch;
    snprintf(r.source_trace_md5, sizeof(r.source_trace_md5), "%s", live->source_trace_md5);
    snprintf(r.event_log_md5, sizeof(r.event_log_md5), "%s", live->huc6280_event_log_md5);
    r.final_track02_record = replay->last_track02_record;
    r.final_raw_sector = replay->last_raw_sector;
    *out = r;
    return 1;
}
