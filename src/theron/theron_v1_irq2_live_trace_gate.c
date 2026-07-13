#include "theron_v1_irq2_live_trace_gate.h"
#include "theron_v1_stage2_runtime_handoff.h"

#include <string.h>

#define THERON_V1_IRQ2_TRACE_MAGIC 0x54514932u /* TQI2 */
#define THERON_V1_IRQ2_TRACE_VERSION 1u
#define THERON_V1_IRQ2_PREFLIGHT_LAUNCHER_MAGIC 0x54515046u /* TQPF */
#define THERON_V1_IRQ2_PREFLIGHT_LAUNCHER_VERSION 1u

static int has_redacted_token(const char *receipt, const char *token) {
    const size_t token_length = strlen(token);
    const char *cursor = receipt;

    if (!receipt || !token || token_length == 0u) return 0;
    while ((cursor = strstr(cursor, token)) != NULL) {
        const char before = cursor == receipt ? ' ' : cursor[-1];
        const char after = cursor[token_length];
        if ((before == ' ' || before == '\n') &&
            (after == '\0' || after == ' ' || after == '\n')) return 1;
        cursor += token_length;
    }
    return 0;
}

const char *theron_v1_irq2_preflight_status_name(
    Theron_V1Irq2PreflightStatus status) {
    switch (status) {
    case THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_MISSING: return "system_card_missing";
    case THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_HASH_MISMATCH: return "system_card_hash_mismatch";
    case THERON_V1_IRQ2_PREFLIGHT_SCHEMA_INVALID: return "capture_schema_invalid";
    case THERON_V1_IRQ2_PREFLIGHT_IPL_STAGE2_UNVERIFIED: return "ipl_stage2_unverified";
    case THERON_V1_IRQ2_PREFLIGHT_STAGE3_STATIC_UNVERIFIED: return "stage3_static_unverified";
    case THERON_V1_IRQ2_PREFLIGHT_STAGE2_STAGE3_INVALID: return "stage2_stage3_transfer_invalid";
    case THERON_V1_IRQ2_PREFLIGHT_COMPLETION_INVALID: return "trace_completion_invalid";
    case THERON_V1_IRQ2_PREFLIGHT_TRACE_MISSING: return "trace_missing";
    case THERON_V1_IRQ2_PREFLIGHT_TRACE_INVALID: return "trace_invalid";
    case THERON_V1_IRQ2_PREFLIGHT_READY: return "ready";
    default: return "malformed";
    }
}

int theron_v1_irq2_preflight_diagnostic_from_redacted_receipt(
    const char *receipt,
    Theron_V1Irq2PreflightDiagnostic *out_diagnostic) {
    Theron_V1Irq2PreflightStatus status = THERON_V1_IRQ2_PREFLIGHT_MALFORMED;

    if (!out_diagnostic) return 0;
    memset(out_diagnostic, 0, sizeof(*out_diagnostic));
    if (!receipt || !has_redacted_token(receipt, "PREFLIGHT") ||
        !has_redacted_token(receipt,
            "system_card_contract=explicit_path_hash_locked")) return 0;
    if (has_redacted_token(receipt, "status=blocked") &&
        has_redacted_token(receipt, "missing_reason=system_card_missing") &&
        has_redacted_token(receipt, "system_card=missing")) {
        status = THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_MISSING;
    } else if (has_redacted_token(receipt, "status=blocked") &&
               has_redacted_token(receipt, "missing_reason=system_card_hash_mismatch") &&
               has_redacted_token(receipt, "system_card=hash_mismatch")) {
        status = THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_HASH_MISMATCH;
    } else if (has_redacted_token(receipt, "status=blocked") &&
               has_redacted_token(receipt, "missing_reason=ipl_stage2_unverified") &&
               has_redacted_token(receipt, "ipl_stage2=unverified")) {
        status = THERON_V1_IRQ2_PREFLIGHT_IPL_STAGE2_UNVERIFIED;
    } else if (has_redacted_token(receipt, "status=blocked") &&
               has_redacted_token(receipt, "missing_reason=stage3_static_unverified") &&
               has_redacted_token(receipt, "stage3_static=unverified")) {
        status = THERON_V1_IRQ2_PREFLIGHT_STAGE3_STATIC_UNVERIFIED;
    } else if (has_redacted_token(receipt, "status=blocked") &&
               has_redacted_token(receipt, "missing_reason=stage2_stage3_transfer_invalid") &&
               has_redacted_token(receipt, "stage2_stage3_transfer=invalid")) {
        status = THERON_V1_IRQ2_PREFLIGHT_STAGE2_STAGE3_INVALID;
    } else if (has_redacted_token(receipt, "status=blocked") &&
               (has_redacted_token(receipt, "missing_reason=trace_completion_missing") ||
                has_redacted_token(receipt, "missing_reason=trace_completion_invalid") ||
                has_redacted_token(receipt, "missing_reason=extra_event_before_completion")) &&
               has_redacted_token(receipt, "trace_completion=invalid")) {
        status = THERON_V1_IRQ2_PREFLIGHT_COMPLETION_INVALID;
    } else if (has_redacted_token(receipt, "status=blocked") &&
               (has_redacted_token(receipt, "missing_reason=capture_schema_invalid") ||
                has_redacted_token(receipt, "missing_reason=trace_schema_invalid")) &&
               has_redacted_token(receipt, "capture_schema=invalid")) {
        status = THERON_V1_IRQ2_PREFLIGHT_SCHEMA_INVALID;
    } else if (has_redacted_token(receipt, "status=blocked") &&
               has_redacted_token(receipt, "missing_reason=trace_missing") &&
               has_redacted_token(receipt, "trace=missing")) {
        status = THERON_V1_IRQ2_PREFLIGHT_TRACE_MISSING;
    } else if (has_redacted_token(receipt, "status=blocked") &&
               has_redacted_token(receipt, "trace_format=invalid")) {
        status = THERON_V1_IRQ2_PREFLIGHT_TRACE_INVALID;
    } else if (has_redacted_token(receipt, "status=ready") &&
               has_redacted_token(receipt, "trace_fields=complete") &&
               has_redacted_token(receipt, "trace_completion=complete") &&
               has_redacted_token(receipt, "capture_schema=v2-bound")) {
        status = THERON_V1_IRQ2_PREFLIGHT_READY;
    } else {
        return 0;
    }
    out_diagnostic->status = status;
    out_diagnostic->redacted_receipt_valid = 1;
    return 1;
}

int theron_v1_irq2_preflight_launcher_receipt_from_redacted_receipt(
    const char *receipt,
    Theron_V1Irq2PreflightLauncherReceipt *out_receipt) {
    Theron_V1Irq2PreflightDiagnostic diagnostic;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!theron_v1_irq2_preflight_diagnostic_from_redacted_receipt(
            receipt, &diagnostic) || !diagnostic.redacted_receipt_valid ||
        diagnostic.runtime_allowed) return 0;
    out_receipt->magic = THERON_V1_IRQ2_PREFLIGHT_LAUNCHER_MAGIC;
    out_receipt->version = THERON_V1_IRQ2_PREFLIGHT_LAUNCHER_VERSION;
    out_receipt->status = diagnostic.status;
    out_receipt->redacted_receipt_valid = 1;
    out_receipt->runtime_blocked = 1;
    return 1;
}

int theron_v1_irq2_live_branch_from_trace(
    const Theron_V1Irq2LiveTrace *trace,
    Theron_V1Irq2LiveBranchReceipt *out_receipt) {
    uint32_t expected_record;
    uint8_t merged_f2;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!trace || trace->magic != THERON_V1_IRQ2_TRACE_MAGIC ||
        trace->version != THERON_V1_IRQ2_TRACE_VERSION ||
        trace->source != THERON_V1_IRQ2_TRACE_SOURCE_MEDNAFEN_PCE ||
        (trace->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         trace->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        trace->cd_read_return_pc != 0x4093u || trace->irq2_entry_pc != 0xe736u ||
        trace->cd_state_pc != 0xe742u || trace->cd_state_branch_pc != 0xe74cu ||
        trace->f5_after_cd_read != trace->f5_at_irq2_entry) {
        return 0;
    }

    expected_record = trace->variant == THERON_TRACK02_VARIANT_JP_BIN ?
        THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP :
        THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US;
    if (trace->stage3_track02_record != expected_record) return 0;

    merged_f2 = (uint8_t)((trace->cd_status_1802 & trace->cd_status_1803) |
                          trace->f2_before_merge);
    if (trace->f2_at_branch != merged_f2) return 0;

    out_receipt->valid = 1;
    out_receipt->variant = trace->variant;
    out_receipt->stage3_track02_record = trace->stage3_track02_record;
    out_receipt->f5_at_irq2_entry = trace->f5_at_irq2_entry;
    out_receipt->merged_f2 = merged_f2;
    out_receipt->entry_indirect_branch_selected =
        (trace->f5_at_irq2_entry & 0x01u) != 0u;
    out_receipt->cd_state_branch_to_e7b3_selected =
        (merged_f2 & 0x04u) == 0u;
    return 1;
}

int theron_v1_irq2_live_branch_from_full_track02_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5_hex,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    const Theron_V1Irq2LiveTrace *trace,
    Theron_V1Irq2FullMediaTraceReceipt *out_receipt) {
    Theron_V1Stage2RuntimeHandoff handoff;
    Theron_V1SystemCardIrq2CdStateGate state_gate;
    Theron_V1Irq2LiveBranchReceipt branch;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&handoff, 0, sizeof(handoff));
    memset(&state_gate, 0, sizeof(state_gate));
    memset(&branch, 0, sizeof(branch));
    if (!theron_v1_stage2_runtime_handoff_from_original_media(
            track02_data, track02_size, track02_md5_hex, &handoff) ||
        !handoff.physical_stage3_entry_verified ||
        !handoff.stage3_mode1_header_verified ||
        !handoff.stage2_cd_read_setup_verified ||
        !handoff.stage2_post_read_transfer_verified ||
        !theron_v1_system_card_irq2_cd_state_gate_from_full_track02_media(
            track02_data, track02_size, track02_md5_hex,
            system_card_rom, system_card_rom_size, system_card_rom_md5_hex,
            &state_gate) ||
        !state_gate.valid ||
        !theron_v1_irq2_live_branch_from_trace(trace, &branch) ||
        branch.variant != handoff.variant ||
        branch.stage3_track02_record != handoff.track02_record ||
        state_gate.variant != handoff.variant ||
        state_gate.stage3_track02_record != handoff.track02_record ||
        !trace) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->variant = handoff.variant;
    out_receipt->stage3_track02_record = handoff.track02_record;
    out_receipt->handler_address = state_gate.handler_address;
    out_receipt->cd_state_address = state_gate.clear_path_address;
    out_receipt->cd_state_branch_address = trace->cd_state_branch_pc;
    out_receipt->f5_after_cd_read = trace->f5_after_cd_read;
    out_receipt->f5_at_irq2_entry = trace->f5_at_irq2_entry;
    out_receipt->cd_status_1802 = trace->cd_status_1802;
    out_receipt->cd_status_1803 = trace->cd_status_1803;
    out_receipt->f2_before_merge = trace->f2_before_merge;
    out_receipt->f2_at_branch = trace->f2_at_branch;
    out_receipt->branch = branch;
    return 1;
}
