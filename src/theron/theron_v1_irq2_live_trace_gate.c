#include "theron_v1_irq2_live_trace_gate.h"
#include "theron_v1_stage2_runtime_handoff.h"

#include <stdio.h>
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

static int theron_v1_capture_line(const char *capture, const char *prefix,
                                  const char **out_line, size_t *out_length) {
    const char *cursor = capture;
    const char *found = NULL;
    size_t prefix_length;
    size_t found_length = 0u;

    if (!capture || !prefix || !out_line || !out_length) return 0;
    prefix_length = strlen(prefix);
    while (*cursor) {
        const char *end = strchr(cursor, '\n');
        size_t length = end ? (size_t)(end - cursor) : strlen(cursor);

        if (length >= prefix_length &&
            memcmp(cursor, prefix, prefix_length) == 0) {
            if (found) return 0;
            found = cursor;
            found_length = length;
        }
        if (!end) break;
        cursor = end + 1;
    }
    if (!found) return 0;
    *out_line = found;
    *out_length = found_length;
    return 1;
}

static int theron_v1_capture_exact_line(const char *capture, const char *line) {
    const char *found;
    size_t length;

    return theron_v1_capture_line(capture, line, &found, &length) &&
        length == strlen(line);
}

static int theron_v1_capture_hex4(const char *text) {
    size_t index;

    if (!text) return 0;
    for (index = 0u; index < 4u; ++index) {
        if (!((text[index] >= '0' && text[index] <= '9') ||
              (text[index] >= 'a' && text[index] <= 'f'))) return 0;
    }
    return 1;
}

static int theron_v1_capture_has_post_e98a_transfer(const char *capture) {
    const char *boot;
    const char *transfer;
    const char *next_pc;
    size_t boot_length;
    size_t transfer_length;
    const char *boot_instruction;

    if (!theron_v1_capture_line(capture, "boot_pc=e98a ", &boot,
                                &boot_length) ||
        !theron_v1_capture_line(capture,
            "post_e98a_controller_transfer_source_pc=", &transfer,
            &transfer_length) ||
        boot_length < strlen("boot_pc=e98a ") ||
        transfer_length < strlen(
            "post_e98a_controller_transfer_source_pc=e800")) {
        return 0;
    }
    boot_instruction = strstr(boot, "instruction=LDA $22A4");
    if (!boot_instruction || boot_instruction + strlen("instruction=LDA $22A4") >
            boot + boot_length ||
        transfer[strlen("post_e98a_controller_transfer_source_pc=")] != 'e' ||
        (transfer[strlen("post_e98a_controller_transfer_source_pc=") + 1u] != '8' &&
         transfer[strlen("post_e98a_controller_transfer_source_pc=") + 1u] != '9' &&
         transfer[strlen("post_e98a_controller_transfer_source_pc=") + 1u] != 'a') ||
        !theron_v1_capture_hex4(transfer +
            strlen("post_e98a_controller_transfer_source_pc="))) {
        return 0;
    }
    next_pc = strstr(transfer, "next_pc=");
    return next_pc && next_pc + strlen("next_pc=") + 4u <=
            transfer + transfer_length &&
        theron_v1_capture_hex4(next_pc + strlen("next_pc="));
}

int theron_v1_system_card_controller_wait_from_mednafen_capture(
    const char *capture,
    Theron_V1SystemCardControllerWaitReceipt *out_receipt) {
    const char *command;
    const char *state;
    const char *compare_ready;
    const char *compare_retry;
    const char *retry_branch;
    size_t command_length;
    size_t state_length;
    unsigned int command_1800, response_1801, response_1802, response_1803;
    unsigned int response_1804, controller_state;
    int consumed = 0;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!theron_v1_capture_exact_line(capture,
            "source=mednafen-pce-instrumented") ||
        !theron_v1_capture_line(capture, "post_latch_cd_baseline_pc=c897 ",
                                &command, &command_length) ||
        !theron_v1_capture_line(capture, "c860_window_pc=c8c4 ",
                                &state, &state_length) ||
        !theron_v1_capture_line(capture, "c860_window_pc=c8c7 ",
                                &compare_ready, &(size_t){0}) ||
        !theron_v1_capture_line(capture, "c860_window_pc=c8cb ",
                                &compare_retry, &(size_t){0}) ||
        !theron_v1_capture_line(capture, "c860_window_pc=c8cd ",
                                &retry_branch, &(size_t){0}) ||
        theron_v1_capture_line(capture, "dynamic_cd_read_transaction ",
                               &(const char *){0}, &(size_t){0}) ||
        sscanf(command,
               "post_latch_cd_baseline_pc=c897 cd_1800=%x cd_1801=%x cd_1802=%x cd_1803=%x cd_1804=%x%n",
               &command_1800, &response_1801, &response_1802, &response_1803,
               &response_1804, &consumed) != 5 ||
        consumed != (int)command_length ||
        sscanf(state,
               "c860_window_pc=c8c4 physical_pc=%*x instruction=LDA $222D  @ $222D = $%x%n",
               &controller_state, &consumed) != 1 ||
        consumed <= 0 || (size_t)consumed > state_length || command_1800 != 0xd0u ||
        response_1801 != 0u || response_1802 != 0u || response_1803 != 0x02u ||
        response_1804 != 0u || controller_state != 0u ||
        !strstr(compare_ready, "instruction=CMP #$08") ||
        !strstr(compare_retry, "instruction=CMP #$04") ||
        !strstr(retry_branch, "instruction=BNE $C897")) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->runtime_blocked = 1;
    out_receipt->command_1800 = (uint8_t)command_1800;
    out_receipt->response_1801 = (uint8_t)response_1801;
    out_receipt->response_1802 = (uint8_t)response_1802;
    out_receipt->response_1803 = (uint8_t)response_1803;
    out_receipt->response_1804 = (uint8_t)response_1804;
    out_receipt->controller_state_222d = (uint8_t)controller_state;
    out_receipt->retry_branch_to_c897_observed = 1;
    return 1;
}

int theron_v1_irq2_live_trace_from_mednafen_capture(
    const char *capture,
    Theron_V1Irq2LiveTrace *out_trace) {
    const char *transaction;
    const char *state;
    size_t transaction_length;
    size_t state_length;
    unsigned int pc, return_pc, sector_count, destination, register_mask;
    unsigned int record, state_pc, f5_after, f5_entry, status_1802;
    unsigned int status_1803, f2_before, f2_branch;
    char variant[16];
    int consumed = 0;

    if (!out_trace) return 0;
    memset(out_trace, 0, sizeof(*out_trace));
    if (!theron_v1_capture_exact_line(capture,
            "source=mednafen-pce-instrumented") ||
        !theron_v1_capture_has_post_e98a_transfer(capture) ||
        !theron_v1_capture_line(capture, "dynamic_cd_read_transaction ",
                                &transaction, &transaction_length) ||
        !theron_v1_capture_line(capture,
            "dynamic_cd_read_controller_state ", &state, &state_length) ||
        sscanf(transaction,
               "dynamic_cd_read_transaction pc=%x return_pc=%x sector_count=%x destination=%x record_register_mask=%x variant=%15[a-z_] record=%x%n",
               &pc, &return_pc, &sector_count, &destination, &register_mask,
               variant, &record, &consumed) != 7 ||
        consumed != (int)transaction_length ||
        sscanf(state,
               "dynamic_cd_read_controller_state pc=%x f5_after_cd_read=%x f5_at_irq2_entry=%x status_1802=%x status_1803=%x f2_before_merge=%x f2_at_branch=%x%n",
               &state_pc, &f5_after, &f5_entry, &status_1802, &status_1803,
               &f2_before, &f2_branch, &consumed) != 7 ||
        consumed != (int)state_length || pc != 0x4090u ||
        return_pc != 0x4093u || sector_count != 1u || destination != 0x3800u ||
        register_mask != 0x07u || state_pc != 0xe74cu || record > 0xffffffu ||
        f5_after > 0xffu || f5_entry > 0xffu || status_1802 > 0xffu ||
        status_1803 > 0xffu || f2_before > 0xffu || f2_branch > 0xffu) {
        return 0;
    }
    if (strcmp(variant, "jp_bin") == 0 && record == 0x0004dfu) {
        out_trace->variant = THERON_TRACK02_VARIANT_JP_BIN;
    } else if (strcmp(variant, "us_bin") == 0 && record == 0x0004e0u) {
        out_trace->variant = THERON_TRACK02_VARIANT_US_BIN;
    } else {
        return 0;
    }
    out_trace->magic = THERON_V1_IRQ2_TRACE_MAGIC;
    out_trace->version = THERON_V1_IRQ2_TRACE_VERSION;
    out_trace->source = THERON_V1_IRQ2_TRACE_SOURCE_MEDNAFEN_PCE;
    out_trace->stage3_track02_record = record;
    out_trace->cd_read_return_pc = (uint16_t)return_pc;
    out_trace->irq2_entry_pc = 0xe736u;
    out_trace->cd_state_pc = 0xe742u;
    out_trace->cd_state_branch_pc = (uint16_t)state_pc;
    out_trace->f5_after_cd_read = (uint8_t)f5_after;
    out_trace->f5_at_irq2_entry = (uint8_t)f5_entry;
    out_trace->cd_status_1802 = (uint8_t)status_1802;
    out_trace->cd_status_1803 = (uint8_t)status_1803;
    out_trace->f2_before_merge = (uint8_t)f2_before;
    out_trace->f2_at_branch = (uint8_t)f2_branch;
    return theron_v1_irq2_live_branch_from_trace(out_trace,
                                                  &(Theron_V1Irq2LiveBranchReceipt){0});
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

int theron_v1_irq2_live_branch_from_mednafen_capture_and_full_track02_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5_hex,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    const char *capture,
    Theron_V1Irq2FullMediaTraceReceipt *out_receipt) {
    Theron_V1Irq2LiveTrace trace;

    if (!theron_v1_irq2_live_trace_from_mednafen_capture(capture, &trace)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return theron_v1_irq2_live_branch_from_full_track02_media(
        track02_data, track02_size, track02_md5_hex, system_card_rom,
        system_card_rom_size, system_card_rom_md5_hex, &trace, out_receipt);
}
