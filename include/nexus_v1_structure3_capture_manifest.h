#ifndef NEXUS_V1_STRUCTURE3_CAPTURE_MANIFEST_H
#define NEXUS_V1_STRUCTURE3_CAPTURE_MANIFEST_H

#include "nexus_v1_dungeon.h"

#include <stddef.h>
#include <stdint.h>

/* Text intake for a future original-Saturn Structure3 face capture. This is
 * deliberately a correlation envelope, not a VDP1/PRS3 decoder or a source
 * admission mechanism. The caller must separately verify both media and
 * capture provenance before passing the candidate to the DGN binder. */
#define NEXUS_V1_STRUCTURE3_CAPTURE_MANIFEST_MAGIC \
    "NEXUS_STRUCTURE3_SATURN_CAPTURE_V1"
#define NEXUS_V1_STRUCTURE3_CAPTURE_TARGET_MAGIC \
    "NEXUS_STRUCTURE3_SATURN_CAPTURE_TARGET_V1"
#define NEXUS_V1_STRUCTURE3_CAPTURE_PRODUCER_ATTESTATION_MAGIC \
    "NEXUS_STRUCTURE3_SATURN_PRODUCER_ATTESTATION_V1"
#define NEXUS_V1_STRUCTURE3_CAPTURE_RAW_SPAN_MAX_BYTES (16U * 1024U * 1024U)
#define NEXUS_V1_STRUCTURE3_CAPTURE_TRACE_LANE_COUNT 6U

/* These identify opaque evidence lanes only. Their numeric values and order
 * are a Firestaff transport record, not a statement about Saturn hardware. */
typedef enum {
    NEXUS_V1_STRUCTURE3_TRACE_TEXTURE_SPAN = 0,
    NEXUS_V1_STRUCTURE3_TRACE_PALETTE_STATE = 1,
    NEXUS_V1_STRUCTURE3_TRACE_VDP1_STATE = 2,
    NEXUS_V1_STRUCTURE3_TRACE_TRANSFORM_STATE = 3,
    NEXUS_V1_STRUCTURE3_TRACE_NORMAL_CULLING_STATE = 4,
    NEXUS_V1_STRUCTURE3_TRACE_VDP1_COMMAND = 5
} Nexus_V1_DgnStructure3CaptureTraceLane;

typedef struct {
    int valid;
    int complete;
    uint64_t capture_session_fnv1a64;
    uint32_t texture_span_bytes;
    uint32_t palette_state_bytes;
    uint32_t vdp1_state_bytes;
    uint32_t transform_state_bytes;
    uint32_t normal_culling_state_bytes;
    uint32_t vdp1_command_bytes;
    /* Exact externally captured ordinals for the six opaque observations.
     * Their relative order is recorded, never inferred from lane names. */
    uint64_t trace_sequence[NEXUS_V1_STRUCTURE3_CAPTURE_TRACE_LANE_COUNT];
    Nexus_V1_DgnStructure3FaceCaptureCandidate candidate;
    /* Parsing a manifest never makes it an original-Saturn capture. */
    int original_saturn_capture_verified;
    int renderer_handoff_ready;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3CaptureManifestReceipt;

/* A capture target is generated from canonical DGN bytes before any Saturn
 * trace exists. It names one bounded face and the exact source rows an
 * external producer must correlate with its six raw capture lanes. It is not
 * a trace, decoder, palette, transform, or draw command. */
typedef struct {
    int valid;
    int level_index;
    Nexus_V1_DgnStructure3FaceCaptureCandidate candidate;
    int capture_producer_required;
    int original_saturn_capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure3CaptureTargetReceipt;

/* The capture reader keeps raw observations separate from the text envelope.
 * `capture_bundle_fnv1a64` must be computed over these six length-prefixed
 * spans by the same importer that read the immutable capture artifact. The
 * two verification flags are caller-owned: matching bytes never certifies
 * Saturn provenance. */
typedef struct {
    const uint8_t *texture_span;
    size_t texture_span_size;
    const uint8_t *palette_state;
    size_t palette_state_size;
    const uint8_t *vdp1_state;
    size_t vdp1_state_size;
    const uint8_t *transform_state;
    size_t transform_state_size;
    const uint8_t *normal_culling_state;
    size_t normal_culling_state_size;
    const uint8_t *vdp1_command;
    size_t vdp1_command_size;
    uint64_t capture_session_fnv1a64;
    uint64_t capture_bundle_fnv1a64;
    uint64_t capture_trace_order_fnv1a64;
    int capture_bundle_hash_verified;
    int capture_trace_order_verified;
    int original_saturn_capture_verified;
} Nexus_V1_DgnStructure3CaptureImport;

/* File locations are deliberately six independent raw spans. The reader
 * treats each as opaque bytes; neither path names nor byte content establish
 * VDP1, palette, transform, culling, or draw semantics. */
typedef struct {
    const char *texture_span_path;
    const char *palette_state_path;
    const char *vdp1_state_path;
    const char *transform_state_path;
    const char *normal_culling_state_path;
    const char *vdp1_command_path;
} Nexus_V1_DgnStructure3RawCapturePaths;

/* This attestation is supplied by an original-Saturn trace/capture verifier
 * outside Firestaff. The reader refuses to manufacture it from local files. */
typedef struct {
    uint64_t capture_session_fnv1a64;
    uint64_t capture_bundle_fnv1a64;
    uint64_t capture_trace_order_fnv1a64;
    int original_saturn_source_attested;
} Nexus_V1_DgnStructure3RawCaptureAttestation;

/* This binds an instrumented producer binary to the identities it claims to
 * have emitted. The claim needs independent original-Saturn review, so it
 * cannot by itself import runtime bytes or enable drawing. */
typedef struct {
    int attestation_parsed;
    int producer_binary_bound;
    int capture_mode_declared;
    int original_saturn_execution_claimed;
    int workflow_bound;
    int independent_authentication_required;
    int runtime_import_permitted;
    int no_draw_only;
} Nexus_V1_DgnStructure3ProducerAttestationReceipt;

typedef struct {
    int manifest_accepted;
    int all_spans_read;
    int raw_span_hashes_match;
    int attestation_session_matches;
    int attestation_bundle_matches;
    int attestation_trace_order_matches;
    int original_saturn_source_attested;
    uint64_t capture_trace_order_fnv1a64;
    int import_ready;
    int no_draw_only;
    Nexus_V1_DgnStructure3CaptureImport import_packet;
    uint8_t *texture_span_storage;
    uint8_t *palette_state_storage;
    uint8_t *vdp1_state_storage;
    uint8_t *transform_state_storage;
    uint8_t *normal_culling_state_storage;
    uint8_t *vdp1_command_storage;
} Nexus_V1_DgnStructure3RawCaptureReaderReceipt;

typedef struct {
    int manifest_valid;
    int spans_match_manifest;
    int raw_span_hashes_match;
    int capture_session_matches;
    int capture_bundle_matches;
    int capture_trace_order_matches;
    int capture_bundle_hash_verified;
    int capture_trace_order_verified;
    int original_saturn_capture_verified;
    int dgn_source_hash_verified;
    int binder_invoked;
    int complete_source_binding;
    int renderer_handoff_ready;
    int blocks_real_dgn_mesh_render;
    Nexus_V1_DgnStructure3FaceCaptureBindingReceipt binding;
} Nexus_V1_DgnStructure3CaptureImportReceipt;

/* Host-owned intake receipt. This is the only path that may forward a raw
 * packet to the face binder: an authenticated DGN source and an explicit
 * original-Saturn verdict must both exist first. It remains no-draw. */
typedef struct {
    int host_dgn_source_verified;
    int capture_source_verified;
    int manifest_parsed;
    int importer_invoked;
    int no_draw_only;
    Nexus_V1_DgnStructure3CaptureManifestReceipt manifest;
    Nexus_V1_DgnStructure3CaptureImportReceipt import_receipt;
} Nexus_V1_DgnStructure3CaptureHostReceipt;

/* Owns the opaque buffers produced by the raw reader while they cross the
 * DGN host boundary. This is a transport receipt only: even a fully attested
 * packet remains no-draw until separate Saturn render semantics exist. */
typedef struct {
    int manifest_parsed;
    int raw_reader_invoked;
    int host_intake_invoked;
    int no_draw_only;
    Nexus_V1_DgnStructure3RawCaptureReaderReceipt raw_reader;
    Nexus_V1_DgnStructure3CaptureHostReceipt host;
} Nexus_V1_DgnStructure3RawCaptureHostReceipt;

void nexus_v1_dgn_structure3_capture_host_receipt_clear(
    Nexus_V1_DgnStructure3CaptureHostReceipt *receipt);
void nexus_v1_dgn_structure3_raw_capture_host_receipt_clear(
    Nexus_V1_DgnStructure3RawCaptureHostReceipt *receipt);
void nexus_v1_dgn_structure3_raw_capture_host_receipt_release(
    Nexus_V1_DgnStructure3RawCaptureHostReceipt *receipt);

/* Build one exact source-bound target for an external Saturn/VDP1 producer.
 * `dgn_source_hash_verified` must come from the canonical asset scanner. */
int nexus_v1_dgn_structure3_capture_target_build(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int level_index, int dgn_source_hash_verified, uint32_t entry_index,
    uint32_t face_ordinal, Nexus_V1_DgnStructure3CaptureTargetReceipt *out_receipt);

/* Write a deterministic target request for a concrete external capture
 * producer. It writes no capture bytes, does not manufacture lane hashes,
 * and cannot be imported as a completed capture manifest. */
int nexus_v1_dgn_structure3_capture_target_write(
    const char *path, const Nexus_V1_DgnStructure3CaptureTargetReceipt *target);

/* Checks that a producer's completed manifest correlates to the one exact
 * source-bound target it was asked to capture. This is intentionally a
 * bounded identity check only; it does not attest Saturn provenance, decode
 * any lane, or permit renderer handoff. */
int nexus_v1_dgn_structure3_capture_target_matches_manifest(
    const Nexus_V1_DgnStructure3CaptureTargetReceipt *target,
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest);

/* Parses a producer workflow attestation. Its output raw attestation always
 * leaves original-Saturn admission false; only independent evidence may set
 * that bit before the runtime intake is called. */
int nexus_v1_dgn_structure3_capture_producer_attestation_parse(
    const char *text, size_t text_size,
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest,
    uint64_t producer_binary_fnv1a64,
    Nexus_V1_DgnStructure3RawCaptureAttestation *out_raw_attestation,
    Nexus_V1_DgnStructure3ProducerAttestationReceipt *out_receipt);

void nexus_v1_dgn_structure3_raw_capture_reader_receipt_clear(
    Nexus_V1_DgnStructure3RawCaptureReaderReceipt *receipt);
void nexus_v1_dgn_structure3_raw_capture_reader_receipt_release(
    Nexus_V1_DgnStructure3RawCaptureReaderReceipt *receipt);

/* Reads six raw original-capture spans and validates them atomically against
 * a parsed manifest plus an externally supplied capture attestation. A local
 * byte match is insufficient: without the external original-Saturn verdict
 * this function returns 0 and leaves a no-draw receipt. Call release before
 * reusing a receipt that previously accepted file-backed spans. */
int nexus_v1_dgn_structure3_raw_capture_read(
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest,
    const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure3RawCaptureReaderReceipt *out_receipt);

/* The package-to-host transport route. It accepts only raw spans in the
 * manifest's fixed six-lane order, validates the external attestation, then
 * invokes the existing DGN host intake with the reader-owned opaque packet.
 * Call release before reusing a successful receipt. */
int nexus_v1_dgn_structure3_raw_capture_host_intake(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int dgn_source_hash_verified, const char *manifest_text,
    size_t manifest_size, const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure3RawCaptureHostReceipt *out_receipt);

/* Parse one complete, single-face correlation envelope. The manifest retains
 * opaque byte fingerprints and ordering only. It assigns no texture, palette,
 * VDP1, transform, culling, or drawing semantics. */
int nexus_v1_dgn_structure3_capture_manifest_parse(
    const char *text, size_t text_size,
    Nexus_V1_DgnStructure3CaptureManifestReceipt *out_receipt);

/* Require the captured byte buffers to have exactly the sizes declared by an
 * accepted manifest before their fingerprints are handed to the DGN binder.
 * This validates envelope completeness only; it cannot admit a source or
 * establish any hardware-field semantics. */
int nexus_v1_dgn_structure3_capture_manifest_validate_spans(
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *receipt,
    size_t texture_span_size, size_t palette_state_size,
    size_t vdp1_state_size, size_t transform_state_size,
    size_t normal_culling_state_size, size_t vdp1_command_size);

/* Imports one opaque capture packet into the existing Structure3 byte binder.
 * The manifest, raw spans, session identity, and bundle identity must agree
 * atomically. Original-Saturn provenance is never derived from local bytes;
 * it must come from an external capture verifier. No drawing semantics are
 * assigned here. */
int nexus_v1_dgn_structure3_capture_manifest_bind_import(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int dgn_source_hash_verified,
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest,
    const Nexus_V1_DgnStructure3CaptureImport *capture,
    Nexus_V1_DgnStructure3CaptureImportReceipt *out_receipt);

/* The host supplies its loaded DGN bytes and independent source result.
 * Local byte matches can never substitute for original-Saturn evidence. */
int nexus_v1_dgn_structure3_capture_host_intake(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int dgn_source_hash_verified, const char *manifest_text,
    size_t manifest_size, const Nexus_V1_DgnStructure3CaptureImport *capture,
    Nexus_V1_DgnStructure3CaptureHostReceipt *out_receipt);

#endif
