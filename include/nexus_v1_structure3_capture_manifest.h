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

#endif
