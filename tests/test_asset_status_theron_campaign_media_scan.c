#include "asset_status_m12.h"

#include <stdio.h>
#include <string.h>

static int check_missing_media_is_diagnostic_only(void)
{
    M12_AssetStatus status;
    Theron_V1Track02CaptureTargetPlan plan;
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media;

    memset(&plan, 0, sizeof(plan));
    if (M12_AssetStatus_ScanTheronCampaignMedia(
            &status, "/definitely/not/a/theron-track02.cue",
            THERON_TRACK02_MD5_US_BIN, &plan)) return 1;
    media = M12_AssetStatus_GetTheronCampaignMedia(&status);
    if (!media || media->status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_UNAVAILABLE ||
        M12_AssetStatus_TheronCampaignMediaLaunchReady(&status) ||
        M12_AssetStatus_GameAvailable(&status, "theron") ||
        status.originalFileCandidateFound || media->no_media_extracted) return 2;
    return 0;
}

static int check_unknown_input_never_enters_generic_launch_scan(void)
{
    M12_AssetStatus status;
    Theron_V1Track02CaptureTargetPlan plan;
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media;

    memset(&plan, 0, sizeof(plan));
    if (M12_AssetStatus_ScanTheronCampaignMedia(&status, "/tmp/not-theron.bin",
                                                "00000000000000000000000000000000",
                                                &plan)) return 1;
    media = M12_AssetStatus_GetTheronCampaignMedia(&status);
    if (!media || media->status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_REJECTED ||
        M12_AssetStatus_TheronCampaignMediaLaunchReady(&status) ||
        M12_AssetStatus_GameAvailable(&status, "theron") ||
        M12_AssetStatus_GetTheronLaunchMediaPath(&status) != NULL) return 2;
    return 0;
}

static int check_rejected_rescan_clears_prior_diagnostic(void)
{
    M12_AssetStatus status;
    Theron_V1Track02CaptureTargetPlan plan;
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media;

    memset(&plan, 0, sizeof(plan));
    (void)M12_AssetStatus_ScanTheronCampaignMedia(
        &status, "/definitely/not/a/theron-track02.cue",
        THERON_TRACK02_MD5_US_BIN, &plan);
    media = M12_AssetStatus_GetTheronCampaignMedia(&status);
    if (!media || media->status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_UNAVAILABLE) return 1;

    if (M12_AssetStatus_ScanTheronCampaignMedia(
            &status, "/tmp/not-theron.bin",
            "00000000000000000000000000000000", &plan)) return 2;
    media = M12_AssetStatus_GetTheronCampaignMedia(&status);
    if (!media || media->status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_REJECTED ||
        media->candidate_path[0] != '\0' || media->track02_md5[0] != '\0' ||
        M12_AssetStatus_TheronCampaignMediaLaunchReady(&status) ||
        M12_AssetStatus_GameAvailable(&status, "theron")) return 3;
    return 0;
}

int main(void)
{
    int result = check_missing_media_is_diagnostic_only();
    if (result) return result;
    result = check_unknown_input_never_enters_generic_launch_scan();
    if (result) return 10 + result;
    result = check_rejected_rescan_clears_prior_diagnostic();
    if (result) return 20 + result;
    puts("test_asset_status_theron_campaign_media_scan: PASS");
    return 0;
}
