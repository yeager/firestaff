/*
 * Theron's Quest Track 02 non-startup sector receipt probe.
 *
 * This probe records only MODE1/2352 container facts for structurally
 * post-descriptor windows.  It does not identify a bitmap, level, object,
 * palette, text payload, or a runtime route.
 */

#include "asset_status_m12.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

#define RAW_FIXTURE_SIZE 0x713000u
#define DESCRIPTOR_OFFSET_DELTA 0x1584u
#define WINDOW_FIRST_RELATIVE_OFFSET 0x0020u
#define WINDOW_STRIDE 0x0400u

static int g_fail = 0;
static int g_skip = 0;

static const uint8_t g_descriptor[18] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static const uint8_t g_boundary_span[44] = {
    0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
    0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
    0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
    0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
    0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
    0x93, 0x80, 0x00, 0x3f
};

static const size_t g_us_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70be06u, 0x70e2c6u, 0x710904u
};

static const size_t g_us_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x2d53e0u, 0x47d040u, 0x712840u
};

static const size_t g_jp_descriptor_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x70b4d6u, 0x70d996u, 0x70ffd4u
};

static const size_t g_jp_span_offsets[THERON_TRACK02_MAX_BANK_ANCHORS] = {
    0x2d4ab0u, 0x47c710u, 0x711f10u
};

static void check(int condition, const char *label) {
    if (!condition) {
        printf("FAIL %s\n", label);
        ++g_fail;
    }
}

static size_t window_offset(size_t descriptor_offset, size_t entry_index) {
    return descriptor_offset - DESCRIPTOR_OFFSET_DELTA +
        WINDOW_FIRST_RELATIVE_OFFSET + entry_index * WINDOW_STRIDE;
}

static void build_raw_fixture(uint8_t *bytes, size_t byte_count,
                              const size_t *descriptor_offsets,
                              const size_t *span_offsets,
                              uint8_t payload_tag) {
    size_t anchor_index;

    memset(bytes, 0, byte_count);
    for (anchor_index = 0u;
         anchor_index < THERON_TRACK02_MAX_BANK_ANCHORS;
         ++anchor_index) {
        size_t entry_index;

        memcpy(bytes + descriptor_offsets[anchor_index],
               g_descriptor, sizeof(g_descriptor));
        memcpy(bytes + span_offsets[anchor_index],
               g_boundary_span, sizeof(g_boundary_span));
        for (entry_index = 6u; entry_index < 9u; ++entry_index) {
            bytes[window_offset(descriptor_offsets[anchor_index],
                                entry_index)] =
                (uint8_t)(payload_tag + anchor_index * 3u + entry_index);
        }
    }
}

static void compare_layouts(const Theron_Track02NonstartupSectorReceipt *jp,
                            const Theron_Track02NonstartupSectorReceipt *us,
                            const char *label) {
    Theron_Track02NonstartupSectorLayoutComparisonReceipt comparison;
    Theron_Track02NonstartupSectorLayoutComparisonStatus status =
        theron_v1_track02_compare_nonstartup_sector_layout_variants(
            jp, us, &comparison);
    unsigned int expected_mask =
        (1u << THERON_TRACK02_MAX_BANK_ANCHORS) - 1u;

    check(status == THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_OK &&
              comparison.valid,
          "JP/US opaque layout comparison is hash-gated and valid");
    check(comparison.opaque_only && comparison.promotion_blocked,
          "JP/US opaque layout comparison cannot promote a payload");
    check(comparison.comparable_anchor_mask == expected_mask &&
              comparison.layout_matching_anchor_mask == expected_mask,
          "JP/US descriptor-relative opaque geometry agrees at all anchors");
    check(comparison.comparison_hash != 0u,
          "JP/US opaque layout comparison has a reproducible hash");
    printf("RECEIPT %s comparable=0x%x layout=0x%x content-match=0x%x "
           "content-mismatch=0x%x hash=0x%08x opaque=%d blocked=%d\n",
           label, comparison.comparable_anchor_mask,
           comparison.layout_matching_anchor_mask,
           comparison.content_matching_anchor_mask,
           comparison.content_mismatch_anchor_mask,
           (unsigned)comparison.comparison_hash, comparison.opaque_only,
           comparison.promotion_blocked);
}

static void check_receipt(const Theron_Track02NonstartupSectorReceipt *receipt,
                          const char *label) {
    size_t anchor_index;
    int is_synthetic = strstr(label, "synthetic") != NULL;
    const size_t *descriptor_offsets = strstr(label, "-jp")
        ? g_jp_descriptor_offsets : g_us_descriptor_offsets;

    check(receipt->valid && receipt->verified_track02,
          "receipt valid and known-variant-gated");
    check(receipt->opaque_only && receipt->promotion_blocked,
          "receipt remains opaque and promotion-blocked");
    check(receipt->anchor_count == THERON_TRACK02_MAX_BANK_ANCHORS,
          "receipt retains all descriptor anchors");
    check(receipt->receipt_hash != 0u, "receipt has reproducible hash");

    for (anchor_index = 0u; anchor_index < receipt->anchor_count; ++anchor_index) {
        size_t window_index;

        if (is_synthetic) {
            check(receipt->window_count[anchor_index] == 3u,
                  "fixture has three post-descriptor windows per anchor");
        } else {
            check(receipt->window_count[anchor_index] > 0u,
                  "real anchor has opaque post-descriptor windows");
        }
        for (window_index = 0u;
             window_index < receipt->window_count[anchor_index];
             ++window_index) {
            const Theron_Track02NonstartupSectorWindowReceipt *window =
                &receipt->windows[anchor_index][window_index];
            size_t expected_entry = 6u + window_index;

            if (is_synthetic) {
                check(window->descriptor_entry_index == expected_entry,
                      "receipt preserves post-descriptor entry index");
                check(window->raw_offset ==
                          window_offset(descriptor_offsets[anchor_index],
                                        expected_entry),
                      "receipt preserves exact descriptor-derived raw boundary");
            }
            check(window->byte_count == WINDOW_STRIDE,
                  "receipt window remains exactly 0x400 bytes");
            check(window->raw_span_hash != 0u && window->opaque &&
                      window->promotion_blocked,
                  "receipt fingerprints bytes without semantic promotion");
            check(window->raw_span_contains_non_user_data ||
                      window->user_data_span_contiguous,
                  "receipt classifies the MODE1 container boundary");
            printf("RECEIPT %s anchor=%zu entry=%zu raw=0x%zx bytes=%zu "
                   "sectors=%zu..%zu user=0x%zx..0x%zx hash=0x%08x "
                   "opaque=%d blocked=%d\n",
                   label, anchor_index, window->descriptor_entry_index,
                   window->raw_offset, window->byte_count,
                   window->first_raw_sector, window->last_raw_sector,
                   window->user_data_offset, window->user_data_end_offset,
                   (unsigned)window->raw_span_hash, window->opaque,
                   window->promotion_blocked);
        }
    }
}

static int read_file(const char *path, uint8_t **out_bytes, size_t *out_size) {
    FILE *file;
    long size;
    uint8_t *bytes;

    *out_bytes = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_bytes = bytes;
    *out_size = (size_t)size;
    return 1;
}

static void probe_real_media(const char *env_name,
                             const char *default_file,
                             const char *expected_md5,
                             const char *label,
                             Theron_Track02NonstartupSectorReceipt *out_receipt,
                             int *out_have_receipt) {
    char default_path[1024];
    const char *path = getenv(env_name);
    uint8_t *bytes;
    size_t byte_count;
    char md5[33];
    Theron_Track02NonstartupSectorReceipt receipt;
    Theron_Track02SignalStatus status;

    if (!path || !path[0]) {
        const char *home = getenv("HOME");
        snprintf(default_path, sizeof(default_path), "%s%s.firestaff%sdata%s%s",
                 (home && home[0]) ? home : ".", PATH_SEP, PATH_SEP,
                 PATH_SEP, default_file);
        path = default_path;
    }
    if (!read_file(path, &bytes, &byte_count)) {
        printf("SKIP %s media absent: %s\n", label, path);
        ++g_skip;
        return;
    }
    if (!m12_file_md5_hex(path, md5) || strcmp(md5, expected_md5) != 0) {
        printf("SKIP %s media MD5 is not the verified Track 02 variant\n", label);
        ++g_skip;
        free(bytes);
        return;
    }
    status = theron_v1_track02_capture_nonstartup_sector_receipt(
        bytes, byte_count, expected_md5, &receipt);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        printf("FAIL %s sector receipt status=%s\n", label,
               theron_v1_track02_signal_status_name(status));
    }
    check(status == THERON_TRACK02_SIGNAL_OK,
          "real media sector receipt captures");
    if (receipt.valid) check_receipt(&receipt, label);
    if (receipt.valid && out_receipt && out_have_receipt) {
        *out_receipt = receipt;
        *out_have_receipt = 1;
    }
    free(bytes);
}

int main(void) {
    uint8_t *fixture = (uint8_t *)malloc(RAW_FIXTURE_SIZE);
    Theron_Track02NonstartupSectorReceipt receipt;
    Theron_Track02NonstartupSectorReceipt synthetic_us;
    Theron_Track02NonstartupSectorReceipt synthetic_jp;
    Theron_Track02NonstartupSectorReceipt real_us;
    Theron_Track02NonstartupSectorReceipt real_jp;
    int have_real_us = 0;
    int have_real_jp = 0;
    Theron_Track02SignalStatus status;

    check(fixture != NULL, "fixture allocation");
    if (!fixture) return 1;
    build_raw_fixture(fixture, RAW_FIXTURE_SIZE, g_us_descriptor_offsets,
                      g_us_span_offsets, 0xa0u);
    status = theron_v1_track02_capture_nonstartup_sector_receipt(
        fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_US_BIN, &receipt);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        printf("FAIL synthetic sector receipt status=%s\n",
               theron_v1_track02_signal_status_name(status));
    }
    check(status == THERON_TRACK02_SIGNAL_OK,
          "synthetic raw container receipt captures");
    synthetic_us = receipt;
    if (receipt.valid) check_receipt(&receipt, "synthetic-us");
    build_raw_fixture(fixture, RAW_FIXTURE_SIZE, g_jp_descriptor_offsets,
                      g_jp_span_offsets, 0xb0u);
    status = theron_v1_track02_capture_nonstartup_sector_receipt(
        fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_JP_BIN, &receipt);
    check(status == THERON_TRACK02_SIGNAL_OK,
          "synthetic JP raw container receipt captures");
    synthetic_jp = receipt;
    if (receipt.valid) check_receipt(&receipt, "synthetic-jp");
    compare_layouts(&synthetic_jp, &synthetic_us, "synthetic-jp-us");
    check(theron_v1_track02_capture_nonstartup_sector_receipt(
              fixture, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_US_ISO, &receipt) ==
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT,
          "ISO cannot enter raw-sector receipt");
    check(theron_v1_track02_capture_nonstartup_sector_receipt(
              NULL, RAW_FIXTURE_SIZE, THERON_TRACK02_MD5_US_BIN, &receipt) ==
              THERON_TRACK02_SIGNAL_BAD_INPUT,
          "NULL raw container rejects");
    free(fixture);

    probe_real_media("FIRESTAFF_THERON_TRACK02_US_BIN",
                     "theron/TQUS02.bin",
                     THERON_TRACK02_MD5_US_BIN, "real-us", &real_us,
                     &have_real_us);
    probe_real_media("FIRESTAFF_THERON_TRACK02_JP_BIN",
                     "theron/TQJP02.bin",
                     THERON_TRACK02_MD5_JP_BIN, "real-jp", &real_jp,
                     &have_real_jp);
    if (have_real_jp && have_real_us) {
        compare_layouts(&real_jp, &real_us, "real-jp-us");
    } else {
        printf("SKIP real JP/US opaque layout comparison needs both staged raw Track 02 BINs\n");
        ++g_skip;
    }

    printf("Theron Track 02 non-startup sector receipt probe: %s (%d skipped)\n",
           g_fail == 0 ? "PASS" : "FAIL", g_skip);
    return g_fail == 0 ? 0 : 1;
}
