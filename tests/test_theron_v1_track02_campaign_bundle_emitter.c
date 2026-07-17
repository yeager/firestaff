#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_campaign_bundle_emitter.h"

static void plan_fixture(Theron_V1Track02CaptureTargetPlan *plan)
{
    size_t i;
    memset(plan, 0, sizeof(*plan)); plan->valid = 1; plan->cue_track_consumed = 1; plan->cd_read_chain_consumed = 1;
    plan->loader_output_consumed = 1; plan->palette_output_consumed = 1; plan->bitmap_transfer_consumed = 1; plan->destination_record_consumed = 1;
    for (i = 0u; i < 3u; ++i) strcpy(plan->targets[i].track02_md5, THERON_TRACK02_MD5_US_BIN);
}

int main(void)
{
    Theron_V1Track02CaptureTargetPlan plan; Theron_V1Track02CampaignBundleEmitRequest request = {0};
    Theron_V1Track02CampaignBundleEmitReceipt receipt;
    plan_fixture(&plan);
    if (!theron_v1_track02_campaign_bundle_emit(NULL, &request, &receipt) || receipt.status != THERON_V1_TRACK02_CAMPAIGN_EMIT_REJECTED) return 1;
    request.media_path = "/tmp/firestaff-no-campaign-media.cue"; request.expected_track02_md5 = THERON_TRACK02_MD5_US_BIN;
    request.mednafen_trace_path = "/tmp/firestaff-no-campaign-trace.txt"; request.expected_mednafen_trace_md5 = THERON_TRACK02_MD5_US_BIN;
    request.bundle_path[0] = "/tmp/firestaff-campaign-start.bundle"; request.bundle_path[1] = "/tmp/firestaff-campaign-soul.bundle"; request.bundle_path[2] = "/tmp/firestaff-campaign-dungeon.bundle";
    request.dry_run = 1;
    if (!theron_v1_track02_campaign_bundle_emit(&plan, &request, &receipt) || receipt.status != THERON_V1_TRACK02_CAMPAIGN_EMIT_UNAVAILABLE ||
        receipt.media_copied || receipt.synthetic_capture_row_created || receipt.decoder_invoked) return 2;
    request.bundle_path[1] = request.bundle_path[0];
    if (!theron_v1_track02_campaign_bundle_emit(&plan, &request, &receipt) || receipt.status != THERON_V1_TRACK02_CAMPAIGN_EMIT_UNAVAILABLE) return 3;
    puts("test_theron_v1_track02_campaign_bundle_emitter: PASS"); return 0;
}
