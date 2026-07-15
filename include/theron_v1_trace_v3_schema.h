#ifndef THERON_V1_TRACE_V3_SCHEMA_H
#define THERON_V1_TRACE_V3_SCHEMA_H

#include "theron_v1_track02.h"
#include "theron_v1_track_media_availability.h"

#include <stddef.h>

typedef struct {
    int valid;
    int has_pre4090;
    int has_post4093;
    int has_pcecd_epoch;
} Theron_V1TraceV3SchemaReceipt;

typedef struct {
    int valid;
    int stage3_cd_read_record_proven;
    unsigned int stage3_cd_read_record;
} Theron_V1Stage2RuntimeHandoff;

typedef struct {
    int valid;
    int runtime_allowed;
    int source_hashes_bound;
    int epoch_matches_stage3;
} Theron_V1TraceV3BindingReceipt;

typedef struct {
    int valid;
    int runtime_blocked;
} Theron_V1TraceV3ImportRequest;

typedef struct {
    int valid;
    int runtime_blocked;
    const char *cue_path;
    const char *track02_path;
    const char *system_card_path;
    const char *system_card_hash;
} Theron_V1TraceV3CaptureLaunch;

typedef struct {
    int valid;
    int runtime_blocked;
    char command[256];
} Theron_V1TraceV3CommandPlan;

typedef struct {
    int capture_command_ready;
    int trace_validated;
    int runtime_blocked;
} Theron_V1TraceV3LauncherReceipt;

typedef enum {
    THERON_V1_IRQ2_PREFLIGHT_OK = 0,
    THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_MISSING = 1,
    THERON_V1_IRQ2_PREFLIGHT_TRACE_INVALID = 2
} Theron_V1Irq2PreflightStatus;

const char *theron_v1_trace_v3_preflight_status(const char *system_card_path,
                                                const char *system_card_hash,
                                                int trace_validated);

const char *theron_v1_trace_v3_raw_track02_status(const char *track02_md5,
                                                  const char *system_card_path);

int theron_v1_trace_v3_schema_validate(
    const char *text,
    Theron_V1TraceV3SchemaReceipt *out_receipt);

int theron_v1_trace_v3_bind_stage3(
    const Theron_V1TraceV3SchemaReceipt *schema,
    const Theron_V1Stage2RuntimeHandoff *stage3,
    const char *cue_hash,
    const char *track02_hash,
    int pcecd_epoch_bound,
    Theron_V1TraceV3BindingReceipt *out_receipt);

int theron_v1_trace_v3_import_request(
    const Theron_V1TraceV3BindingReceipt *binding,
    const Theron_V1Stage2RuntimeHandoff *stage3,
    const char *cue_hash,
    const char *track02_hash,
    Theron_V1TraceV3ImportRequest *out_request);

int theron_v1_trace_v3_capture_launch(
    const Theron_V1TraceV3ImportRequest *request,
    const char *cue_path,
    const char *track02_path,
    const char *system_card_path,
    const char *system_card_hash,
    Theron_V1TraceV3CaptureLaunch *out_launch);

int theron_v1_trace_v3_format_command(
    const Theron_V1TraceV3CaptureLaunch *launch,
    const char *mednafen_path,
    const char *cue_path,
    const char *track02_path,
    const char *system_card_path,
    const char *trace_path,
    char *out_command,
    size_t out_command_cap);

int theron_v1_trace_v3_plan_command(
    const Theron_V1TraceV3ImportRequest *request,
    const Theron_V1TraceV3CaptureLaunch *launch,
    const char *mednafen_path,
    const char *cue_path,
    const char *track02_path,
    const char *system_card_path,
    const char *trace_path,
    Theron_V1TraceV3CommandPlan *out_plan);

int theron_v1_trace_v3_plan_command_with_media(
    const Theron_V1_TrackMediaAvailabilityReceipt *media,
    const Theron_V1TraceV3ImportRequest *request,
    const Theron_V1TraceV3CaptureLaunch *launch,
    const char *mednafen_path,
    const char *cue_path,
    const char *track02_path,
    const char *system_card_path,
    const char *trace_path,
    Theron_V1TraceV3CommandPlan *out_plan);

int theron_v1_trace_v3_launcher_receipt(
    const Theron_V1TraceV3CommandPlan *plan,
    Theron_V1TraceV3LauncherReceipt *out_receipt);

Theron_V1Irq2PreflightStatus theron_v1_trace_v3_preflight_adapter(
    const Theron_V1TraceV3LauncherReceipt *launcher,
    const char *status);

#endif
