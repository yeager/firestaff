#include "nexus_v1_structure3_capture_manifest.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static const char valid_manifest[] =
    "NEXUS_STRUCTURE3_SATURN_CAPTURE_V1\n"
    "capture_session_fnv1a64=1\n"
    "dgn_fnv1a64=2\n"
    "structure3_payload_fnv1a32=3\n"
    "typed_mesh_corpus_fnv1a32=d3f42b1f\n"
    "entry_index=4\nface_ordinal=5\nface_row_fnv1a32=6\n"
    "referenced_vertex_rows_fnv1a32=7\nnormal_row_fnv1a32=8\n"
    "fill_selector=9\ntexture_span_bytes=a\ntexture_span_fnv1a64=b\n"
    "palette_state_bytes=c\npalette_state_fnv1a64=d\n"
    "vdp1_state_bytes=e\nvdp1_state_fnv1a64=f\n"
    "transform_state_bytes=10\ntransform_state_fnv1a64=11\n"
    "normal_culling_state_bytes=12\nnormal_culling_state_fnv1a64=13\n"
    "vdp1_command_bytes=14\nvdp1_command_fnv1a64=15\n"
    "first_sequence=16\nlast_sequence=17\n";

int main(void) {
    Nexus_V1_DgnStructure3CaptureManifestReceipt receipt;
    char malformed[sizeof(valid_manifest)];

    expect(nexus_v1_dgn_structure3_capture_manifest_parse(
               valid_manifest, strlen(valid_manifest), &receipt) &&
               receipt.valid && receipt.complete &&
               receipt.capture_session_fnv1a64 == 1U &&
               receipt.candidate.entry_index == 4U &&
               receipt.candidate.face_ordinal == 5U &&
               receipt.candidate.fill_selector == 9U &&
               receipt.texture_span_bytes == 10U &&
               receipt.palette_state_bytes == 12U &&
               !receipt.original_saturn_capture_verified &&
               !receipt.renderer_handoff_ready &&
               receipt.blocks_real_dgn_mesh_render,
           "complete correlation manifest remains no-draw provenance only");
    expect(nexus_v1_dgn_structure3_capture_manifest_validate_spans(
               &receipt, 10U, 12U, 14U, 16U, 18U, 20U),
           "the admitted manifest requires every captured span size exactly");
    expect(!nexus_v1_dgn_structure3_capture_manifest_validate_spans(
               &receipt, 9U, 12U, 14U, 16U, 18U, 20U),
           "a truncated texture span cannot reach the DGN binder");

    snprintf(malformed, sizeof(malformed), "%s", valid_manifest);
    memcpy(strstr(malformed, "texture_span_bytes=a"),
           "texture_span_bytes=0", 20U);
    expect(!nexus_v1_dgn_structure3_capture_manifest_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid &&
               receipt.blocks_real_dgn_mesh_render,
           "empty texture span rejects the manifest and retains no-draw gate");

    snprintf(malformed, sizeof(malformed), "%s", valid_manifest);
    memcpy(strstr(malformed, "last_sequence=17"), "last_sequence=16", 16U);
    expect(!nexus_v1_dgn_structure3_capture_manifest_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "non-increasing capture sequence is rejected");

    snprintf(malformed, sizeof(malformed), "%s", valid_manifest);
    memcpy(strstr(malformed, "palette_state_fnv1a64=d"),
           "palette_state_missing=", 22U);
    expect(!nexus_v1_dgn_structure3_capture_manifest_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "missing palette correlation rejects the manifest");

    return failures ? 1 : 0;
}
