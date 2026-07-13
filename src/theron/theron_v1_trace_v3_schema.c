#include "theron_v1_trace_v3_schema.h"

#include <string.h>
#include <stdio.h>

static int hash32(const char *hash) {
    size_t i;
    if (!hash || strlen(hash) != 32u) return 0;
    for (i = 0u; i < 32u; ++i)
        if (!((hash[i] >= '0' && hash[i] <= '9') ||
              (hash[i] >= 'a' && hash[i] <= 'f'))) return 0;
    return 1;
}

const char *theron_v1_trace_v3_preflight_status(const char *system_card,
                                                const char *system_card_md5,
                                                int trace_present) {
    if (!system_card || !system_card[0]) return "system_card_missing";
    if (!hash32(system_card_md5)) return "system_card_hash_mismatch";
    return trace_present ? "trace_v3_full_import_required" :
        "trace_capture_required";
}

const char *theron_v1_trace_v3_raw_track02_status(const char *track02_md5,
                                                   const char *system_card) {
    if (!track02_md5 || !track02_md5[0]) return "raw_track02_missing";
    if (strcmp(track02_md5, THERON_TRACK02_MD5_US_ISO) == 0 ||
        strcmp(track02_md5, THERON_TRACK02_MD5_JP_REV1_ISO) == 0)
        return "raw_track02_iso_end_variant";
    if (strcmp(track02_md5, THERON_TRACK02_MD5_JP_BIN) != 0 &&
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0)
        return "raw_track02_missing";
    return (!system_card || !system_card[0]) ? "system_card_missing" : "raw_track02_ready";
}

static int safe_path(const char *text) {
    return text && text[0] && !strpbrk(text, "\n\r\"'");
}
int theron_v1_trace_v3_format_command(const Theron_V1TraceV3CaptureLaunch *launch,
                                       const char *mednafen, const char *cue,
                                       const char *track02, const char *system_card,
                                       const char *trace, char *out, size_t out_size) {
    int written;
    if (!out || !out_size) return 0;
    out[0] = '\0';
    if (!launch || !launch->valid || !launch->runtime_blocked ||
        !safe_path(mednafen) || !safe_path(cue) || !safe_path(track02) ||
        !safe_path(system_card) || !safe_path(trace)) return 0;
    written = snprintf(out, out_size,
        "FIRESTAFF_THERON_IRQ2_TRACE=%s %s -sound 0 -pce.cdbios %s %s",
        trace, mednafen, system_card, cue);
    return written > 0 && (size_t)written < out_size;
}
int theron_v1_trace_v3_plan_command(const Theron_V1TraceV3ImportRequest *request,
                                    const Theron_V1TraceV3CaptureLaunch *launch,
                                    const char *mednafen, const char *cue,
                                    const char *track02, const char *system_card,
                                    const char *trace, Theron_V1TraceV3CommandPlan *out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!request || !request->valid || !request->runtime_blocked ||
        !theron_v1_trace_v3_format_command(launch, mednafen, cue, track02,
                                            system_card, trace, out->command,
                                            sizeof(out->command))) return 0;
    out->valid = 1;
    out->runtime_blocked = 1;
    return 1;
}
int theron_v1_trace_v3_plan_command_with_media(const Theron_V1_TrackMediaAvailabilityReceipt *media,const Theron_V1TraceV3ImportRequest *request,const Theron_V1TraceV3CaptureLaunch *launch,const char *mednafen,const char *cue,const char *track02,const char *system_card,const char *trace,Theron_V1TraceV3CommandPlan *out){if(!media||media->availability!=THERON_V1_TRACK_MEDIA_RAW_READY||!media->loader_usable)return 0;return theron_v1_trace_v3_plan_command(request,launch,mednafen,cue,track02,system_card,trace,out);}
int theron_v1_trace_v3_launcher_receipt(const Theron_V1TraceV3CommandPlan *plan,
                                        Theron_V1TraceV3LauncherReceipt *out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!plan || !plan->valid || !plan->runtime_blocked) return 0;
    out->magic = 0x54515633u;
    out->version = 1u;
    out->capture_command_ready = 1;
    out->trace_validated = 0;
    out->runtime_blocked = 1;
    return 1;
}
Theron_V1Irq2PreflightStatus theron_v1_trace_v3_preflight_adapter(
    const Theron_V1TraceV3LauncherReceipt *receipt, const char *status) {
    if (!receipt || !receipt->runtime_blocked || !status)
        return THERON_V1_IRQ2_PREFLIGHT_TRACE_INVALID;
    if (strcmp(status, "system_card_missing") == 0)
        return THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_MISSING;
    if (strcmp(status, "system_card_hash_mismatch") == 0)
        return THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_HASH_MISMATCH;
    return THERON_V1_IRQ2_PREFLIGHT_TRACE_INVALID;
}

int theron_v1_trace_v3_schema_validate(const char *text,
                                       Theron_V1TraceV3SchemaReceipt *out) {
    static const char *const rows[] = {
        "trace_format=theron_irq2_capture_v3",
        "loader_registers phase=pre4090 ",
        "loader_registers phase=post4093 ",
        "pcecd_epoch transfer_epoch=physical_stage2_stage3 "
    };
    const char *cursor = text;
    unsigned int index;
    if (out) out->valid = 0;
    if (!text || !out) return 0;
    for (index = 0u; index < 4u; ++index) {
        size_t length = strlen(rows[index]);
        if (strncmp(cursor, rows[index], length) != 0) return 0;
        cursor = strchr(cursor, '\n');
        if (!cursor) return 0;
        ++cursor;
    }
    if (*cursor != '\0') return 0;
    out->valid = 1;
    return 1;
}

int theron_v1_trace_v3_bind_stage3(const Theron_V1TraceV3SchemaReceipt *schema,
                                   const Theron_V1Stage2RuntimeHandoff *stage3,
                                   const char *cue_hash, const char *track02_hash,
                                   int epoch_matches_stage3,
                                   Theron_V1TraceV3BindingReceipt *out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->status = "trace_v3_full_import_required";
    if (!schema || !schema->valid || !stage3 || !stage3->valid ||
        !cue_hash || !cue_hash[0] || !track02_hash || !track02_hash[0] ||
        !epoch_matches_stage3) return 0;
    out->valid = 1;
    out->source_hashes_bound = 1;
    out->epoch_matches_stage3 = 1;
    return 1;
}

int theron_v1_trace_v3_import_request(const Theron_V1TraceV3BindingReceipt *binding,
                                      const Theron_V1Stage2RuntimeHandoff *stage3,
                                      const char *cue_md5, const char *track02_md5,
                                      Theron_V1TraceV3ImportRequest *out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!binding || !binding->valid || !binding->source_hashes_bound ||
        !binding->epoch_matches_stage3 || !stage3 || !stage3->valid ||
        !stage3->stage3_cd_read_record_proven || !hash32(cue_md5) ||
        !hash32(track02_md5)) return 0;
    snprintf(out->line, sizeof(out->line),
             "trace_v3_import_request cue_md5=%s track02_md5=%s stage3_record=%x "
             "snapshots=pre4090,post4093 epoch=physical_stage2_stage3 runtime_blocked=1",
             cue_md5, track02_md5, stage3->stage3_cd_read_record);
    out->valid = 1;
    out->runtime_blocked = 1;
    return 1;
}

int theron_v1_trace_v3_capture_launch(const Theron_V1TraceV3ImportRequest *request,
                                      const char *cue, const char *track02,
                                      const char *system_card, const char *system_card_md5,
                                      Theron_V1TraceV3CaptureLaunch *out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!request || !request->valid || !request->runtime_blocked ||
        !cue || !cue[0] || !track02 || !track02[0] || !system_card ||
        !system_card[0] || !hash32(system_card_md5)) return 0;
    snprintf(out->config, sizeof(out->config),
             "capture=v3 cue=explicit track02=explicit system_card=explicit "
             "system_card_md5=%s trace=required runtime_blocked=1",
             system_card_md5);
    out->valid = 1;
    out->runtime_blocked = 1;
    return 1;
}
