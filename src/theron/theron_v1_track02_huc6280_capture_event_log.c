#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

#include "theron_v1_track02_huc6280_capture_event_log.h"

#define THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_MAX_BYTES (64u * 1024u)

#if defined(_WIN32)
#include <io.h>
#define THERON_V1_HUC6280_LOG_FILENO _fileno
#else
#define THERON_V1_HUC6280_LOG_FILENO fileno
#endif

static int theron_v1_huc6280_log_is_regular_file(FILE *file) {
    struct stat file_status;

    return file && fstat(THERON_V1_HUC6280_LOG_FILENO(file), &file_status) == 0 &&
        S_ISREG(file_status.st_mode);
}

static int theron_v1_huc6280_log_is_symlink(const char *path) {
#if defined(_WIN32)
    (void)path;
    return 0;
#else
    struct stat path_status;
    return lstat(path, &path_status) == 0 && S_ISLNK(path_status.st_mode);
#endif
}

static int theron_v1_huc6280_log_number(const char *text,
                                         unsigned long long *out) {
    char *end;
    if (!text || !text[0] || !out || text[0] == '-') return 0;
    *out = strtoull(text, &end, 0);
    return *end == '\0';
}

static int theron_v1_huc6280_log_read_line(FILE *file, const char *expected_key,
                                            unsigned long long *out) {
    char line[160];
    char *equals;
    char *end;
    if (!fgets(line, sizeof(line), file)) return 0;
    end = line + strlen(line);
    while (end > line && isspace((unsigned char)end[-1])) *--end = '\0';
    equals = strchr(line, '=');
    if (!equals) return 0;
    *equals = '\0';
    return !strcmp(line, expected_key) && theron_v1_huc6280_log_number(equals + 1, out);
}

static int theron_v1_huc6280_log_read_event(FILE *file, const char *expected) {
    char line[160];
    char *end;
    if (!fgets(line, sizeof(line), file)) return 0;
    end = line + strlen(line);
    while (end > line && isspace((unsigned char)end[-1])) *--end = '\0';
    return !strcmp(line, expected);
}

int theron_v1_track02_huc6280_capture_event_log_parse(
    const char *path, Theron_V1Track02Huc6280CaptureEventLog *out) {
    Theron_V1Track02Huc6280CaptureEventLog log = {0};
    FILE *file;
    long bytes;
    unsigned long long value;
    char header[160];
    char *end;

    if (!out) return 0;
    *out = log;
    if (!path || !path[0] || !(file = fopen(path, "rb"))) {
        log.status = THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_UNAVAILABLE;
        *out = log;
        return 1;
    }
    if (theron_v1_huc6280_log_is_symlink(path)) {
        fclose(file);
        log.status = THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_REJECTED;
        *out = log;
        return 1;
    }
    if (!theron_v1_huc6280_log_is_regular_file(file) ||
        fseek(file, 0L, SEEK_END) != 0 || (bytes = ftell(file)) <= 0 ||
        (unsigned long)bytes > THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_MAX_BYTES ||
        fseek(file, 0L, SEEK_SET) != 0 || !fgets(header, sizeof(header), file)) goto rejected;
    end = header + strlen(header);
    while (end > header && isspace((unsigned char)end[-1])) *--end = '\0';
    if (strcmp(header, "THERON_HUC6280_CAPTURE_EVENT_LOG_V1") ||
        !theron_v1_huc6280_log_read_line(file, "consumer_trace_checksum", &value) ||
        value > UINT32_MAX) goto rejected;
    log.consumer_trace_checksum = (uint32_t)value;
    if (!theron_v1_huc6280_log_read_event(file, "event=loader_cd_read") ||
        !theron_v1_huc6280_log_read_line(file, "pc", &value) || value > UINT32_MAX) goto rejected;
    log.loader_pc = (uint32_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "record", &value) || value > UINT32_MAX) goto rejected;
    log.loader_record = (uint32_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "destination", &value) || value > UINT32_MAX) goto rejected;
    log.loader_destination = (uint32_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "byte_count", &value) || value > SIZE_MAX) goto rejected;
    log.loader_payload_bytes = (size_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "payload_checksum", &value) || value > UINT32_MAX) goto rejected;
    log.loader_payload_checksum = (uint32_t)value;
    if (!theron_v1_huc6280_log_read_event(file, "event=dungeon_consumer") ||
        !theron_v1_huc6280_log_read_line(file, "pc", &value) || value > UINT32_MAX) goto rejected;
    log.dungeon_record_consumer_pc = (uint32_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "payload_offset", &value) || value > SIZE_MAX) goto rejected;
    log.dungeon_record_payload_offset = (size_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "byte_count", &value) || value > SIZE_MAX) goto rejected;
    log.dungeon_record_byte_count = (size_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "window_checksum", &value) || value > UINT32_MAX) goto rejected;
    log.dungeon_record_window_checksum = (uint32_t)value;
    if (!theron_v1_huc6280_log_read_event(file, "event=object_consumer") ||
        !theron_v1_huc6280_log_read_line(file, "pc", &value) || value > UINT32_MAX) goto rejected;
    log.object_table_consumer_pc = (uint32_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "payload_offset", &value) || value > SIZE_MAX) goto rejected;
    log.object_table_payload_offset = (size_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "byte_count", &value) || value > SIZE_MAX) goto rejected;
    log.object_table_byte_count = (size_t)value;
    if (!theron_v1_huc6280_log_read_line(file, "window_checksum", &value) || value > UINT32_MAX) goto rejected;
    log.object_table_window_checksum = (uint32_t)value;
    if (fgets(header, sizeof(header), file)) goto rejected;
    fclose(file);
    log.opaque_cd_read_window_retained = 1;
    log.opaque_dungeon_window_retained = 1;
    log.opaque_object_window_retained = 1;
    log.status = THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_READY;
    *out = log;
    return 1;
rejected:
    fclose(file);
    log.status = THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_REJECTED;
    *out = log;
    return 1;
}

int theron_v1_track02_huc6280_capture_event_log_bind_manifest(
    const Theron_V1Track02Huc6280CaptureEventLog *log,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceManifest *out_manifest) {
    Theron_V1Track02CaptureTraceManifest manifest = {0};

    if (!out_manifest) return 0;
    *out_manifest = manifest;
    if (!log || !provenance || !preparation ||
        log->status != THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_READY ||
        !log->opaque_cd_read_window_retained || !log->opaque_dungeon_window_retained ||
        !log->opaque_object_window_retained || !provenance->valid || !preparation->valid ||
        log->loader_pc != THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS ||
        log->loader_record != provenance->loader_record ||
        log->loader_record != preparation->loader_record ||
        log->loader_destination != provenance->loader_destination ||
        log->loader_payload_bytes != provenance->loader_payload_bytes ||
        log->loader_payload_checksum != provenance->loader_payload_checksum ||
        log->consumer_trace_checksum != preparation->consumer_trace_checksum ||
        log->dungeon_record_consumer_pc != preparation->dungeon_record_consumer_pc ||
        log->dungeon_record_payload_offset != preparation->dungeon_record_payload_offset ||
        log->dungeon_record_byte_count != preparation->dungeon_record_byte_count ||
        log->dungeon_record_window_checksum != preparation->dungeon_record_window_checksum ||
        log->object_table_consumer_pc != preparation->object_table_consumer_pc ||
        log->object_table_payload_offset != preparation->object_table_payload_offset ||
        log->object_table_byte_count != preparation->object_table_byte_count ||
        log->object_table_window_checksum != preparation->object_table_window_checksum) return 0;

    manifest.status = THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_READY;
    manifest.track02_variant = provenance->track02_variant;
    snprintf(manifest.track02_md5, sizeof(manifest.track02_md5), "%s", provenance->track02_md5);
    manifest.loader_record = log->loader_record;
    manifest.loader_destination = log->loader_destination;
    manifest.loader_payload_bytes = log->loader_payload_bytes;
    manifest.loader_payload_checksum = log->loader_payload_checksum;
    manifest.consumer_trace_checksum = log->consumer_trace_checksum;
    manifest.dungeon_record_consumer_pc = log->dungeon_record_consumer_pc;
    manifest.dungeon_record_payload_offset = log->dungeon_record_payload_offset;
    manifest.dungeon_record_byte_count = log->dungeon_record_byte_count;
    manifest.dungeon_record_window_checksum = log->dungeon_record_window_checksum;
    manifest.object_table_consumer_pc = log->object_table_consumer_pc;
    manifest.object_table_payload_offset = log->object_table_payload_offset;
    manifest.object_table_byte_count = log->object_table_byte_count;
    manifest.object_table_window_checksum = log->object_table_window_checksum;
    *out_manifest = manifest;
    return 1;
}
