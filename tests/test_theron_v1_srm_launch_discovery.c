#include "theron_v1_srm_launch_discovery.h"

#include <stdio.h>
#include <string.h>

static void fixture(Theron_V1SrmCorpusReceipt *corpus,
                    Theron_V1Track02CampaignMediaDiscoveryReceipt *media) {
    Theron_V1SrmOpaqueAdmissionReceipt *save;
    memset(corpus, 0, sizeof(*corpus));
    memset(media, 0, sizeof(*media));
    corpus->status = THERON_V1_SRM_CORPUS_READY;
    corpus->admitted_count = 1;
    save = &corpus->candidates[0].admission;
    save->status = THERON_V1_SRM_OPAQUE_READY;
    save->source_regular_file_verified = save->source_md5_verified = 1;
    save->source_size_verified = save->admission_version_verified = 1;
    save->source_shape_verified = save->track02_identity_verified = 1;
    save->opaque_save_route_ready = 1;
    save->srm_size = 128u;
    save->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(save->srm_md5, sizeof(save->srm_md5), "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    snprintf(save->track02_md5, sizeof(save->track02_md5), "f23601102138f87c33025877767ebf76");
    media->status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
    media->launchable_direct_media = 1;
    media->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(media->track02_md5, sizeof(media->track02_md5), "%s", save->track02_md5);
}

int main(void) {
    Theron_V1SrmCorpusReceipt corpus;
    Theron_V1Track02CampaignMediaDiscoveryReceipt media;
    Theron_V1SrmLaunchDiscoveryReceipt receipt;
    fixture(&corpus, &media);
    if (!theron_v1_srm_launch_discovery_select(&corpus, &media, 0u, &receipt) ||
        receipt.status != THERON_V1_SRM_LAUNCH_DISCOVERY_READY ||
        receipt.direct_candidate_count != 1u) return 1;
    if (!theron_v1_srm_launch_discovery_select(&corpus, &media, 1u, &receipt) ||
        receipt.status != THERON_V1_SRM_LAUNCH_DISCOVERY_REJECTED) return 2;
    fixture(&corpus, &media);
    corpus.candidates[1].admission = corpus.candidates[0].admission;
    corpus.admitted_count = 2;
    if (!theron_v1_srm_launch_discovery_select(&corpus, &media, 0u, &receipt) ||
        receipt.status != THERON_V1_SRM_LAUNCH_DISCOVERY_REJECTED) return 3;
    fixture(&corpus, &media);
    snprintf(media.track02_md5, sizeof(media.track02_md5), "00000000000000000000000000000000");
    if (!theron_v1_srm_launch_discovery_select(&corpus, &media, 0u, &receipt) ||
        receipt.status != THERON_V1_SRM_LAUNCH_DISCOVERY_REJECTED) return 4;
    puts("test_theron_v1_srm_launch_discovery: PASS");
    return 0;
}
