#include "theron_v1_capture_manifest.h"

#include <stdio.h>
#include <string.h>

static int theron_v1_capture_manifest_md5_is_valid(const char *md5) {
    size_t index;

    if (!md5 || strlen(md5) != 32u) return 0;
    for (index = 0u; index < 32u; ++index) {
        const char value = md5[index];
        if (!((value >= '0' && value <= '9') ||
              (value >= 'a' && value <= 'f'))) {
            return 0;
        }
    }
    return 1;
}

static int theron_v1_capture_manifest_path_is_valid(const char *path) {
    const unsigned char *cursor = (const unsigned char *)path;

    if (!path || !path[0]) return 0;
    while (*cursor) {
        if (*cursor < 0x20u || *cursor == 0x7fu) return 0;
        ++cursor;
    }
    return 1;
}

static int theron_v1_capture_manifest_copy_field(
    const char **cursor, const char *prefix, char *out, size_t out_capacity) {
    const char *value;
    const char *newline;
    size_t length;

    if (!cursor || !*cursor || !prefix || !out || out_capacity == 0u) return 0;
    if (strncmp(*cursor, prefix, strlen(prefix)) != 0) return 0;
    value = *cursor + strlen(prefix);
    newline = strchr(value, '\n');
    length = newline ? (size_t)(newline - value) : strlen(value);
    if (length == 0u || length >= out_capacity) return 0;
    memcpy(out, value, length);
    out[length] = '\0';
    *cursor = newline ? newline + 1 : value + length;
    return 1;
}

int theron_v1_capture_manifest_parse(const char *text,
                                     Theron_V1CaptureManifest *out) {
    const char *cursor;
    Theron_V1CaptureManifest parsed;

    if (out) memset(out, 0, sizeof(*out));
    if (!text || !out) return 0;
    cursor = text;
    if (strncmp(cursor, "THERON_CAPTURE_MANIFEST_V1\n", 27u) != 0) return 0;
    cursor += 27u;
    if (!theron_v1_capture_manifest_copy_field(
            &cursor, "track02_path=", parsed.track02_path,
            sizeof(parsed.track02_path)) ||
        !theron_v1_capture_manifest_copy_field(
            &cursor, "track02_md5=", parsed.track02_md5,
            sizeof(parsed.track02_md5)) ||
        !theron_v1_capture_manifest_copy_field(
            &cursor, "system_card_path=", parsed.system_card_path,
            sizeof(parsed.system_card_path)) ||
        !theron_v1_capture_manifest_copy_field(
            &cursor, "system_card_md5=", parsed.system_card_md5,
            sizeof(parsed.system_card_md5)) ||
        !theron_v1_capture_manifest_copy_field(
            &cursor, "loader_trace_path=", parsed.trace_path,
            sizeof(parsed.trace_path)) ||
        (*cursor != '\0' && !(cursor[0] == '\n' && cursor[1] == '\0')) ||
        !theron_v1_capture_manifest_path_is_valid(parsed.track02_path) ||
        !theron_v1_capture_manifest_path_is_valid(parsed.system_card_path) ||
        !theron_v1_capture_manifest_path_is_valid(parsed.trace_path) ||
        !theron_v1_capture_manifest_md5_is_valid(parsed.track02_md5) ||
        !theron_v1_capture_manifest_md5_is_valid(parsed.system_card_md5)) {
        return 0;
    }
    parsed.valid = 1;
    *out = parsed;
    return 1;
}

int theron_v1_capture_manifest_matches(
    const Theron_V1CaptureManifest *manifest, const char *track02_path,
    const char *track02_md5, const char *system_card_path,
    const char *system_card_md5, const char *trace_path) {
    return manifest && manifest->valid && track02_path && track02_md5 &&
           system_card_path && system_card_md5 && trace_path &&
           strcmp(manifest->track02_path, track02_path) == 0 &&
           strcmp(manifest->track02_md5, track02_md5) == 0 &&
           strcmp(manifest->system_card_path, system_card_path) == 0 &&
           strcmp(manifest->system_card_md5, system_card_md5) == 0 &&
           strcmp(manifest->trace_path, trace_path) == 0;
}

int theron_v1_capture_manifest_matches_preflight_inputs(
    const Theron_V1CaptureManifest *manifest, const char *track02_path,
    const char *track02_md5, const char *system_card_path,
    const char *system_card_md5, const char *trace_path) {
    return theron_v1_capture_manifest_matches(
        manifest, track02_path, track02_md5, system_card_path,
        system_card_md5, trace_path);
}

int theron_v1_capture_manifest_write(const Theron_V1CaptureManifest *manifest,
                                     char *out, size_t capacity) {
    int written;

    if (!manifest || !manifest->valid || !out || capacity == 0u ||
        !theron_v1_capture_manifest_path_is_valid(manifest->track02_path) ||
        !theron_v1_capture_manifest_path_is_valid(manifest->system_card_path) ||
        !theron_v1_capture_manifest_path_is_valid(manifest->trace_path) ||
        !theron_v1_capture_manifest_md5_is_valid(manifest->track02_md5) ||
        !theron_v1_capture_manifest_md5_is_valid(manifest->system_card_md5)) {
        return 0;
    }
    written = snprintf(
        out, capacity,
        "THERON_CAPTURE_MANIFEST_V1\n"
        "track02_path=%s\ntrack02_md5=%s\n"
        "system_card_path=%s\nsystem_card_md5=%s\n"
        "loader_trace_path=%s",
        manifest->track02_path, manifest->track02_md5,
        manifest->system_card_path, manifest->system_card_md5,
        manifest->trace_path);
    return written >= 0 && (size_t)written < capacity;
}
