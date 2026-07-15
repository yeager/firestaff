/*
 * Collect one already-recorded Mednafen debugger trace beside its exact
 * Firestaff owner/material capture target. This tool never launches an
 * emulator, fabricates a trace, or attests Saturn provenance.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TARGET_MAGIC \
    "FIRESTAFF_NEXUS_STRUCTURE1A_STRUCTURE3_MATERIAL_CAPTURE_TARGET_V1"
#define TRACE_MAGIC "FIRESTAFF_NEXUS_STRUCTURE2_SATURN_RAW_TRACE_V1"

static const char *const required_fields[] = {
    "level", "source_fnv1a64", "descriptor_index", "descriptor_fnv1a64",
    "opaque_payload_fnv1a64", "image_anchor_offset",
    "image_next_anchor_offset", "image_candidate_byte_count",
    "image_candidate_fnv1a64", "palette_candidate_present",
    "palette_anchor_offset", "palette_next_anchor_offset",
    "palette_candidate_byte_count", "palette_candidate_fnv1a64",
    "capture_target_fnv1a64", "owner_x", "owner_y",
    "structure1f_entry_index", "structure1a_index",
    "structure3_entry_index", "face_ordinal", "face_row_fnv1a32"
};

static uint64_t fnv1a64(const uint8_t *data, size_t size) {
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) {
        hash ^= data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *file;
    long length;
    uint8_t *data;

    if (!path || !out_data || !out_size) return 0;
    *out_data = NULL;
    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)length + 1U);
    if (!data || fread(data, 1, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    data[length] = 0;
    *out_data = data;
    *out_size = (size_t)length;
    return 1;
}

static int find_value(const uint8_t *text, size_t size, const char *key,
                      const uint8_t **out_value, size_t *out_value_size) {
    size_t key_size;
    size_t offset;

    if (!text || !key || !out_value || !out_value_size) return 0;
    key_size = strlen(key);
    for (offset = 0; offset + key_size + 1U <= size;) {
        size_t line_end = offset;
        if ((offset == 0 || text[offset - 1U] == '\n') &&
            memcmp(text + offset, key, key_size) == 0 &&
            text[offset + key_size] == '=') {
            size_t value_offset = offset + key_size + 1U;
            while (line_end < size && text[line_end] != '\n') ++line_end;
            *out_value = text + value_offset;
            *out_value_size = line_end - value_offset;
            return *out_value_size > 0;
        }
        while (line_end < size && text[line_end] != '\n') ++line_end;
        offset = line_end + 1U;
    }
    return 0;
}

static int write_field(FILE *file, const uint8_t *target, size_t target_size,
                       const char *key) {
    const uint8_t *value;
    size_t value_size;
    return find_value(target, target_size, key, &value, &value_size) &&
        fprintf(file, "%s=", key) > 0 &&
        fwrite(value, 1, value_size, file) == value_size &&
        fputc('\n', file) != EOF;
}

int main(int argc, char **argv) {
    const char *target_path;
    const char *raw_trace_path;
    const char *manifest_path;
    uint8_t *target = NULL;
    uint8_t *raw_trace = NULL;
    size_t target_size = 0;
    size_t raw_trace_size = 0;
    char temporary_path[1024] = {0};
    FILE *output = NULL;
    size_t index;
    int ok = 0;

    if (argc != 4) {
        fprintf(stderr, "usage: %s TARGET RAW_MEDNAFEN_TRACE OUTPUT_MANIFEST\n",
                argv[0]);
        return 2;
    }
    target_path = argv[1];
    raw_trace_path = argv[2];
    manifest_path = argv[3];
    if (!read_file(target_path, &target, &target_size) ||
        target_size <= strlen(TARGET_MAGIC) ||
        memcmp(target, TARGET_MAGIC, strlen(TARGET_MAGIC)) != 0 ||
        target[strlen(TARGET_MAGIC)] != '\n' ||
        !read_file(raw_trace_path, &raw_trace, &raw_trace_size) ||
        raw_trace_size == 0 ||
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", manifest_path) >=
            (int)sizeof(temporary_path)) {
        fprintf(stderr, "collector requires one valid capture target and nonempty raw trace\n");
        goto cleanup;
    }
    output = fopen(temporary_path, "wb");
    if (!output || fprintf(output, "magic=%s\nproducer=mednafen-debugger\n",
                           TRACE_MAGIC) <= 0) {
        goto cleanup;
    }
    for (index = 0; index < sizeof(required_fields) / sizeof(required_fields[0]);
         ++index) {
        if (!write_field(output, target, target_size, required_fields[index])) {
            fprintf(stderr, "capture target is missing %s\n", required_fields[index]);
            goto cleanup;
        }
    }
    if (fprintf(output, "raw_trace_size=%zu\nraw_trace_fnv1a64=%016llx\n"
                        "collector_does_not_attest_original_saturn=1\n",
                raw_trace_size, (unsigned long long)fnv1a64(raw_trace,
                                                             raw_trace_size)) <= 0 ||
        fclose(output) != 0 || rename(temporary_path, manifest_path) != 0) {
        output = NULL;
        goto cleanup;
    }
    output = NULL;
    printf("wrote unauthenticated Mednafen trace manifest: %s\n", manifest_path);
    ok = 1;

cleanup:
    if (output) fclose(output);
    if (!ok) remove(temporary_path);
    free(target);
    free(raw_trace);
    return ok ? 0 : 1;
}
