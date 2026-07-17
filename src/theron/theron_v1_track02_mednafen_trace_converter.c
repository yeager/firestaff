#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "asset_status_m12.h"

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#define THERON_V1_MEDNAFEN_CLOSE _close
#define THERON_V1_MEDNAFEN_FDOPEN _fdopen
#define THERON_V1_MEDNAFEN_FILENO _fileno
#else
#include <fcntl.h>
#include <unistd.h>
#define THERON_V1_MEDNAFEN_CLOSE close
#define THERON_V1_MEDNAFEN_FDOPEN fdopen
#define THERON_V1_MEDNAFEN_FILENO fileno
#endif

#include "asset_status_m12.h"
#include "theron_v1_track02_mednafen_trace_converter.h"

#define THERON_V1_TRACK02_MEDNAFEN_TRACE_MAX_BYTES (64u * 1024u)

typedef struct {
    unsigned int loader_pc;
    unsigned int loader_record;
    unsigned int loader_destination;
    size_t loader_bytes;
    unsigned int loader_checksum;
    unsigned int dungeon_pc;
    size_t dungeon_offset;
    size_t dungeon_bytes;
    unsigned int dungeon_checksum;
    unsigned int object_pc;
    size_t object_offset;
    size_t object_bytes;
    unsigned int object_checksum;
    unsigned int consumer_checksum;
} Theron_V1Track02MednafenObservedRows;

static int theron_v1_mednafen_trace_same_file(const char *left,
                                               const char *right) {
    struct stat left_stat;
    struct stat right_stat;

    if (!strcmp(left, right)) return 1;
    return stat(left, &left_stat) == 0 && stat(right, &right_stat) == 0 &&
        left_stat.st_dev == right_stat.st_dev && left_stat.st_ino == right_stat.st_ino;
}

static int theron_v1_mednafen_trace_is_symlink(const char *path) {
#if defined(_WIN32)
    (void)path;
    return 0;
#else
    struct stat path_stat;
    return lstat(path, &path_stat) == 0 && S_ISLNK(path_stat.st_mode);
#endif
}

static FILE *theron_v1_mednafen_trace_open_new(const char *path) {
    int descriptor = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    FILE *file;

    if (descriptor < 0) return NULL;
    file = THERON_V1_MEDNAFEN_FDOPEN(descriptor, "wb");
    if (!file) {
        THERON_V1_MEDNAFEN_CLOSE(descriptor);
        remove(path);
    }
    return file;
}

static int theron_v1_mednafen_trace_read_line(FILE *file, char *line,
                                              size_t capacity) {
    size_t length;
    if (!fgets(line, capacity, file)) return 0;
    length = strlen(line);
    while (length && (line[length - 1] == '\n' || line[length - 1] == '\r')) line[--length] = '\0';
    return 1;
}

static int theron_v1_mednafen_trace_parse_loader(const char *line,
                                                   Theron_V1Track02MednafenObservedRows *rows) {
    int consumed = 0;
    return sscanf(line,
                  "huc6280_loader_cd_read pc=%x record=%x destination=%x byte_count=%zu payload_checksum=%x%n",
                  &rows->loader_pc, &rows->loader_record, &rows->loader_destination,
                  &rows->loader_bytes, &rows->loader_checksum, &consumed) == 5 &&
        line[consumed] == '\0';
}

static int theron_v1_mednafen_trace_parse_dungeon(const char *line,
                                                    Theron_V1Track02MednafenObservedRows *rows) {
    int consumed = 0;
    return sscanf(line,
                  "huc6280_dungeon_consumer pc=%x payload_offset=%zx byte_count=%zu window_checksum=%x%n",
                  &rows->dungeon_pc, &rows->dungeon_offset, &rows->dungeon_bytes,
                  &rows->dungeon_checksum, &consumed) == 4 && line[consumed] == '\0';
}

static int theron_v1_mednafen_trace_parse_object(const char *line,
                                                   Theron_V1Track02MednafenObservedRows *rows) {
    int consumed = 0;
    return sscanf(line,
                  "huc6280_object_consumer pc=%x payload_offset=%zx byte_count=%zu window_checksum=%x%n",
                  &rows->object_pc, &rows->object_offset, &rows->object_bytes,
                  &rows->object_checksum, &consumed) == 4 && line[consumed] == '\0';
}

static int theron_v1_mednafen_trace_parse_consumer_checksum(
    const char *line, Theron_V1Track02MednafenObservedRows *rows) {
    int consumed = 0;
    return sscanf(line, "consumer_trace_checksum=%x%n", &rows->consumer_checksum,
                  &consumed) == 1 && line[consumed] == '\0';
}

static Theron_V1Track02MednafenTraceStatus theron_v1_mednafen_trace_inspect(
    const char *source_trace_path, const char *expected_source_trace_md5,
    Theron_V1Track02MednafenObservedRows *rows, char md5[33]) {
    FILE *input;
    char line[320];
    char final_source_md5[33];
    long source_bytes;
    struct stat source_stat;

    if (!source_trace_path || !source_trace_path[0] ||
        !(input = fopen(source_trace_path, "rb"))) {
        return THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE;
    }
    if (theron_v1_mednafen_trace_is_symlink(source_trace_path)) {
        fclose(input);
        return THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED;
    }
    if (fstat(THERON_V1_MEDNAFEN_FILENO(input), &source_stat) != 0 ||
        !S_ISREG(source_stat.st_mode) ||
        fseek(input, 0L, SEEK_END) != 0 || (source_bytes = ftell(input)) <= 0 ||
        (unsigned long)source_bytes > THERON_V1_TRACK02_MEDNAFEN_TRACE_MAX_BYTES ||
        fseek(input, 0L, SEEK_SET) != 0 ||
        !m12_file_md5_hex(source_trace_path, md5) ||
        (expected_source_trace_md5 && strcmp(md5, expected_source_trace_md5)) ||
        !theron_v1_mednafen_trace_read_line(input, line, sizeof(line)) ||
        strcmp(line, "source=mednafen-pce-instrumented") ||
        !theron_v1_mednafen_trace_read_line(input, line, sizeof(line)) ||
        !theron_v1_mednafen_trace_parse_loader(line, rows) ||
        !theron_v1_mednafen_trace_read_line(input, line, sizeof(line)) ||
        !theron_v1_mednafen_trace_parse_dungeon(line, rows) ||
        !theron_v1_mednafen_trace_read_line(input, line, sizeof(line)) ||
        !theron_v1_mednafen_trace_parse_object(line, rows) ||
        !theron_v1_mednafen_trace_read_line(input, line, sizeof(line)) ||
        !theron_v1_mednafen_trace_parse_consumer_checksum(line, rows) ||
        fgets(line, sizeof(line), input) ||
        !m12_file_md5_hex(source_trace_path, final_source_md5) ||
        strcmp(md5, final_source_md5)) {
        fclose(input);
        return THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED;
    }
    fclose(input);
    return THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED;
}

int theron_v1_track02_mednafen_trace_inspect_file(
    const char *source_trace_path,
    Theron_V1Track02MednafenTraceConvertReceipt *out) {
    Theron_V1Track02MednafenTraceConvertReceipt receipt = {0};
    Theron_V1Track02MednafenObservedRows rows = {0};
    char md5[33];

    if (!out) return 0;
    *out = receipt;
    receipt.status = theron_v1_mednafen_trace_inspect(source_trace_path, NULL,
                                                        &rows, md5);
    if (receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED) {
        receipt.source_trace_md5_verified = 1;
        receipt.source_rows_observed = 1;
        snprintf(receipt.source_trace_path, sizeof(receipt.source_trace_path), "%s",
                 source_trace_path);
        snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s", md5);
    }
    *out = receipt;
    return 1;
}

int theron_v1_track02_mednafen_trace_convert_file(
    const Theron_V1Track02MednafenTraceConvertRequest *request,
    Theron_V1Track02MednafenTraceConvertReceipt *out) {
    Theron_V1Track02MednafenTraceConvertReceipt receipt = {0};
    Theron_V1Track02MednafenObservedRows rows = {0};
    FILE *output;
    char md5[33];
    char event_log_md5[33];

    if (!out) return 0;
    *out = receipt;
    if (!request || !request->source_trace_path || !request->expected_source_trace_md5 ||
        !request->event_log_path || !request->source_trace_path[0] ||
        !request->expected_source_trace_md5[0] || !request->event_log_path[0] ||
        !request->expected_source_trace_md5[0]) {
        receipt.status = THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (theron_v1_mednafen_trace_same_file(request->source_trace_path,
                                            request->event_log_path)) {
        receipt.status = THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED;
        *out = receipt;
        return 1;
    }
    {
        struct stat output_stat;
        if (stat(request->event_log_path, &output_stat) == 0) {
            receipt.status = THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED;
            *out = receipt;
            return 1;
        }
    }
    receipt.status = theron_v1_mednafen_trace_inspect(request->source_trace_path,
                                                        request->expected_source_trace_md5,
                                                        &rows, md5);
    if (receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE) {
        *out = receipt;
        return 1;
    }
    if (receipt.status != THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED) goto rejected_closed;
    if (!(output = theron_v1_mednafen_trace_open_new(request->event_log_path))) {
        goto rejected_closed;
    }
    if (fprintf(output,
            "THERON_HUC6280_CAPTURE_EVENT_LOG_V1\n"
            "consumer_trace_checksum=0x%x\n"
            "event=loader_cd_read\npc=0x%x\nrecord=0x%x\ndestination=0x%x\n"
            "byte_count=%zu\npayload_checksum=0x%x\n"
            "event=dungeon_consumer\npc=0x%x\npayload_offset=0x%zx\n"
            "byte_count=%zu\nwindow_checksum=0x%x\n"
            "event=object_consumer\npc=0x%x\npayload_offset=0x%zx\n"
            "byte_count=%zu\nwindow_checksum=0x%x\n",
            rows.consumer_checksum, rows.loader_pc, rows.loader_record,
            rows.loader_destination, rows.loader_bytes, rows.loader_checksum,
            rows.dungeon_pc, rows.dungeon_offset, rows.dungeon_bytes,
            rows.dungeon_checksum, rows.object_pc, rows.object_offset,
            rows.object_bytes, rows.object_checksum) < 0) {
        fclose(output);
        remove(request->event_log_path);
        goto rejected_closed;
    }
    if (fclose(output) != 0) {
        remove(request->event_log_path);
        goto rejected_closed;
    }
    if (!m12_file_md5_hex(request->event_log_path, event_log_md5)) {
        remove(request->event_log_path);
        goto rejected_closed;
    }
    receipt.status = THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED;
    receipt.source_trace_md5_verified = 1;
    receipt.source_rows_observed = 1;
    receipt.huc6280_event_log_written = 1;
    receipt.huc6280_event_log_md5_verified = 1;
    snprintf(receipt.source_trace_path, sizeof(receipt.source_trace_path), "%s", request->source_trace_path);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s", md5);
    snprintf(receipt.event_log_path, sizeof(receipt.event_log_path), "%s", request->event_log_path);
    snprintf(receipt.event_log_md5, sizeof(receipt.event_log_md5), "%s", event_log_md5);
    *out = receipt;
    return 1;
rejected_closed:
    receipt.status = THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED;
    *out = receipt;
    return 1;
}

int theron_v1_track02_mednafen_trace_stamp_handoff_envelope(
    const Theron_V1Track02MednafenTraceConvertReceipt *trace,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const char *envelope_path, char out_envelope_md5[33])
{
    FILE *file; size_t i;
    if (out_envelope_md5) out_envelope_md5[0] = '\0';
    if (!trace || !plan || !envelope_path || !envelope_path[0] ||
        trace->status != THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED ||
        !trace->source_trace_md5_verified || !trace->huc6280_event_log_md5_verified ||
        !trace->source_rows_observed || !plan->valid || !plan->cue_track_consumed ||
        !plan->cd_read_chain_consumed || !plan->loader_output_consumed ||
        !plan->palette_output_consumed || !plan->bitmap_transfer_consumed ||
        !plan->destination_record_consumed || plan->level_object_semantics_allowed ||
        plan->pixel_decode_allowed || plan->render_allowed || plan->fallback_visuals_allowed ||
        (file = fopen(envelope_path, "rb")) != NULL) { if (file) fclose(file); return 0; }
    file = fopen(envelope_path, "wb"); if (!file) return 0;
    if (fprintf(file, "THERON_TRACK02_CAPTURE_ARTIFACT_BUNDLE_V1\ntrack02_md5=%s\nmednafen_trace_md5=%s\ncampaign_route=2\n", plan->targets[0].track02_md5, trace->source_trace_md5) < 0) goto failed;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        const Theron_V1Track02CaptureTarget *t = &plan->targets[i];
        if (t->route != (Theron_V1Track02CaptureTargetRoute)i || !t->cd_read_record ||
            !t->loader_output_checksum || !t->palette_output_identity || !t->bitmap_identity ||
            !t->destination_record || !t->destination_identity ||
            fprintf(file, "route=%zu cd_record=%x loader_offset=%zx loader_bytes=%zu loader_checksum=%x palette_identity=%x bitmap_offset=%zx bitmap_bytes=%zu bitmap_identity=%x destination_record=%x destination_offset=%zx destination_bytes=%zu destination_identity=%x\n", i,t->cd_read_record,t->loader_output_raw_offset,t->loader_output_bytes,t->loader_output_checksum,t->palette_output_identity,t->bitmap_raw_offset,t->bitmap_bytes,t->bitmap_identity,t->destination_record,t->destination_offset,t->destination_bytes,t->destination_identity) < 0) goto failed;
    }
    if (fclose(file) != 0 || !m12_file_md5_hex(envelope_path, out_envelope_md5)) { remove(envelope_path); return 0; }
    return 1;
failed:
    fclose(file); remove(envelope_path); return 0;
}
