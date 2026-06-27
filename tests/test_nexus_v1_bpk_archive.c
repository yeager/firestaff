#include "nexus_v1_bpk_archive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static void wr32_be(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static void make_synthetic_bpk(uint8_t *data, size_t size) {
    memset(data, 0, size);
    memcpy(data + 0, "BPPK", 4);
    wr32_be(data + 4, (uint32_t)size);
    memcpy(data + 12, "BMPD", 4);
    wr32_be(data + 16, (uint32_t)(size - 20U));
    wr32_be(data + 20, 3U);
    wr32_be(data + 24, 64U);
    wr32_be(data + 28, 96U);
    wr32_be(data + 32, 128U);
    memcpy(data + 64U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);
    memcpy(data + 96U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);
}

static void test_synthetic_parse(void) {
    uint8_t data[160];
    Nexus_V1_BpkArchiveInfo info;
    Nexus_V1_BpkEntry entry;

    make_synthetic_bpk(data, sizeof(data));

    expect(nexus_v1_bpk_archive_parse(data, sizeof(data), &info) == 0,
           "synthetic BPK parses");
    expect(info.outer_size == sizeof(data), "outer size is recorded");
    expect(info.entry_count_hint == 3U, "entry count hint is recorded");
    expect(info.prs3_payload_count == 2U, "PRS3 payloads are counted");
    expect(info.raw_payload_count == 1U, "raw payloads are counted");
    expect(info.first_candidate_offset == 64U, "first offset is recorded");
    expect(info.last_candidate_offset == 128U, "last offset is recorded");

    expect(nexus_v1_bpk_archive_get_entry(data, sizeof(data), 0, &entry) == 0,
           "entry 0 is readable");
    expect(entry.offset == 64U, "entry 0 offset");
    expect(entry.next_offset == 96U, "entry 0 next offset");
    expect(entry.stored_size == 32U, "entry 0 stored span");
    expect(entry.has_prs3 == 1, "entry 0 has PRS3 marker");
    expect(entry.payload_offset == 88U, "entry 0 payload skips prefix+PRS3");
    expect(entry.payload_size == 8U, "entry 0 payload size");

    expect(nexus_v1_bpk_archive_get_entry(data, sizeof(data), 2, &entry) == 0,
           "last entry is readable");
    expect(entry.has_prs3 == 0, "last entry is raw");
    expect(entry.next_offset == sizeof(data), "last entry spans to file end");
}

static void test_rejections(void) {
    uint8_t data[160];
    Nexus_V1_BpkArchiveInfo info;

    make_synthetic_bpk(data, sizeof(data));
    expect(nexus_v1_bpk_archive_parse(NULL, sizeof(data), &info) != 0,
           "NULL data rejected");
    expect(nexus_v1_bpk_archive_parse(data, sizeof(data), NULL) != 0,
           "NULL output rejected");

    data[0] = 'X';
    expect(nexus_v1_bpk_archive_parse(data, sizeof(data), &info) != 0,
           "bad BPPK magic rejected");
    make_synthetic_bpk(data, sizeof(data));

    wr32_be(data + 4, 159U);
    expect(nexus_v1_bpk_archive_parse(data, sizeof(data), &info) != 0,
           "wrong outer size rejected");
    make_synthetic_bpk(data, sizeof(data));

    wr32_be(data + 28, 60U);
    expect(nexus_v1_bpk_archive_parse(data, sizeof(data), &info) != 0,
           "non-monotonic offsets rejected");
    make_synthetic_bpk(data, sizeof(data));

    wr32_be(data + 32, 148U);
    expect(nexus_v1_bpk_archive_parse(data, sizeof(data), &info) != 0,
           "truncated entry prefix rejected");
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *fp = fopen(path, "rb");
    long size;
    uint8_t *data;

    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(fp);
        return 0;
    }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static void test_optional_local_menu_bpk(void) {
    const char *home = getenv("HOME");
    char path[1024];
    uint8_t *data = NULL;
    size_t size = 0;
    Nexus_V1_BpkArchiveInfo info;
    Nexus_V1_BpkEntry entry;
    Nexus_V1_BpkModeDistribution dist;
    Nexus_V1_BpkEntryPrefix prefix;
    Nexus_V1_BpkPrs3Info prs3;
    uint32_t prs3_seen;
    uint32_t pix_matches;

    if (!home || !home[0]) {
        puts("SKIP: HOME is unset; no local Nexus MENU.BPK check");
        return;
    }
    if (snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/MENU.BPK",
                 home) < 0) {
        return;
    }
    if (!read_file(path, &data, &size)) {
        puts("SKIP: local Nexus MENU.BPK not present");
        return;
    }

    expect(nexus_v1_bpk_archive_parse(data, size, &info) == 0,
           "local MENU.BPK BPPK/BMPD directory parses");
    expect(info.outer_size == 89060U, "local MENU.BPK outer size");
    expect(info.bmpd_size == 88524U, "local MENU.BPK BMPD size");
    expect(info.entry_count_hint == 163U, "local MENU.BPK count hint");
    expect(info.candidate_offset_count == 163U,
           "local MENU.BPK candidate count");
    expect(info.prs3_payload_count == 162U,
           "local MENU.BPK PRS3 candidate count");
    expect(info.raw_payload_count == 1U, "local MENU.BPK raw candidate count");
    expect(info.first_candidate_offset == 0x29CU,
           "local MENU.BPK first candidate offset");
    expect(info.last_candidate_offset == 0x15980U,
           "local MENU.BPK last candidate offset");

    expect(nexus_v1_bpk_archive_get_entry(data, size, 1, &entry) == 0,
           "local MENU.BPK first PRS3 entry readable");
    expect(entry.offset == 0x36CU, "local MENU.BPK first PRS3 offset");
    expect(entry.has_prs3 == 1, "local MENU.BPK first PRS3 marker");

    /* pass1082: byte-level boundary inspection of the real MENU.BPK.
     * Walks the observed prefix/mode/PRS3 layout and asserts the mode
     * distribution the reverse-engineering pass identified. */
    expect(nexus_v1_bpk_archive_mode_distribution(data, size, &dist) == 0,
           "local MENU.BPK mode distribution walks");
    expect(dist.total_with_prefix == 163U,
           "local MENU.BPK all 163 entries span the 20-byte prefix");
    expect(dist.mode_count[NEXUS_V1_BPK_MODE_8BPP] == 14U,
           "local MENU.BPK mode tag 6: 14 entries");
    expect(dist.mode_count[NEXUS_V1_BPK_MODE_16BPP] == 62U,
           "local MENU.BPK mode tag 14: 62 entries");
    expect(dist.mode_count[NEXUS_V1_BPK_MODE_24BPP] == 39U,
           "local MENU.BPK mode tag 22: 39 entries");
    expect(dist.mode_count[NEXUS_V1_BPK_MODE_32BPP] == 47U,
           "local MENU.BPK mode tag 30: 47 entries");
    expect(dist.mode_count[NEXUS_V1_BPK_MODE_TRAILER] == 1U,
           "local MENU.BPK mode tag 10: 1 entry (directory trailer)");
    expect(dist.trailer_found == 1,
           "local MENU.BPK directory trailer detected");
    expect(dist.trailer_index == 0U,
           "local MENU.BPK directory trailer at entry index 0");

    expect(nexus_v1_bpk_archive_get_entry_prefix(data, size, 1, &prefix)
               == 0,
           "local MENU.BPK entry[1] prefix readable");
    expect(prefix.prefix_complete == 1,
           "local MENU.BPK entry[1] spans >= 20 bytes");
    expect(prefix.width == 16 && prefix.height == 15 &&
               prefix.mode == NEXUS_V1_BPK_MODE_16BPP,
           "local MENU.BPK entry[1] width=16 height=15 mode=14");

    expect(nexus_v1_bpk_archive_inspect_prs3(data, size, 1, &prs3) == 0,
           "local MENU.BPK entry[1] PRS3 inspect ok");
    expect(prs3.has_prs3 == 1, "local MENU.BPK entry[1] has PRS3 magic");
    expect(prs3.prs3_version_matches == 1,
           "local MENU.BPK entry[1] PRS3 version == 0x00000001");
    expect(prs3.prs3_pixel_count == 240U,
           "local MENU.BPK entry[1] PRS3 pixel count = 240");
    expect(prs3.prefix_pixels == 240U,
           "local MENU.BPK entry[1] prefix pixels = 240");
    expect(prs3.pixel_count_matches == 1,
           "local MENU.BPK entry[1] prefix_pixels == prs3_pixel_count");

    expect(nexus_v1_bpk_archive_inspect_prs3(data, size, 0, &prs3) == 0,
           "local MENU.BPK entry[0] PRS3 inspect ok (no PRS3)");
    expect(prs3.has_prs3 == 0,
           "local MENU.BPK entry[0] directory trailer has no PRS3 magic");

    /* Cross-check every PRS3 entry: prefix width*height must match the
     * PRS3+8 pixel count. This is the strongest single signal we have
     * that the reverse-engineered 20-byte prefix layout is right. */
    prs3_seen = 0U;
    pix_matches = 0U;
    for (uint32_t i = 0; i < info.entry_count_hint; ++i) {
        if (nexus_v1_bpk_archive_inspect_prs3(data, size, i, &prs3) != 0) {
            continue;
        }
        if (!prs3.has_prs3) continue;
        ++prs3_seen;
        if (prs3.pixel_count_matches) ++pix_matches;
    }
    expect(prs3_seen == 162U,
           "local MENU.BPK inspected 162/162 PRS3-bearing entries");
    expect(pix_matches == 162U,
           "local MENU.BPK every PRS3 entry has width*height == pixel count");

    free(data);
}

static void test_prefix_prs3_rejections(void) {
    uint8_t data[160];
    Nexus_V1_BpkModeDistribution dist;
    Nexus_V1_BpkEntryPrefix prefix;
    Nexus_V1_BpkPrs3Info prs3;

    make_synthetic_bpk(data, sizeof(data));
    expect(nexus_v1_bpk_archive_get_entry_prefix(NULL, sizeof(data), 0,
                                                 &prefix) != 0,
           "NULL data rejected in get_entry_prefix");
    expect(nexus_v1_bpk_archive_get_entry_prefix(data, sizeof(data), 0,
                                                 NULL) != 0,
           "NULL output rejected in get_entry_prefix");
    expect(nexus_v1_bpk_archive_get_entry_prefix(data, sizeof(data),
                                                 99U, &prefix) != 0,
           "out-of-range index rejected in get_entry_prefix");

    make_synthetic_bpk(data, sizeof(data));
    expect(nexus_v1_bpk_archive_inspect_prs3(NULL, sizeof(data), 0,
                                              &prs3) != 0,
           "NULL data rejected in inspect_prs3");
    expect(nexus_v1_bpk_archive_inspect_prs3(data, sizeof(data), 0,
                                              NULL) != 0,
           "NULL output rejected in inspect_prs3");
    expect(nexus_v1_bpk_archive_inspect_prs3(data, sizeof(data),
                                              99U, &prs3) != 0,
           "out-of-range index rejected in inspect_prs3");

    make_synthetic_bpk(data, sizeof(data));
    expect(nexus_v1_bpk_archive_mode_distribution(NULL, sizeof(data),
                                                  &dist) != 0,
           "NULL data rejected in mode_distribution");
    expect(nexus_v1_bpk_archive_mode_distribution(data, sizeof(data),
                                                  NULL) != 0,
           "NULL output rejected in mode_distribution");
    data[0] = 'X';
    expect(nexus_v1_bpk_archive_mode_distribution(data, sizeof(data),
                                                  &dist) != 0,
           "bad BPPK magic rejected in mode_distribution");
}

int main(void) {
    test_synthetic_parse();
    test_rejections();
    test_prefix_prs3_rejections();
    test_optional_local_menu_bpk();

    if (g_failures) return 1;
    puts("test_nexus_v1_bpk_archive: PASS");
    return 0;
}
