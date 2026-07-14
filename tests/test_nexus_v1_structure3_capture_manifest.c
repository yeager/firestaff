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

static unsigned long long fnv1a64_update(unsigned long long hash,
                                          const unsigned char *data,
                                          size_t size) {
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= (unsigned long long)data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static unsigned long long fnv1a64(const unsigned char *data, size_t size) {
    return fnv1a64_update(1469598103934665603ULL, data, size);
}

static unsigned long long bundle_hash(const Nexus_V1_DgnStructure3CaptureImport *in) {
    const unsigned char *spans[6] = {
        in->texture_span, in->palette_state, in->vdp1_state,
        in->transform_state, in->normal_culling_state, in->vdp1_command
    };
    const size_t sizes[6] = {
        in->texture_span_size, in->palette_state_size, in->vdp1_state_size,
        in->transform_state_size, in->normal_culling_state_size,
        in->vdp1_command_size
    };
    unsigned long long hash = 1469598103934665603ULL;
    unsigned char length[8];
    size_t i;
    size_t byte;

    for (i = 0U; i < 6U; ++i) {
        for (byte = 0U; byte < sizeof(length); ++byte)
            length[byte] = (unsigned char)(sizes[i] >> (byte * 8U));
        hash = fnv1a64_update(hash, length, sizeof(length));
        hash = fnv1a64_update(hash, spans[i], sizes[i]);
    }
    return hash;
}

int main(void) {
    Nexus_V1_DgnStructure3CaptureManifestReceipt receipt;
    Nexus_V1_DgnStructure3CaptureImport capture;
    Nexus_V1_DgnStructure3CaptureImportReceipt import_receipt;
    Nexus_V1_DgnStructure3CaptureHostReceipt host_receipt;
    Nexus_V1_Level level;
    char imported_manifest[2048];
    static const unsigned char texture[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    static const unsigned char palette[] = { 11, 12, 13, 14, 15, 16, 17, 18,
                                              19, 20, 21, 22 };
    static const unsigned char vdp1[] = { 23, 24, 25, 26, 27, 28, 29, 30,
                                           31, 32, 33, 34, 35, 36 };
    static const unsigned char transform[] = { 37, 38, 39, 40, 41, 42, 43, 44,
                                                45, 46, 47, 48, 49, 50, 51, 52 };
    static const unsigned char culling[] = { 53, 54, 55, 56, 57, 58, 59, 60,
                                              61, 62, 63, 64, 65, 66, 67, 68,
                                              69, 70 };
    static const unsigned char command[] = { 71, 72, 73, 74, 75, 76, 77, 78,
                                              79, 80, 81, 82, 83, 84, 85, 86,
                                              87, 88, 89, 90 };
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

    memset(&capture, 0, sizeof(capture));
    capture.texture_span = texture;
    capture.texture_span_size = sizeof(texture);
    capture.palette_state = palette;
    capture.palette_state_size = sizeof(palette);
    capture.vdp1_state = vdp1;
    capture.vdp1_state_size = sizeof(vdp1);
    capture.transform_state = transform;
    capture.transform_state_size = sizeof(transform);
    capture.normal_culling_state = culling;
    capture.normal_culling_state_size = sizeof(culling);
    capture.vdp1_command = command;
    capture.vdp1_command_size = sizeof(command);
    capture.capture_session_fnv1a64 = receipt.capture_session_fnv1a64;
    capture.capture_bundle_fnv1a64 = bundle_hash(&capture);
    capture.capture_bundle_hash_verified = 1;
    memset(&level, 0, sizeof(level));

    expect(!nexus_v1_dgn_structure3_capture_host_intake(
               &level, texture, (int)sizeof(texture),
               1, valid_manifest, strlen(valid_manifest), &capture,
               &host_receipt) &&
               host_receipt.host_dgn_source_verified &&
               !host_receipt.capture_source_verified &&
               host_receipt.manifest_parsed &&
               !host_receipt.importer_invoked && host_receipt.no_draw_only &&
               host_receipt.import_receipt.blocks_real_dgn_mesh_render,
           "host intake fail-closes before the binder without Saturn evidence");

    snprintf(imported_manifest, sizeof(imported_manifest),
        "NEXUS_STRUCTURE3_SATURN_CAPTURE_V1\n"
        "capture_session_fnv1a64=1\ndgn_fnv1a64=2\n"
        "structure3_payload_fnv1a32=3\n"
        "typed_mesh_corpus_fnv1a32=d3f42b1f\nentry_index=4\n"
        "face_ordinal=5\nface_row_fnv1a32=6\n"
        "referenced_vertex_rows_fnv1a32=7\nnormal_row_fnv1a32=8\n"
        "fill_selector=9\ntexture_span_bytes=%zx\ntexture_span_fnv1a64=%llx\n"
        "palette_state_bytes=%zx\npalette_state_fnv1a64=%llx\n"
        "vdp1_state_bytes=%zx\nvdp1_state_fnv1a64=%llx\n"
        "transform_state_bytes=%zx\ntransform_state_fnv1a64=%llx\n"
        "normal_culling_state_bytes=%zx\nnormal_culling_state_fnv1a64=%llx\n"
        "vdp1_command_bytes=%zx\nvdp1_command_fnv1a64=%llx\n"
        "first_sequence=16\nlast_sequence=17\n",
        sizeof(texture), fnv1a64(texture, sizeof(texture)),
        sizeof(palette), fnv1a64(palette, sizeof(palette)),
        sizeof(vdp1), fnv1a64(vdp1, sizeof(vdp1)),
        sizeof(transform), fnv1a64(transform, sizeof(transform)),
        sizeof(culling), fnv1a64(culling, sizeof(culling)),
        sizeof(command), fnv1a64(command, sizeof(command)));
    capture.original_saturn_capture_verified = 1;
    expect(!nexus_v1_dgn_structure3_capture_host_intake(
               &level, texture, (int)sizeof(texture), 1,
               imported_manifest, strlen(imported_manifest), &capture,
               &host_receipt) && host_receipt.capture_source_verified &&
               host_receipt.importer_invoked &&
               host_receipt.import_receipt.binder_invoked &&
               !host_receipt.import_receipt.complete_source_binding &&
               host_receipt.no_draw_only,
           "attested Saturn evidence can reach the existing no-draw binder");
    capture.original_saturn_capture_verified = 0;
    expect(nexus_v1_dgn_structure3_capture_manifest_parse(
               imported_manifest, strlen(imported_manifest), &receipt) &&
               !nexus_v1_dgn_structure3_capture_manifest_bind_import(
                   NULL, NULL, 0, 0, &receipt, &capture, &import_receipt) &&
               import_receipt.raw_span_hashes_match &&
               import_receipt.capture_session_matches &&
               import_receipt.capture_bundle_matches &&
               import_receipt.binder_invoked &&
               !import_receipt.complete_source_binding &&
               import_receipt.blocks_real_dgn_mesh_render,
           "a byte-complete import reaches the binder but remains no-draw without verified DGN/Saturn sources");
    expect(nexus_v1_dgn_structure3_capture_manifest_parse(
               valid_manifest, strlen(valid_manifest), &receipt),
           "the baseline no-draw manifest reparses for rejection coverage");
    capture.capture_session_fnv1a64 = receipt.capture_session_fnv1a64;
    expect(!nexus_v1_dgn_structure3_capture_manifest_bind_import(
               NULL, NULL, 0, 0, &receipt, &capture, &import_receipt) &&
               import_receipt.manifest_valid &&
               import_receipt.spans_match_manifest &&
               !import_receipt.raw_span_hashes_match &&
               !import_receipt.binder_invoked &&
               import_receipt.blocks_real_dgn_mesh_render,
           "raw capture import rejects spans whose bytes do not match the manifest");
    capture.capture_bundle_fnv1a64 ^= 1ULL;
    expect(!nexus_v1_dgn_structure3_capture_manifest_bind_import(
               NULL, NULL, 0, 0, &receipt, &capture, &import_receipt) &&
               !import_receipt.capture_bundle_matches &&
               !import_receipt.binder_invoked &&
               import_receipt.blocks_real_dgn_mesh_render,
           "a modified capture-bundle identity cannot reach the DGN binder");

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
