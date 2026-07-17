#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_capture_campaign.h"

static void bundle(Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *out, const char *md5)
{
    size_t i;
    memset(out, 0, sizeof(*out)); out->status = THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY;
    out->bundle_md5_verified = 1; out->mednafen_trace_md5_verified = 1; out->complete_route_set_consumed = out->opaque_envelope_verified = 1;
    out->opaque_runtime_ready = 1; out->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    strcpy(out->track02_md5, THERON_TRACK02_MD5_US_BIN); strcpy(out->bundle_md5, md5);
    strcpy(out->mednafen_trace_md5, "1234567890abcdef1234567890abcdef");
    for (i = 0u; i < 3u; ++i) {
        out->cd_read_record[i] = 0x4e0u; out->loader_output_identity[i] = 0x1000u + (uint32_t)i;
        out->palette_output_identity[i] = 0x2000u + (uint32_t)i; out->bitmap_transfer_identity[i] = 0x3000u + (uint32_t)i;
        out->destination_record[i] = 0xb52u; out->destination_identity[i] = 0x4000u + (uint32_t)i;
    }
}

int main(void)
{
    Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt start, soul, dungeon;
    Theron_V1Track02CaptureCampaignRouteInput inputs[3]; Theron_V1Track02CaptureCampaignReceipt receipt;
    bundle(&start, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"); bundle(&soul, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"); bundle(&dungeon, "cccccccccccccccccccccccccccccccc");
    inputs[0] = (Theron_V1Track02CaptureCampaignRouteInput){THERON_V1_TRACK02_CAPTURE_TARGET_START, &start};
    inputs[1] = (Theron_V1Track02CaptureCampaignRouteInput){THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM, &soul};
    inputs[2] = (Theron_V1Track02CaptureCampaignRouteInput){THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF, &dungeon};
    start.campaign_route = inputs[0].route; soul.campaign_route = inputs[1].route; dungeon.campaign_route = inputs[2].route;
    if (!theron_v1_track02_capture_campaign_verify(inputs, 3u, &receipt) || !receipt.valid || !receipt.independent_bundles_verified || receipt.render_allowed) return 1;
    inputs[1].route = THERON_V1_TRACK02_CAPTURE_TARGET_START;
    if (theron_v1_track02_capture_campaign_verify(inputs, 3u, &receipt)) return 2;
    inputs[1].route = THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM; strcpy(soul.bundle_md5, start.bundle_md5);
    if (theron_v1_track02_capture_campaign_verify(inputs, 3u, &receipt)) return 3;
    strcpy(soul.bundle_md5, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"); dungeon.palette_output_identity[1]++;
    if (theron_v1_track02_capture_campaign_verify(inputs, 3u, &receipt)) return 4;
    puts("test_theron_v1_track02_capture_campaign: PASS"); return 0;
}
