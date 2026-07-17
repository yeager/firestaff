#ifndef THERON_V1_SRM_LAUNCH_DISCOVERY_H
#define THERON_V1_SRM_LAUNCH_DISCOVERY_H

#include "theron_v1_srm_corpus_manifest.h"
#include "theron_v1_track02_campaign_media_discovery.h"

typedef enum {
    THERON_V1_SRM_LAUNCH_DISCOVERY_UNAVAILABLE = 0,
    THERON_V1_SRM_LAUNCH_DISCOVERY_REJECTED,
    THERON_V1_SRM_LAUNCH_DISCOVERY_READY
} Theron_V1SrmLaunchDiscoveryStatus;

typedef struct {
    Theron_V1SrmLaunchDiscoveryStatus status;
    unsigned int direct_candidate_count;
    unsigned int virtual_candidate_count;
    int direct_regular_file_verified;
    int source_md5_verified;
    int track02_identity_verified;
    int save_semantics_decoded;
    int synthetic_fallback_used;
    Theron_V1SrmOpaqueAdmissionReceipt admission;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
} Theron_V1SrmLaunchDiscoveryReceipt;

/* Selects exactly one already-attested direct SRM for the current direct
 * campaign. Virtual candidates are diagnostic-only and reject launch. */
int theron_v1_srm_launch_discovery_select(
    const Theron_V1SrmCorpusReceipt *corpus,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    unsigned int virtual_candidate_count,
    Theron_V1SrmLaunchDiscoveryReceipt *out);

#endif
