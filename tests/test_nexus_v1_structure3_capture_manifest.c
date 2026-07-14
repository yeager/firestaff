#include "nexus_v1_structure3_capture_manifest.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
    "texture_span_sequence=17\npalette_state_sequence=18\n"
    "vdp1_state_sequence=19\ntransform_state_sequence=1a\n"
    "normal_culling_state_sequence=1b\nvdp1_command_sequence=1c\n"
    "first_sequence=16\nlast_sequence=1d\n";

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

static unsigned long long trace_order_hash(
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest) {
    unsigned long long hash = 1469598103934665603ULL;
    size_t lane;

    for (lane = 0U; lane < NEXUS_V1_STRUCTURE3_CAPTURE_TRACE_LANE_COUNT;
         ++lane) {
        unsigned char lane_id = (unsigned char)lane;
        unsigned char sequence[8];
        size_t byte;
        hash = fnv1a64_update(hash, &lane_id, sizeof(lane_id));
        for (byte = 0U; byte < sizeof(sequence); ++byte)
            sequence[byte] = (unsigned char)(manifest->trace_sequence[lane] >>
                                             (byte * 8U));
        hash = fnv1a64_update(hash, sequence, sizeof(sequence));
    }
    return hash;
}

static int write_capture_file(const char *path, const unsigned char *data,
                              size_t size) {
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (fwrite(data, 1U, size, file) != size) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

static void wb16(unsigned char *p, unsigned int value) {
    p[0] = (unsigned char)(value >> 8);
    p[1] = (unsigned char)value;
}

static void wb32(unsigned char *p, unsigned int value) {
    p[0] = (unsigned char)(value >> 24);
    p[1] = (unsigned char)(value >> 16);
    p[2] = (unsigned char)(value >> 8);
    p[3] = (unsigned char)value;
}

static unsigned int fnv1a32(const unsigned char *data, size_t size) {
    unsigned int hash = 2166136261U;
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619U;
    }
    return hash;
}

int main(void) {
    Nexus_V1_DgnStructure3CaptureManifestReceipt receipt;
    Nexus_V1_DgnStructure3CaptureImport capture;
    Nexus_V1_DgnStructure3CaptureImportReceipt import_receipt;
    Nexus_V1_DgnStructure3CaptureHostReceipt host_receipt;
    Nexus_V1_DgnStructure3RawCaptureReaderReceipt raw_receipt;
    Nexus_V1_DgnStructure3RawCaptureHostReceipt raw_host_receipt;
    Nexus_V1_DgnStructure3RawCapturePaths raw_paths;
    Nexus_V1_DgnStructure3RawCaptureAttestation raw_attestation;
    Nexus_V1_DgnStructure3CaptureTargetReceipt target;
    Nexus_V1_Level level;
    Nexus_V1_Level target_level;
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
    unsigned char altered_palette[sizeof(palette)];
    char malformed[sizeof(valid_manifest)];
    char raw_paths_storage[6][128];
    char target_path[128];
    char target_text[1024];
    unsigned char target_dgn[128];
    FILE *target_file;

    memset(&target_level, 0, sizeof(target_level));
    memset(target_dgn, 0, sizeof(target_dgn));
    wb32(target_dgn + 4, 16U);
    wb16(target_dgn + 16 + 4, 3U);
    wb16(target_dgn + 16 + 6, 1U);
    wb32(target_dgn + 16 + 8, 56U);
    wb32(target_dgn + 16 + 16, 92U);
    wb32(target_dgn + 16 + 20, 104U);
    wb16(target_dgn + 56, 0U); wb16(target_dgn + 56 + 4, 0U); wb16(target_dgn + 56 + 8, 0U);
    wb16(target_dgn + 68, 1U); wb16(target_dgn + 68 + 4, 0U); wb16(target_dgn + 68 + 8, 0U);
    wb16(target_dgn + 80, 0U); wb16(target_dgn + 80 + 4, 1U); wb16(target_dgn + 80 + 8, 0U);
    wb16(target_dgn + 92, 0U); wb16(target_dgn + 94, 1U);
    wb16(target_dgn + 96, 2U); wb16(target_dgn + 98, 2U);
    wb16(target_dgn + 102, 0x0001U);
    target_level.structure3_payload.valid = 1;
    target_level.structure3_payload.byte_offset = 0;
    target_level.structure3_payload.byte_size = 116;
    target_level.structure3_payload.raw_payload_hash = fnv1a32(target_dgn, 116U);
    target_level.structure3_directory.valid = 1;
    target_level.structure3_directory.entry_count = 1;
    target_level.structure3_entry_headers.valid = 1;
    target_level.structure3_faces.valid = 1;
    target_level.structure3_vectors.valid = 1;
    target_level.structure3_face_normal_pairs.valid = 1;
    expect(nexus_v1_dgn_structure3_capture_target_build(
               &target_level, target_dgn, 116, 1, 1, 0U, 0U, &target) &&
               target.valid && target.level_index == 1 &&
               target.candidate.entry_index == 0U &&
               target.candidate.face_ordinal == 0U &&
               target.candidate.fill_selector == 1U &&
               target.capture_producer_required &&
               target.original_saturn_capture_required && target.no_draw_only &&
               !target.fallback_visuals_permitted,
           "canonical Structure3 rows build one no-draw external capture target");
    expect(!nexus_v1_dgn_structure3_capture_target_build(
               &target_level, target_dgn, 116, 1, 0, 0U, 0U, &target) &&
               !target.valid && target.no_draw_only,
           "an unverified DGN source cannot generate a capture target");
    expect(nexus_v1_dgn_structure3_capture_target_build(
               &target_level, target_dgn, 116, 1, 1, 0U, 0U, &target),
           "verified target rebuilds for producer request output");
    snprintf(target_path, sizeof(target_path),
             "/tmp/firestaff-nexus-structure3-target-%ld.txt", (long)getpid());
    remove(target_path);
    expect(nexus_v1_dgn_structure3_capture_target_write(target_path, &target) &&
               (target_file = fopen(target_path, "rb")) != NULL &&
               fread(target_text, 1U, sizeof(target_text) - 1U, target_file) > 0U &&
               fclose(target_file) == 0,
           "capture target writer emits a producer request from verified DGN rows");
    target_file = fopen(target_path, "rb");
    if (target_file) {
        size_t target_size = fread(target_text, 1U, sizeof(target_text) - 1U,
                                   target_file);
        target_text[target_size] = '\0';
        fclose(target_file);
        expect(strstr(target_text, NEXUS_V1_STRUCTURE3_CAPTURE_TARGET_MAGIC) != NULL &&
                   strstr(target_text, "required_lanes=texture_span,palette_state,vdp1_state,transform_state,normal_culling_state,vdp1_command") != NULL &&
                   strstr(target_text, "no_draw_only=1") != NULL,
               "capture target names every required raw lane without manufacturing bytes");
    }
    remove(target_path);

    expect(nexus_v1_dgn_structure3_capture_manifest_parse(
               valid_manifest, strlen(valid_manifest), &receipt) &&
               receipt.valid && receipt.complete &&
               receipt.capture_session_fnv1a64 == 1U &&
               receipt.candidate.entry_index == 4U &&
               receipt.candidate.face_ordinal == 5U &&
               receipt.candidate.fill_selector == 9U &&
               receipt.texture_span_bytes == 10U &&
               receipt.palette_state_bytes == 12U &&
               receipt.trace_sequence[NEXUS_V1_STRUCTURE3_TRACE_TEXTURE_SPAN] ==
                   0x17U &&
               !receipt.original_saturn_capture_verified &&
               !receipt.renderer_handoff_ready &&
               receipt.blocks_real_dgn_mesh_render,
           "complete correlation manifest remains no-draw provenance only");
    expect(!nexus_v1_dgn_structure3_capture_target_matches_manifest(
               &target, &receipt),
           "a producer manifest for another source face cannot satisfy the target");
    receipt.candidate = target.candidate;
    expect(nexus_v1_dgn_structure3_capture_target_matches_manifest(
               &target, &receipt),
           "a completed no-draw manifest must match every requested source row");
    receipt.candidate.face_row_fnv1a32 ^= 1U;
    expect(!nexus_v1_dgn_structure3_capture_target_matches_manifest(
               &target, &receipt),
           "a changed requested face row rejects the producer manifest");
    receipt.candidate = target.candidate;
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
        "texture_span_sequence=17\npalette_state_sequence=18\n"
        "vdp1_state_sequence=19\ntransform_state_sequence=1a\n"
        "normal_culling_state_sequence=1b\nvdp1_command_sequence=1c\n"
        "first_sequence=16\nlast_sequence=1d\n",
        sizeof(texture), fnv1a64(texture, sizeof(texture)),
        sizeof(palette), fnv1a64(palette, sizeof(palette)),
        sizeof(vdp1), fnv1a64(vdp1, sizeof(vdp1)),
        sizeof(transform), fnv1a64(transform, sizeof(transform)),
        sizeof(culling), fnv1a64(culling, sizeof(culling)),
        sizeof(command), fnv1a64(command, sizeof(command)));
    capture.original_saturn_capture_verified = 1;
    capture.capture_trace_order_fnv1a64 = trace_order_hash(&receipt);
    capture.capture_trace_order_verified = 1;
    expect(!nexus_v1_dgn_structure3_capture_host_intake(
               &level, texture, (int)sizeof(texture), 1,
               imported_manifest, strlen(imported_manifest), &capture,
               &host_receipt) && host_receipt.capture_source_verified &&
               host_receipt.importer_invoked &&
               host_receipt.import_receipt.binder_invoked &&
               !host_receipt.import_receipt.complete_source_binding &&
               host_receipt.no_draw_only,
           "attested Saturn evidence can reach the existing no-draw binder");
    expect(nexus_v1_dgn_structure3_capture_manifest_parse(
               imported_manifest, strlen(imported_manifest), &receipt),
           "raw reader starts from a byte-correlated capture manifest");
    for (int path_index = 0; path_index < 6; ++path_index) {
        snprintf(raw_paths_storage[path_index], sizeof(raw_paths_storage[path_index]),
                 "/tmp/firestaff-nexus-structure3-%ld-%d.bin",
                 (long)getpid(), path_index);
    }
    expect(write_capture_file(raw_paths_storage[0], texture, sizeof(texture)) &&
               write_capture_file(raw_paths_storage[1], palette, sizeof(palette)) &&
               write_capture_file(raw_paths_storage[2], vdp1, sizeof(vdp1)) &&
               write_capture_file(raw_paths_storage[3], transform, sizeof(transform)) &&
               write_capture_file(raw_paths_storage[4], culling, sizeof(culling)) &&
               write_capture_file(raw_paths_storage[5], command, sizeof(command)),
           "raw capture fixture writes six opaque original-trace lanes");
    memset(&raw_paths, 0, sizeof(raw_paths));
    raw_paths.texture_span_path = raw_paths_storage[0];
    raw_paths.palette_state_path = raw_paths_storage[1];
    raw_paths.vdp1_state_path = raw_paths_storage[2];
    raw_paths.transform_state_path = raw_paths_storage[3];
    raw_paths.normal_culling_state_path = raw_paths_storage[4];
    raw_paths.vdp1_command_path = raw_paths_storage[5];
    memset(&raw_attestation, 0, sizeof(raw_attestation));
    raw_attestation.capture_session_fnv1a64 = receipt.capture_session_fnv1a64;
    raw_attestation.capture_bundle_fnv1a64 = bundle_hash(&capture);
    raw_attestation.capture_trace_order_fnv1a64 = trace_order_hash(&receipt);
    raw_attestation.original_saturn_source_attested = 1;
    nexus_v1_dgn_structure3_raw_capture_reader_receipt_clear(&raw_receipt);
    expect(nexus_v1_dgn_structure3_raw_capture_read(
               &receipt, &raw_paths, &raw_attestation, &raw_receipt) &&
               raw_receipt.manifest_accepted && raw_receipt.all_spans_read &&
               raw_receipt.raw_span_hashes_match &&
               raw_receipt.attestation_session_matches &&
               raw_receipt.attestation_bundle_matches &&
               raw_receipt.attestation_trace_order_matches &&
               raw_receipt.import_ready && raw_receipt.no_draw_only &&
               raw_receipt.import_packet.original_saturn_capture_verified &&
               raw_receipt.import_packet.texture_span != texture,
           "raw capture reader retains six file-backed spans only after atomically matching manifest and external attestation");
    nexus_v1_dgn_structure3_raw_capture_reader_receipt_release(&raw_receipt);
    raw_attestation.capture_trace_order_fnv1a64 ^= 1ULL;
    expect(!nexus_v1_dgn_structure3_raw_capture_read(
               &receipt, &raw_paths, &raw_attestation, &raw_receipt) &&
               raw_receipt.all_spans_read && raw_receipt.raw_span_hashes_match &&
               raw_receipt.attestation_session_matches &&
               raw_receipt.attestation_bundle_matches &&
               !raw_receipt.attestation_trace_order_matches &&
               !raw_receipt.import_ready && raw_receipt.no_draw_only,
           "a mismatched trace-order attestation rejects otherwise matching raw bytes atomically");
    nexus_v1_dgn_structure3_raw_capture_reader_receipt_release(&raw_receipt);
    raw_attestation.capture_trace_order_fnv1a64 = trace_order_hash(&receipt);
    expect(!nexus_v1_dgn_structure3_raw_capture_host_intake(
               &level, texture, (int)sizeof(texture), 1, imported_manifest,
               strlen(imported_manifest), &raw_paths, &raw_attestation,
               &raw_host_receipt) && raw_host_receipt.manifest_parsed &&
               raw_host_receipt.raw_reader_invoked &&
               raw_host_receipt.raw_reader.import_ready &&
               raw_host_receipt.host_intake_invoked &&
               raw_host_receipt.host.capture_source_verified &&
               raw_host_receipt.host.importer_invoked &&
               raw_host_receipt.no_draw_only &&
               raw_host_receipt.host.no_draw_only,
           "the package-to-host route forwards only the atomically attested opaque packet and remains no-draw");
    nexus_v1_dgn_structure3_raw_capture_host_receipt_release(&raw_host_receipt);
    memcpy(altered_palette, palette, sizeof(altered_palette));
    altered_palette[0] ^= 1U;
    expect(write_capture_file(raw_paths_storage[1], altered_palette,
                              sizeof(altered_palette)) &&
               !nexus_v1_dgn_structure3_raw_capture_read(
                   &receipt, &raw_paths, &raw_attestation, &raw_receipt) &&
               raw_receipt.manifest_accepted && raw_receipt.all_spans_read &&
               !raw_receipt.raw_span_hashes_match && !raw_receipt.import_ready &&
               raw_receipt.no_draw_only &&
               !raw_receipt.import_packet.original_saturn_capture_verified,
           "one altered raw span rejects the complete capture atomically before any host import");
    nexus_v1_dgn_structure3_raw_capture_reader_receipt_release(&raw_receipt);
    expect(!nexus_v1_dgn_structure3_raw_capture_host_intake(
               &level, texture, (int)sizeof(texture), 1, imported_manifest,
               strlen(imported_manifest), &raw_paths, &raw_attestation,
               &raw_host_receipt) && raw_host_receipt.manifest_parsed &&
               raw_host_receipt.raw_reader_invoked &&
               !raw_host_receipt.raw_reader.import_ready &&
               !raw_host_receipt.host_intake_invoked &&
               raw_host_receipt.no_draw_only,
           "one altered raw lane prevents the host intake atomically");
    nexus_v1_dgn_structure3_raw_capture_host_receipt_release(&raw_host_receipt);
    for (int path_index = 0; path_index < 6; ++path_index)
        remove(raw_paths_storage[path_index]);
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
    memcpy(strstr(malformed, "palette_state_sequence=18"),
           "palette_state_sequence=17", 25U);
    expect(!nexus_v1_dgn_structure3_capture_manifest_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "duplicate capture trace ordinal is rejected");

    snprintf(malformed, sizeof(malformed), "%s", valid_manifest);
    memcpy(strstr(malformed, "palette_state_fnv1a64=d"),
           "palette_state_missing=", 22U);
    expect(!nexus_v1_dgn_structure3_capture_manifest_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "missing palette correlation rejects the manifest");

    return failures ? 1 : 0;
}
