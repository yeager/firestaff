#ifndef THERON_V1_IRQ2_LIVE_TRACE_GATE_H
#define THERON_V1_IRQ2_LIVE_TRACE_GATE_H

#include "theron_v1_track02.h"
#include "theron_v1_system_card_irq2_cd_state_gate.h"

#include <stddef.h>
#include <stdint.h>

/*
 * A live emulator trace is required before the System Card IRQ2 branch can
 * be selected. This contract intentionally rejects media-only observations
 * and incomplete snapshots.
 */

typedef enum {
    THERON_V1_IRQ2_TRACE_SOURCE_NONE = 0,
    THERON_V1_IRQ2_TRACE_SOURCE_MEDNAFEN_PCE = 1
} Theron_V1Irq2TraceSource;

typedef struct {
    uint32_t magic;
    uint16_t version;
    Theron_V1Irq2TraceSource source;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint16_t cd_read_return_pc;
    uint16_t irq2_entry_pc;
    uint16_t cd_state_pc;
    uint16_t cd_state_branch_pc;
    uint8_t f5_after_cd_read;
    uint8_t f5_at_irq2_entry;
    uint8_t cd_status_1802;
    uint8_t cd_status_1803;
    uint8_t f2_before_merge;
    uint8_t f2_at_branch;
} Theron_V1Irq2LiveTrace;

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint8_t f5_at_irq2_entry;
    uint8_t merged_f2;
    int entry_indirect_branch_selected;
    int cd_state_branch_to_e7b3_selected;
} Theron_V1Irq2LiveBranchReceipt;

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint16_t handler_address;
    uint16_t cd_state_address;
    uint16_t cd_state_branch_address;
    uint8_t f5_after_cd_read;
    uint8_t f5_at_irq2_entry;
    uint8_t cd_status_1802;
    uint8_t cd_status_1803;
    uint8_t f2_before_merge;
    uint8_t f2_at_branch;
    Theron_V1Irq2LiveBranchReceipt branch;
} Theron_V1Irq2FullMediaTraceReceipt;

/* Redacted capture-preflight status only. A ready preflight never selects an
 * IRQ2 path; a complete live trace remains mandatory. */
typedef enum {
    THERON_V1_IRQ2_PREFLIGHT_MALFORMED = 0,
    THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_MISSING,
    THERON_V1_IRQ2_PREFLIGHT_SYSTEM_CARD_HASH_MISMATCH,
    THERON_V1_IRQ2_PREFLIGHT_SCHEMA_INVALID,
    THERON_V1_IRQ2_PREFLIGHT_IPL_STAGE2_UNVERIFIED,
    THERON_V1_IRQ2_PREFLIGHT_STAGE3_STATIC_UNVERIFIED,
    THERON_V1_IRQ2_PREFLIGHT_STAGE2_STAGE3_INVALID,
    THERON_V1_IRQ2_PREFLIGHT_COMPLETION_INVALID,
    THERON_V1_IRQ2_PREFLIGHT_TRACE_MISSING,
    THERON_V1_IRQ2_PREFLIGHT_TRACE_INVALID,
    THERON_V1_IRQ2_PREFLIGHT_READY
} Theron_V1Irq2PreflightStatus;

typedef struct {
    Theron_V1Irq2PreflightStatus status;
    int redacted_receipt_valid;
    int runtime_allowed;
} Theron_V1Irq2PreflightDiagnostic;

typedef struct {
    uint32_t magic;
    uint16_t version;
    Theron_V1Irq2PreflightStatus status;
    int redacted_receipt_valid;
    int runtime_blocked;
} Theron_V1Irq2PreflightLauncherReceipt;

int theron_v1_irq2_preflight_diagnostic_from_redacted_receipt(
    const char *receipt,
    Theron_V1Irq2PreflightDiagnostic *out_diagnostic);
const char *theron_v1_irq2_preflight_status_name(
    Theron_V1Irq2PreflightStatus status);
int theron_v1_irq2_preflight_launcher_receipt_from_redacted_receipt(
    const char *receipt,
    Theron_V1Irq2PreflightLauncherReceipt *out_receipt);

/* Rejects unless a complete original Mednafen PCE trace proves every state
 * boundary from CD_READ return through the first IRQ2 CD-state branch. */
int theron_v1_irq2_live_branch_from_trace(
    const Theron_V1Irq2LiveTrace *trace,
    Theron_V1Irq2LiveBranchReceipt *out_receipt);

/* Requires the complete authenticated Track 02/System Card chain and a live
 * Mednafen trace. Media-only inputs and partial register snapshots reject. */
int theron_v1_irq2_live_branch_from_full_track02_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5_hex,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    const Theron_V1Irq2LiveTrace *trace,
    Theron_V1Irq2FullMediaTraceReceipt *out_receipt);

#endif /* THERON_V1_IRQ2_LIVE_TRACE_GATE_H */
