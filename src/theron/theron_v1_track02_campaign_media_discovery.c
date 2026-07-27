#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include "asset_find_by_hash.h"
#include "theron_v1_track02_campaign_media_discovery.h"

static int is_virtual_path(const char *path)
{
    return path && strstr(path, "::") != NULL;
}

static const char *extension(const char *path)
{
    const char *dot = path ? strrchr(path, '.') : NULL;
    return dot ? dot : "";
}

static int ieq(const char *left, const char *right)
{
    unsigned char a;
    unsigned char b;
    if (!left || !right) return 0;
    do {
        a = (unsigned char)*left++;
        b = (unsigned char)*right++;
        if (a >= 'A' && a <= 'Z') a = (unsigned char)(a + ('a' - 'A'));
        if (b >= 'A' && b <= 'Z') b = (unsigned char)(b + ('a' - 'A'));
        if (a != b) return 0;
    } while (a);
    return 1;
}

static int plan_matches_media(const Theron_V1Track02CaptureTargetPlan *plan,
                              const Theron_V1Track02CampaignMediaDiscoveryReceipt *media)
{
    size_t i;
    if (!plan || !media || !plan->valid || media->ambiguous ||
        !media->exact_layout_bound ||
        media->status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY ||
        media->track02_variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        !media->track02_md5[0] || plan->level_object_semantics_allowed ||
        plan->pixel_decode_allowed || plan->render_allowed ||
        plan->fallback_visuals_allowed) return 0;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        const Theron_V1Track02CaptureTarget *target = &plan->targets[i];
        if (target->route != (Theron_V1Track02CaptureTargetRoute)i ||
            target->track02_variant != media->track02_variant ||
            strcmp(target->track02_md5, media->track02_md5) ||
            target->level_object_semantics_allowed || target->pixel_decode_allowed ||
            target->render_allowed || target->fallback_visuals_allowed) return 0;
    }
    return 1;
}

int theron_v1_track02_campaign_media_discover(
    const char *search_path,
    const char *expected_track02_md5,
    int max_depth,
    Theron_V1Track02CampaignMediaDiscoveryReceipt *out)
{
    Theron_V1Track02CampaignMediaDiscoveryReceipt receipt = {0};
    struct stat info;
    char found[ASSET_PATH_MAX] = {0};
    Theron_Track02Variant variant;
    if (!out) return 0;
    *out = receipt;
    if (!search_path || !search_path[0] || !expected_track02_md5 ||
        strlen(expected_track02_md5) != 32u ||
        (variant = theron_v1_track02_variant_for_md5(expected_track02_md5)) ==
            THERON_TRACK02_VARIANT_UNKNOWN) {
        receipt.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_REJECTED;
        receipt.failure_reason = THERON_V1_TRACK02_MEDIA_REASON_EXPECTED_HASH_MISMATCH;
        *out = receipt;
        return 1;
    }
    receipt.track02_variant = variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             expected_track02_md5);
    if (stat(search_path, &info) != 0) {
        if ((ieq(extension(search_path), ".iso") ||
             ieq(extension(search_path), ".bin")) &&
            theron_v1_track02_raw_media_intake_discover(
                search_path, &receipt.direct_media) &&
            receipt.direct_media.status == THERON_V1_TRACK02_MEDIA_INTAKE_READY &&
            receipt.direct_media.variant == variant &&
            !strcmp(receipt.direct_media.track02_md5, expected_track02_md5)) {
            receipt.candidate_count = 1;
            receipt.source = THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_LOOSE;
            snprintf(receipt.candidate_path, sizeof(receipt.candidate_path), "%s",
                     receipt.direct_media.payload_path);
            receipt.exact_layout_bound = 1;
            receipt.launchable_direct_media = 1;
            receipt.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
            *out = receipt;
            return 1;
        }
        receipt.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_UNAVAILABLE;
        receipt.failure_reason = receipt.direct_media.failure_reason !=
            THERON_V1_TRACK02_MEDIA_REASON_NONE
            ? receipt.direct_media.failure_reason
            : THERON_V1_TRACK02_MEDIA_REASON_PATH_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (S_ISREG(info.st_mode) &&
        (ieq(extension(search_path), ".cue") || ieq(extension(search_path), ".bin") ||
         ieq(extension(search_path), ".iso"))) {
        receipt.candidate_count = 1;
        snprintf(receipt.candidate_path, sizeof(receipt.candidate_path), "%s",
                 search_path);
        receipt.source = ieq(extension(search_path), ".cue")
            ? THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CUE
            : THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_LOOSE;
        if (!theron_v1_track02_raw_media_intake_discover(search_path,
                                                          &receipt.direct_media) ||
            receipt.direct_media.status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
            receipt.direct_media.variant != variant ||
            strcmp(receipt.direct_media.track02_md5, expected_track02_md5)) {
            receipt.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_REJECTED;
            receipt.failure_reason = receipt.direct_media.failure_reason !=
                THERON_V1_TRACK02_MEDIA_REASON_NONE
                ? receipt.direct_media.failure_reason
                : THERON_V1_TRACK02_MEDIA_REASON_EXPECTED_HASH_MISMATCH;
            *out = receipt;
            return 1;
        }
        receipt.exact_layout_bound = 1;
        receipt.launchable_direct_media = 1;
        receipt.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
        *out = receipt;
        return 1;
    }
    if (!S_ISDIR(info.st_mode) ||
        !asset_find_by_md5(search_path, expected_track02_md5, found,
                           (int)sizeof(found), max_depth)) {
        receipt.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_UNAVAILABLE;
        receipt.failure_reason = THERON_V1_TRACK02_MEDIA_REASON_PATH_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    /* The caller has already selected one exact, hash-verified Track 02
     * variant. A data root may legitimately contain both regional releases;
     * their presence must not invalidate this explicit selection. */
    receipt.candidate_count = 1;
    snprintf(receipt.candidate_path, sizeof(receipt.candidate_path), "%s", found);
    receipt.virtual_container = is_virtual_path(found);
    receipt.no_media_extracted = receipt.virtual_container;
    receipt.source = receipt.virtual_container
        ? THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CONTAINER
        : THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_LOOSE;
    if (receipt.virtual_container) {
        /* Hash scanner owns container decompression. The known Track 02 MD5
         * identifies the exact supported raw layout, but this path never
         * materializes bytes for an emulator launch. */
        receipt.exact_layout_bound = 1;
        receipt.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
    } else if (theron_v1_track02_raw_media_intake_discover(found,
                                                             &receipt.direct_media) &&
               receipt.direct_media.status == THERON_V1_TRACK02_MEDIA_INTAKE_READY &&
               receipt.direct_media.variant == variant &&
               !strcmp(receipt.direct_media.track02_md5, expected_track02_md5)) {
        receipt.exact_layout_bound = 1;
        receipt.launchable_direct_media = 1;
        receipt.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
    } else {
        receipt.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_REJECTED;
        receipt.failure_reason = receipt.direct_media.failure_reason !=
            THERON_V1_TRACK02_MEDIA_REASON_NONE
            ? receipt.direct_media.failure_reason
            : THERON_V1_TRACK02_MEDIA_REASON_EXPECTED_HASH_MISMATCH;
    }
    *out = receipt;
    return 1;
}

const char *theron_v1_track02_campaign_media_failure_reason_id(
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *receipt)
{
    return theron_v1_track02_media_failure_reason_id(
        receipt ? receipt->failure_reason : THERON_V1_TRACK02_MEDIA_REASON_NONE);
}

int theron_v1_track02_campaign_media_bind_capture_plan(
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02CaptureTargetPlan *plan)
{
    return plan_matches_media(plan, media);
}

int theron_v1_track02_campaign_media_direct_layout_current(
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    const Theron_V1Track02CaptureTargetPlan *plan)
{
    const Theron_V1Track02RawMediaIntakeReceipt *bound;

    if (!media || !refreshed || !plan || media->status !=
            THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY || media->ambiguous ||
        media->virtual_container || media->no_media_extracted ||
        !media->launchable_direct_media || !media->exact_layout_bound ||
        !plan_matches_media(plan, media)) return 0;
    bound = &media->direct_media;
    if (bound->status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        refreshed->status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        bound->variant != media->track02_variant ||
        refreshed->variant != bound->variant ||
        strcmp(bound->track02_md5, media->track02_md5) ||
        strcmp(refreshed->track02_md5, bound->track02_md5) ||
        bound->cue_consumed != refreshed->cue_consumed ||
        bound->mode1_2352 != refreshed->mode1_2352 ||
        bound->mode1_2048 != refreshed->mode1_2048 ||
        bound->raw_trace_preparation_allowed != refreshed->raw_trace_preparation_allowed ||
        strcmp(bound->media_path, refreshed->media_path) ||
        strcmp(bound->payload_path, refreshed->payload_path) ||
        bound->cue_index01_sector != refreshed->cue_index01_sector ||
        bound->payload_bytes != refreshed->payload_bytes ||
        bound->sector_count != refreshed->sector_count ||
        bound->first_user_data_offset != refreshed->first_user_data_offset ||
        bound->logical_user_data_window_bytes != refreshed->logical_user_data_window_bytes) return 0;
    return 1;
}
