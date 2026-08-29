#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_iso_reader.h"
#include "asset_find_by_hash.h"

#include <limits.h>
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

static void test_exact_prefix_raw_entry(void) {
    uint8_t data[116];
    Nexus_V1_BpkArchiveInfo info;
    Nexus_V1_BpkEntry entry;

    memset(data, 0, sizeof(data));
    memcpy(data + 0, "BPPK", 4);
    wr32_be(data + 4, (uint32_t)sizeof(data));
    memcpy(data + 12, "BMPD", 4);
    wr32_be(data + 16, (uint32_t)(sizeof(data) - 20U));
    wr32_be(data + 20, 2U);
    wr32_be(data + 24, 64U);
    wr32_be(data + 28, 96U);
    memcpy(data + 64U + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);

    expect(nexus_v1_bpk_archive_parse(data, sizeof(data), &info) == 0,
           "exact-prefix raw entry archive parses");
    expect(info.prs3_payload_count == 1U,
           "exact-prefix archive counts one PRS3 entry");
    expect(info.raw_payload_count == 1U,
           "exact-prefix archive counts one raw entry");

    expect(nexus_v1_bpk_archive_get_entry(data, sizeof(data), 1U,
                                          &entry) == 0,
           "exact-prefix raw entry is readable");
    expect(entry.stored_size == NEXUS_V1_BPK_ENTRY_PREFIX_BYTES,
           "exact-prefix raw entry stored size is 20");
    expect(entry.has_prs3 == 0,
           "exact-prefix raw entry has no PRS3 marker");
    expect(entry.payload_offset == entry.next_offset &&
               entry.payload_size == 0U,
           "exact-prefix raw entry has empty payload span");
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

/* Read MENU.BPK through the production ISO reader.  This deliberately keeps
 * the retail CD payload in its original Track 1 container: the test owns only
 * the bounded RAM buffer returned by nexus_iso_read_file(), never a materialized
 * game-data file. */
static int read_menu_bpk_from_track(const char *track_path,
                                    uint8_t **out_data,
                                    size_t *out_size) {
    Nexus_ISOReader reader;
    const Nexus_ISOFile *menu;
    uint8_t *data;

    if (!track_path || !out_data || !out_size) return 0;
    memset(&reader, 0, sizeof(reader));
    if (nexus_iso_open(&reader, track_path) < 0) return 0;
    menu = nexus_iso_find(&reader, "MENU.BPK");
    if (!menu || menu->is_dir || menu->size == 0U || menu->size > (uint32_t)INT_MAX) {
        nexus_iso_close(&reader);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)menu->size);
    if (!data || nexus_iso_read_file(&reader, menu, data, (int)menu->size) !=
                     (int)menu->size) {
        free(data);
        nexus_iso_close(&reader);
        return 0;
    }
    nexus_iso_close(&reader);
    *out_data = data;
    *out_size = (size_t)menu->size;
    return 1;
}

static void test_optional_local_menu_bpk(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *track_path = getenv("FIRESTAFF_NEXUS_TRACK1_BIN");
    const char *home = getenv("HOME");
    char path[2048];
    char track[2048];
    uint8_t *data = NULL;
    size_t size = 0;
    int is_loose_menu = 0;
    Nexus_V1_BpkArchiveInfo info;
    Nexus_V1_BpkEntry entry;
    Nexus_V1_BpkModeDistribution dist;
    Nexus_V1_BpkEntryPrefix prefix;
    Nexus_V1_BpkPrs3Info prs3;
    Nexus_V1_BpkPaletteTrailerReceipt palette_trailer;
    uint16_t palette_words[NEXUS_V1_BPK_PALT_ENTRY_COUNT];
    uint64_t palette_fnv = 0U;
    uint32_t prs3_seen;
    uint32_t pix_matches;
    uint32_t decoded_seen = 0U;
    uint32_t decoded_failures = 0U;
    uint64_t decoded_pixels = 0U;
    Nexus_V1_BpkRuntimeDecodeReceipt decoded_receipt;

    if (data_dir && data_dir[0]) {
        if (snprintf(path, sizeof(path), "%s/MENU.BPK", data_dir) < 0) {
            return;
        }
    } else if (home && home[0]) {
        if (snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/MENU.BPK",
                     home) < 0) {
            return;
        }
    } else {
        puts("SKIP: Nexus data root is unset; no local MENU.BPK check");
        return;
    }
    if (read_file(path, &data, &size)) {
        is_loose_menu = 1;
    } else {
        if (!track_path || !track_path[0]) {
            if (data_dir && data_dir[0]) {
                if (snprintf(track, sizeof(track),
                             "%s/Dungeon Master Nexus (Japan) (Track 1).bin",
                             data_dir) < 0) return;
                track_path = track;
            } else {
                puts("SKIP: local Nexus MENU.BPK and Track 1 are not present");
                return;
            }
        }
        if (!read_menu_bpk_from_track(track_path, &data, &size)) {
            puts("SKIP: local Nexus MENU.BPK and readable Track 1 are not present");
            return;
        }
        puts("INFO: validated MENU.BPK directly from original Nexus Track 1");
    }

    if (is_loose_menu) {
        expect(asset_file_matches_md5(path,
                                      "c2776768ff25287c79013a1452253ca0") ||
                   asset_file_matches_md5(path,
                                      "a6f2272a4f6cb3c6b3b33012bc5b15ed") ||
                   asset_file_matches_md5(path,
                                      "fcf8a00fbb92593ed9ae908f8e285cda"),
               "local MENU.BPK matches an authenticated Japanese/English/French retail revision");
    }

    expect(nexus_v1_bpk_archive_parse(data, size, &info) == 0,
           "local MENU.BPK BPPK/BMPD directory parses");
    /* The verified corpus contains the 89,060-byte capture revision, the
     * 87,684-byte English European revision, and the 87,820-byte French
     * European revision. Their directory grammar is identical; only the
     * outer size and final candidate offset differ. */
    expect(info.outer_size == 89060U || info.outer_size == 87684U ||
               info.outer_size == 87820U,
           "local MENU.BPK outer size is a verified retail revision");
    expect(info.bmpd_size == info.outer_size - 536U,
           "local MENU.BPK BMPD size follows verified framing");
    expect(info.entry_count_hint == 163U, "local MENU.BPK count hint");
    expect(info.candidate_offset_count == 163U,
           "local MENU.BPK candidate count");
    expect(info.prs3_payload_count == 162U,
           "local MENU.BPK PRS3 candidate count");
    expect(info.raw_payload_count == 1U, "local MENU.BPK raw candidate count");
    expect(info.first_candidate_offset == 0x29CU,
           "local MENU.BPK first candidate offset");
    expect(info.last_candidate_offset ==
               (info.outer_size == 87684U ? 0x15420U :
                (info.outer_size == 87820U ? 0x154A8U : 0x15980U)),
           "local MENU.BPK last candidate offset");

    expect(nexus_v1_bpk_archive_inspect_palette_trailer(
               data, size, &palette_trailer) == 0 && palette_trailer.valid &&
               palette_trailer.entry_count == 256U &&
               palette_trailer.entry_bytes == 512U,
           "local MENU.BPK PALT trailer is bounded at 256 BE16 entries");
    expect(nexus_v1_bpk_archive_copy_palette_words_be16(
               data, size, palette_words, &palette_fnv) == 0 &&
               palette_fnv == UINT64_C(0x0ec4e98ca3a18f85),
           "local MENU.BPK PALT words retain their authenticated byte order");

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
        Nexus_V1_BpkEntry bounded_entry;
        if (nexus_v1_bpk_archive_inspect_prs3(data, size, i, &prs3) != 0) {
            continue;
        }
        if (!prs3.has_prs3) continue;
        ++prs3_seen;
        if (prs3.pixel_count_matches) ++pix_matches;
        expect(nexus_v1_bpk_archive_get_entry(data, size, i,
                                              &bounded_entry) == 0,
               "local MENU.BPK PRS3 entry span is readable");
        expect(!prs3.payload_available ||
                   prs3.compressed_size <= bounded_entry.payload_size,
               "local MENU.BPK PRS3 payload stays inside its entry span");

        /* DMWeb DMNDataFileDecoder.vbs::DecodePRS3: verify the actual
         * retail stream against its declared width*height output. This is
         * a decoder regression only; Saturn VDP1/VDP2 upload remains a
         * separate authenticated-capture gate in the runtime. */
        {
            Nexus_V1_BpkEntryPrefix surface_prefix;
            size_t output_size;
            uint8_t *decoded;
            size_t written = 0U;
            Nexus_V1_BpkSurfaceEntry surface;

            expect(nexus_v1_bpk_archive_get_entry_prefix(
                       data, size, i, &surface_prefix) == 0,
                   "local MENU.BPK PRS3 prefix available for decode");
            output_size = (size_t)surface_prefix.width *
                          (size_t)surface_prefix.height;
            decoded = (uint8_t *)malloc(output_size);
            expect(decoded != NULL && output_size > 0U,
                   "local MENU.BPK PRS3 decode buffer allocated");
            if (decoded && output_size > 0U &&
                nexus_v1_bpk_archive_decode_surface(
                    data, size, i, decoded, output_size, &surface,
                    &written) == NEXUS_V1_BPK_DECODE_OK &&
                written == output_size &&
                surface.pixel_count == output_size &&
                surface.layout.bpp == 1U &&
                surface.layout.rowstride == surface.width &&
                surface.layout.surface_bytes == output_size &&
                surface.layout.surface_class ==
                    NEXUS_V1_BPK_SURFACE_INDEXED_8BPP) {
                ++decoded_seen;
                decoded_pixels += (uint64_t)written;
            } else {
                ++decoded_failures;
            }
            free(decoded);
        }
    }
    expect(prs3_seen == 162U,
           "local MENU.BPK inspected 162/162 PRS3-bearing entries");
    expect(pix_matches == 162U,
           "local MENU.BPK every PRS3 entry has width*height == pixel count");
    expect(decoded_seen == 162U,
           "local MENU.BPK every PRS3 entry decodes to its declared pixels");
    expect(decoded_failures == 0U,
           "local MENU.BPK has no retail PRS3 decode failures");
    expect(decoded_pixels > 0U,
           "local MENU.BPK PRS3 decode census emitted real pixels");

    expect(nexus_v1_bpk_archive_runtime_decode_receipt(
               data, size, &decoded_receipt) == 0,
           "local MENU.BPK runtime PRS3 receipt is bounded");
    expect(decoded_receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED &&
               decoded_receipt.prs3_decoder_promoted &&
               decoded_receipt.prs3_decode_successes == 162U &&
               decoded_receipt.prs3_decode_failures == 0U &&
               decoded_receipt.prs3_decoded_pixels_fnv1a64 ==
                   UINT64_C(0xb60830cf2f601003),
           "Japanese retail MENU.BPK PRS3 bytes retain their exact decoded receipt");

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
    test_exact_prefix_raw_entry();
    test_prefix_prs3_rejections();
    test_optional_local_menu_bpk();

    if (g_failures) return 1;
    puts("test_nexus_v1_bpk_archive: PASS");
    return 0;
}
