#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_capture_campaign.h"

static int bundle_complete(const Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *bundle)
{
    return bundle && bundle->status == THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY &&
        bundle->bundle_md5_verified && bundle->mednafen_trace_md5_verified &&
        bundle->complete_route_set_consumed && bundle->opaque_envelope_verified &&
        bundle->opaque_runtime_ready &&
        !bundle->pixel_decode_allowed && !bundle->level_object_semantics_allowed &&
        !bundle->render_allowed && !bundle->fallback_visuals_allowed;
}

int theron_v1_track02_capture_campaign_verify(
    const Theron_V1Track02CaptureCampaignRouteInput *inputs,
    size_t input_count,
    Theron_V1Track02CaptureCampaignReceipt *out)
{
    Theron_V1Track02CaptureCampaignReceipt receipt = {0};
    const Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *first;
    size_t i, route;
    if (!out) return 0;
    *out = receipt;
    if (!inputs || input_count != THERON_V1_TRACK02_CAPTURE_TARGET_COUNT ||
        inputs[0].route != THERON_V1_TRACK02_CAPTURE_TARGET_START ||
        inputs[1].route != THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM ||
        inputs[2].route != THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF ||
        !bundle_complete(inputs[0].bundle) || !bundle_complete(inputs[1].bundle) || !bundle_complete(inputs[2].bundle)) return 0;
    first = inputs[0].bundle;
    for (i = 0u; i < input_count; ++i) {
        const Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *bundle = inputs[i].bundle;
        if (bundle->campaign_route != inputs[i].route || bundle->track02_variant != first->track02_variant || strcmp(bundle->track02_md5, first->track02_md5) ||
            strcmp(bundle->mednafen_trace_md5, first->mednafen_trace_md5) || !bundle->bundle_md5[0] ||
            (i && strcmp(bundle->bundle_md5, inputs[0].bundle->bundle_md5) == 0) ||
            (i == 2u && strcmp(bundle->bundle_md5, inputs[1].bundle->bundle_md5) == 0)) return 0;
        for (route = 0u; route < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++route) {
            if (bundle->cd_read_record[route] != first->cd_read_record[route] ||
                bundle->loader_output_identity[route] != first->loader_output_identity[route] ||
                bundle->palette_output_identity[route] != first->palette_output_identity[route] ||
                bundle->bitmap_transfer_identity[route] != first->bitmap_transfer_identity[route] ||
                bundle->destination_record[route] != first->destination_record[route] ||
                bundle->destination_identity[route] != first->destination_identity[route]) return 0;
        }
    }
    receipt.valid = 1; receipt.independent_bundles_verified = 1;
    receipt.shared_track02_provenance_verified = 1; receipt.shared_loader_provenance_verified = 1;
    receipt.track02_variant = first->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", first->track02_md5);
    snprintf(receipt.mednafen_trace_md5, sizeof(receipt.mednafen_trace_md5), "%s", first->mednafen_trace_md5);
    for (i = 0u; i < input_count; ++i) {
        snprintf(receipt.bundle_md5[i], sizeof(receipt.bundle_md5[i]), "%s", inputs[i].bundle->bundle_md5);
        receipt.route_destination_identity[i] = first->destination_identity[i];
    }
    *out = receipt;
    return 1;
}
