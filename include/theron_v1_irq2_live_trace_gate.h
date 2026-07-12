#ifndef THERON_V1_IRQ2_LIVE_TRACE_GATE_H
#define THERON_V1_IRQ2_LIVE_TRACE_GATE_H

#include "theron_v1_track02.h"

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

/* Rejects unless a complete original Mednafen PCE trace proves every state
 * boundary from CD_READ return through the first IRQ2 CD-state branch. */
int theron_v1_irq2_live_branch_from_trace(
    const Theron_V1Irq2LiveTrace *trace,
    Theron_V1Irq2LiveBranchReceipt *out_receipt);

#endif /* THERON_V1_IRQ2_LIVE_TRACE_GATE_H */
