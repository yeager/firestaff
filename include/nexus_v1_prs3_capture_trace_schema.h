#ifndef NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_H
#define NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_H

#include <stddef.h>
#include <stdint.h>

/* Standalone schema gate for externally captured SH-2 PRS3 traces. It does
 * not read assets, decode a stream, or authorize a runtime route. */
#define NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_MAGIC "NEXUS_PRS3_SH2_TRACE_V1"

typedef struct {
    int valid;
    int complete_evidence;
    int decoder_promotion_eligible;
    uint64_t menu_bpk_fnv1a64;
    uint64_t dm_bin_fnv1a64;
    uint32_t entry_index;
    uint32_t stream_offset;
    uint32_t stream_size;
    uint32_t expected_output_bytes;
    uint32_t first_opcode_pc;
    uint32_t last_opcode_pc;
    uint64_t opcode_first_sequence;
    uint64_t opcode_last_sequence;
    uint64_t payload_first_read_sequence;
    uint64_t payload_last_read_sequence;
    uint64_t decoder_return_sequence;
    uint64_t capture_completion_sequence;
    uint32_t opcode_fetch_count;
    uint32_t payload_read_bytes;
    uint32_t output_write_bytes;
    uint64_t output_fnv1a64;
} Nexus_V1_Prs3CaptureTraceSchemaReceipt;

/* Asset-bound result for a captured PRS3 execution.  A syntactically valid
 * trace alone is not enough: the entry window must agree with the supplied
 * BPK bytes and both capture fingerprints must agree with the supplied
 * original assets.  This remains evidence only; it never enables decoding. */
typedef struct {
    int valid;
    int trace_valid;
    int menu_bpk_matches;
    int dm_bin_matches;
    int entry_plan_matches;
    int asset_bound_capture;
    int decoder_promotion_eligible;
} Nexus_V1_Prs3CaptureAssetBindingReceipt;

int nexus_v1_prs3_capture_trace_schema_parse(
    const char *text, size_t text_size,
    Nexus_V1_Prs3CaptureTraceSchemaReceipt *out_receipt);

/* Bind a parsed trace to the exact source bytes it claims to consume.  The
 * MENU.BPK entry must be a bounded PRS3 stream whose offset, size, and
 * declared output size all match the capture.  DM.BIN is fingerprinted as a
 * whole because the current trace schema has no proven executable segment
 * grammar.  Returns one for a complete asset-bound evidence receipt, zero
 * otherwise.  No failure permits a substitute route. */
int nexus_v1_prs3_capture_trace_schema_bind_assets(
    const Nexus_V1_Prs3CaptureTraceSchemaReceipt *trace,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size,
    Nexus_V1_Prs3CaptureAssetBindingReceipt *out_receipt);

#endif
