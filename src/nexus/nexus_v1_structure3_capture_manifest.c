#include "nexus_v1_structure3_capture_manifest.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) | bytes[3];
}

static uint32_t fnv1a32(const uint8_t *data, size_t size)
{
    uint32_t hash = UINT32_C(2166136261);
    size_t index;

    if (!data || size == 0U) return 0U;
    for (index = 0U; index < size; ++index) {
        hash ^= data[index];
        hash *= UINT32_C(16777619);
    }
    return hash;
}

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

int nexus_v1_dgn_structure3_capture_target_build(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int level_index, int dgn_source_hash_verified, uint32_t entry_index,
    uint32_t face_ordinal, Nexus_V1_DgnStructure3CaptureTargetReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3CaptureTargetReceipt receipt;
    const Nexus_V1_DgnStructure3PayloadReceipt *payload;
    const uint8_t *header;
    const uint8_t *face;
    const uint8_t *normal;
    uint32_t entry_offset;
    uint32_t vertex_offset;
    uint32_t face_offset;
    uint32_t normal_offset;
    uint16_t vertex_count;
    uint16_t face_count;
    uint16_t indexes[4];
    uint32_t vertex_hash = UINT32_C(2166136261);
    int slot_count;
    int slot;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    if (!level || !dgn_data || dgn_size <= 0 || level_index < 0 ||
        !dgn_source_hash_verified) {
        *out_receipt = receipt;
        return 0;
    }
    payload = &level->structure3_payload;
    if (!payload->valid || !level->structure3_directory.valid ||
        !level->structure3_entry_headers.valid || !level->structure3_faces.valid ||
        !level->structure3_vectors.valid || !level->structure3_face_normal_pairs.valid ||
        payload->byte_offset < 0 || payload->byte_size < 4 ||
        payload->byte_offset > dgn_size ||
        payload->byte_size > dgn_size - payload->byte_offset ||
        entry_index >= (uint32_t)level->structure3_directory.entry_count ||
        entry_index > ((uint32_t)payload->byte_size - 8U) / 4U) {
        *out_receipt = receipt;
        return 0;
    }
    entry_offset = read_be32(dgn_data + payload->byte_offset + 4U +
                             entry_index * 4U);
    if (entry_offset > (uint32_t)payload->byte_size ||
        (uint32_t)payload->byte_size - entry_offset < 40U) {
        *out_receipt = receipt;
        return 0;
    }
    header = dgn_data + payload->byte_offset + entry_offset;
    vertex_count = read_be16(header + 4U);
    face_count = read_be16(header + 6U);
    vertex_offset = read_be32(header + 8U);
    face_offset = read_be32(header + 16U);
    normal_offset = read_be32(header + 20U);
    if (face_ordinal >= (uint32_t)face_count ||
        vertex_offset > (uint32_t)payload->byte_size ||
        face_offset > (uint32_t)payload->byte_size ||
        normal_offset > (uint32_t)payload->byte_size ||
        (uint32_t)vertex_count >
            ((uint32_t)payload->byte_size - vertex_offset) / 12U ||
        (uint32_t)face_count >
            ((uint32_t)payload->byte_size - face_offset) / 12U ||
        (uint32_t)face_count >
            ((uint32_t)payload->byte_size - normal_offset) / 12U) {
        *out_receipt = receipt;
        return 0;
    }
    face = dgn_data + payload->byte_offset + face_offset + face_ordinal * 12U;
    normal = dgn_data + payload->byte_offset + normal_offset + face_ordinal * 12U;
    for (slot = 0; slot < 4; ++slot) indexes[slot] = read_be16(face + slot * 2U);
    slot_count = indexes[2] == indexes[3] ? 3 : 4;
    for (slot = 0; slot < slot_count; ++slot) {
        const uint8_t *vertex;
        int byte;
        if (indexes[slot] >= vertex_count) {
            *out_receipt = receipt;
            return 0;
        }
        vertex = dgn_data + payload->byte_offset + vertex_offset +
            indexes[slot] * 12U;
        for (byte = 0; byte < 12; ++byte) {
            vertex_hash ^= vertex[byte];
            vertex_hash *= UINT32_C(16777619);
        }
    }
    receipt.level_index = level_index;
    receipt.candidate.dgn_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    receipt.candidate.structure3_payload_fnv1a32 = payload->raw_payload_hash;
    receipt.candidate.typed_mesh_corpus_fnv1a32 =
        NEXUS_DGN_RETAIL_TYPED_MESH_CORPUS_FNV1A32;
    receipt.candidate.entry_index = entry_index;
    receipt.candidate.face_ordinal = face_ordinal;
    receipt.candidate.face_row_fnv1a32 = fnv1a32(face, 12U);
    receipt.candidate.referenced_vertex_rows_fnv1a32 = vertex_hash;
    receipt.candidate.normal_row_fnv1a32 = fnv1a32(normal, 12U);
    receipt.candidate.fill_selector = read_be16(face + 10U);
    receipt.capture_producer_required = 1;
    receipt.original_saturn_capture_required = 1;
    receipt.valid = receipt.candidate.dgn_fnv1a64 != 0U &&
        receipt.candidate.structure3_payload_fnv1a32 != 0U &&
        receipt.candidate.face_row_fnv1a32 != 0U &&
        receipt.candidate.referenced_vertex_rows_fnv1a32 != 0U &&
        receipt.candidate.normal_row_fnv1a32 != 0U;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_dgn_structure3_capture_target_write(
    const char *path, const Nexus_V1_DgnStructure3CaptureTargetReceipt *target)
{
    char temporary_path[1024];
    FILE *file;

    if (!path || !target || !target->valid ||
        !target->capture_producer_required ||
        !target->original_saturn_capture_required || !target->no_draw_only ||
        target->fallback_visuals_permitted || target->level_index < 0 ||
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp.%ld", path,
                 (long)getpid()) >= (int)sizeof(temporary_path)) return 0;
    file = fopen(temporary_path, "wb");
    if (!file) return 0;
    if (fprintf(file,
                NEXUS_V1_STRUCTURE3_CAPTURE_TARGET_MAGIC "\n"
                "level_index=%x\n"
                "dgn_fnv1a64=%llx\n"
                "structure3_payload_fnv1a32=%x\n"
                "typed_mesh_corpus_fnv1a32=%x\n"
                "entry_index=%x\n"
                "face_ordinal=%x\n"
                "face_row_fnv1a32=%x\n"
                "referenced_vertex_rows_fnv1a32=%x\n"
                "normal_row_fnv1a32=%x\n"
                "fill_selector=%x\n"
                "capture_manifest_magic=" NEXUS_V1_STRUCTURE3_CAPTURE_MANIFEST_MAGIC "\n"
                "required_lanes=texture_span,palette_state,vdp1_state,transform_state,normal_culling_state,vdp1_command\n"
                "original_saturn_capture_required=1\n"
                "no_draw_only=1\n",
                target->level_index,
                (unsigned long long)target->candidate.dgn_fnv1a64,
                target->candidate.structure3_payload_fnv1a32,
                target->candidate.typed_mesh_corpus_fnv1a32,
                target->candidate.entry_index, target->candidate.face_ordinal,
                target->candidate.face_row_fnv1a32,
                target->candidate.referenced_vertex_rows_fnv1a32,
                target->candidate.normal_row_fnv1a32,
                target->candidate.fill_selector) < 0) {
        fclose(file);
        remove(temporary_path);
        return 0;
    }
    if (fclose(file) != 0) {
        remove(temporary_path);
        return 0;
    }
    return rename(temporary_path, path) == 0;
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

static uint64_t capture_trace_order_fnv1a64(
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t lane;

    if (!manifest) return 0U;
    for (lane = 0U; lane < NEXUS_V1_STRUCTURE3_CAPTURE_TRACE_LANE_COUNT;
         ++lane) {
        uint8_t lane_id = (uint8_t)lane;
        uint8_t sequence[8];
        size_t byte;
        if (!manifest->trace_sequence[lane]) return 0U;
        hash = fnv1a64_update(hash, &lane_id, sizeof(lane_id));
        for (byte = 0U; byte < sizeof(sequence); ++byte)
            sequence[byte] = (uint8_t)(manifest->trace_sequence[lane] >>
                                       (byte * 8U));
        hash = fnv1a64_update(hash, sequence, sizeof(sequence));
    }
    return hash;
}

static int trace_sequence_is_valid(
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest)
{
    size_t lane;
    size_t other;

    if (!manifest || !manifest->candidate.first_sequence ||
        manifest->candidate.first_sequence >= manifest->candidate.last_sequence)
        return 0;
    for (lane = 0U; lane < NEXUS_V1_STRUCTURE3_CAPTURE_TRACE_LANE_COUNT;
         ++lane) {
        uint64_t sequence = manifest->trace_sequence[lane];
        if (sequence <= manifest->candidate.first_sequence ||
            sequence >= manifest->candidate.last_sequence) return 0;
        for (other = 0U; other < lane; ++other)
            if (sequence == manifest->trace_sequence[other]) return 0;
    }
    return 1;
}

static uint8_t *read_raw_capture_span(const char *path, size_t expected_size)
{
    FILE *file;
    long file_size;
    uint8_t *bytes;

    if (!path || expected_size == 0U ||
        expected_size > NEXUS_V1_STRUCTURE3_CAPTURE_RAW_SPAN_MAX_BYTES) {
        return NULL;
    }
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (file_size = ftell(file)) < 0L ||
        (size_t)file_size != expected_size || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc(expected_size);
    if (!bytes || fread(bytes, 1U, expected_size, file) != expected_size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    return bytes;
}

void nexus_v1_dgn_structure3_raw_capture_reader_receipt_clear(
    Nexus_V1_DgnStructure3RawCaptureReaderReceipt *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->no_draw_only = 1;
}

void nexus_v1_dgn_structure3_raw_capture_reader_receipt_release(
    Nexus_V1_DgnStructure3RawCaptureReaderReceipt *receipt)
{
    if (!receipt) return;
    free(receipt->texture_span_storage);
    free(receipt->palette_state_storage);
    free(receipt->vdp1_state_storage);
    free(receipt->transform_state_storage);
    free(receipt->normal_culling_state_storage);
    free(receipt->vdp1_command_storage);
    nexus_v1_dgn_structure3_raw_capture_reader_receipt_clear(receipt);
}

int nexus_v1_dgn_structure3_raw_capture_read(
    const Nexus_V1_DgnStructure3CaptureManifestReceipt *manifest,
    const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure3RawCaptureReaderReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3RawCaptureReaderReceipt receipt;
    Nexus_V1_DgnStructure3CaptureImport *packet;

    if (!out_receipt) return 0;
    nexus_v1_dgn_structure3_raw_capture_reader_receipt_clear(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!manifest || !paths || !attestation || !manifest->valid ||
        !manifest->complete || manifest->original_saturn_capture_verified ||
        manifest->renderer_handoff_ready ||
        !manifest->blocks_real_dgn_mesh_render) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.manifest_accepted = 1;
    receipt.texture_span_storage = read_raw_capture_span(
        paths->texture_span_path, manifest->texture_span_bytes);
    receipt.palette_state_storage = read_raw_capture_span(
        paths->palette_state_path, manifest->palette_state_bytes);
    receipt.vdp1_state_storage = read_raw_capture_span(
        paths->vdp1_state_path, manifest->vdp1_state_bytes);
    receipt.transform_state_storage = read_raw_capture_span(
        paths->transform_state_path, manifest->transform_state_bytes);
    receipt.normal_culling_state_storage = read_raw_capture_span(
        paths->normal_culling_state_path, manifest->normal_culling_state_bytes);
    receipt.vdp1_command_storage = read_raw_capture_span(
        paths->vdp1_command_path, manifest->vdp1_command_bytes);
    if (!receipt.texture_span_storage || !receipt.palette_state_storage ||
        !receipt.vdp1_state_storage || !receipt.transform_state_storage ||
        !receipt.normal_culling_state_storage || !receipt.vdp1_command_storage) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.all_spans_read = 1;
    packet = &receipt.import_packet;
    packet->texture_span = receipt.texture_span_storage;
    packet->texture_span_size = manifest->texture_span_bytes;
    packet->palette_state = receipt.palette_state_storage;
    packet->palette_state_size = manifest->palette_state_bytes;
    packet->vdp1_state = receipt.vdp1_state_storage;
    packet->vdp1_state_size = manifest->vdp1_state_bytes;
    packet->transform_state = receipt.transform_state_storage;
    packet->transform_state_size = manifest->transform_state_bytes;
    packet->normal_culling_state = receipt.normal_culling_state_storage;
    packet->normal_culling_state_size = manifest->normal_culling_state_bytes;
    packet->vdp1_command = receipt.vdp1_command_storage;
    packet->vdp1_command_size = manifest->vdp1_command_bytes;
    receipt.raw_span_hashes_match =
        fnv1a64(packet->texture_span, packet->texture_span_size) ==
            manifest->candidate.texture_span_fnv1a64 &&
        fnv1a64(packet->palette_state, packet->palette_state_size) ==
            manifest->candidate.palette_state_fnv1a64 &&
        fnv1a64(packet->vdp1_state, packet->vdp1_state_size) ==
            manifest->candidate.vdp1_state_fnv1a64 &&
        fnv1a64(packet->transform_state, packet->transform_state_size) ==
            manifest->candidate.transform_state_fnv1a64 &&
        fnv1a64(packet->normal_culling_state,
                packet->normal_culling_state_size) ==
            manifest->candidate.normal_culling_state_fnv1a64 &&
        fnv1a64(packet->vdp1_command, packet->vdp1_command_size) ==
            manifest->candidate.vdp1_command_fnv1a64;
    packet->capture_session_fnv1a64 = attestation->capture_session_fnv1a64;
    packet->capture_bundle_fnv1a64 = capture_bundle_fnv1a64(packet);
    receipt.capture_trace_order_fnv1a64 = capture_trace_order_fnv1a64(manifest);
    packet->capture_trace_order_fnv1a64 = receipt.capture_trace_order_fnv1a64;
    receipt.attestation_session_matches =
        attestation->capture_session_fnv1a64 != 0U &&
        attestation->capture_session_fnv1a64 == manifest->capture_session_fnv1a64;
    receipt.attestation_bundle_matches = packet->capture_bundle_fnv1a64 != 0U &&
        packet->capture_bundle_fnv1a64 == attestation->capture_bundle_fnv1a64;
    receipt.attestation_trace_order_matches =
        receipt.capture_trace_order_fnv1a64 != 0U &&
        receipt.capture_trace_order_fnv1a64 ==
            attestation->capture_trace_order_fnv1a64;
    receipt.original_saturn_source_attested =
        attestation->original_saturn_source_attested != 0;
    packet->capture_bundle_hash_verified = receipt.raw_span_hashes_match &&
        receipt.attestation_session_matches && receipt.attestation_bundle_matches &&
        receipt.attestation_trace_order_matches;
    packet->original_saturn_capture_verified =
        packet->capture_bundle_hash_verified &&
        receipt.original_saturn_source_attested;
    packet->capture_trace_order_verified =
        receipt.attestation_trace_order_matches;
    receipt.import_ready = packet->original_saturn_capture_verified;
    *out_receipt = receipt;
    return receipt.import_ready;
}

void nexus_v1_dgn_structure3_capture_host_receipt_clear(
    Nexus_V1_DgnStructure3CaptureHostReceipt *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->no_draw_only = 1;
    receipt->manifest.blocks_real_dgn_mesh_render = 1;
    receipt->import_receipt.blocks_real_dgn_mesh_render = 1;
}

void nexus_v1_dgn_structure3_raw_capture_host_receipt_clear(
    Nexus_V1_DgnStructure3RawCaptureHostReceipt *receipt)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->no_draw_only = 1;
    nexus_v1_dgn_structure3_raw_capture_reader_receipt_clear(
        &receipt->raw_reader);
    nexus_v1_dgn_structure3_capture_host_receipt_clear(&receipt->host);
}

void nexus_v1_dgn_structure3_raw_capture_host_receipt_release(
    Nexus_V1_DgnStructure3RawCaptureHostReceipt *receipt)
{
    if (!receipt) return;
    nexus_v1_dgn_structure3_raw_capture_reader_receipt_release(
        &receipt->raw_reader);
    nexus_v1_dgn_structure3_raw_capture_host_receipt_clear(receipt);
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
        !read_u64(&cursor, "texture_span_sequence=",
                  &receipt.trace_sequence[NEXUS_V1_STRUCTURE3_TRACE_TEXTURE_SPAN]) ||
        !read_u64(&cursor, "palette_state_sequence=",
                  &receipt.trace_sequence[NEXUS_V1_STRUCTURE3_TRACE_PALETTE_STATE]) ||
        !read_u64(&cursor, "vdp1_state_sequence=",
                  &receipt.trace_sequence[NEXUS_V1_STRUCTURE3_TRACE_VDP1_STATE]) ||
        !read_u64(&cursor, "transform_state_sequence=",
                  &receipt.trace_sequence[NEXUS_V1_STRUCTURE3_TRACE_TRANSFORM_STATE]) ||
        !read_u64(&cursor, "normal_culling_state_sequence=",
                  &receipt.trace_sequence[NEXUS_V1_STRUCTURE3_TRACE_NORMAL_CULLING_STATE]) ||
        !read_u64(&cursor, "vdp1_command_sequence=",
                  &receipt.trace_sequence[NEXUS_V1_STRUCTURE3_TRACE_VDP1_COMMAND]) ||
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
        !trace_sequence_is_valid(&receipt)) return 0;

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
    receipt.capture_trace_order_matches =
        capture->capture_trace_order_fnv1a64 != 0U &&
        capture->capture_trace_order_fnv1a64 ==
            capture_trace_order_fnv1a64(manifest);
    receipt.capture_bundle_hash_verified =
        capture->capture_bundle_hash_verified != 0;
    receipt.capture_trace_order_verified =
        capture->capture_trace_order_verified != 0;
    receipt.original_saturn_capture_verified =
        capture->original_saturn_capture_verified != 0;
    if (!receipt.raw_span_hashes_match || !receipt.capture_session_matches ||
        !receipt.capture_bundle_matches || !receipt.capture_trace_order_matches ||
        !receipt.capture_bundle_hash_verified ||
        !receipt.capture_trace_order_verified) {
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
    capture_source_verified = receipt.original_saturn_capture_verified &&
        receipt.capture_trace_order_verified;
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

int nexus_v1_dgn_structure3_capture_host_intake(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int dgn_source_hash_verified, const char *manifest_text,
    size_t manifest_size, const Nexus_V1_DgnStructure3CaptureImport *capture,
    Nexus_V1_DgnStructure3CaptureHostReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3CaptureHostReceipt receipt;

    if (!out_receipt) return -1;
    nexus_v1_dgn_structure3_capture_host_receipt_clear(&receipt);
    *out_receipt = receipt;
    if (!level || !dgn_data || dgn_size <= 0 || !manifest_text || !capture)
        return 0;
    receipt.host_dgn_source_verified = dgn_source_hash_verified != 0;
    receipt.capture_source_verified =
        capture->original_saturn_capture_verified != 0 &&
        capture->capture_trace_order_verified != 0;
    receipt.manifest_parsed = nexus_v1_dgn_structure3_capture_manifest_parse(
        manifest_text, manifest_size, &receipt.manifest);
    /* The external Saturn verdict is an admission precondition, not a byte
     * property. Do not let a locally assembled packet probe the binder. */
    if (!receipt.host_dgn_source_verified || !receipt.capture_source_verified ||
        !receipt.manifest_parsed) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.importer_invoked = 1;
    (void)nexus_v1_dgn_structure3_capture_manifest_bind_import(
        level, dgn_data, dgn_size, receipt.host_dgn_source_verified,
        &receipt.manifest, capture, &receipt.import_receipt);
    *out_receipt = receipt;
    return receipt.import_receipt.complete_source_binding ? 1 : 0;
}

int nexus_v1_dgn_structure3_raw_capture_host_intake(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int dgn_source_hash_verified, const char *manifest_text,
    size_t manifest_size, const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure3RawCaptureHostReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3RawCaptureHostReceipt receipt;
    Nexus_V1_DgnStructure3CaptureManifestReceipt manifest;

    if (!out_receipt) return -1;
    nexus_v1_dgn_structure3_raw_capture_host_receipt_clear(&receipt);
    *out_receipt = receipt;
    if (!manifest_text || !paths || !attestation) return 0;
    receipt.manifest_parsed = nexus_v1_dgn_structure3_capture_manifest_parse(
        manifest_text, manifest_size, &manifest);
    if (!receipt.manifest_parsed) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_reader_invoked = 1;
    if (!nexus_v1_dgn_structure3_raw_capture_read(
            &manifest, paths, attestation, &receipt.raw_reader)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.host_intake_invoked = 1;
    (void)nexus_v1_dgn_structure3_capture_host_intake(
        level, dgn_data, dgn_size, dgn_source_hash_verified, manifest_text,
        manifest_size, &receipt.raw_reader.import_packet, &receipt.host);
    *out_receipt = receipt;
    return receipt.host.import_receipt.complete_source_binding ? 1 : 0;
}
