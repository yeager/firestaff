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
    int sh2_loop_back_target_verified;
    int sh2_loop_body_bound;
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

/* Parse one complete SH-2-to-VDP1 capture for a single frame. V1 records a
 * command's texture-source address. V2 additionally records VDP1's actual
 * texture-read interval and fingerprint, which must exactly cover the
 * decoder-output range. V3 additionally requires raw VDP1 command and
 * palette read intervals/fingerprints from the original capture. All versions
 * remain evidence-only until an independently reviewed opcode and Saturn
 * palette grammar exists. */
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

/* Read a text V3 capture plus ordinary canonical MENU.BPK and DM.BIN files.
 * The two source files must match the original Track 1 MD5 identities before
 * their bytes are handed to the schema binder. This importer is read-only and
 * deliberately never enables decoding, rendering, or fallback visuals. */
int nexus_v1_prs3_vdp1_capture_validate_files(
    const char *trace_path, const char *menu_bpk_path, const char *dm_bin_path,
    Nexus_V1_Prs3Vdp1CaptureFileReceipt *out_receipt);

/* Admit a V3 candidate only when its three raw capture sidecars exactly
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
