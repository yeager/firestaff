#ifndef NEXUS_V1_SATURN_CAPTURE_CAMPAIGN_IMPORT_H
#define NEXUS_V1_SATURN_CAPTURE_CAMPAIGN_IMPORT_H

#include "nexus_v1_prs3_dgn_placement_adapter.h"
#include "nexus_v1_structure1f_corpus_capture_plan.h"
#include "nexus_v1_slev_task_body_capture_plan.h"
#include "nexus_v1_sal_capture_plan.h"

#define NEXUS_V1_SATURN_CAPTURE_CAMPAIGN_MAGIC \
    "FIRESTAFF_NEXUS_MEDNAFEN_SATURN_CAMPAIGN_V1"

/* The exporter is an external Mednafen debugger workflow. Firestaff compares
 * its complete text and raw trace identity, then retains evidence only. */
typedef struct {
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *prs3;
    const Nexus_V1_Structure1FCorpusTraceTarget *structure1f;
    const Nexus_V1_SlevTaskBodyCaptureTarget *slev;
    const Nexus_V1_SalCapturePlanTarget *sal;
    const uint8_t *raw_trace;
    size_t raw_trace_size;
    const char *trace_sha256;
    const char *export_text;
    size_t export_text_size;
} Nexus_V1_SaturnCaptureCampaignImportInput;

typedef struct {
    int valid;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    uint64_t dgn_fnv1a64;
    uint32_t descriptor_index;
    uint32_t level_index;
    uint32_t raw_map_selector;
    int prs3_evidence_bound;
    int structure1f_evidence_bound;
    int slev_evidence_bound;
    int sal_evidence_bound;
    int evidence_only;
    int decoder_permitted;
    int geometry_semantics_permitted;
    int script_semantics_permitted;
    int playback_permitted;
} Nexus_V1_SaturnCaptureCampaignReceipt;

/* Imports exactly one complete, identity-consistent campaign export. It never
 * decodes PRS3/SAL, interprets geometry or scripts, or enables playback. */
int nexus_v1_saturn_capture_campaign_import(
    const Nexus_V1_SaturnCaptureCampaignImportInput *input,
    Nexus_V1_SaturnCaptureCampaignReceipt *out_receipt);

#endif
