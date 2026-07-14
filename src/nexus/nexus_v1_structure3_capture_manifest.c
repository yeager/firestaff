#include "nexus_v1_structure3_capture_manifest.h"

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
