#include "nexus_v1_structure3_capture_manifest.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static int read_u64(const char **cursor, const char *label, uint64_t *out) {
    unsigned long long value;
    int consumed = 0;

    if (!cursor || !*cursor || !label || !out ||
        strncmp(*cursor, label, strlen(label)) != 0 ||
        sscanf(*cursor + strlen(label), "%llx%n", &value, &consumed) != 1 ||
        (*cursor)[strlen(label) + (size_t)consumed] != '\n') return 0;
    *out = (uint64_t)value;
    *cursor += strlen(label) + (size_t)consumed + 1U;
    return 1;
}

static int read_u32(const char **cursor, const char *label, uint32_t *out) {
    uint64_t value;
    if (!read_u64(cursor, label, &value) || value > UINT32_MAX) return 0;
    *out = (uint32_t)value;
    return 1;
}

static uint64_t fnv1a64_update(uint64_t hash, const uint8_t *data, size_t size)
{
    size_t index;
    for (index = 0U; index < size; ++index) {
        hash ^= (uint64_t)data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint64_t fnv1a64_span(uint64_t hash, const uint8_t *data, size_t size)
{
    uint8_t length[8];
    size_t index;

    for (index = 0U; index < sizeof(length); ++index)
        length[index] = (uint8_t)(size >> (index * 8U));
    hash = fnv1a64_update(hash, length, sizeof(length));
    return fnv1a64_update(hash, data, size);
}

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    if (!data || size == 0U) return 0U;
    return fnv1a64_update(UINT64_C(1469598103934665603), data, size);
}

static uint64_t capture_bundle_fnv1a64(
    const Nexus_V1_DgnStructure3CaptureImport *capture)
{
    uint64_t hash = UINT64_C(1469598103934665603);

    if (!capture || !capture->texture_span || !capture->palette_state ||
        !capture->vdp1_state || !capture->transform_state ||
        !capture->normal_culling_state || !capture->vdp1_command ||
        capture->texture_span_size == 0U || capture->palette_state_size == 0U ||
        capture->vdp1_state_size == 0U || capture->transform_state_size == 0U ||
        capture->normal_culling_state_size == 0U ||
        capture->vdp1_command_size == 0U) return 0U;
    hash = fnv1a64_span(hash, capture->texture_span, capture->texture_span_size);
    hash = fnv1a64_span(hash, capture->palette_state, capture->palette_state_size);
    hash = fnv1a64_span(hash, capture->vdp1_state, capture->vdp1_state_size);
    hash = fnv1a64_span(hash, capture->transform_state,
                         capture->transform_state_size);
    hash = fnv1a64_span(hash, capture->normal_culling_state,
                         capture->normal_culling_state_size);
    return fnv1a64_span(hash, capture->vdp1_command,
                         capture->vdp1_command_size);
}

int nexus_v1_dgn_structure3_capture_manifest_parse(
    const char *text, size_t text_size,
    Nexus_V1_DgnStructure3CaptureManifestReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3CaptureManifestReceipt receipt;
    const size_t magic_size =
        sizeof(NEXUS_V1_STRUCTURE3_CAPTURE_MANIFEST_MAGIC) - 1U;
    const char *cursor;
    uint32_t fill_selector;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.blocks_real_dgn_mesh_render = 1;
    *out_receipt = receipt;
    if (!text || text_size <= magic_size ||
        memcmp(text, NEXUS_V1_STRUCTURE3_CAPTURE_MANIFEST_MAGIC,
               magic_size) != 0 || text[magic_size] != '\n') return 0;
    cursor = text + magic_size + 1U;
    if (!read_u64(&cursor, "capture_session_fnv1a64=",
                  &receipt.capture_session_fnv1a64) ||
        !read_u64(&cursor, "dgn_fnv1a64=", &receipt.candidate.dgn_fnv1a64) ||
        !read_u32(&cursor, "structure3_payload_fnv1a32=",
                  &receipt.candidate.structure3_payload_fnv1a32) ||
        !read_u32(&cursor, "typed_mesh_corpus_fnv1a32=",
                  &receipt.candidate.typed_mesh_corpus_fnv1a32) ||
        !read_u32(&cursor, "entry_index=", &receipt.candidate.entry_index) ||
        !read_u32(&cursor, "face_ordinal=", &receipt.candidate.face_ordinal) ||
        !read_u32(&cursor, "face_row_fnv1a32=",
                  &receipt.candidate.face_row_fnv1a32) ||
        !read_u32(&cursor, "referenced_vertex_rows_fnv1a32=",
                  &receipt.candidate.referenced_vertex_rows_fnv1a32) ||
        !read_u32(&cursor, "normal_row_fnv1a32=",
                  &receipt.candidate.normal_row_fnv1a32) ||
        !read_u32(&cursor, "fill_selector=", &fill_selector) ||
        !read_u32(&cursor, "texture_span_bytes=", &receipt.texture_span_bytes) ||
        !read_u64(&cursor, "texture_span_fnv1a64=",
                  &receipt.candidate.texture_span_fnv1a64) ||
        !read_u32(&cursor, "palette_state_bytes=", &receipt.palette_state_bytes) ||
        !read_u64(&cursor, "palette_state_fnv1a64=",
                  &receipt.candidate.palette_state_fnv1a64) ||
        !read_u32(&cursor, "vdp1_state_bytes=", &receipt.vdp1_state_bytes) ||
        !read_u64(&cursor, "vdp1_state_fnv1a64=",
                  &receipt.candidate.vdp1_state_fnv1a64) ||
        !read_u32(&cursor, "transform_state_bytes=", &receipt.transform_state_bytes) ||
        !read_u64(&cursor, "transform_state_fnv1a64=",
                  &receipt.candidate.transform_state_fnv1a64) ||
        !read_u32(&cursor, "normal_culling_state_bytes=",
                  &receipt.normal_culling_state_bytes) ||
        !read_u64(&cursor, "normal_culling_state_fnv1a64=",
                  &receipt.candidate.normal_culling_state_fnv1a64) ||
        !read_u32(&cursor, "vdp1_command_bytes=", &receipt.vdp1_command_bytes) ||
        !read_u64(&cursor, "vdp1_command_fnv1a64=",
                  &receipt.candidate.vdp1_command_fnv1a64) ||
        !read_u64(&cursor, "first_sequence=", &receipt.candidate.first_sequence) ||
        !read_u64(&cursor, "last_sequence=", &receipt.candidate.last_sequence) ||
        *cursor != '\0') return 0;

    if (!receipt.capture_session_fnv1a64 || !receipt.candidate.dgn_fnv1a64 ||
        !receipt.candidate.structure3_payload_fnv1a32 ||
        !receipt.candidate.typed_mesh_corpus_fnv1a32 ||
        fill_selector > UINT16_MAX || !receipt.texture_span_bytes ||
        !receipt.palette_state_bytes || !receipt.vdp1_state_bytes ||
        !receipt.transform_state_bytes || !receipt.normal_culling_state_bytes ||
        !receipt.vdp1_command_bytes || !receipt.candidate.face_row_fnv1a32 ||
        !receipt.candidate.referenced_vertex_rows_fnv1a32 ||
        !receipt.candidate.normal_row_fnv1a32 ||
        !receipt.candidate.texture_span_fnv1a64 ||
        !receipt.candidate.palette_state_fnv1a64 ||
        !receipt.candidate.vdp1_state_fnv1a64 ||
        !receipt.candidate.transform_state_fnv1a64 ||
        !receipt.candidate.normal_culling_state_fnv1a64 ||
        !receipt.candidate.vdp1_command_fnv1a64 ||
        !receipt.candidate.first_sequence ||
        receipt.candidate.first_sequence >= receipt.candidate.last_sequence) return 0;

    receipt.candidate.fill_selector = (uint16_t)fill_selector;
    receipt.valid = 1;
    receipt.complete = 1;
    receipt.original_saturn_capture_verified = 0;
    receipt.renderer_handoff_ready = 0;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_dgn_structure3_capture_manifest_validate_spans(
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *receipt,
    size_t texture_span_size, size_t palette_state_size,
    size_t vdp1_state_size, size_t transform_state_size,
    size_t normal_culling_state_size, size_t vdp1_command_size)
{
    if (!receipt || !receipt->valid || !receipt->complete ||
        receipt->original_saturn_capture_verified ||
        receipt->renderer_handoff_ready || !receipt->blocks_real_dgn_mesh_render)
        return 0;
    return texture_span_size == receipt->texture_span_bytes &&
        palette_state_size == receipt->palette_state_bytes &&
        vdp1_state_size == receipt->vdp1_state_bytes &&
        transform_state_size == receipt->transform_state_bytes &&
        normal_culling_state_size == receipt->normal_culling_state_bytes &&
        vdp1_command_size == receipt->vdp1_command_bytes;
}

int nexus_v1_dgn_structure3_capture_manifest_bind_import(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int dgn_source_hash_verified,
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest,
    const Nexus_V1_DgnStructure3CaptureImport *capture,
    Nexus_V1_DgnStructure3CaptureImportReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3CaptureImportReceipt receipt;
    uint64_t bundle_hash;
    int capture_source_verified;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.blocks_real_dgn_mesh_render = 1;
    *out_receipt = receipt;
    if (!manifest || !capture) return -1;
    receipt.manifest_valid = manifest->valid && manifest->complete &&
        !manifest->original_saturn_capture_verified &&
        !manifest->renderer_handoff_ready &&
        manifest->blocks_real_dgn_mesh_render;
    receipt.dgn_source_hash_verified = dgn_source_hash_verified != 0;
    if (!receipt.manifest_valid) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.spans_match_manifest =
        nexus_v1_dgn_structure3_capture_manifest_validate_spans(
            manifest, capture->texture_span_size, capture->palette_state_size,
            capture->vdp1_state_size, capture->transform_state_size,
            capture->normal_culling_state_size, capture->vdp1_command_size);
    if (!receipt.spans_match_manifest || !capture->texture_span ||
        !capture->palette_state || !capture->vdp1_state ||
        !capture->transform_state || !capture->normal_culling_state ||
        !capture->vdp1_command) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_span_hashes_match =
        fnv1a64(capture->texture_span, capture->texture_span_size) ==
            manifest->candidate.texture_span_fnv1a64 &&
        fnv1a64(capture->palette_state, capture->palette_state_size) ==
            manifest->candidate.palette_state_fnv1a64 &&
        fnv1a64(capture->vdp1_state, capture->vdp1_state_size) ==
            manifest->candidate.vdp1_state_fnv1a64 &&
        fnv1a64(capture->transform_state, capture->transform_state_size) ==
            manifest->candidate.transform_state_fnv1a64 &&
        fnv1a64(capture->normal_culling_state,
                capture->normal_culling_state_size) ==
            manifest->candidate.normal_culling_state_fnv1a64 &&
        fnv1a64(capture->vdp1_command, capture->vdp1_command_size) ==
            manifest->candidate.vdp1_command_fnv1a64;
    receipt.capture_session_matches = capture->capture_session_fnv1a64 != 0U &&
        capture->capture_session_fnv1a64 == manifest->capture_session_fnv1a64;
    bundle_hash = capture_bundle_fnv1a64(capture);
    receipt.capture_bundle_matches = bundle_hash != 0U &&
        bundle_hash == capture->capture_bundle_fnv1a64;
    receipt.capture_bundle_hash_verified =
        capture->capture_bundle_hash_verified != 0;
    receipt.original_saturn_capture_verified =
        capture->original_saturn_capture_verified != 0;
    if (!receipt.raw_span_hashes_match || !receipt.capture_session_matches ||
        !receipt.capture_bundle_matches || !receipt.capture_bundle_hash_verified) {
        *out_receipt = receipt;
        return 0;
    }
    if (capture->texture_span_size > (size_t)INT_MAX ||
        capture->palette_state_size > (size_t)INT_MAX ||
        capture->vdp1_state_size > (size_t)INT_MAX ||
        capture->transform_state_size > (size_t)INT_MAX ||
        capture->normal_culling_state_size > (size_t)INT_MAX ||
        capture->vdp1_command_size > (size_t)INT_MAX) {
        *out_receipt = receipt;
        return 0;
    }
    capture_source_verified = receipt.original_saturn_capture_verified;
    receipt.binder_invoked = 1;
    (void)nexus_v1_dgn_bind_structure3_face_capture_candidate(
        level, dgn_data, dgn_size, receipt.dgn_source_hash_verified,
        capture_source_verified, &manifest->candidate,
        capture->texture_span, (int)capture->texture_span_size,
        capture->palette_state, (int)capture->palette_state_size,
        capture->vdp1_state, (int)capture->vdp1_state_size,
        capture->transform_state, (int)capture->transform_state_size,
        capture->normal_culling_state,
        (int)capture->normal_culling_state_size,
        capture->vdp1_command, (int)capture->vdp1_command_size,
        &receipt.binding);
    receipt.complete_source_binding = receipt.binding.complete_source_binding;
    receipt.renderer_handoff_ready = receipt.binding.renderer_handoff_ready;
    *out_receipt = receipt;
    return receipt.complete_source_binding ? 1 : 0;
}
