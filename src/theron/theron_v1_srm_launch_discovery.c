#include "theron_v1_srm_launch_discovery.h"

#include <string.h>

int theron_v1_srm_launch_discovery_select(const Theron_V1SrmCorpusReceipt *corpus,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    unsigned int virtual_candidate_count, Theron_V1SrmLaunchDiscoveryReceipt *out) {
    Theron_V1SrmLaunchDiscoveryReceipt receipt = {0};
    const Theron_V1SrmOpaqueAdmissionReceipt *selected = NULL;
    unsigned int i;
    if (!out) return 0;
    *out = receipt;
    receipt.virtual_candidate_count = virtual_candidate_count;
    if (!corpus || corpus->status == THERON_V1_SRM_CORPUS_UNAVAILABLE) return 1;
    if (!media || media->status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY ||
        media->virtual_container || !media->launchable_direct_media ||
        corpus->status != THERON_V1_SRM_CORPUS_READY || corpus->rejected_count ||
        virtual_candidate_count) { receipt.status = THERON_V1_SRM_LAUNCH_DISCOVERY_REJECTED; *out = receipt; return 1; }
    for (i = 0; i < THERON_V1_SRM_CORPUS_MAX_CANDIDATES; ++i) {
        const Theron_V1SrmOpaqueAdmissionReceipt *admission = &corpus->candidates[i].admission;
        if (admission->status != THERON_V1_SRM_OPAQUE_READY) continue;
        ++receipt.direct_candidate_count;
        selected = admission;
    }
    if (receipt.direct_candidate_count != 1u || !selected ||
        selected->track02_variant != media->track02_variant ||
        strcmp(selected->track02_md5, media->track02_md5) ||
        !selected->source_regular_file_verified || !selected->source_md5_verified ||
        !selected->source_size_verified || !selected->admission_version_verified ||
        !selected->source_shape_verified || !selected->track02_identity_verified ||
        !selected->opaque_save_route_ready || selected->save_semantics_decoded ||
        selected->synthetic_fallback_used) { receipt.status = THERON_V1_SRM_LAUNCH_DISCOVERY_REJECTED; *out = receipt; return 1; }
    receipt.status = THERON_V1_SRM_LAUNCH_DISCOVERY_READY;
    receipt.direct_regular_file_verified = receipt.source_md5_verified = receipt.track02_identity_verified = 1;
    receipt.admission = *selected;
    receipt.track02_variant = selected->track02_variant;
    memcpy(receipt.track02_md5, selected->track02_md5, sizeof(receipt.track02_md5));
    *out = receipt;
    return 1;
}
