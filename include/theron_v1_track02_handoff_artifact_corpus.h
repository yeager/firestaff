#ifndef THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_H
#define THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_H

#include "theron_v1_track02_capture_artifact_importer.h"
#include "theron_v1_track02_external_capture_launcher.h"
#include "theron_v1_track02_sector_record_corpus_discovery.h"

typedef enum {
    THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_UNAVAILABLE = 0,
    THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_REJECTED,
    THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_READY
} Theron_V1Track02HandoffArtifactCorpusStatus;

typedef struct {
    const char *bundle_path;
    const char *expected_bundle_md5;
} Theron_V1Track02HandoffArtifactCorpusCandidate;

/* Opaque operator-corpus result. READY remains no-draw evidence and retains
 * no payload bytes, level/object fields, palette entries, or bitmap data. */
typedef struct {
    Theron_V1Track02HandoffArtifactCorpusStatus status;
    unsigned int supplied_candidate_count;
    unsigned int direct_candidate_count;
    unsigned int rejected_candidate_count;
    int direct_cue_bin_consumed;
    int source_trace_md5_verified;
    int capture_target_plan_consumed;
    int opaque_artifact_consumed;
    int capture_required_only;
    int no_draw_only;
    char track02_md5[33];
    char source_trace_md5[33];
    uint32_t capture_target_plan_identity;
    Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt artifact;
} Theron_V1Track02HandoffArtifactCorpusReceipt;

/* Imports exactly one explicit direct artifact bundle only after the prior
 * handoff receipt, current direct CUE/BIN, source trace, and plan identity all
 * agree. It never creates a bundle or interprets its opaque route rows. */
int theron_v1_track02_handoff_artifact_corpus_import(
    const Theron_V1Track02ExternalCaptureReceipt *handoff,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02HandoffArtifactCorpusCandidate *candidates,
    unsigned int candidate_count,
    Theron_V1Track02HandoffArtifactCorpusReceipt *out);

/* Rechecks the retained opaque artifact rows against their exact source-owned
 * plan. `expected_source_trace_md5` may be NULL only when the caller has no
 * separately bound trace receipt; the artifact and corpus trace identities
 * must still agree. This does not inspect artifact payload bytes. */
int theron_v1_track02_handoff_artifact_corpus_matches_plan(
    const Theron_V1Track02HandoffArtifactCorpusReceipt *receipt,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const char *expected_source_trace_md5);

/* Runtime-facing subset for callers that retain the plan identity but not the
 * whole plan. It still verifies the complete opaque envelope is internally
 * consistent and no decoder or draw permission was introduced. */
int theron_v1_track02_handoff_artifact_corpus_matches_identity(
    const Theron_V1Track02HandoffArtifactCorpusReceipt *receipt,
    const char *track02_md5,
    const char *source_trace_md5,
    uint32_t capture_target_plan_identity);

/* Joins the imported dungeon row to the one descriptor-selected later $e009
 * transaction re-admitted from the original CUE/BIN and coalesced trace.
 * The comparison retains CD_READ and loader-output evidence only. */
int theron_v1_track02_handoff_artifact_corpus_matches_sector_record(
    const Theron_V1Track02HandoffArtifactCorpusReceipt *receipt,
    const Theron_V1Track02SectorRecordCorpusDiscoveryReceipt *sector_corpus,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const char *expected_source_trace_md5);

#endif
