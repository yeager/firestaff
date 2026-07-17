#ifndef THERON_V1_TRACK02_EXTERNAL_CAPTURE_LAUNCHER_H
#define THERON_V1_TRACK02_EXTERNAL_CAPTURE_LAUNCHER_H

#include "theron_v1_track02_capture_trace_runtime_admission.h"
#include "theron_v1_track02_huc6280_capture_event_log.h"
#include "theron_v1_track02_mednafen_trace_converter.h"

#define THERON_V1_TRACK02_EXTERNAL_CAPTURE_PATH_CAPACITY 512

typedef enum {
    THERON_V1_TRACK02_EXTERNAL_CAPTURE_UNAVAILABLE = 0,
    THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED,
    THERON_V1_TRACK02_EXTERNAL_CAPTURE_SKELETON_WRITTEN,
    THERON_V1_TRACK02_EXTERNAL_CAPTURE_RUNTIME_READY
} Theron_V1Track02ExternalCaptureStatus;

/* All paths are operator-provided. The launcher never starts the emulator,
 * copies media, or emits a trace; it only writes a manifest request skeleton
 * beside the caller-selected output path. */
typedef struct {
    const char *emulator_path;
    const char *media_path;
    const char *expected_track02_md5;
    const char *manifest_path;
} Theron_V1Track02ExternalCaptureRequest;

typedef struct {
    Theron_V1Track02ExternalCaptureStatus status;
    int external_launch_performed;
    int media_copied;
    int synthetic_trace_created;
    int decoder_invoked;
    int raw_media_intake_verified;
    int manifest_skeleton_written;
    int strict_manifest_consumed;
    int runtime_admission_consumed;
    int mednafen_trace_source_verified;
    int mednafen_event_log_verified;
    int capture_target_plan_verified;
    int positive_handoff_capture_required;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    char emulator_path[THERON_V1_TRACK02_EXTERNAL_CAPTURE_PATH_CAPACITY];
    char media_path[THERON_V1_TRACK02_EXTERNAL_CAPTURE_PATH_CAPACITY];
    char manifest_path[THERON_V1_TRACK02_EXTERNAL_CAPTURE_PATH_CAPACITY];
    char mednafen_trace_source_path[THERON_V1_TRACK02_EXTERNAL_CAPTURE_PATH_CAPACITY];
    char mednafen_trace_source_md5[33];
    char mednafen_event_log_md5[33];
    uint32_t capture_target_plan_identity;
} Theron_V1Track02ExternalCaptureReceipt;

/* Verifies an explicit emulator path plus CUE/BIN media against the caller's
 * expected MD5, then writes a non-validating strict-manifest skeleton. The
 * skeleton contains no consumer PCs, payload windows, checksums, or trace
 * facts; an original external capture must supply those later. */
int theron_v1_track02_external_capture_write_skeleton(
    const Theron_V1Track02ExternalCaptureRequest *request,
    Theron_V1Track02ExternalCaptureReceipt *out);

/* Validates an operator-filled manifest through raw-media intake, existing
 * manifest binding, and runtime admission. It never starts the named emulator
 * and never treats a skeleton as a trace. */
int theron_v1_track02_external_capture_validate(
    const Theron_V1Track02ExternalCaptureRequest *request,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out_admission,
    Theron_V1Track02ExternalCaptureReceipt *out);

/* Same validation chain, but the strict manifest is generated in memory only
 * from the observed HuC6280 event log. The caller-supplied manifest path is
 * not read or written by this route. */
int theron_v1_track02_external_capture_validate_huc6280_log(
    const Theron_V1Track02ExternalCaptureRequest *request,
    const char *event_log_path,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out_admission,
    Theron_V1Track02ExternalCaptureReceipt *out);

/* Converts an explicitly MD5-bound external Mednafen export, then consumes
 * only its strict HuC6280 event log through the existing opaque validation
 * route. Capture media is verified before an output log can be written. */
int theron_v1_track02_external_capture_validate_mednafen_trace(
    const Theron_V1Track02ExternalCaptureRequest *request,
    const Theron_V1Track02MednafenTraceConvertRequest *trace_request,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out_admission,
    Theron_V1Track02ExternalCaptureReceipt *out);

/* Handoff-only operator path. It first binds one direct MODE1/2352 CUE/BIN
 * intake and one complete opaque capture plan to the requested plan FNV, then
 * converts only the explicitly MD5-bound observed Mednafen trace. A ready
 * receipt still names capture-required/no-draw evidence; it is not a decoder
 * or presentation admission. */
int theron_v1_track02_external_capture_validate_mednafen_handoff_plan(
    const Theron_V1Track02ExternalCaptureRequest *request,
    const Theron_V1Track02MednafenTraceConvertRequest *trace_request,
    const Theron_V1Track02CaptureTargetPlan *plan,
    uint32_t expected_capture_target_plan_identity,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out_admission,
    Theron_V1Track02ExternalCaptureReceipt *out);

#endif
