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
#define NEXUS_V1_STRUCTURE3_CAPTURE_RAW_SPAN_MAX_BYTES (16U * 1024U * 1024U)

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
    Nexus_V1_DgnStructure3FaceCaptureCandidate candidate;
    /* Parsing a manifest never makes it an original-Saturn capture. */
    int original_saturn_capture_verified;
    int renderer_handoff_ready;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3CaptureManifestReceipt;

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
    int capture_bundle_hash_verified;
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
    int original_saturn_source_attested;
} Nexus_V1_DgnStructure3RawCaptureAttestation;

typedef struct {
    int manifest_accepted;
    int all_spans_read;
    int raw_span_hashes_match;
    int attestation_session_matches;
    int attestation_bundle_matches;
    int original_saturn_source_attested;
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
    int capture_bundle_hash_verified;
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

void nexus_v1_dgn_structure3_capture_host_receipt_clear(
    Nexus_V1_DgnStructure3CaptureHostReceipt *receipt);

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
