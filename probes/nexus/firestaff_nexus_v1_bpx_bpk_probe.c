/*
 * firestaff_nexus_v1_bpx_bpk_probe.c
 * ===================================
 *
 * Data-free Nexus BPX/BPK archive-boundary probe.
 *
 * This is intentionally not a real MENU.BPK decompression claim. Local
 * source-lock docs identify MENU.BPK as a packed, game-specific Nexus file
 * but also state that no formal compression analysis has been performed.
 * The probe therefore verifies:
 *   - the verified MENU.BPK size/hash marker is recognized;
 *   - unknown .BPK/.BPX payloads stay unsupported;
 *   - a synthetic BPX0 table contract parses deterministically;
 *   - stored entries can be extracted without allocation;
 *   - compressed entries report method-unsupported;
 *   - malformed headers/counts/bounds are rejected.
 *
 * Source evidence:
 *   docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md:291-306
 *   docs/VERIFIED_HASHES.md:103
 */

#include "nexus_v1_bpx_bpk.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                      \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }          \
    else { printf("  FAIL: %s\n", msg); ++g_fail; }               \
} while (0)

static void wb16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)((v >> 8) & 0xffu);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wb32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)((v >> 24) & 0xffu);
    p[1] = (uint8_t)((v >> 16) & 0xffu);
    p[2] = (uint8_t)((v >> 8) & 0xffu);
    p[3] = (uint8_t)(v & 0xffu);
}

static void make_entry(uint8_t *record,
                       const char *name,
                       uint32_t offset,
                       uint32_t packed_size,
                       uint32_t unpacked_size,
                       uint8_t method) {
    memset(record, 0, NEXUS_V1_BPX0_ENTRY_SIZE);
    strncpy((char *)record, name, 15);
    wb32(record + 16, offset);
    wb32(record + 20, packed_size);
    wb32(record + 24, unpacked_size);
    record[28] = method;
}

static size_t make_good_bpx0(uint8_t *buf, size_t cap) {
    const uint32_t table_offset = NEXUS_V1_BPX0_HEADER_SIZE;
    const uint32_t data_offset = NEXUS_V1_BPX0_HEADER_SIZE +
        (2u * NEXUS_V1_BPX0_ENTRY_SIZE);
    if (cap < data_offset + 7u) return 0;

    memset(buf, 0, cap);
    memcpy(buf, "BPX0", 4);
    wb16(buf + 4, 1);
    wb16(buf + 6, 2);
    wb32(buf + 8, table_offset);
    wb32(buf + 12, data_offset);

    make_entry(buf + table_offset, "MENU.RAW", data_offset, 4, 4,
               NEXUS_V1_BPX_BPK_METHOD_STORED);
    make_entry(buf + table_offset + NEXUS_V1_BPX0_ENTRY_SIZE, "MENU.PCK",
               data_offset + 4u, 3, 9,
               NEXUS_V1_BPX_BPK_METHOD_COMPRESSED_UNKNOWN);

    memcpy(buf + data_offset, "ABCDXYZ", 7);
    return data_offset + 7u;
}

int main(void) {
    uint8_t archive_data[128];
    size_t archive_size;
    Nexus_V1_BpxBpkArchive archive;
    Nexus_V1_BpxBpkFormat format;
    const Nexus_V1_BpxBpkEntry *entry;
    uint8_t out[8];
    int rc;

    printf("=== Nexus V1 BPX/BPK archive-boundary probe ===\n\n");

    rc = nexus_v1_bpx_bpk_identify_marker(
        "nexus/MENU.BPK",
        NEXUS_V1_MENU_BPK_SIZE,
        NEXUS_V1_MENU_BPK_SHA256,
        &format);
    CHECK(rc == NEXUS_V1_BPX_BPK_OK &&
          format == NEXUS_V1_BPX_BPK_FORMAT_VERIFIED_MENU_BPK_MARKER,
          "verified MENU.BPK size/hash marker recognized");

    rc = nexus_v1_bpx_bpk_identify_marker(
        "nexus/MENU.BPK",
        NEXUS_V1_MENU_BPK_SIZE,
        "0000000000000000000000000000000000000000000000000000000000000000",
        &format);
    CHECK(rc == NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED &&
          format == NEXUS_V1_BPX_BPK_FORMAT_UNKNOWN,
          "unknown .BPK marker remains unsupported");

    rc = nexus_v1_bpx_bpk_identify_marker(
        "nexus/MENU.BIN",
        NEXUS_V1_MENU_BPK_SIZE,
        "0000000000000000000000000000000000000000000000000000000000000000",
        &format);
    CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BAD_MAGIC,
          "non-BPK/BPX marker is not claimed");

    archive_size = make_good_bpx0(archive_data, sizeof(archive_data));
    rc = nexus_v1_bpx0_parse(archive_data, archive_size, &archive);
    CHECK(rc == NEXUS_V1_BPX_BPK_OK, "synthetic BPX0 archive parses");
    CHECK(archive.format == NEXUS_V1_BPX_BPK_FORMAT_SYNTHETIC_BPX0,
          "synthetic archive format recorded");
    CHECK(archive.entry_count == 2, "entry count parsed");
    CHECK(archive.data_offset == 80, "data offset parsed");

    entry = nexus_v1_bpx_bpk_find_entry(&archive, "MENU.RAW");
    CHECK(entry != NULL && entry->offset == 80 && entry->packed_size == 4,
          "stored entry lookup returns exact table fields");

    memset(out, 0, sizeof(out));
    rc = nexus_v1_bpx_bpk_extract_stored(archive_data, archive_size, entry,
                                         out, sizeof(out));
    CHECK(rc == 4 && memcmp(out, "ABCD", 4) == 0,
          "stored entry extracts exact bytes");

    rc = nexus_v1_bpx_bpk_extract_stored(archive_data, archive_size, entry,
                                         out, 3);
    CHECK(rc == NEXUS_V1_BPX_BPK_ERR_OUTPUT_TOO_SMALL,
          "stored extraction rejects undersized output buffer");

    entry = nexus_v1_bpx_bpk_find_entry(&archive, "MENU.PCK");
    CHECK(entry != NULL && entry->method == NEXUS_V1_BPX_BPK_METHOD_COMPRESSED_UNKNOWN,
          "compressed synthetic entry is visible but marked unknown");
    rc = nexus_v1_bpx_bpk_extract_stored(archive_data, archive_size, entry,
                                         out, sizeof(out));
    CHECK(rc == NEXUS_V1_BPX_BPK_ERR_METHOD,
          "unknown-compression entry is not extracted as stored");

    entry = nexus_v1_bpx_bpk_find_entry(&archive, "MISSING");
    CHECK(entry == NULL, "missing entry lookup returns NULL");

    rc = nexus_v1_bpx0_parse(NULL, archive_size, &archive);
    CHECK(rc == NEXUS_V1_BPX_BPK_ERR_NULL, "NULL data rejected");

    rc = nexus_v1_bpx0_parse(archive_data, 8, &archive);
    CHECK(rc == NEXUS_V1_BPX_BPK_ERR_TOO_SMALL, "short header rejected");

    {
        uint8_t bad[128];
        size_t bad_size = make_good_bpx0(bad, sizeof(bad));
        bad[0] = 'X';
        rc = nexus_v1_bpx0_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BAD_MAGIC, "bad magic rejected");
    }

    {
        uint8_t bad[128];
        size_t bad_size = make_good_bpx0(bad, sizeof(bad));
        wb16(bad + 6, NEXUS_V1_BPX_BPK_MAX_ENTRIES + 1u);
        rc = nexus_v1_bpx0_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_COUNT, "oversized entry count rejected");
    }

    {
        uint8_t bad[128];
        size_t bad_size = make_good_bpx0(bad, sizeof(bad));
        wb32(bad + 12, 32);
        rc = nexus_v1_bpx0_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BOUNDS,
              "data offset overlapping table rejected");
    }

    {
        uint8_t bad[128];
        size_t bad_size = make_good_bpx0(bad, sizeof(bad));
        wb32(bad + 16 + 16, (uint32_t)bad_size + 1u);
        rc = nexus_v1_bpx0_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_BOUNDS,
              "entry payload outside archive rejected");
    }

    {
        uint8_t bad[128];
        size_t bad_size = make_good_bpx0(bad, sizeof(bad));
        bad[16 + 29] = 1;
        rc = nexus_v1_bpx0_parse(bad, bad_size, &archive);
        CHECK(rc == NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED,
              "reserved entry bytes rejected");
    }

    CHECK(strcmp(nexus_v1_bpx_bpk_status_string(NEXUS_V1_BPX_BPK_OK), "ok") == 0,
          "status string exposes OK");
    CHECK(strcmp(nexus_v1_bpx_bpk_status_string(12345), "unknown-status") == 0,
          "unknown status string is stable");

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
