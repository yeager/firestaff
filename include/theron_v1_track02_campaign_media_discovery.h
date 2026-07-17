#ifndef THERON_V1_TRACK02_CAMPAIGN_MEDIA_DISCOVERY_H
#define THERON_V1_TRACK02_CAMPAIGN_MEDIA_DISCOVERY_H

#include "theron_v1_track02_capture_target_plan.h"

typedef enum {
    THERON_V1_TRACK02_CAMPAIGN_MEDIA_UNAVAILABLE = 0,
    THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY,
    THERON_V1_TRACK02_CAMPAIGN_MEDIA_AMBIGUOUS,
    THERON_V1_TRACK02_CAMPAIGN_MEDIA_REJECTED
} Theron_V1Track02CampaignMediaDiscoveryStatus;

typedef enum {
    THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_NONE = 0,
    THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CUE,
    THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_LOOSE,
    THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CONTAINER
} Theron_V1Track02CampaignMediaSource;

typedef struct {
    Theron_V1Track02CampaignMediaDiscoveryStatus status;
    Theron_V1Track02CampaignMediaSource source;
    int candidate_count;
    int ambiguous;
    int virtual_container;
    int no_media_extracted;
    int exact_layout_bound;
    int launchable_direct_media;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    char candidate_path[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY];
    Theron_V1Track02RawMediaIntakeReceipt direct_media;
} Theron_V1Track02CampaignMediaDiscoveryReceipt;

/* Hash-first discovery. `search_path` may name one direct CUE/BIN/ISO file or
 * a directory scanned by Firestaff's existing loose/container hash scanner.
 * Virtual matches are never extracted or launched. */
int theron_v1_track02_campaign_media_discover(
    const char *search_path,
    const char *expected_track02_md5,
    int max_depth,
    Theron_V1Track02CampaignMediaDiscoveryReceipt *out);

/* Binds one unambiguous, exact-layout candidate to all three opaque capture
 * targets. This is identity-only and never grants decode or render access. */
int theron_v1_track02_campaign_media_bind_capture_plan(
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02CaptureTargetPlan *plan);

/* Rechecks a refreshed direct-media receipt before an existing campaign plan
 * is reused. Virtual/container candidates are diagnostic-only here. */
int theron_v1_track02_campaign_media_direct_layout_current(
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    const Theron_V1Track02CaptureTargetPlan *plan);

#endif
