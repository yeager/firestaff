#include "theron_v1_irq2_live_trace_gate.h"

#include <string.h>

#define THERON_V1_IRQ2_TRACE_MAGIC 0x54514932u /* TQI2 */
#define THERON_V1_IRQ2_TRACE_VERSION 1u

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
