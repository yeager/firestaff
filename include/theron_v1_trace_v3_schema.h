#ifndef THERON_V1_TRACE_V3_SCHEMA_H
#define THERON_V1_TRACE_V3_SCHEMA_H

#include "theron_v1_stage2_runtime_handoff.h"
#include "theron_v1_irq2_live_trace_gate.h"
#include "theron_v1_track_media_availability.h"

/* Transport contract only. Register values remain opaque. */
typedef struct { int valid; } Theron_V1TraceV3SchemaReceipt;
typedef struct {
    int valid;
    int source_hashes_bound;
    int epoch_matches_stage3;
    int runtime_allowed;
    const char *status;
} Theron_V1TraceV3BindingReceipt;
typedef struct {
    int valid;
    int runtime_blocked;
    char line[256];
} Theron_V1TraceV3ImportRequest;
typedef struct { int valid; int runtime_blocked; char config[512]; } Theron_V1TraceV3CaptureLaunch;
const char *theron_v1_trace_v3_preflight_status(const char *system_card,
                                                const char *system_card_md5,
                                                int trace_present);
const char *theron_v1_trace_v3_raw_track02_status(const char *track02_md5,
                                                   const char *system_card);
int theron_v1_trace_v3_format_command(const Theron_V1TraceV3CaptureLaunch *launch,
                                       const char *mednafen, const char *cue,
                                       const char *track02, const char *system_card,
                                       const char *trace, char *out, size_t out_size);
typedef struct { int valid; int runtime_blocked; char command[512]; } Theron_V1TraceV3CommandPlan;
typedef struct { unsigned int magic, version; int capture_command_ready, trace_validated, runtime_blocked; } Theron_V1TraceV3LauncherReceipt;
int theron_v1_trace_v3_plan_command(const Theron_V1TraceV3ImportRequest *request,
                                    const Theron_V1TraceV3CaptureLaunch *launch,
                                    const char *mednafen, const char *cue,
                                    const char *track02, const char *system_card,
                                    const char *trace, Theron_V1TraceV3CommandPlan *out);
int theron_v1_trace_v3_plan_command_with_media(const Theron_V1_TrackMediaAvailabilityReceipt *media,
    const Theron_V1TraceV3ImportRequest *request,const Theron_V1TraceV3CaptureLaunch *launch,
    const char *mednafen,const char *cue,const char *track02,const char *system_card,const char *trace,Theron_V1TraceV3CommandPlan *out);
int theron_v1_trace_v3_launcher_receipt(const Theron_V1TraceV3CommandPlan *plan,
                                        Theron_V1TraceV3LauncherReceipt *out);
Theron_V1Irq2PreflightStatus theron_v1_trace_v3_preflight_adapter(
    const Theron_V1TraceV3LauncherReceipt *receipt, const char *status);
int theron_v1_trace_v3_schema_validate(const char *text,
                                       Theron_V1TraceV3SchemaReceipt *out);
int theron_v1_trace_v3_bind_stage3(const Theron_V1TraceV3SchemaReceipt *schema,
                                   const Theron_V1Stage2RuntimeHandoff *stage3,
                                   const char *cue_hash, const char *track02_hash,
                                   int epoch_matches_stage3,
                                   Theron_V1TraceV3BindingReceipt *out);
int theron_v1_trace_v3_import_request(const Theron_V1TraceV3BindingReceipt *binding,
                                      const Theron_V1Stage2RuntimeHandoff *stage3,
                                      const char *cue_md5, const char *track02_md5,
                                      Theron_V1TraceV3ImportRequest *out);
int theron_v1_trace_v3_capture_launch(const Theron_V1TraceV3ImportRequest *request,
                                      const char *cue, const char *track02,
                                      const char *system_card, const char *system_card_md5,
                                      Theron_V1TraceV3CaptureLaunch *out);
#endif
