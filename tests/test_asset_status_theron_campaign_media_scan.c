#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

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
    if (M12_AssetStatus_ScanTheronCampaignMedia(&status, "./not-theron.bin",
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
            &status, "./not-theron.bin",
            "00000000000000000000000000000000", &plan)) return 2;
    media = M12_AssetStatus_GetTheronCampaignMedia(&status);
    if (!media || media->status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_REJECTED ||
        media->candidate_path[0] != '\0' || media->track02_md5[0] != '\0' ||
        M12_AssetStatus_TheronCampaignMediaLaunchReady(&status) ||
        M12_AssetStatus_GameAvailable(&status, "theron")) return 3;
    return 0;
}

static int check_known_empty_jp_iso_is_not_launchable(void)
{
#if defined(_WIN32)
    puts("SKIP: authentic-media symlink probe requires POSIX");
    return 0;
#else
    const char *home = getenv("HOME");
    char source[1024];
    char linked[1024];
    char md5[33];
    char root[] = "theron-empty-jp-track02-XXXXXX";
    M12_AssetStatus status;
    M12_AssetStatusScanOptions options;
    const M12_AssetVersionStatus *version;
    int result = 0;

    if (!home || !home[0] ||
        snprintf(source, sizeof(source), "%s/.firestaff/data/theron/TQJP02End.iso",
                 home) >= (int)sizeof(source) ||
        access(source, R_OK) != 0 ||
        !m12_file_md5_hex(source, md5) ||
        strcmp(md5, THERON_TRACK02_MD5_JP_REV1_ISO) != 0) {
        puts("SKIP: authentic JP Rev 1 Track 02 ISO is unavailable");
        return 0;
    }
    if (!mkdtemp(root)) return 1;
    if (snprintf(linked, sizeof(linked), "%s/TQJP02End.iso", root) >=
            (int)sizeof(linked) || symlink(source, linked) != 0) {
        (void)rmdir(root);
        return 2;
    }

    memset(&options, 0, sizeof(options));
    options.honorRequestedDataDir = 1;
    if (!M12_AssetStatus_ScanWithOptions(&status, root, &options)) {
        result = 3;
    } else {
        version = M12_AssetStatus_GetFirstMatchedVersion(&status, "theron");
        if (M12_AssetStatus_GameAvailable(&status, "theron") || version) {
            fprintf(stderr,
                    "FAIL: known zero-filled JP Track 02 ISO remained launchable\n");
            result = 4;
        }
    }

    (void)unlink(linked);
    (void)rmdir(root);
    return result;
#endif
}

int main(void)
{
    int result = check_missing_media_is_diagnostic_only();
    if (result) return result;
    result = check_unknown_input_never_enters_generic_launch_scan();
    if (result) return 10 + result;
    result = check_rejected_rescan_clears_prior_diagnostic();
    if (result) return 20 + result;
    result = check_known_empty_jp_iso_is_not_launchable();
    if (result) return 30 + result;
    puts("test_asset_status_theron_campaign_media_scan: PASS");
    return 0;
}
