#include "asset_status_m12.h"
#include "theron_v1_stage3_manifest_evidence.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t record;
    uint16_t caller_pc;
    uint16_t return_pc;
    size_t selector_ordinal;
    uint16_t selector;
} LaterCdReadLayoutReceipt;

static int g_fail;
static int g_skip;

static void check(int condition, const char *name) {
    if (!condition) {
        ++g_fail;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static uint8_t *read_file_bytes(const char *path, size_t *out_size) {
    FILE *file = NULL;
    long size;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size + 1u)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    bytes[size] = '\0';
    *out_size = (size_t)size;
    return bytes;
}

static int find_exact_line(const char *text, const char *prefix,
                           const char **out_line, size_t *out_length) {
    const char *line = text;
    size_t prefix_length;

    if (!text || !prefix || !out_line || !out_length) return 0;
    prefix_length = strlen(prefix);
    while (*line) {
        const char *end = strchr(line, '\n');
        size_t length = end ? (size_t)(end - line) : strlen(line);

        if (length >= prefix_length &&
            memcmp(line, prefix, prefix_length) == 0) {
            *out_line = line;
            *out_length = length;
            return 1;
        }
        line = end ? end + 1 : line + length;
    }
    return 0;
}

static int parse_later_trace(const char *path, uint32_t *out_record,
                             uint16_t *out_caller_pc,
                             uint16_t *out_return_pc) {
    uint8_t *bytes;
    size_t size;
    const char *text;
    const char *dispatch;
    const char *returned;
    size_t dispatch_length;
    size_t returned_length;
    unsigned int caller_pc;
    unsigned int return_pc;
    unsigned int sector_count;
    unsigned int cl;
    unsigned int dl;
    unsigned int ch;
    unsigned int record;
    unsigned int return_caller_pc;
    unsigned int return_return_pc;
    unsigned int return_record;
    int consumed = 0;
    int ok;

    if (!path || !out_record || !out_caller_pc ||
        !out_return_pc || !(bytes = read_file_bytes(path, &size))) {
        return 0;
    }
    text = (const char *)bytes;
    ok =
        find_exact_line(text, "source=mednafen-pce-instrumented", &dispatch,
                        &dispatch_length) &&
        dispatch_length == strlen("source=mednafen-pce-instrumented") &&
        find_exact_line(text, "later_system_card_e009_dispatch ", &dispatch,
                        &dispatch_length) &&
        find_exact_line(text, "later_system_card_e009_return ", &returned,
                        &returned_length) &&
        sscanf(dispatch,
               "later_system_card_e009_dispatch caller_pc=%x return_pc=%x sector_count=%x record_cl=%x record_dl=%x record_ch=%x record=%x%n",
               &caller_pc, &return_pc, &sector_count, &cl, &dl,
               &ch, &record, &consumed) == 8 && consumed == (int)dispatch_length &&
        sscanf(returned,
               "later_system_card_e009_return caller_pc=%x return_pc=%x record=%x%n",
               &return_caller_pc, &return_return_pc, &return_record,
               &consumed) == 3 && consumed == (int)returned_length &&
        return_caller_pc == caller_pc && return_return_pc == return_pc &&
        return_record == record;
    free(bytes);
    if (!ok || caller_pc > 0xffffu || return_pc > 0xffffu ||
        sector_count == 0u || cl > 0xffu || dl > 0xffu ||
        ch > 0xffu || record > 0xffffffu ||
        record != (cl | (dl << 8) | (ch << 16)) ||
        return_pc != caller_pc + 3u) {
        return 0;
    }
    *out_record = record;
    *out_caller_pc = (uint16_t)caller_pc;
    *out_return_pc = (uint16_t)return_pc;
    return 1;
}

static int inspect(const char *track_path, const char *trace_path,
                   const char *expected_md5, LaterCdReadLayoutReceipt *out) {
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1Stage3ManifestEvidence manifest;
    uint8_t *bytes;
    size_t size;
    char actual_md5[33];
    uint32_t record;
    uint16_t caller_pc;
    uint16_t return_pc;
    size_t ordinal;

    if (!out || !(bytes = read_file_bytes(track_path, &size))) return 0;
    memset(out, 0, sizeof(*out));
    if (!m12_file_md5_hex(track_path, actual_md5) ||
        strcmp(actual_md5, expected_md5) != 0 ||
        theron_v1_track02_inspect_stage2_dynamic_payload(
            bytes, size, expected_md5, &payload) != THERON_TRACK02_SIGNAL_OK ||
        !theron_v1_stage3_manifest_evidence_from_payload(
            bytes, size, &payload, &manifest) ||
        !parse_later_trace(trace_path, &record, &caller_pc, &return_pc) ||
        record <= manifest.track02_record || record >= size / 2352u ||
        record < manifest.track02_record - manifest.first_descriptor.word2) {
        free(bytes);
        return 0;
    }
    out->selector = (uint16_t)(record -
        (manifest.track02_record - manifest.first_descriptor.word2));
    for (ordinal = 0u; ordinal < manifest.descriptor_count; ++ordinal) {
        if (manifest.descriptors[ordinal].word2 == out->selector) break;
    }
    free(bytes);
    if (ordinal == manifest.descriptor_count) return 0;
    out->valid = 1;
    out->variant = manifest.variant;
    out->record = record;
    out->caller_pc = caller_pc;
    out->return_pc = return_pc;
    out->selector_ordinal = ordinal;
    return 1;
}

int main(void) {
    const char *jp_track = getenv("FIRESTAFF_THERON_TRACK02_JP_BIN");
    const char *us_track = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    const char *jp_trace = getenv("FIRESTAFF_THERON_LATER_CD_READ_TRACE_JP");
    const char *us_trace = getenv("FIRESTAFF_THERON_LATER_CD_READ_TRACE_US");
    LaterCdReadLayoutReceipt jp;
    LaterCdReadLayoutReceipt us;

    if (!jp_track || !us_track || !jp_trace || !us_trace) {
        ++g_skip;
        printf("[SKIP] set authenticated JP/US Track02 and later Mednafen trace paths\n");
        return 0;
    }
    check(inspect(jp_track, jp_trace, THERON_TRACK02_MD5_JP_BIN, &jp),
          "JP later e009 dispatch resolves to a bounded manifest selector");
    check(inspect(us_track, us_trace, THERON_TRACK02_MD5_US_BIN, &us),
          "US later e009 dispatch resolves to a bounded manifest selector");
    check(jp.valid && us.valid && jp.selector == us.selector &&
              jp.selector_ordinal == us.selector_ordinal &&
              jp.caller_pc == us.caller_pc && jp.return_pc == us.return_pc &&
              us.record - jp.record == 1u,
          "JP/US later dispatches retain one bounded selector and transition anchor");
    printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
