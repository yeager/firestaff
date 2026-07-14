#ifndef NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_H
#define NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_H

#include <stddef.h>
#include <stdint.h>

/* Standalone schema gate for externally captured SH-2 PRS3 traces. It does
 * not read assets, decode a stream, or authorize a runtime route. */
#define NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_MAGIC "NEXUS_PRS3_SH2_TRACE_V1"
#define NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_MAGIC "NEXUS_PRS3_SH2_VDP1_TRACE_V1"
#define NEXUS_V1_PRS3_DM_BIN_MAX_MARKERS 16U

typedef enum {
    NEXUS_V1_PRS3_DM_BIN_MARKER_UNCLASSIFIED = 0,
    NEXUS_V1_PRS3_DM_BIN_MARKER_EXECUTABLE_BYTES = 1,
    NEXUS_V1_PRS3_DM_BIN_MARKER_V1_RECORD = 2
} Nexus_V1_Prs3DmBinMarkerKind;

/* A bounded catalog of literal `PRS3` marker locations in verified DM.BIN
 * bytes. A V1 record is only recognized when its observed BE header is fully
 * present: magic, version, declared target byte count, and first frame word.
 * This is framing evidence only, never a compression decoder. */
typedef struct {
    uint32_t offset;
    Nexus_V1_Prs3DmBinMarkerKind kind;
    uint32_t version;
    uint32_t declared_target_bytes;
    uint32_t first_frame_word;
    int header_complete;
} Nexus_V1_Prs3DmBinMarker;

typedef struct {
    int source_hash_verified;
    uint32_t marker_count;
    uint32_t executable_marker_count;
    uint32_t v1_record_count;
    uint32_t truncated_marker_count;
    int complete;
    int decoder_promoted;
    Nexus_V1_Prs3DmBinMarker markers[NEXUS_V1_PRS3_DM_BIN_MAX_MARKERS];
} Nexus_V1_Prs3DmBinCatalogReceipt;

/* Cross-asset receipt for the observed V1 outer PRS3 frame.  DM.BIN and
 * MENU.BPK share only this bounded header evidence today.  In particular,
 * matching magic/version/frame words does not establish command bit order,
 * literal/back-reference semantics, output termination, or a render route. */
typedef struct {
    int dm_bin_hash_verified;
    int menu_bpk_hash_verified;
    uint32_t dm_bin_marker_count;
    uint32_t dm_bin_v1_record_count;
    uint32_t menu_prs3_entry_count;
    uint32_t menu_v1_stream_count;
    uint32_t menu_missing_frame_word_count;
    uint32_t matching_declared_target_count;
    int outer_v1_framing_matches;
    int shared_opcode_grammar_proven;
    int decoder_promoted;
    int menu_handoff_authorized;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3CrossAssetFrameReceipt;

/* Static SH-2 evidence imported from the hash-locked original DM.BIN V1
 * loader route.  It proves selected instruction-level control/input/output
 * facts only.  It has no live MENU.BPK payload binding, no VDP1 command
 * observation, and therefore cannot promote PRS3 decoding. */
typedef struct {
    int source_hash_verified;
    int dm_bin_v1_frame_verified;
    uint32_t v1_callee_offset;
    uint32_t control_test_offset;
    uint32_t stream_byte_read_offset;
    uint32_t output_byte_store_offset;
    uint32_t loop_branch_offset;
    uint16_t control_test_instruction;
    uint16_t stream_byte_read_instruction;
    uint16_t output_byte_store_instruction;
    uint16_t loop_branch_instruction;
    int sh2_control_path_verified;
    int sh2_stream_read_verified;
    int sh2_output_store_verified;
    int menu_frame_binding_proven;
    int vdp1_command_proven;
    int opcode_grammar_proven;
    int decoder_promoted;
} Nexus_V1_Prs3Sh2V1ExecutionReceipt;

/* A future emulator capture may prove one concrete PRS3 frame's path from a
 * verified MENU.BPK span through the selected SH-2 V1 loader into an observed
 * VDP1 texture source. This schema deliberately captures addresses, counts,
 * ordering, and output fingerprint only; it does not encode a PRS3 grammar. */
typedef struct {
    int valid;
    int complete_capture;
    uint64_t menu_bpk_fnv1a64;
    uint64_t dm_bin_fnv1a64;
    uint32_t entry_index;
    uint32_t stream_offset;
    uint32_t stream_size;
    uint32_t expected_output_bytes;
    uint32_t payload_ram_address;
    uint32_t first_input_read_address;
    uint32_t last_input_read_address;
    uint32_t input_read_bytes;
    uint32_t output_ram_address;
    uint32_t first_output_write_address;
    uint32_t last_output_write_address;
    uint32_t output_write_bytes;
    uint64_t output_fnv1a64;
    uint64_t first_opcode_sequence;
    uint64_t first_input_read_sequence;
    uint64_t last_input_read_sequence;
    uint64_t first_output_write_sequence;
    uint64_t last_output_write_sequence;
    uint64_t decoder_return_sequence;
    uint64_t vdp1_command_sequence;
    uint32_t vdp1_command_address;
    uint32_t vdp1_texture_source_address;
    uint32_t vdp1_texture_source_bytes;
    int exact_vdp1_handoff_observed;
    int opcode_grammar_proven;
    int decoder_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3Vdp1CaptureReceipt;

typedef struct {
    int valid;
    int trace_valid;
    int menu_bpk_matches;
    int dm_bin_matches;
    int entry_plan_matches;
    int exact_vdp1_handoff_observed;
    int decoder_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3Vdp1CaptureBindingReceipt;

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

/* Catalogs only literal PRS3 framing markers in hash-verified original
 * DM.BIN bytes. Unknown executable occurrences and truncated records remain
 * unclassified; no catalog result may authorize PRS3 decompression. */
int nexus_v1_prs3_dm_bin_catalog_verified(
    const uint8_t *dm_bin, size_t dm_bin_size, int source_hash_verified,
    Nexus_V1_Prs3DmBinCatalogReceipt *out_receipt);

/* Compare only hash-verified DM.BIN and MENU.BPK V1 outer frames.  A return
 * value of one means all observed MENU PRS3 entries have complete V1 framing
 * alongside at least one complete DM.BIN V1 record.  It never authorizes a
 * decoder or a menu surface handoff; capture-backed opcode evidence is still
 * required for that separate decision. */
int nexus_v1_prs3_cross_asset_frame_receipt_verified(
    const uint8_t *dm_bin, size_t dm_bin_size, int dm_bin_hash_verified,
    const uint8_t *menu_bpk, size_t menu_bpk_size, int menu_bpk_hash_verified,
    Nexus_V1_Prs3CrossAssetFrameReceipt *out_receipt);

/* Import exact, read-only SH-2 instruction evidence from the selected V1
 * route in hash-verified original DM.BIN.  This validates the control test,
 * bounded R12 post-increment byte read, R13/R0 byte store, and loop branch
 * used by the existing provenance probe.  It deliberately does not claim
 * that any observed byte is a PRS3 opcode or a MENU.BPK frame. */
int nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
    const uint8_t *dm_bin, size_t dm_bin_size, int source_hash_verified,
    Nexus_V1_Prs3Sh2V1ExecutionReceipt *out_receipt);

/* Parse one complete SH-2-to-VDP1 capture for a single frame. The parser
 * rejects incomplete ranges or invalid ordering. A valid capture remains
 * evidence-only until an independently reviewed opcode grammar exists. */
int nexus_v1_prs3_vdp1_capture_schema_parse(
    const char *text, size_t text_size,
    Nexus_V1_Prs3Vdp1CaptureReceipt *out_receipt);

/* Bind a parsed capture to exact MENU.BPK/DM.BIN bytes and its bounded PRS3
 * stream plan. This does not enable generic decompression or fallback art. */
int nexus_v1_prs3_vdp1_capture_schema_bind_assets(
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size,
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt *out_receipt);

#endif
