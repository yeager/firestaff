#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_track02_capture_trace_manifest.h"

#define THERON_V1_TRACK02_CAPTURE_MANIFEST_MAX_BYTES (64u * 1024u)
#define THERON_V1_TRACK02_CAPTURE_MANIFEST_REQUIRED_KEYS 15u

enum {
    THERON_V1_CAPTURE_KEY_FORMAT = 1u << 0,
    THERON_V1_CAPTURE_KEY_MD5 = 1u << 1,
    THERON_V1_CAPTURE_KEY_VARIANT = 1u << 2,
    THERON_V1_CAPTURE_KEY_RECORD = 1u << 3,
    THERON_V1_CAPTURE_KEY_DESTINATION = 1u << 4,
    THERON_V1_CAPTURE_KEY_PAYLOAD_BYTES = 1u << 5,
    THERON_V1_CAPTURE_KEY_PAYLOAD_CHECKSUM = 1u << 6,
    THERON_V1_CAPTURE_KEY_TRACE_CHECKSUM = 1u << 7,
    THERON_V1_CAPTURE_KEY_DUNGEON_PC = 1u << 8,
    THERON_V1_CAPTURE_KEY_DUNGEON_OFFSET = 1u << 9,
    THERON_V1_CAPTURE_KEY_DUNGEON_BYTES = 1u << 10,
    THERON_V1_CAPTURE_KEY_DUNGEON_CHECKSUM = 1u << 11,
    THERON_V1_CAPTURE_KEY_OBJECT_PC = 1u << 12,
    THERON_V1_CAPTURE_KEY_OBJECT_OFFSET = 1u << 13,
    THERON_V1_CAPTURE_KEY_OBJECT_BYTES = 1u << 14,
    THERON_V1_CAPTURE_KEY_OBJECT_CHECKSUM = 1u << 15
};

#define THERON_V1_CAPTURE_KEY_ALL ((1u << 16) - 1u)

static int theron_v1_capture_parse_unsigned(const char *text,
                                             unsigned long long *out) {
    char *end;
    unsigned long long value;
    if (!text || !text[0] || !out || text[0] == '-') return 0;
    value = strtoull(text, &end, 0);
    if (*end != '\0') return 0;
    *out = value;
    return 1;
}

static Theron_Track02Variant theron_v1_capture_parse_variant(const char *text) {
    if (!strcmp(text, "jp_bin")) return THERON_TRACK02_VARIANT_JP_BIN;
    if (!strcmp(text, "us_bin")) return THERON_TRACK02_VARIANT_US_BIN;
    if (!strcmp(text, "us_iso")) return THERON_TRACK02_VARIANT_US_ISO;
    if (!strcmp(text, "jp_rev1_iso")) return THERON_TRACK02_VARIANT_JP_REV1_ISO;
    return THERON_TRACK02_VARIANT_UNKNOWN;
}

static int theron_v1_capture_parse_value(const char *key, const char *value,
                                         unsigned int *seen,
                                         Theron_V1Track02CaptureTraceManifest *out) {
    unsigned long long number;
    unsigned int bit;

    if (!strcmp(key, "format")) {
        bit = THERON_V1_CAPTURE_KEY_FORMAT;
        if (strcmp(value, "theron_track02_capture_trace_v1")) return 0;
    } else if (!strcmp(key, "track02_md5")) {
        bit = THERON_V1_CAPTURE_KEY_MD5;
        if (strlen(value) != 32u || theron_v1_track02_variant_for_md5(value) ==
                                      THERON_TRACK02_VARIANT_UNKNOWN) return 0;
        snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", value);
    } else if (!strcmp(key, "track02_variant")) {
        bit = THERON_V1_CAPTURE_KEY_VARIANT;
        if ((out->track02_variant = theron_v1_capture_parse_variant(value)) ==
            THERON_TRACK02_VARIANT_UNKNOWN) return 0;
    } else {
        if (!theron_v1_capture_parse_unsigned(value, &number) ||
            number > UINT32_MAX) return 0;
        if (!strcmp(key, "loader_record")) {
            bit = THERON_V1_CAPTURE_KEY_RECORD; out->loader_record = (uint32_t)number;
        } else if (!strcmp(key, "loader_destination")) {
            bit = THERON_V1_CAPTURE_KEY_DESTINATION; out->loader_destination = (uint32_t)number;
        } else if (!strcmp(key, "loader_payload_bytes")) {
            bit = THERON_V1_CAPTURE_KEY_PAYLOAD_BYTES; out->loader_payload_bytes = (size_t)number;
        } else if (!strcmp(key, "loader_payload_checksum")) {
            bit = THERON_V1_CAPTURE_KEY_PAYLOAD_CHECKSUM; out->loader_payload_checksum = (uint32_t)number;
        } else if (!strcmp(key, "consumer_trace_checksum")) {
            bit = THERON_V1_CAPTURE_KEY_TRACE_CHECKSUM; out->consumer_trace_checksum = (uint32_t)number;
        } else if (!strcmp(key, "dungeon_record_consumer_pc")) {
            bit = THERON_V1_CAPTURE_KEY_DUNGEON_PC; out->dungeon_record_consumer_pc = (uint32_t)number;
        } else if (!strcmp(key, "dungeon_record_payload_offset")) {
            bit = THERON_V1_CAPTURE_KEY_DUNGEON_OFFSET; out->dungeon_record_payload_offset = (size_t)number;
        } else if (!strcmp(key, "dungeon_record_byte_count")) {
            bit = THERON_V1_CAPTURE_KEY_DUNGEON_BYTES; out->dungeon_record_byte_count = (size_t)number;
        } else if (!strcmp(key, "dungeon_record_window_checksum")) {
            bit = THERON_V1_CAPTURE_KEY_DUNGEON_CHECKSUM; out->dungeon_record_window_checksum = (uint32_t)number;
        } else if (!strcmp(key, "object_table_consumer_pc")) {
            bit = THERON_V1_CAPTURE_KEY_OBJECT_PC; out->object_table_consumer_pc = (uint32_t)number;
        } else if (!strcmp(key, "object_table_payload_offset")) {
            bit = THERON_V1_CAPTURE_KEY_OBJECT_OFFSET; out->object_table_payload_offset = (size_t)number;
        } else if (!strcmp(key, "object_table_byte_count")) {
            bit = THERON_V1_CAPTURE_KEY_OBJECT_BYTES; out->object_table_byte_count = (size_t)number;
        } else if (!strcmp(key, "object_table_window_checksum")) {
            bit = THERON_V1_CAPTURE_KEY_OBJECT_CHECKSUM; out->object_table_window_checksum = (uint32_t)number;
        } else return 0;
    }
    if (*seen & bit) return 0;
    *seen |= bit;
    return 1;
}

int theron_v1_track02_capture_trace_manifest_parse(
    const char *manifest_path, Theron_V1Track02CaptureTraceManifest *out) {
    Theron_V1Track02CaptureTraceManifest manifest = {0};
    FILE *file;
    long bytes;
    char line[512];
    unsigned int seen = 0u;

    if (!out) return 0;
    *out = manifest;
    if (!manifest_path || !manifest_path[0] || !(file = fopen(manifest_path, "rb"))) {
        manifest.status = THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_UNAVAILABLE;
        *out = manifest;
        return 1;
    }
    if (fseek(file, 0L, SEEK_END) != 0 || (bytes = ftell(file)) <= 0 ||
        (unsigned long)bytes > THERON_V1_TRACK02_CAPTURE_MANIFEST_MAX_BYTES ||
        fseek(file, 0L, SEEK_SET) != 0) goto rejected;
    while (fgets(line, sizeof(line), file)) {
        char *key = line;
        char *equals;
        char *value;
        char *end;
        while (isspace((unsigned char)*key)) ++key;
        if (*key == '#' || *key == '\0') continue;
        end = key + strlen(key);
        while (end > key && isspace((unsigned char)end[-1])) *--end = '\0';
        equals = strchr(key, '=');
        if (!equals || equals == key || strchr(equals + 1, '=')) goto rejected;
        *equals = '\0';
        value = equals + 1;
        if (!*value || strpbrk(key, " \t\r\n") || strpbrk(value, " \t\r\n") ||
            !theron_v1_capture_parse_value(key, value, &seen, &manifest)) goto rejected;
    }
    fclose(file);
    if (seen != THERON_V1_CAPTURE_KEY_ALL ||
        manifest.track02_variant != theron_v1_track02_variant_for_md5(manifest.track02_md5)) goto rejected_closed;
    manifest.status = THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_READY;
    *out = manifest;
    return 1;
rejected:
    fclose(file);
rejected_closed:
    manifest.status = THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_REJECTED;
    *out = manifest;
    return 1;
}

int theron_v1_track02_capture_trace_manifest_bind(
    const Theron_V1Track02RawMediaIntakeReceipt *intake,
    const Theron_V1Track02RawTraceMediaInput *raw_trace,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    const Theron_V1Track02CaptureTraceManifest *manifest,
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt *out) {
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt receipt = {0};

    if (!out) return 0;
    *out = receipt;
    if (!intake || !raw_trace || !provenance || !preparation || !manifest ||
        intake->status != THERON_V1_TRACK02_MEDIA_INTAKE_READY || !raw_trace->valid ||
        !provenance->valid || !preparation->valid ||
        manifest->status != THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_READY ||
        !intake->raw_trace_preparation_allowed ||
        intake->variant != raw_trace->variant || intake->variant != provenance->track02_variant ||
        intake->variant != preparation->track02_variant || intake->variant != manifest->track02_variant ||
        strcmp(intake->track02_md5, raw_trace->track02_md5) ||
        strcmp(intake->track02_md5, provenance->track02_md5) ||
        strcmp(intake->track02_md5, preparation->track02_md5) ||
        strcmp(intake->track02_md5, manifest->track02_md5) ||
        manifest->loader_record != provenance->loader_record ||
        manifest->loader_record != preparation->loader_record ||
        manifest->loader_destination != provenance->loader_destination ||
        manifest->loader_payload_bytes != provenance->loader_payload_bytes ||
        manifest->loader_payload_checksum != provenance->loader_payload_checksum ||
        manifest->consumer_trace_checksum != preparation->consumer_trace_checksum ||
        manifest->dungeon_record_consumer_pc != preparation->dungeon_record_consumer_pc ||
        manifest->dungeon_record_payload_offset != preparation->dungeon_record_payload_offset ||
        manifest->dungeon_record_byte_count != preparation->dungeon_record_byte_count ||
        manifest->dungeon_record_window_checksum != preparation->dungeon_record_window_checksum ||
        manifest->object_table_consumer_pc != preparation->object_table_consumer_pc ||
        manifest->object_table_payload_offset != preparation->object_table_payload_offset ||
        manifest->object_table_byte_count != preparation->object_table_byte_count ||
        manifest->object_table_window_checksum != preparation->object_table_window_checksum ||
        preparation->level_admission_allowed || preparation->object_admission_allowed ||
        preparation->bitmap_palette_admission_allowed || preparation->dungeon_draw_allowed ||
        preparation->fallback_visuals_allowed) return 0;

    receipt.valid = 1;
    receipt.raw_media_intake_consumed = 1;
    receipt.raw_trace_input_consumed = 1;
    receipt.provenance_runtime_consumed = 1;
    receipt.trace_preparation_consumed = 1;
    receipt.external_capture_manifest_consumed = 1;
    receipt.track02_variant = intake->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", intake->track02_md5);
    receipt.first_user_data_offset = raw_trace->first_user_data_offset;
    receipt.logical_user_data_window_bytes = raw_trace->logical_user_data_window_bytes;
    receipt.loader_record = manifest->loader_record;
    receipt.consumer_trace_checksum = manifest->consumer_trace_checksum;
    receipt.dungeon_record_consumer_pc = manifest->dungeon_record_consumer_pc;
    receipt.dungeon_record_payload_offset = manifest->dungeon_record_payload_offset;
    receipt.dungeon_record_byte_count = manifest->dungeon_record_byte_count;
    receipt.dungeon_record_window_checksum = manifest->dungeon_record_window_checksum;
    receipt.object_table_consumer_pc = manifest->object_table_consumer_pc;
    receipt.object_table_payload_offset = manifest->object_table_payload_offset;
    receipt.object_table_byte_count = manifest->object_table_byte_count;
    receipt.object_table_window_checksum = manifest->object_table_window_checksum;
    receipt.level_fields_blocked = 1;
    receipt.object_fields_blocked = 1;
    *out = receipt;
    return 1;
}

int theron_v1_track02_capture_trace_manifest_discover_and_bind(
    const char *media_path, const char *manifest_path,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt *out) {
    Theron_V1Track02RawMediaIntakeReceipt intake;
    Theron_V1Track02RawTraceMediaInput raw_trace;
    Theron_V1Track02CaptureTraceManifest manifest;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!theron_v1_track02_raw_media_intake_discover(media_path, &intake) ||
        intake.status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        !theron_v1_track02_raw_media_intake_prepare_trace_input(&intake, &raw_trace) ||
        !theron_v1_track02_capture_trace_manifest_parse(manifest_path, &manifest)) return 0;
    return theron_v1_track02_capture_trace_manifest_bind(
        &intake, &raw_trace, provenance, preparation, &manifest, out);
}
