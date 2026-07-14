/*
 * Bounded integration check for the future positive Track 02 loader/CD
 * handoff. A fixture receipt only exercises composition; it is not presented
 * as an original capture. A real positive result requires the operator to
 * supply a coalesced Mednafen transcript that the production parser accepts.
 */
#include "asset_status_m12.h"
#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file = NULL;
    long size;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static uint32_t fnv1a_bytes(const uint8_t *bytes, size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static void fixture_receipt(Theron_V1RawLoaderTraceCoalescedLaterReceipt *out,
                            const char *md5, const uint8_t *raw)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", md5);
    out->later_track02_record = 0x0b52u;
    out->descriptor_selector = 0x067cu;
    out->descriptor_selector_ordinal = 0u;
    out->caller_pc = 0xea00u;
    out->return_pc = 0xea03u;
    out->later_caller_opcode = 0x20u;
    out->later_caller_target = 0xe009u;
    out->later_caller_control_verified = 1;
    out->sector_count = 1u;
    out->later_local_destination = 0x3800u;
    out->later_destination_span_bytes = 32u;
    out->later_destination_span_checksum = fnv1a_bytes(
        raw + 0x0b52u * THERON_TRACK02_RAW_SECTOR_BYTES +
        THERON_TRACK02_RAW_USER_DATA_OFFSET, 32u);
    out->later_destination_local_ram_verified = 1;
    out->later_destination_media_span_verified = 1;
    out->later_post_return_resume_pc = 0xea03u;
    out->later_post_return_next_pc = 0xea04u;
    out->later_post_return_step_verified = 1;
    out->observation_order_verified = 1;
    out->selector_sector_bytes_verified = 1;
}

int main(void)
{
    const char *raw_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    const char *trace_path = getenv("FIRESTAFF_THERON_COALESCED_LOADER_TRACE");
    char md5[33];
    uint8_t *raw;
    size_t raw_size;
    Theron_V1RawLoaderTraceCoalescedLaterReceipt coalesced;
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt handoff;

    if (!raw_path) {
        printf("SKIP: set FIRESTAFF_THERON_TRACK02_US_BIN for raw-media handoff coverage\n");
        return 0;
    }
    raw = read_file(raw_path, &raw_size);
    if (!raw || !m12_file_md5_hex(raw_path, md5) ||
        strcmp(md5, THERON_TRACK02_MD5_US_BIN) != 0) {
        free(raw);
        printf("FAIL: supplied US Track 02 is not the source-locked raw corpus\n");
        return 1;
    }

    fixture_receipt(&coalesced, md5, raw);
    if (!theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff) || !handoff.valid ||
        handoff.observed_track02_record != 0x0b52u ||
        !handoff.complete_initial_level_envelope_proven ||
        handoff.object_tail_semantics_proven || handoff.fallback_visuals_allowed) {
        free(raw);
        printf("FAIL: fixture composition did not preserve the bounded handoff contract\n");
        return 1;
    }
    coalesced.later_post_return_step_verified = 0;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt without the post-return control step admitted the level route\n");
        return 1;
    }
    coalesced.later_post_return_step_verified = 1;
    coalesced.later_post_return_resume_pc = 0xea04u;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt without the returned-PC control edge admitted the level route\n");
        return 1;
    }
    coalesced.later_post_return_resume_pc = coalesced.return_pc;
    coalesced.later_caller_control_verified = 0;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt without the observed e009 caller control admitted the level route\n");
        return 1;
    }
    coalesced.later_caller_control_verified = 1;
    coalesced.later_caller_target = 0xe00cu;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt with a non-e009 caller target admitted the level route\n");
        return 1;
    }
    coalesced.later_caller_target = 0xe009u;

    ++coalesced.later_destination_span_checksum;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt with a mismatched source span admitted the level route\n");
        return 1;
    }
    --coalesced.later_destination_span_checksum;
    coalesced.later_track02_record = 0x0b53u;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: adjacent CD record unexpectedly admitted the level route\n");
        return 1;
    }
    printf("PASS: source-bound receipt composes only record 0x0b52; no object or visual semantics promoted\n");

    if (trace_path) {
        uint8_t *trace;
        size_t trace_size;
        char *trace_text;

        trace = read_file(trace_path, &trace_size);
        trace_text = (char *)trace;
        if (!trace || memchr(trace_text, '\0', trace_size) != NULL) {
            free(raw);
            free(trace);
            printf("FAIL: supplied coalesced trace is not bounded text\n");
            return 1;
        }
        trace_text = (char *)realloc(trace, trace_size + 1u);
        if (!trace_text) {
            free(raw);
            free(trace);
            return 1;
        }
        trace_text[trace_size] = '\0';
        if (!theron_v1_raw_loader_trace_bind_coalesced_later_e009_raw_sector(
                trace_text, raw, raw_size, md5, &coalesced) ||
            !theron_v1_raw_loader_trace_bind_initial_level_handoff(
                &coalesced, raw, raw_size, md5, &handoff)) {
            free(trace_text);
            free(raw);
            printf("FAIL: supplied coalesced capture does not prove the initial-level CD handoff\n");
            return 1;
        }
        free(trace_text);
        printf("PASS: authenticated coalesced capture observes the source-locked initial-level record\n");
    } else {
        printf("SKIP: set FIRESTAFF_THERON_COALESCED_LOADER_TRACE for an authenticated capture result\n");
    }
    free(raw);
    return 0;
}
