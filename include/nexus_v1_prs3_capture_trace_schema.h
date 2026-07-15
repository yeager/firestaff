#ifndef NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_H
#define NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_dungeon.h"

/* Standalone schema gate for externally captured SH-2 PRS3 traces. It does
 * not read assets, decode a stream, or authorize a runtime route. */
#define NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_MAGIC "NEXUS_PRS3_SH2_TRACE_V1"
#define NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_MAGIC "NEXUS_PRS3_SH2_VDP1_TRACE_V1"
#define NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V2_MAGIC "NEXUS_PRS3_SH2_VDP1_TRACE_V2"
#define NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V3_MAGIC "NEXUS_PRS3_SH2_VDP1_TRACE_V3"
#define NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V4_MAGIC "NEXUS_PRS3_SH2_VDP1_TRACE_V4"
#define NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V5_MAGIC "NEXUS_PRS3_SH2_VDP1_TRACE_V5"
#define NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V6_MAGIC "NEXUS_PRS3_SH2_VDP1_TRACE_V6"
#define NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V7_MAGIC "NEXUS_PRS3_SH2_VDP1_TRACE_V7"
#define NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V8_MAGIC "NEXUS_PRS3_SH2_VDP1_TRACE_V8"
#define NEXUS_V1_PRS3_VDP1_PRODUCER_ATTESTATION_MAGIC \
    "NEXUS_PRS3_V3_PRODUCER_ATTESTATION_V1"
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
    uint32_t loop_body_start_offset;
    uint32_t loop_body_byte_count;
    uint32_t loop_back_target_offset;
    uint64_t loop_body_fnv1a64;
    uint16_t control_test_instruction;
    uint16_t stream_byte_read_instruction;
    uint16_t output_byte_store_instruction;
    uint16_t loop_branch_instruction;
    int sh2_control_path_verified;
    int sh2_stream_read_verified;
    int sh2_output_store_verified;
    uint32_t output_index_copy_offset;
    int sh2_output_store_predecessor_verified;
    int sh2_loop_back_target_verified;
    int sh2_loop_body_bound;
    uint32_t control_reentry_offset;
    uint32_t control_shift_offset;
    uint32_t control_refill_test_offset;
    uint32_t control_refill_skip_branch_offset;
    uint32_t control_refill_guard_offset;
    uint32_t control_refill_failure_branch_offset;
    uint32_t control_refill_byte_read_offset;
    uint32_t control_refill_merge_offset;
    uint32_t terminal_refill_guard_target_offset;
    uint32_t terminal_nonzero_guard_target_offset;
    uint32_t terminal_zero_guard_target_offset;
    uint32_t terminal_result_offset;
    uint16_t terminal_result_instruction;
    uint32_t terminal_return_offset;
    uint32_t control_low_bit_test_offset;
    uint32_t control_zero_branch_offset;
    uint32_t control_zero_branch_target_offset;
    uint32_t control_sentinel_literal_offset;
    uint16_t control_sentinel_word;
    uint16_t control_low_bit_mask;
    int sh2_control_refill_verified;
    /* Three source-counter guards share one epilogue that writes zero to R0
     * before RTS. This identifies a static failure-shaped terminal path, not
     * a successful-stream terminator or decoder result contract. */
    int sh2_terminal_failure_path_proven;
    /* Exact SH-2 semantics for the nonzero low-bit fallthrough. This proves
     * a guarded source-byte-to-output-byte path only, not a complete PRS3
     * token grammar, output buffer ownership, or drawable pixels. */
    int sh2_control_low_bit_semantics_proven;
    int sh2_nonzero_direct_byte_path_proven;
    uint32_t nonzero_post_store_r6_increment_offset;
    int32_t nonzero_post_store_r6_increment;
    uint32_t nonzero_post_store_r6_mask_offset;
    uint32_t nonzero_post_store_r6_mask_source_register;
    uint32_t nonzero_post_store_r6_mask_destination_register;
    uint32_t nonzero_control_reentry_branch_offset;
    uint32_t nonzero_control_reentry_target_offset;
    /* The nonzero store has a fixed post-store R6 update/mask corridor and
     * returns to the shared control block. This is not a literal-token or
     * output-buffer semantic claim. */
    int sh2_nonzero_output_commit_reentry_proven;
    uint32_t nonzero_source_counter_decrement_offset;
    int32_t nonzero_source_counter_delta;
    uint32_t zero_source_counter_decrement_offset;
    int32_t zero_source_counter_delta;
    /* The low-bit branches debit the source counter by different exact
     * amounts before their direct reads. This proves source consumption only,
     * never a PRS3 token class, byte field, or output/pixel behavior. */
    int sh2_control_dependent_source_consumption_proven;
    uint32_t zero_first_byte_read_offset;
    uint32_t zero_second_byte_read_offset;
    uint32_t zero_sequential_input_byte_count;
    uint32_t zero_upper_mask_literal_load_offset;
    uint32_t zero_upper_mask_literal_offset;
    uint16_t zero_upper_mask_word;
    uint32_t zero_second_byte_copy_offset;
    uint32_t zero_first_shift_offset;
    uint32_t zero_second_shift_offset;
    uint32_t zero_upper_mask_and_offset;
    uint32_t zero_merge_or_offset;
    uint32_t zero_low_mask_load_offset;
    int32_t zero_low_mask_immediate;
    uint32_t zero_low_mask_and_offset;
    uint32_t zero_low_fragment_increment_offset;
    int32_t zero_low_fragment_increment;
    uint32_t zero_merged_value_add_offset;
    uint32_t zero_merged_branch_compare_offset;
    uint32_t zero_merged_branch_offset;
    uint32_t zero_merged_branch_target_offset;
    uint32_t zero_index_mask_literal_offset;
    uint32_t zero_index_mask_word;
    uint32_t zero_index_mask_offset;
    uint32_t zero_indexed_byte_read_offset;
    uint32_t zero_indexed_byte_base_register;
    uint32_t zero_indexed_byte_index_register;
    uint32_t zero_indexed_byte_destination_register;
    uint32_t zero_first_value_compare_offset;
    uint32_t zero_first_value_compare_source_register;
    uint32_t zero_first_value_compare_destination_register;
    uint32_t zero_repeat_value_compare_source_register;
    uint32_t zero_repeat_value_compare_destination_register;
    int sh2_zero_side_index_read_verified;
    /* Two adjacent `MOV.B @R12+` instructions consume a bounded sequential
     * source span before the zero-side merge. This assigns no field meaning
     * to either byte and does not make the span a decoder input contract. */
    int sh2_zero_side_two_byte_input_span_proven;
    /* Exact static merge algebra: zero-extended first byte ORed with the
     * low nibble of the zero-extended second byte shifted left four places.
     * This proves byte order and masks, not a PRS3 field, copy, or texture. */
    int sh2_zero_byte_merge_order_proven;
    /* The post-merge `CMP/GT; BT` branch is a source-owned control re-entry
     * condition only. It cannot be named a token rejection or termination
     * without an execution trace that establishes the registers' roles. */
    int sh2_zero_merged_branch_condition_proven;
    uint32_t zero_post_read_compare_offset;
    uint32_t zero_repeat_counter_increment_offset;
    uint32_t zero_repeat_branch_offset;
    uint16_t zero_repeat_compare_instruction;
    uint16_t zero_repeat_branch_instruction;
    uint32_t zero_repeat_branch_target_offset;
    uint32_t zero_repeat_delay_mask_offset;
    uint32_t zero_outer_loop_branch_offset;
    uint32_t zero_outer_loop_target_offset;
    int sh2_zero_side_repeat_control_verified;
    /* The zero-side's `CMP/EQ; ADD #1; BF/S` loop exits only when its two
     * working registers compare equal. This is a static SH-2 termination
     * condition, not proof that either register is a PRS3 run length. */
    int sh2_zero_repeat_termination_proven;
    /* The R13/R0 indexed byte reaches the two subsequent R1 comparisons.
     * This is static operand/control flow only, not proof of a copy source,
     * decoded value, length, termination marker, or any pixel behavior. */
    int sh2_zero_indexed_byte_control_operands_proven;
    uint32_t zero_side_linear_begin_offset;
    uint32_t zero_side_linear_end_offset;
    uint32_t zero_side_linear_byte_count;
    uint64_t zero_side_linear_fnv1a64;
    int sh2_zero_side_linear_route_verified;
    uint32_t zero_side_output_store_instruction_count;
    int sh2_zero_side_has_no_direct_output_store;
    int zero_side_copy_or_backreference_proven;
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
    uint32_t schema_version;
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
    uint64_t payload_fnv1a64;
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
    uint64_t vdp1_texture_first_read_sequence;
    uint64_t vdp1_texture_last_read_sequence;
    uint32_t vdp1_texture_first_read_address;
    uint32_t vdp1_texture_last_read_address;
    uint32_t vdp1_texture_read_bytes;
    uint64_t vdp1_texture_fnv1a64;
    uint64_t vdp1_command_first_read_sequence;
    uint64_t vdp1_command_last_read_sequence;
    uint32_t vdp1_command_first_read_address;
    uint32_t vdp1_command_last_read_address;
    uint32_t vdp1_command_read_bytes;
    uint64_t vdp1_command_fnv1a64;
    uint64_t palette_first_read_sequence;
    uint64_t palette_last_read_sequence;
    uint32_t palette_first_read_address;
    uint32_t palette_last_read_address;
    uint32_t palette_read_bytes;
    uint64_t palette_fnv1a64;
    /* V4 capture claims. They remain external evidence until independent
     * original-Saturn provenance is admitted. */
    uint32_t output_index_copy_instruction_offset;
    uint32_t output_store_instruction_offset;
    uint64_t first_output_store_sequence;
    uint64_t last_output_store_sequence;
    uint32_t first_output_store_address;
    uint32_t last_output_store_address;
    uint32_t output_store_bytes;
    uint64_t output_store_fnv1a64;
    int output_store_predecessor_observed;
    int complete_output_store_range_observed;
    /* V5 complete-stream control coverage. This is still a capture claim;
     * neither outcome is assigned PRS3 token semantics here. */
    uint32_t control_test_instruction_offset;
    uint32_t control_zero_branch_instruction_offset;
    uint32_t control_zero_branch_target_offset;
    uint32_t control_branch_outcomes_mask;
    uint32_t nonzero_control_observation_count;
    uint32_t zero_control_observation_count;
    uint64_t first_control_sequence;
    uint64_t last_control_sequence;
    int complete_control_branch_coverage_observed;
    /* V6 carries concrete, branch-local SH-2 operand observations. They are
     * externally captured evidence only: values and sequencing may be bound
     * to retail instructions, but no PRS3 token/output meaning is inferred. */
    uint32_t nonzero_counter_decrement_instruction_offset;
    uint32_t zero_counter_decrement_instruction_offset;
    uint32_t nonzero_counter_before;
    uint32_t nonzero_counter_after;
    uint32_t zero_counter_before;
    uint32_t zero_counter_after;
    uint32_t nonzero_source_cursor_before;
    uint32_t nonzero_source_cursor_after;
    uint32_t zero_source_cursor_before;
    uint32_t zero_source_cursor_after;
    uint64_t nonzero_counter_decrement_sequence;
    uint64_t nonzero_input_read_sequence;
    uint64_t zero_counter_decrement_sequence;
    uint64_t zero_first_input_read_sequence;
    uint64_t zero_second_input_read_sequence;
    int dynamic_control_operands_observed;
    /* V7 adds one nonzero branch's observed byte transfer. The original
     * source route can bind its PCs and input byte, but this remains capture
     * evidence rather than a complete literal-token/decode conclusion. */
    uint32_t nonzero_input_payload_byte_offset;
    uint32_t nonzero_observed_input_byte;
    uint32_t nonzero_observed_output_byte;
    uint32_t nonzero_output_store_instruction_offset;
    uint32_t nonzero_output_byte_offset;
    uint32_t nonzero_output_address;
    uint64_t nonzero_output_write_sequence;
    int dynamic_nonzero_byte_transfer_observed;
    /* V8 captures the zero-side's two real source bytes and static SH-2
     * merge result. This path has no verified direct output store, so V8
     * deliberately does not invent one. */
    uint32_t zero_first_input_instruction_offset;
    uint32_t zero_second_input_instruction_offset;
    uint32_t zero_first_input_payload_byte_offset;
    uint32_t zero_second_input_payload_byte_offset;
    uint32_t zero_observed_first_input_byte;
    uint32_t zero_observed_second_input_byte;
    uint32_t zero_observed_merged_control_value;
    int dynamic_zero_source_merge_observed;
    int exact_vdp1_handoff_observed;
    int vdp1_texture_consumption_observed;
    int vdp1_command_consumption_observed;
    int palette_consumption_observed;
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
    int payload_span_matches;
    int exact_vdp1_handoff_observed;
    int vdp1_texture_consumption_observed;
    int vdp1_command_consumption_observed;
    int palette_consumption_observed;
    int decoder_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3Vdp1CaptureBindingReceipt;

/* A source-bound capture may be ready for opcode-grammar review only after it
 * proves one entire input/output interval and both observed low-bit outcomes.
 * It is deliberately not decoder readiness: no observed branch is assigned a
 * literal/copy meaning, and `decoder_ready` remains false. */
typedef struct {
    int valid;
    int capture_source_bound;
    int complete_input_range_bound;
    int complete_output_range_bound;
    int control_branch_coverage_bound;
    int original_saturn_execution_authenticated;
    int opcode_grammar_proven;
    int decoder_ready;
    int decoder_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3DecoderReadinessReceipt;

/* One source-bound nonzero path can be joined to a complete-stream review
 * receipt. This is an opaque byte-emission candidate, not a PRS3 literal or
 * token grammar rule: independent Saturn authentication remains required. */
typedef struct {
    int valid;
    int decoder_review_bound;
    int nonzero_transfer_bound;
    int input_output_sequence_bound;
    int original_saturn_execution_authenticated;
    int token_operation_proven;
    int opcode_grammar_proven;
    int decoder_ready;
    int decoder_promoted;
    int fallback_visuals_permitted;
    uint32_t payload_byte_offset;
    uint32_t output_byte_offset;
    uint8_t observed_input_byte;
    uint8_t observed_output_byte;
} Nexus_V1_Prs3TokenGrammarReceipt;

/* File-backed import receipt for an externally captured V3 trace. The trace
 * is evidence supplied by a capture tool, not a runtime asset: successful
 * import only proves that its stated spans bind to canonical MENU.BPK/DM.BIN
 * bytes and that its internal VDP1/palette intervals are complete. */
typedef struct {
    int trace_file_read;
    int menu_bpk_original_hash_verified;
    int dm_bin_original_hash_verified;
    int v3_trace_parsed;
    int source_bound_capture;
    int runtime_import_permitted;
    int decoder_promoted;
    int fallback_visuals_permitted;
    Nexus_V1_Prs3Vdp1CaptureReceipt trace;
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt binding;
} Nexus_V1_Prs3Vdp1CaptureFileReceipt;

/* Additional raw-byte admission for a V3 trace. The three sidecars are
 * capture artifacts, never decoded texture or palette input for Firestaff. */
typedef struct {
    int trace_source_bound;
    int output_sidecar_bound;
    int vdp1_command_sidecar_bound;
    int palette_sidecar_bound;
    int raw_sidecars_bound;
    int capture_producer_authenticated;
    int runtime_import_permitted;
    int decoder_promoted;
    int fallback_visuals_permitted;
    Nexus_V1_Prs3Vdp1CaptureFileReceipt trace_file;
} Nexus_V1_Prs3Vdp1RawSidecarReceipt;

/* Parsed hardware framing from a source-bound V3 VDP1 command sidecar. This
 * does not authenticate the external producer and cannot establish PRS3
 * opcodes, decoded pixels, palette interpretation, or a runtime draw route. */
typedef struct {
    int valid;
    int capture_source_bound;
    int command_sidecar_hash_bound;
    int complete_vdp1_command_record;
    int command_format_parsed;
    Nexus_V1_Vdp1TextureCommand command;
    int original_saturn_capture_verified;
    int pixel_format_proven;
    int palette_format_proven;
    int decoder_promoted;
    int runtime_import_permitted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3Vdp1CommandSidecarReceipt;

/* Provenance ledger for a raw-sidecar admission. It binds the supplied files
 * to a named capture producer binary, but does not authenticate that producer
 * or claim original-Saturn execution. */
typedef struct {
    int ledger_parsed;
    int raw_sidecars_bound;
    int trace_bytes_match;
    int output_bytes_match;
    int vdp1_command_bytes_match;
    int palette_bytes_match;
    int producer_binary_bound;
    int provenance_complete;
    int capture_producer_authenticated;
    int runtime_import_permitted;
} Nexus_V1_Prs3Vdp1ProvenanceReceipt;

/* A producer attestation is a strict description of a proposed external
 * capture workflow. It binds the producer binary and artifact fingerprints,
 * but its text is not a trust authority: independent original-Saturn review
 * remains mandatory and no receipt can promote decoding or rendering. */
typedef struct {
    int attestation_file_read;
    int attestation_parsed;
    int raw_sidecars_bound;
    int provenance_complete;
    int producer_binary_bound;
    int capture_mode_declared;
    int original_saturn_execution_claimed;
    int artifact_hashes_bound;
    int workflow_complete;
    int independent_authentication_required;
    int capture_producer_authenticated;
    int runtime_import_permitted;
    int decoder_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3Vdp1ProducerAttestationReceipt;

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

/* A single, externally observed transfer through the hash-verified V1 SH-2
 * loop.  This is deliberately smaller than a decoder command: it proves only
 * that one MENU.BPK payload byte was read at the known load instruction and
 * later stored at the known output instruction.  The trace must come from an
 * original-Saturn capture tool; this parser does not authenticate a producer
 * and never promotes a pixel, palette, or PRS3 grammar. */
#define NEXUS_V1_PRS3_SH2_TRANSFER_TRACE_MAGIC \
    "NEXUS_PRS3_SH2_TRANSFER_TRACE_V1"
typedef struct {
    int valid;
    uint64_t menu_bpk_fnv1a64;
    uint64_t dm_bin_fnv1a64;
    uint32_t entry_index;
    uint32_t stream_offset;
    uint32_t stream_size;
    uint32_t expected_output_bytes;
    uint32_t payload_byte_offset;
    uint32_t control_test_instruction_offset;
    uint32_t zero_branch_instruction_offset;
    uint32_t fallthrough_counter_decrement_offset;
    uint32_t observed_control_low_bit;
    uint32_t observed_zero_branch_taken;
    uint32_t input_instruction_offset;
    uint32_t output_instruction_offset;
    uint64_t input_read_sequence;
    uint64_t output_write_sequence;
    uint32_t output_byte_offset;
    uint32_t input_byte;
    uint32_t output_byte;
} Nexus_V1_Prs3Sh2TransferTrace;

typedef struct {
    int trace_valid;
    int menu_bpk_matches;
    int dm_bin_matches;
    int entry_plan_matches;
    int sh2_instruction_route_matches;
    int nonzero_control_fallthrough_observed;
    int source_byte_matches;
    int observed_byte_transfer;
    int original_saturn_provenance_verified;
    int decoder_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3Sh2TransferReceipt;

/* Captured zero-low-bit side observation. The two source bytes and their
 * observed SH-2 merge value are retained as opaque control data. In
 * particular, `merged_control_value` is not an offset, length, token, or
 * output address. */
typedef struct {
    uint64_t menu_bpk_fnv1a64;
    uint64_t dm_bin_fnv1a64;
    uint32_t entry_index, stream_offset, stream_size, expected_output_bytes;
    uint32_t first_payload_byte_offset, second_payload_byte_offset;
    uint32_t zero_branch_instruction_offset, zero_branch_target_offset;
    uint32_t counter_decrement_offset;
    uint32_t first_input_instruction_offset, second_input_instruction_offset;
    uint64_t first_input_read_sequence, second_input_read_sequence;
    uint32_t first_input_byte, second_input_byte, merged_control_value;
} Nexus_V1_Prs3Sh2ZeroSideTrace;

typedef struct {
    int menu_bpk_matches, dm_bin_matches, entry_plan_matches;
    int zero_side_instruction_route_matches, source_bytes_match;
    int observed_zero_side_merge;
    int original_saturn_provenance_verified;
    int decoder_promoted, fallback_visuals_permitted;
} Nexus_V1_Prs3Sh2ZeroSideReceipt;

/* Joins the two observed low-bit paths for one V5 complete stream. This
 * establishes review coverage only: neither branch is assigned a PRS3 token,
 * literal, copy, length, or offset meaning. */
typedef struct {
    int valid;
    int decoder_review_bound;
    int nonzero_path_bound;
    int zero_side_path_bound;
    int control_branch_evidence_complete;
    int original_saturn_execution_authenticated;
    int token_grammar_proven;
    int decoder_ready;
    int decoder_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3ControlGrammarReviewReceipt;

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

/* Parse and bind one original-capture byte-transfer observation. The source
 * bytes, selected BPK span, exact SH-2 read/store instructions, byte value,
 * and sequence ordering must all agree. A successful receipt remains a
 * no-draw, non-decoder fact until a complete authenticated command grammar is
 * independently established. */
int nexus_v1_prs3_sh2_transfer_trace_parse_and_bind(
    const char *text, size_t text_size,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size, int source_hash_verified,
    Nexus_V1_Prs3Sh2TransferTrace *out_trace,
    Nexus_V1_Prs3Sh2TransferReceipt *out_receipt);

/* Join a complete V5 decoder-review receipt with one separately source-bound
 * nonzero SH-2 transfer. The result records only a byte-emission candidate;
 * it never calls that candidate a literal or enables decoder execution. */
int nexus_v1_prs3_token_grammar_nonzero_candidate_bind(
    const Nexus_V1_Prs3DecoderReadinessReceipt *decoder_review,
    const Nexus_V1_Prs3Vdp1CaptureReceipt *complete_trace,
    const Nexus_V1_Prs3Sh2TransferTrace *transfer_trace,
    const Nexus_V1_Prs3Sh2TransferReceipt *transfer_receipt,
    Nexus_V1_Prs3TokenGrammarReceipt *out_receipt);

/* Bind one external zero-low-bit observation to exact original bytes and the
 * SH-2 two-byte path. This validates only the observed merge algebra already
 * present in DM.BIN; it never gives that value PRS3 backreference semantics. */
int nexus_v1_prs3_sh2_zero_side_trace_bind(
    const Nexus_V1_Prs3Sh2ZeroSideTrace *trace,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size, int source_hash_verified,
    Nexus_V1_Prs3Sh2ZeroSideReceipt *out_receipt);

/* Combine source-bound nonzero and zero-side observations under the same V5
 * complete-stream receipt. A successful result is still grammar review only. */
int nexus_v1_prs3_control_grammar_review_bind(
    const Nexus_V1_Prs3DecoderReadinessReceipt *decoder_review,
    const Nexus_V1_Prs3Vdp1CaptureReceipt *complete_trace,
    const Nexus_V1_Prs3Sh2TransferTrace *nonzero_trace,
    const Nexus_V1_Prs3Sh2TransferReceipt *nonzero_receipt,
    const Nexus_V1_Prs3Sh2ZeroSideTrace *zero_trace,
    const Nexus_V1_Prs3Sh2ZeroSideReceipt *zero_receipt,
    Nexus_V1_Prs3ControlGrammarReviewReceipt *out_receipt);

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

/* Parse one complete SH-2-to-VDP1 capture for a single frame. V1 records a
 * command's texture-source address. V2 additionally records VDP1's actual
 * texture-read interval and fingerprint, which must exactly cover the
 * decoder-output range. V3 additionally requires raw VDP1 command and
 * palette read intervals/fingerprints from the original capture. All versions
 * remain evidence-only until an independently reviewed opcode and Saturn
 * palette grammar exists. V4 additionally carries the complete output-store
 * lane; V5 adds both observed low-bit outcomes. Binding rechecks source PCs
 * against the original DM.BIN receipt. */
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

/* Bind a V5 complete-stream capture to its exact original assets and static
 * control/store route. This is a review receipt only, never a decoder or draw
 * authorization. */
int nexus_v1_prs3_decoder_readiness_bind_capture(
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size,
    Nexus_V1_Prs3DecoderReadinessReceipt *out_receipt);

/* Read a text V3/V4 capture plus ordinary canonical MENU.BPK and DM.BIN files.
 * The two source files must match the original Track 1 MD5 identities before
 * their bytes are handed to the schema binder. This importer is read-only and
 * deliberately never enables decoding, rendering, or fallback visuals. */
int nexus_v1_prs3_vdp1_capture_validate_files(
    const char *trace_path, const char *menu_bpk_path, const char *dm_bin_path,
    Nexus_V1_Prs3Vdp1CaptureFileReceipt *out_receipt);

/* Admit a V3/V4 candidate only when its three raw capture sidecars exactly
 * match the trace's lengths and FNV witnesses. The caller must separately
 * establish the emulator/original-Saturn provenance of the artifacts; this
 * routine does not infer it and never exposes sidecar bytes to runtime. */
int nexus_v1_prs3_vdp1_capture_validate_raw_sidecars(
    const char *trace_path, const char *menu_bpk_path, const char *dm_bin_path,
    const char *output_path, const char *vdp1_command_path,
    const char *palette_path, Nexus_V1_Prs3Vdp1RawSidecarReceipt *out_receipt);
/* Consume an already source-bound V3 sidecar command as one documented VDP1
 * packet. The caller must separately obtain independent Saturn provenance;
 * this function is strictly a no-draw capture-analysis boundary. */
int nexus_v1_prs3_vdp1_capture_inspect_command_sidecar(
    const Nexus_V1_Prs3Vdp1RawSidecarReceipt *raw_sidecars,
    const uint8_t *vdp1_command, size_t vdp1_command_size,
    Nexus_V1_Prs3Vdp1CommandSidecarReceipt *out_receipt);

/* Validate a text provenance ledger against already admitted raw sidecars and
 * the exact trace/sidecar/producer files. This never changes the producer
 * authentication verdict or permits runtime import. */
int nexus_v1_prs3_vdp1_capture_validate_provenance(
    const char *ledger_path, const char *trace_path, const char *output_path,
    const char *vdp1_command_path, const char *palette_path,
    const char *producer_binary_path,
    const Nexus_V1_Prs3Vdp1RawSidecarReceipt *raw_sidecars,
    Nexus_V1_Prs3Vdp1ProvenanceReceipt *out_receipt);

/* Write the deterministic V3 provenance ledger for an already source-bound
 * raw capture bundle. This copies no capture bytes and never creates a trace,
 * pixels, palette, decoder, or runtime route. */
int nexus_v1_prs3_vdp1_capture_write_provenance_ledger(
    const char *ledger_path, const char *trace_path,
    const char *menu_bpk_path, const char *dm_bin_path,
    const char *output_path, const char *vdp1_command_path,
    const char *palette_path, const char *producer_binary_path,
    Nexus_V1_Prs3Vdp1RawSidecarReceipt *out_receipt);

/* Validate a source-owned producer-attestation file after sidecar and ledger
 * checks. The fixed capture mode is `SH2_VDP1_BUS_TRACE`; the claimed
 * original-Saturn execution is recorded but never trusted as authentication. */
int nexus_v1_prs3_vdp1_capture_validate_producer_attestation(
    const char *attestation_path, const char *trace_path,
    const char *output_path, const char *vdp1_command_path,
    const char *palette_path, const char *producer_binary_path,
    const Nexus_V1_Prs3Vdp1RawSidecarReceipt *raw_sidecars,
    const Nexus_V1_Prs3Vdp1ProvenanceReceipt *provenance,
    Nexus_V1_Prs3Vdp1ProducerAttestationReceipt *out_receipt);

#endif
