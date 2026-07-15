#include "theron_v1_trace_v3_schema.h"

#include <stdio.h>
#include <string.h>

static int tqr_nonempty(const char *text) {
    return text && text[0] != '\0';
}

static int tqr_md5ish(const char *text) {
    return tqr_nonempty(text) && strlen(text) == 32u;
}

static int tqr_safe_arg(const char *text) {
    return tqr_nonempty(text) && !strchr(text, '\n') && !strchr(text, '\r');
}

const char *theron_v1_trace_v3_preflight_status(const char *system_card_path,
                                                const char *system_card_hash,
                                                int trace_validated) {
    if (!tqr_nonempty(system_card_path)) {
        return "system_card_missing";
    }
    if (!tqr_md5ish(system_card_hash)) {
        return "system_card_hash_mismatch";
    }
    return trace_validated ? "trace_ready" : "trace_capture_required";
}

const char *theron_v1_trace_v3_raw_track02_status(const char *track02_md5,
                                                  const char *system_card_path) {
    if (!tqr_nonempty(track02_md5)) {
        return "raw_track02_missing";
    }
    if (!tqr_nonempty(system_card_path)) {
        return "system_card_missing";
    }
    if (strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) == 0 ||
        strcmp(track02_md5, THERON_TRACK02_MD5_JP_BIN) == 0) {
        return "raw_track02_ready";
    }
    if (strcmp(track02_md5, THERON_TRACK02_MD5_US_ISO) == 0 ||
        strcmp(track02_md5, THERON_TRACK02_MD5_JP_REV1_ISO) == 0) {
        return "raw_track02_iso_end_variant";
    }
    return "raw_track02_missing";
}

int theron_v1_trace_v3_schema_validate(
    const char *text,
    Theron_V1TraceV3SchemaReceipt *out_receipt) {
    Theron_V1TraceV3SchemaReceipt receipt = {0};

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!text || !out_receipt ||
        !strstr(text, "trace_format=theron_irq2_capture_v3\n") ||
        strstr(text, "trace_format=theron_irq2_capture_v2")) {
        return 0;
    }

    receipt.has_pre4090 =
        strstr(text, "loader_registers phase=pre4090 fc=00\n") != NULL;
    receipt.has_post4093 =
        strstr(text, "loader_registers phase=post4093 fc=00\n") != NULL;
    receipt.has_pcecd_epoch =
        strstr(text, "pcecd_epoch transfer_epoch=physical_stage2_stage3 ") !=
        NULL;
    receipt.valid = receipt.has_pre4090 && receipt.has_post4093 &&
        receipt.has_pcecd_epoch;
    *out_receipt = receipt;
    return receipt.valid;
}

int theron_v1_trace_v3_bind_stage3(
    const Theron_V1TraceV3SchemaReceipt *schema,
    const Theron_V1Stage2RuntimeHandoff *stage3,
    const char *cue_hash,
    const char *track02_hash,
    int pcecd_epoch_bound,
    Theron_V1TraceV3BindingReceipt *out_receipt) {
    Theron_V1TraceV3BindingReceipt receipt = {0};

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!schema || !schema->valid || !stage3 || !stage3->valid ||
        !stage3->stage3_cd_read_record_proven ||
        stage3->stage3_cd_read_record != 0x4dfu ||
        !tqr_nonempty(cue_hash) || !tqr_nonempty(track02_hash) ||
        !pcecd_epoch_bound || !out_receipt) {
        return 0;
    }

    receipt.valid = 1;
    receipt.runtime_allowed = 0;
    receipt.source_hashes_bound = 1;
    receipt.epoch_matches_stage3 = 1;
    *out_receipt = receipt;
    return 1;
}

int theron_v1_trace_v3_import_request(
    const Theron_V1TraceV3BindingReceipt *binding,
    const Theron_V1Stage2RuntimeHandoff *stage3,
    const char *cue_hash,
    const char *track02_hash,
    Theron_V1TraceV3ImportRequest *out_request) {
    Theron_V1TraceV3ImportRequest request = {0};

    if (out_request) {
        memset(out_request, 0, sizeof(*out_request));
    }
    if (!binding || !binding->valid || binding->runtime_allowed ||
        !binding->source_hashes_bound || !binding->epoch_matches_stage3 ||
        !stage3 || !stage3->valid || !tqr_md5ish(cue_hash) ||
        !tqr_md5ish(track02_hash) || !out_request) {
        return 0;
    }

    request.valid = 1;
    request.runtime_blocked = 1;
    *out_request = request;
    return 1;
}

int theron_v1_trace_v3_capture_launch(
    const Theron_V1TraceV3ImportRequest *request,
    const char *cue_path,
    const char *track02_path,
    const char *system_card_path,
    const char *system_card_hash,
    Theron_V1TraceV3CaptureLaunch *out_launch) {
    Theron_V1TraceV3CaptureLaunch launch = {0};

    if (out_launch) {
        memset(out_launch, 0, sizeof(*out_launch));
    }
    if (!request || !request->valid || !request->runtime_blocked ||
        !tqr_safe_arg(cue_path) || !tqr_safe_arg(track02_path) ||
        !tqr_safe_arg(system_card_path) || !tqr_md5ish(system_card_hash) ||
        !out_launch) {
        return 0;
    }

    launch.valid = 1;
    launch.runtime_blocked = 1;
    launch.cue_path = cue_path;
    launch.track02_path = track02_path;
    launch.system_card_path = system_card_path;
    launch.system_card_hash = system_card_hash;
    *out_launch = launch;
    return 1;
}

int theron_v1_trace_v3_format_command(
    const Theron_V1TraceV3CaptureLaunch *launch,
    const char *mednafen_path,
    const char *cue_path,
    const char *track02_path,
    const char *system_card_path,
    const char *trace_path,
    char *out_command,
    size_t out_command_cap) {
    int written;

    if (out_command && out_command_cap > 0u) {
        out_command[0] = '\0';
    }
    if (!launch || !launch->valid || !launch->runtime_blocked ||
        !tqr_safe_arg(mednafen_path) || !tqr_safe_arg(cue_path) ||
        !tqr_safe_arg(track02_path) || !tqr_safe_arg(system_card_path) ||
        !tqr_safe_arg(trace_path) || !out_command || out_command_cap == 0u) {
        return 0;
    }

    written = snprintf(out_command, out_command_cap,
                       "FIRESTAFF_THERON_IRQ2_TRACE=%s "
                       "FIRESTAFF_THERON_TRACK02=%s "
                       "FIRESTAFF_THERON_SYSTEM_CARD=%s %s %s",
                       trace_path, track02_path, system_card_path,
                       mednafen_path, cue_path);
    return written > 0 && (size_t)written < out_command_cap;
}

int theron_v1_trace_v3_plan_command(
    const Theron_V1TraceV3ImportRequest *request,
    const Theron_V1TraceV3CaptureLaunch *launch,
    const char *mednafen_path,
    const char *cue_path,
    const char *track02_path,
    const char *system_card_path,
    const char *trace_path,
    Theron_V1TraceV3CommandPlan *out_plan) {
    Theron_V1TraceV3CommandPlan plan = {0};

    if (out_plan) {
        memset(out_plan, 0, sizeof(*out_plan));
    }
    if (!request || !request->valid || !request->runtime_blocked || !launch ||
        !launch->valid || !launch->runtime_blocked || !out_plan ||
        !theron_v1_trace_v3_format_command(launch, mednafen_path, cue_path,
                                           track02_path, system_card_path,
                                           trace_path, plan.command,
                                           sizeof(plan.command))) {
        return 0;
    }

    plan.valid = 1;
    plan.runtime_blocked = 1;
    *out_plan = plan;
    return 1;
}

int theron_v1_trace_v3_plan_command_with_media(
    const Theron_V1_TrackMediaAvailabilityReceipt *media,
    const Theron_V1TraceV3ImportRequest *request,
    const Theron_V1TraceV3CaptureLaunch *launch,
    const char *mednafen_path,
    const char *cue_path,
    const char *track02_path,
    const char *system_card_path,
    const char *trace_path,
    Theron_V1TraceV3CommandPlan *out_plan) {
    if (out_plan) {
        memset(out_plan, 0, sizeof(*out_plan));
    }
    if (!media || media->availability != THERON_V1_TRACK_MEDIA_RAW_READY ||
        !media->loader_usable) {
        return 0;
    }
    return theron_v1_trace_v3_plan_command(request, launch, mednafen_path,
                                           cue_path, track02_path,
                                           system_card_path, trace_path,
                                           out_plan);
}

int theron_v1_trace_v3_launcher_receipt(
    const Theron_V1TraceV3CommandPlan *plan,
    Theron_V1TraceV3LauncherReceipt *out_receipt) {
    Theron_V1TraceV3LauncherReceipt receipt = {0};

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!plan || !plan->valid || !plan->runtime_blocked || !out_receipt) {
        return 0;
    }

    receipt.capture_command_ready = 1;
    receipt.trace_validated = 0;
    receipt.runtime_blocked = 1;
    *out_receipt = receipt;
    return 1;
}

Theron_V1Irq2PreflightStatus theron_v1_trace_v3_preflight_adapter(
    const Theron_V1TraceV3LauncherReceipt *launcher,
    const char *status) {
    if (status && strcmp(status, "system_card_missing") == 0) {
        return THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_MISSING;
    }
    if (!launcher || !launcher->capture_command_ready || !status ||
        strcmp(status, "trace_capture_required") == 0) {
        return THERON_V1_IRQ2_PREFLIGHT_TRACE_INVALID;
    }
    return THERON_V1_IRQ2_PREFLIGHT_OK;
}
