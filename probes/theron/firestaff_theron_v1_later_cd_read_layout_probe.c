#include "asset_status_m12.h"
#include "theron_v1_raw_loader_trace.h"
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

static int find_unique_exact_line(const char *text, const char *prefix,
                                  const char **out_line,
                                  size_t *out_length) {
    const char *line = text;
    size_t prefix_length;
    size_t match_count = 0u;

    if (!text || !prefix || !out_line || !out_length) return 0;
    prefix_length = strlen(prefix);
    while (*line) {
        const char *end = strchr(line, '\n');
        size_t length = end ? (size_t)(end - line) : strlen(line);

        if (length >= prefix_length &&
            memcmp(line, prefix, prefix_length) == 0) {
            *out_line = line;
            *out_length = length;
            ++match_count;
        }
        line = end ? end + 1 : line + length;
    }
    return match_count == 1u;
}

static int parse_later_trace_text(const char *text, uint32_t *out_record,
                                  uint16_t *out_caller_pc,
                                  uint16_t *out_return_pc,
                                  Theron_Track02Variant expected_variant) {
    const char *dispatch;
    const char *returned;
    const char *dynamic;
    size_t source_length;
    size_t dynamic_length;
    size_t dispatch_length;
    size_t returned_length;
    unsigned int caller_pc;
    unsigned int return_pc;
    unsigned int caller_opcode;
    unsigned int caller_target;
    unsigned int sector_count;
    unsigned int cl;
    unsigned int dl;
    unsigned int ch;
    unsigned int record;
    unsigned int dynamic_pc;
    unsigned int dynamic_return_pc;
    unsigned int dynamic_sector_count;
    unsigned int dynamic_destination;
    unsigned int dynamic_record_mask;
    unsigned int dynamic_cl;
    unsigned int dynamic_dl;
    unsigned int dynamic_ch;
    unsigned int dynamic_record;
    unsigned int return_caller_pc;
    unsigned int return_return_pc;
    unsigned int return_record;
    char dynamic_variant[16];
    const char *expected_variant_name;
    uint32_t expected_dynamic_record;
    int consumed = 0;
    int ok;

    if (!text || !out_record || !out_caller_pc || !out_return_pc) {
        return 0;
    }
    if (expected_variant == THERON_TRACK02_VARIANT_JP_BIN) {
        expected_variant_name = "jp_bin";
        expected_dynamic_record = THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP;
    } else if (expected_variant == THERON_TRACK02_VARIANT_US_BIN) {
        expected_variant_name = "us_bin";
        expected_dynamic_record = THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US;
    } else {
        return 0;
    }
    ok =
        find_unique_exact_line(text, "source=mednafen-pce-instrumented",
                               &dynamic, &source_length) &&
        source_length == strlen("source=mednafen-pce-instrumented") &&
        find_unique_exact_line(text, "dynamic_cd_read_transaction ", &dynamic,
                               &dynamic_length) &&
        find_unique_exact_line(text, "later_system_card_e009_dispatch ",
                               &dispatch, &dispatch_length) &&
        find_unique_exact_line(text, "later_system_card_e009_return ",
                               &returned, &returned_length) &&
        sscanf(dynamic,
               "dynamic_cd_read_transaction pc=%x return_pc=%x sector_count=%x destination=%x record_register_mask=%x record_cl=%x record_dl=%x record_ch=%x variant=%15[a-z_] record=%x%n",
               &dynamic_pc, &dynamic_return_pc, &dynamic_sector_count,
               &dynamic_destination, &dynamic_record_mask, &dynamic_cl,
               &dynamic_dl, &dynamic_ch, dynamic_variant, &dynamic_record,
               &consumed) == 10 && consumed == (int)dynamic_length &&
        sscanf(dispatch,
               "later_system_card_e009_dispatch caller_pc=%x return_pc=%x caller_opcode=%x caller_target=%x sector_count=%x record_cl=%x record_dl=%x record_ch=%x record=%x%n",
               &caller_pc, &return_pc, &caller_opcode, &caller_target,
               &sector_count, &cl, &dl, &ch, &record, &consumed) == 9 &&
        consumed == (int)dispatch_length &&
        sscanf(returned,
               "later_system_card_e009_return caller_pc=%x return_pc=%x record=%x%n",
               &return_caller_pc, &return_return_pc, &return_record,
               &consumed) == 3 && consumed == (int)returned_length &&
        return_caller_pc == caller_pc && return_return_pc == return_pc &&
        return_record == record;
    if (!ok || dynamic_pc != 0x4090u || dynamic_return_pc != 0x4093u ||
        dynamic_sector_count != 1u || dynamic_destination != 0x3800u ||
        dynamic_record_mask != 0x07u || dynamic_cl > 0xffu ||
        dynamic_dl > 0xffu || dynamic_ch > 0xffu ||
        dynamic_record > 0xffffffu ||
        dynamic_record != (dynamic_cl | (dynamic_dl << 8) |
                           (dynamic_ch << 16)) ||
        dynamic_record != expected_dynamic_record ||
        strcmp(dynamic_variant, expected_variant_name) != 0 ||
        caller_pc > 0xffffu || return_pc > 0xffffu ||
        caller_opcode != 0x20u || caller_target != 0xe009u ||
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

static int parse_later_trace(const char *path, uint32_t *out_record,
                             uint16_t *out_caller_pc,
                             uint16_t *out_return_pc,
                             Theron_Track02Variant expected_variant) {
    uint8_t *bytes;
    size_t size;
    int ok;

    if (!path || !(bytes = read_file_bytes(path, &size))) return 0;
    (void)size;
    ok = parse_later_trace_text((const char *)bytes, out_record,
                                out_caller_pc, out_return_pc, expected_variant);
    free(bytes);
    return ok;
}

static void test_later_trace_envelope_parser(void) {
    static const char valid_trace[] =
        "source=mednafen-pce-instrumented\n"
        "dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=1 destination=3800 record_register_mask=7 record_cl=df record_dl=4 record_ch=0 variant=jp_bin record=4df\n"
        "later_system_card_e009_dispatch caller_pc=ea00 return_pc=ea03 caller_opcode=20 caller_target=e009 sector_count=1 record_cl=10 record_dl=5 record_ch=0 record=510\n"
        "later_system_card_e009_return caller_pc=ea00 return_pc=ea03 record=510\n";
    char duplicate_trace[sizeof(valid_trace) * 2u];
    char wrong_return_trace[sizeof(valid_trace)];
    uint32_t record = 0u;
    uint16_t caller_pc = 0u;
    uint16_t return_pc = 0u;

    check(parse_later_trace_text(valid_trace, &record, &caller_pc, &return_pc,
                                 THERON_TRACK02_VARIANT_JP_BIN) &&
              record == 0x510u && caller_pc == 0xea00u && return_pc == 0xea03u,
          "later e009 trace parser accepts one complete JP envelope");
    snprintf(duplicate_trace, sizeof(duplicate_trace), "%s%s", valid_trace,
             "later_system_card_e009_return caller_pc=ea00 return_pc=ea03 record=510\n");
    check(!parse_later_trace_text(duplicate_trace, &record, &caller_pc,
                                  &return_pc, THERON_TRACK02_VARIANT_JP_BIN),
          "later e009 trace parser rejects duplicate return rows");
    snprintf(wrong_return_trace, sizeof(wrong_return_trace), "%s", valid_trace);
    wrong_return_trace[sizeof(wrong_return_trace) - 3u] = '1';
    check(!parse_later_trace_text(wrong_return_trace, &record, &caller_pc,
                                  &return_pc, THERON_TRACK02_VARIANT_JP_BIN),
          "later e009 trace parser rejects a changed return record");
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
        !parse_later_trace(trace_path, &record, &caller_pc, &return_pc,
                           manifest.variant) ||
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

static int inspect_coalesced(const char *track_path, const char *trace_path,
                             const char *system_card_path,
                             const char *manifest_path,
                             const char *expected_md5,
                             Theron_V1RawLoaderTraceCoalescedLaterReceipt *out)
{
    uint8_t *track_bytes;
    uint8_t *trace_bytes;
    uint8_t *manifest_bytes;
    size_t track_size;
    size_t trace_size;
    size_t manifest_size;
    char actual_md5[33];
    char system_card_md5[33];
    char trace_md5[33];
    Theron_V1CaptureManifest manifest;
    int ok;

    track_bytes = read_file_bytes(track_path, &track_size);
    trace_bytes = read_file_bytes(trace_path, &trace_size);
    manifest_bytes = read_file_bytes(manifest_path, &manifest_size);
    if (!track_bytes || !trace_bytes || !manifest_bytes) {
        free(track_bytes);
        free(trace_bytes);
        free(manifest_bytes);
        return 0;
    }
    (void)trace_size;
    (void)manifest_size;
    ok = m12_file_md5_hex(track_path, actual_md5) &&
        strcmp(actual_md5, expected_md5) == 0 &&
        m12_file_md5_hex(system_card_path, system_card_md5) &&
        m12_file_md5_hex(trace_path, trace_md5) &&
        theron_v1_capture_manifest_parse((const char *)manifest_bytes,
                                         &manifest) &&
        theron_v1_raw_loader_trace_capture_manifest_matches(
            &manifest, track_path, actual_md5, system_card_path,
            system_card_md5, trace_path, trace_md5) &&
        theron_v1_raw_loader_trace_bind_coalesced_later_e009_raw_sector(
            (const char *)trace_bytes, track_bytes, track_size, expected_md5,
            out);
    free(track_bytes);
    free(trace_bytes);
    free(manifest_bytes);
    return ok;
}

int main(void) {
    const char *jp_track = getenv("FIRESTAFF_THERON_TRACK02_JP_BIN");
    const char *us_track = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    const char *jp_trace = getenv("FIRESTAFF_THERON_LATER_CD_READ_TRACE_JP");
    const char *us_trace = getenv("FIRESTAFF_THERON_LATER_CD_READ_TRACE_US");
    const char *jp_coalesced =
        getenv("FIRESTAFF_THERON_LATER_CD_READ_COALESCED_TRACE_JP");
    const char *us_coalesced =
        getenv("FIRESTAFF_THERON_LATER_CD_READ_COALESCED_TRACE_US");
    const char *jp_coalesced_manifest =
        getenv("FIRESTAFF_THERON_LATER_CD_READ_COALESCED_MANIFEST_JP");
    const char *us_coalesced_manifest =
        getenv("FIRESTAFF_THERON_LATER_CD_READ_COALESCED_MANIFEST_US");
    const char *system_card = getenv("FIRESTAFF_THERON_SYSTEM_CARD3_ROM");
    LaterCdReadLayoutReceipt jp;
    LaterCdReadLayoutReceipt us;
    Theron_V1RawLoaderTraceCoalescedLaterReceipt jp_coalesced_receipt;
    Theron_V1RawLoaderTraceCoalescedLaterReceipt us_coalesced_receipt;

    test_later_trace_envelope_parser();
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
    if (jp_coalesced || us_coalesced || jp_coalesced_manifest ||
        us_coalesced_manifest || system_card) {
        check(jp_coalesced && us_coalesced && jp_coalesced_manifest &&
                  us_coalesced_manifest && system_card,
              "coalesced Mednafen evidence supplies both manifests and System Card 3.0");
        if (jp_coalesced && us_coalesced && jp_coalesced_manifest &&
            us_coalesced_manifest && system_card) {
            check(inspect_coalesced(jp_track, jp_coalesced, system_card,
                                    jp_coalesced_manifest,
                                    THERON_TRACK02_MD5_JP_BIN,
                                    &jp_coalesced_receipt) &&
                      inspect_coalesced(us_track, us_coalesced, system_card,
                                        us_coalesced_manifest,
                                        THERON_TRACK02_MD5_US_BIN,
                                        &us_coalesced_receipt),
                  "manifest-bound coalesced transcripts bind each later selector to original sector bytes");
            check(jp_coalesced_receipt.valid && us_coalesced_receipt.valid &&
                      jp_coalesced_receipt.descriptor_selector ==
                          us_coalesced_receipt.descriptor_selector &&
                      jp_coalesced_receipt.descriptor_selector_ordinal ==
                          us_coalesced_receipt.descriptor_selector_ordinal &&
                      jp_coalesced_receipt.observation_order_verified &&
                      us_coalesced_receipt.observation_order_verified &&
                      jp_coalesced_receipt.selector_sector_bytes_verified &&
                      us_coalesced_receipt.selector_sector_bytes_verified,
                  "coalesced JP/US receipts retain one selector and verified observation order");
        }
    }
    printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
