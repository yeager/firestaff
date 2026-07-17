#include <stdio.h>
#include <string.h>

#include "theron_v1_runtime_admission.h"

static void fixture(Theron_StartupMediaStateReceipt *media,
                    Theron_V1Track02CaptureCampaignReceipt *campaign,
                    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *window)
{
    memset(media, 0, sizeof(*media)); memset(campaign, 0, sizeof(*campaign)); memset(window, 0, sizeof(*window));
    media->startup_media_ready = 1; media->track02_variant = THERON_TRACK02_VARIANT_US_BIN; strcpy(media->track02_md5, THERON_TRACK02_MD5_US_BIN);
    campaign->valid = 1; campaign->independent_bundles_verified = 1; campaign->shared_track02_provenance_verified = 1; campaign->shared_loader_provenance_verified = 1;
    campaign->track02_variant = THERON_TRACK02_VARIANT_US_BIN; strcpy(campaign->track02_md5, THERON_TRACK02_MD5_US_BIN);
    campaign->route_destination_identity[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] = 0x11223344u;
    window->valid = 1; window->opaque_route_ready = 1; window->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    strcpy(window->track02_md5, THERON_TRACK02_MD5_US_BIN); window->dungeon_record_window_checksum = 0x11223344u;
}

int main(void)
{
    Theron_StartupMediaStateReceipt media; Theron_V1Track02CaptureCampaignReceipt campaign;
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt window;
    Theron_V1RuntimeTrack02CaptureCampaignAdmissionReceipt receipt;
    fixture(&media, &campaign, &window);
    if (!theron_v1_runtime_bind_track02_capture_campaign_admission(&media, &campaign, &window, &receipt) ||
        !receipt.startup_capture_ready || !receipt.soul_room_capture_ready || !receipt.dungeon_capture_ready ||
        receipt.render_allowed || receipt.pixel_decode_allowed || receipt.level_object_semantics_allowed || receipt.fallback_visuals_allowed) return 1;
    campaign.route_destination_identity[2]++;
    if (theron_v1_runtime_bind_track02_capture_campaign_admission(&media, &campaign, &window, &receipt)) return 2;
    fixture(&media, &campaign, &window); campaign.valid = 0;
    if (theron_v1_runtime_bind_track02_capture_campaign_admission(&media, &campaign, &window, &receipt)) return 3;
    fixture(&media, &campaign, &window); media.track02_md5[0] = '0';
    if (theron_v1_runtime_bind_track02_capture_campaign_admission(&media, &campaign, &window, &receipt)) return 4;
    puts("test_theron_v1_runtime_track02_capture_campaign_admission: PASS"); return 0;
}
