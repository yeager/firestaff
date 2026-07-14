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

#endif
