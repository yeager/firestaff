#include "theron_v1_track02_campaign_media_discovery.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fixture(Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
                    Theron_V1Track02CaptureTargetPlan *plan)
{
    size_t i;
    memset(media, 0, sizeof(*media));
    memset(plan, 0, sizeof(*plan));
    media->status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
    media->source = THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CONTAINER;
    media->candidate_count = 1;
    media->virtual_container = 1;
    media->no_media_extracted = 1;
    media->exact_layout_bound = 1;
    media->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(media->track02_md5, sizeof(media->track02_md5),
             "f23601102138f87c33025877767ebf76");
    plan->valid = 1;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        plan->targets[i].route = (Theron_V1Track02CaptureTargetRoute)i;
        plan->targets[i].track02_variant = media->track02_variant;
        snprintf(plan->targets[i].track02_md5,
                 sizeof(plan->targets[i].track02_md5), "%s", media->track02_md5);
    }
}

int main(void)
{
    Theron_V1Track02CampaignMediaDiscoveryReceipt media;
    Theron_V1Track02CaptureTargetPlan plan;
    Theron_V1Track02RawMediaIntakeReceipt refreshed;

    if (!theron_v1_track02_campaign_media_discover(
            "/definitely/not/a/theron-media-root",
            "f23601102138f87c33025877767ebf76", 2, &media) ||
        media.status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_UNAVAILABLE ||
        media.failure_reason != THERON_V1_TRACK02_MEDIA_REASON_PATH_UNAVAILABLE ||
        strcmp(theron_v1_track02_campaign_media_failure_reason_id(&media),
               "path_unavailable") ||
        media.candidate_count || media.no_media_extracted) return 1;
    if (!theron_v1_track02_campaign_media_discover(
            ".", "00000000000000000000000000000000", 1, &media) ||
        media.status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_REJECTED ||
        media.failure_reason != THERON_V1_TRACK02_MEDIA_REASON_EXPECTED_HASH_MISMATCH ||
        strcmp(theron_v1_track02_campaign_media_failure_reason_id(&media),
               "expected_hash_mismatch")) return 2;

    fixture(&media, &plan);
    if (!theron_v1_track02_campaign_media_bind_capture_plan(&media, &plan)) return 3;
    plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM].track02_md5[0] = '0';
    if (theron_v1_track02_campaign_media_bind_capture_plan(&media, &plan)) return 4;
    fixture(&media, &plan);
    media.ambiguous = 1;
    if (theron_v1_track02_campaign_media_bind_capture_plan(&media, &plan)) return 5;
    fixture(&media, &plan);
    media.source = THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CUE;
    media.virtual_container = media.no_media_extracted = 0;
    media.launchable_direct_media = 1;
    media.direct_media.status = THERON_V1_TRACK02_MEDIA_INTAKE_READY;
    media.direct_media.cue_consumed = media.direct_media.mode1_2352 = 1;
    media.direct_media.raw_trace_preparation_allowed = 1;
    media.direct_media.variant = media.track02_variant;
    snprintf(media.direct_media.track02_md5, sizeof(media.direct_media.track02_md5), "%s", media.track02_md5);
    snprintf(media.direct_media.media_path, sizeof(media.direct_media.media_path), "/real/theron.cue");
    snprintf(media.direct_media.payload_path, sizeof(media.direct_media.payload_path), "/real/theron.bin");
    media.direct_media.cue_index01_sector = 225u;
    media.direct_media.payload_bytes = 2352u * 0x600u;
    media.direct_media.sector_count = 0x600u;
    media.direct_media.first_user_data_offset = 225u * 2352u + 16u;
    media.direct_media.logical_user_data_window_bytes = 2048u;
    refreshed = media.direct_media;
    if (!theron_v1_track02_campaign_media_direct_layout_current(
            &media, &refreshed, &plan)) return 6;
    refreshed.cue_index01_sector++;
    if (theron_v1_track02_campaign_media_direct_layout_current(
            &media, &refreshed, &plan)) return 7;

    {
        const char *media_root = getenv("FIRESTAFF_THERON_TRACK02_MEDIA_ROOT");
        const char *expected_md5 = getenv("FIRESTAFF_THERON_TRACK02_EXPECTED_MD5");
        if (media_root && media_root[0] && expected_md5 && expected_md5[0]) {
            if (!theron_v1_track02_campaign_media_discover(
                    media_root, expected_md5, 4, &media) ||
                media.status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY ||
                media.ambiguous || media.candidate_count != 1u ||
                !media.launchable_direct_media ||
                strcmp(media.track02_md5, expected_md5)) return 8;
        }
    }

    puts("test_theron_v1_track02_campaign_media_discovery: PASS");
    return 0;
}
