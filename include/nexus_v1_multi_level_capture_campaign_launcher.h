#ifndef NEXUS_V1_MULTI_LEVEL_CAPTURE_CAMPAIGN_LAUNCHER_H
#define NEXUS_V1_MULTI_LEVEL_CAPTURE_CAMPAIGN_LAUNCHER_H

#include "nexus_v1_dgn_multi_level_capture_adjudicator.h"
#include "nexus_v1_lev_corpus_discovery.h"
#include "nexus_v1_slev_task_body_capture_plan.h"
#include "nexus_v1_sal_capture_plan.h"

#define NEXUS_V1_MULTI_LEVEL_CAPTURE_JOB_COUNT 16U

typedef struct {
    int operator_opt_in;
    int retail_assets_available;
    const char *disc_path;
    const char *menu_bpk_path;
    const char *dm_bin_path;
    const char *disc_sha256;
    const char *menu_bpk_sha256;
    const char *dm_bin_sha256;
    const Nexus_V1_LevCorpusDiscoveryReceipt *direct_lev_corpus;
    const Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt *dgn;
    const Nexus_V1_SlevTaskBodyCapturePlan *slev;
    const Nexus_V1_SalCapturePlan *sal;
} Nexus_V1_MultiLevelCaptureCampaignLauncherInput;

typedef struct {
    int valid;
    uint32_t level_index;
    uint64_t dgn_fnv1a64;
    uint64_t trace_fnv1a64;
    uint32_t trace_size;
    uint64_t slev_source_fnv1a64;
    uint32_t sal_selector;
} Nexus_V1_MultiLevelCaptureJob;

typedef struct {
    int valid;
    int skipped_missing_retail_assets;
    int operator_only;
    int evidence_generated;
    int graphics_permitted;
    char manifest_sha256[65];
    Nexus_V1_MultiLevelCaptureJob jobs[NEXUS_V1_MULTI_LEVEL_CAPTURE_JOB_COUNT];
} Nexus_V1_MultiLevelCaptureCampaignLaunchPlan;

int nexus_v1_multi_level_capture_campaign_launcher_plan(
    const Nexus_V1_MultiLevelCaptureCampaignLauncherInput *input,
    Nexus_V1_MultiLevelCaptureCampaignLaunchPlan *out_plan);

#endif
