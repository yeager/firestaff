#include "theron_v1_boot.h"

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
    media->source = THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CUE;
    media->candidate_count = 1;
    media->exact_layout_bound = 1;
    media->launchable_direct_media = 1;
    media->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(media->track02_md5, sizeof(media->track02_md5),
             "f23601102138f87c33025877767ebf76");
    snprintf(media->candidate_path, sizeof(media->candidate_path),
             "/original/TQUS-Raw.cue");
    snprintf(media->direct_media.payload_path,
             sizeof(media->direct_media.payload_path), "/original/TQUS02.bin");
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        plan->targets[i].route = (Theron_V1Track02CaptureTargetRoute)i;
        plan->targets[i].track02_variant = media->track02_variant;
        snprintf(plan->targets[i].track02_md5,
                 sizeof(plan->targets[i].track02_md5), "%s", media->track02_md5);
    }
    plan->valid = 1;
}

int main(void)
{
    Theron_V1_BootStartupLaunch launch;
    Theron_V1_BootProfile profile;
    Theron_V1Track02CampaignMediaDiscoveryReceipt media;
    Theron_V1Track02CaptureTargetPlan plan;
    Theron_V1_BootStartupLaunch runtime_launch;
    Theron_V1_BootStartupRuntimeReceipt runtime_receipt;

    memset(&launch, 0, sizeof(launch));
    memset(&profile, 0, sizeof(profile));
    launch.profile = &profile;
    fixture(&media, &plan);
    snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "%s",
             media.track02_md5);
    if (!theron_v1_boot_startup_launch_bind_campaign_media(&launch, &media, &plan) ||
        !launch.campaign_media_launchable ||
        launch.campaign_media_discovery.virtual_container) return 1;

    memset(&runtime_launch, 0, sizeof(runtime_launch));
    runtime_launch.profile = calloc(1u, sizeof(*runtime_launch.profile));
    runtime_launch.world = calloc(1u, sizeof(*runtime_launch.world));
    runtime_launch.viewport = calloc(1u, sizeof(*runtime_launch.viewport));
    runtime_launch.assets = calloc(1u, sizeof(*runtime_launch.assets));
    if (!runtime_launch.profile || !runtime_launch.world || !runtime_launch.viewport ||
        !runtime_launch.assets) return 10;
    snprintf(runtime_launch.profile->graphics_md5,
             sizeof(runtime_launch.profile->graphics_md5), "%s", media.track02_md5);
    if (!theron_v1_boot_startup_launch_bind_campaign_media(
            &runtime_launch, &media, &plan) ||
        !theron_v1_boot_startup_launch_detach_runtime(&runtime_launch, &runtime_receipt) ||
        !runtime_receipt.campaign_media_launchable ||
        strcmp(runtime_receipt.campaign_media_discovery.track02_md5,
               media.track02_md5)) return 11;
    free(runtime_receipt.profile);
    free(runtime_receipt.world);
    free(runtime_receipt.viewport);
    free(runtime_receipt.assets);

    snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "deadbeef");
    if (theron_v1_boot_startup_launch_bind_campaign_media(&launch, &media, &plan) ||
        launch.campaign_media_launchable ||
        strcmp(launch.campaign_media_discovery.track02_md5, media.track02_md5)) return 2;

    snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "%s",
             media.track02_md5);
    media.virtual_container = 1;
    media.no_media_extracted = 1;
    media.launchable_direct_media = 0;
    if (theron_v1_boot_startup_launch_bind_campaign_media(&launch, &media, &plan) ||
        launch.campaign_media_launchable ||
        !launch.campaign_media_discovery.virtual_container) return 3;

    fixture(&media, &plan);
    media.ambiguous = 1;
    media.candidate_count = 2;
    if (theron_v1_boot_startup_launch_bind_campaign_media(&launch, &media, &plan) ||
        launch.campaign_media_launchable ||
        !launch.campaign_media_discovery.ambiguous) return 4;

    puts("test_theron_v1_track02_campaign_media_launch: PASS");
    return 0;
}
