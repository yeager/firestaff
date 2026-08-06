/*
 * firestaff_nexus_v1_s2d_glyph_decode_probe.c
 * ===========================================
 *
 * Probe for the Nexus V1 S2D per-glyph byte-window decoder API
 * declared in include/nexus_v1_s2d_glyph_decode.h.
 *
 * Scope (deliberately bounded, data-free by default):
 *
 *   [1]  NULL-safety: build + lookup + decode reject NULL inputs.
 *   [2]  Empty section table yields an empty byte map.
 *   [3]  Single-populated-section fixture: every glyph index maps
 *        into section [0] at `local_offset = glyph * bytes_per_glyph`,
 *        `file_offset = section_offset + local_offset`, and
 *        `size_bytes == bytes_per_glyph`.
 *   [4]  Four-populated synthetic SCR: every glyph index still maps
 *        into section [0] (because section [0] is the only one that
 *        can carry bytes for chars 0..255 under the existing 4-populated
 *        synthetic fixture), and the byte windows form a contiguous
 *        partition inside section [0].
 *   [5]  Header cap: a 1-section fixture whose header char_count is
 *        smaller than the section byte budget caps the window count
 *        at the header value.
 *   [6]  decode copies exactly `bytes_per_glyph` bytes out of the
 *        SCR file at the documented `file_offset`; reading char 0
 *        yields the first bytes of section [0]'s data region.
 *   [7]  decode rejects an out-of-coverage char_index.
 *   [8]  decode rejects a too-small destination buffer.
 *   [9]  section_byte_count sums the bytes that land in a given
 *        parsed-section index for isolated fixture layouts.
 *  [10]  Determinism: building the same byte map twice yields the
 *        same `bytes_total`, the same per-window byte content, and
 *        the same FNV-1a hash of the decode output.
 *  [11]  Real FONT256.S2D optional path: the verified 25,012-byte
 *        asset is decoded through its five named regions and the
 *        character-generator region exposes exactly 242 real 8x8
 *        tiles. No 256-slot or section-0 glyph map is inferred.
 *
 * Non-claim:
 *   This probe does NOT prove full Saturn SCR glyph parity. Its byte-window
 *   API remains fixture-only; the real branch locks named S2D regions and
 *   the 242-tile source handoff. Capturing an actual Nexus screen and
 *   binding character codes to page/attribute words remain separate gaps.
 *
 * Run:
 *   ./build/firestaff_nexus_v1_s2d_glyph_decode_probe
 *
 * CTest:
 *   ctest --test-dir build -R nexus_v1_s2d_glyph_decode --output-on-failure
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nexus_v1_s2d_glyph_decode.h"
#include "nexus_v1_s2d_text_layout.h"
#include "nexus_v1_saturn_font.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                          \
    if (cond) { fprintf(stderr, "  PASS: %s\n", msg); ++g_pass; }      \
    else      { fprintf(stderr, "  FAIL: %s\n", msg); ++g_fail; }      \
} while (0)

static uint64_t fnv1a64(const uint8_t *data, size_t size) {
    uint64_t h = UINT64_C(0xcbf29ce484222325);
    size_t i;
    for (i = 0; i < size; ++i) {
        h ^= data[i];
        h *= UINT64_C(0x100000001b3);
    }
    return h;
}

static uint8_t *read_entire_file(const char *path, long *out_size) {
    FILE *fp;
    long size;
    uint8_t *data;

    if (out_size) *out_size = 0;
    if (!path) return NULL;

    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return NULL; }
    size = ftell(fp);
    if (size <= 0) { fclose(fp); return NULL; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return NULL; }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) { fclose(fp); return NULL; }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data); fclose(fp); return NULL;
    }
    fclose(fp);
    if (out_size) *out_size = size;
    return data;
}

/* Build a synthetic SCR with a 32-byte header + 256-byte section
 * table + section data, matching the SEGA SATURN SCR layout. The
 * caller provides populated offsets[] / sizes[] (one entry per
 * table index, in [0,32)) and the header char_count. */
static uint8_t *build_scr(const uint32_t *offsets,
                          const uint32_t *sizes,
                          int header_char_count,
                          int *out_size)
{
    const int header = 32;
    const int table  = 32 * 8;
    const int data_offset = header + table;
    int data_total = 0;
    int i;
    uint8_t *buf;
    int cur;

    for (i = 0; i < 32; ++i) {
        if (sizes[i] == 0 && offsets[i] == 0) continue;
        data_total += (int)sizes[i];
    }

    buf = (uint8_t *)calloc(1, (size_t)(data_offset + data_total));
    if (!buf) {
        if (out_size) *out_size = 0;
        return NULL;
    }

    memcpy(buf, "SEGA SATURN SCR", 15);
    buf[0x10] = 0; buf[0x11] = 0;
    buf[0x12] = (uint8_t)((header_char_count >> 8) & 0xFF);
    buf[0x13] = (uint8_t)(header_char_count & 0xFF);
    buf[0x14] = 0; buf[0x15] = 0; buf[0x16] = 0; buf[0x17] = 0x12;

    cur = data_offset;
    for (i = 0; i < 32; ++i) {
        uint8_t *entry = buf + header + i * 8;
        uint32_t off = offsets[i];
        uint32_t sz  = sizes[i];
        if (off == 0 && sz == 0) continue;
        entry[0] = (uint8_t)((off >> 24) & 0xFF);
        entry[1] = (uint8_t)((off >> 16) & 0xFF);
        entry[2] = (uint8_t)((off >> 8) & 0xFF);
        entry[3] = (uint8_t)(off & 0xFF);
        entry[4] = (uint8_t)((sz >> 24) & 0xFF);
        entry[5] = (uint8_t)((sz >> 16) & 0xFF);
        entry[6] = (uint8_t)((sz >> 8) & 0xFF);
        entry[7] = (uint8_t)(sz & 0xFF);
        if (sz > 0 && cur + (int)sz <= data_offset + data_total) {
            memset(buf + cur, (uint8_t)(0x40 + (i & 0x3F)), sz);
            cur += (int)sz;
        }
    }

    if (out_size) *out_size = data_offset + data_total;
    return buf;
}

static void run_map_only_gate(void) {
    uint32_t offsets[32] = {0};
    uint32_t sizes[32]   = {0};
    uint8_t *scr;
    int scr_size = 0;
    Nexus_V1_FontSections sections;
    Nexus_V1_S2D_SectionGlyphMap range_map;
    Nexus_V1_S2D_GlyphByteMap byte_map;
    int rc;
    int i;

    fprintf(stderr, "\n-- synthetic section→glyph-byte-window map gate --\n");

    /* [1] NULL-safety. */
    rc = nexus_v1_s2d_glyph_byte_map_build(&sections, NULL, &byte_map);
    CHECK(rc == -1, "byte_map_build(NULL range_map, ...) rejected");

    rc = nexus_v1_s2d_glyph_byte_map_build(NULL, &range_map, &byte_map);
    CHECK(rc == -1, "byte_map_build(NULL sections, ...) rejected");

    rc = nexus_v1_s2d_glyph_byte_map_build(&sections, &range_map, NULL);
    CHECK(rc == -1, "byte_map_build(NULL out, ...) rejected");

    /* lookup + decode NULL-safety */
    CHECK(nexus_v1_s2d_glyph_byte_window_lookup(NULL, 0) == NULL,
          "byte_window_lookup(NULL map, ...) returns NULL");
    CHECK(nexus_v1_s2d_glyph_byte_decode(NULL, 100, &byte_map, 0, NULL, 32) == -1,
          "byte_decode(NULL data, ...) rejected");

    /* [2] Empty section table. Build an empty SCR (no populated
     * sections, header char_count=256) so `nexus_v1_font_load_sections`
     * produces an empty range_map and `glyph_byte_map_build` walks
     * zero ranges. */
    memset(offsets, 0, sizeof(offsets));
    memset(sizes, 0, sizeof(sizes));
    scr = build_scr(offsets, sizes, 256, &scr_size);
    rc = nexus_v1_font_load_sections(scr, scr_size, &sections);
    CHECK(rc == 0, "empty-section-table SCR parses");
    rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &range_map);
    CHECK(rc == 0, "empty-section-table range_map builds");
    rc = nexus_v1_s2d_glyph_byte_map_build(&sections, &range_map, &byte_map);
    CHECK(rc == 0, "empty-section-table byte_map builds");
    CHECK(byte_map.window_count == 0, "empty-section-table byte_map has 0 windows");
    CHECK(byte_map.char_count == 0, "empty-section-table byte_map.char_count == 0");
    CHECK(byte_map.bytes_total == 0, "empty-section-table byte_map.bytes_total == 0");
    CHECK(nexus_v1_s2d_glyph_byte_map_total_bytes(&byte_map) == 0,
          "empty byte_map total_bytes == 0");
    free(scr);

    /* [3] Single-populated-section fixture: section [5] holds
     * 64 * 32 = 2048 bytes covering 64 glyphs. */
    memset(offsets, 0, sizeof(offsets));
    memset(sizes, 0, sizeof(sizes));
    offsets[5] = 0x120;
    sizes[5] = 32 * 64;
    scr = build_scr(offsets, sizes, 256, &scr_size);
    rc = nexus_v1_font_load_sections(scr, scr_size, &sections);
    CHECK(rc == 0, "single-populated-section SCR parses");
    rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &range_map);
    CHECK(rc == 0, "single-populated-section range_map builds");
    rc = nexus_v1_s2d_glyph_byte_map_build(&sections, &range_map, &byte_map);
    CHECK(rc == 0, "single-populated-section byte_map builds");
    CHECK(byte_map.window_count == 64,
          "single-section byte_map has 64 windows");
    CHECK(byte_map.char_count == 64,
          "single-section byte_map.char_count == 64");
    CHECK(byte_map.bytes_total == 64 * 32,
          "single-section byte_map.bytes_total == 2048");
    CHECK(byte_map.windows[0].parsed_section_index == 0,
          "single-section window 0 parsed_section_index == 0");
    CHECK(byte_map.windows[0].table_index == 5,
          "single-section window 0 table_index == 5 (preserved)");
    CHECK(byte_map.windows[0].file_offset == 0x120,
          "single-section window 0 file_offset == 0x120");
    CHECK(byte_map.windows[0].local_offset == 0,
          "single-section window 0 local_offset == 0");
    CHECK(byte_map.windows[0].size_bytes == 32,
          "single-section window 0 size_bytes == 32");
    CHECK(byte_map.windows[0].char_index == 0,
          "single-section window 0 char_index == 0");
    CHECK(byte_map.windows[63].file_offset == 0x120 + 63 * 32,
          "single-section window 63 file_offset == section + 63*32");
    CHECK(byte_map.windows[63].local_offset == 63 * 32,
          "single-section window 63 local_offset == 63*32");
    CHECK(byte_map.windows[63].char_index == 63,
          "single-section window 63 char_index == 63");

    /* A forged range map must not create a byte window beyond the parsed
     * section or redirect it to a different section. */
    {
        Nexus_V1_S2D_SectionGlyphMap forged = range_map;
        forged.ranges[0].char_count++;
        CHECK(nexus_v1_s2d_glyph_byte_map_build(
                  &sections, &forged, &byte_map) == -1,
              "forged range beyond section is rejected");
        forged = range_map;
        forged.ranges[0].table_index = 6;
        CHECK(nexus_v1_s2d_glyph_byte_map_build(
                  &sections, &forged, &byte_map) == -1,
              "forged range redirect is rejected");
        CHECK(nexus_v1_s2d_glyph_byte_map_build(
                  &sections, &range_map, &byte_map) == 0,
              "original section range remains accepted");
    }

    /* Out-of-coverage lookup returns NULL. */
    CHECK(nexus_v1_s2d_glyph_byte_window_lookup(&byte_map, 64) == NULL,
          "lookup char 64 returns NULL (out of coverage)");
    CHECK(nexus_v1_s2d_glyph_byte_window_lookup(&byte_map, -1) == NULL,
          "lookup char -1 returns NULL");
    CHECK(nexus_v1_s2d_glyph_byte_window_lookup(&byte_map, 256) == NULL,
          "lookup char 256 returns NULL");

    /* [6] decode char 0 copies exactly 32 bytes from offset 0x120. */
    {
        uint8_t out[64];
        int copied = nexus_v1_s2d_glyph_byte_decode(
            scr, scr_size, &byte_map, 0, out, sizeof(out));
        CHECK(copied == 32, "decode char 0 returns 32 bytes copied");
        CHECK(memcmp(out, scr + 0x120, 32) == 0,
              "decode char 0 byte window matches SCR bytes at 0x120");
    }

    /* [7] decode rejects out-of-coverage char_index. */
    {
        uint8_t out[64];
        int copied = nexus_v1_s2d_glyph_byte_decode(
            scr, scr_size, &byte_map, 64, out, sizeof(out));
        CHECK(copied == -1, "decode char 64 rejected (out of coverage)");
        copied = nexus_v1_s2d_glyph_byte_decode(
            scr, scr_size, &byte_map, -1, out, sizeof(out));
        CHECK(copied == -1, "decode char -1 rejected");
    }

    /* [8] decode rejects too-small destination. */
    {
        uint8_t out[16];
        int copied = nexus_v1_s2d_glyph_byte_decode(
            scr, scr_size, &byte_map, 0, out, sizeof(out));
        CHECK(copied == -1, "decode rejects out_size=16 (too small for 32)");
    }

    /* [9] section_byte_count. */
    CHECK(nexus_v1_s2d_glyph_byte_map_section_byte_count(&byte_map, 0) == 64 * 32,
          "single-section section_byte_count(0) == 2048");
    CHECK(nexus_v1_s2d_glyph_byte_map_section_byte_count(&byte_map, 1) == 0,
          "single-section section_byte_count(1) == 0 (no other populated sections)");
    CHECK(nexus_v1_s2d_glyph_byte_map_section_byte_count(NULL, 0) == 0,
          "section_byte_count(NULL map, ...) == 0");

    /* [10] Determinism: rebuild the same byte_map and compare
     * byte_total + per-window offsets + an FNV-1a hash of all
     * decoded bytes (chars 0..63 in section order). */
    {
        Nexus_V1_S2D_GlyphByteMap byte_map_b;
        uint8_t decoded_a[64 * 32];
        uint8_t decoded_b[64 * 32];
        uint64_t hash_a, hash_b;

        rc = nexus_v1_s2d_glyph_byte_map_build(
            &sections, &range_map, &byte_map_b);
        CHECK(rc == 0, "second byte_map_build succeeds");
        CHECK(byte_map_b.bytes_total == byte_map.bytes_total,
              "deterministic bytes_total across two builds");
        CHECK(byte_map_b.window_count == byte_map.window_count,
              "deterministic window_count across two builds");
        for (i = 0; i < byte_map.window_count; ++i) {
            CHECK(byte_map_b.windows[i].file_offset ==
                  byte_map.windows[i].file_offset,
                  "deterministic per-window file_offset");
        }

        /* Decode all 64 glyphs through both maps and hash. */
        for (i = 0; i < 64; ++i) {
            int n;
            n = nexus_v1_s2d_glyph_byte_decode(
                scr, scr_size, &byte_map, i,
                decoded_a + i * 32, 32);
            CHECK(n == 32, "decode char i (first build) returns 32");
            n = nexus_v1_s2d_glyph_byte_decode(
                scr, scr_size, &byte_map_b, i,
                decoded_b + i * 32, 32);
            CHECK(n == 32, "decode char i (second build) returns 32");
        }
        hash_a = fnv1a64(decoded_a, sizeof(decoded_a));
        hash_b = fnv1a64(decoded_b, sizeof(decoded_b));
        CHECK(hash_a == hash_b,
              "deterministic FNV-1a hash of decoded bytes across two builds");
        CHECK(hash_a != UINT64_C(0xcbf29ce484222325),
              "non-empty decoded byte region differs from FNV-1a iv");
    }

    free(scr);

    /* [5] Header cap: a 1-section fixture with header char_count=32
     * and 64-glyph section byte budget must produce 32 windows. */
    {
        memset(offsets, 0, sizeof(offsets));
        memset(sizes, 0, sizeof(sizes));
        offsets[0] = 0x120;
        sizes[0] = 32 * 64;
        scr = build_scr(offsets, sizes, 32 /* header cap */, &scr_size);
        rc = nexus_v1_font_load_sections(scr, scr_size, &sections);
        CHECK(rc == 0, "header-capped SCR parses");
        rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &range_map);
        CHECK(rc == 0, "header-capped range_map builds");
        rc = nexus_v1_s2d_glyph_byte_map_build(&sections, &range_map, &byte_map);
        CHECK(rc == 0, "header-capped byte_map builds");
        CHECK(byte_map.window_count == 32,
              "header-capped byte_map.window_count == 32 (cap, not 64)");
        CHECK(byte_map.bytes_total == 32 * 32,
              "header-capped byte_map.bytes_total == 1024");
        free(scr);
    }
}

static const char *default_real_font_path(char *buf, size_t cap) {
    const char *env = getenv("FIRESTAFF_NEXUS_FONT256_S2D");
    const char *home;
    if (env && env[0]) return env;
    home = getenv("HOME");
    if (!home || !home[0] || cap == 0) return NULL;
    snprintf(buf, cap, "%s/.firestaff/data/nexus/FONT256.S2D", home);
    return buf;
}

static void run_optional_real_asset_gate(void) {
    char path_buf[1024];
    const char *path = default_real_font_path(path_buf, sizeof(path_buf));
    long size = 0;
    uint8_t *data;

    fprintf(stderr, "\n-- optional real FONT256.S2D section-bound glyph-decode gate --\n");

    data = read_entire_file(path, &size);
    if (!data) {
        fprintf(stderr, "  SKIP: no local FONT256.S2D at %s\n",
               path ? path : "(unset)");
        return;
    }

    {
        Nexus_V1_FontS2dDecodeResult decoded;
        Nexus_V1_Font font;
        uint8_t tile[64];
        uint64_t hash_a = UINT64_C(0xcbf29ce484222325);
        uint64_t hash_b = UINT64_C(0xcbf29ce484222325);
        int rc;
        int i;

        memset(&decoded, 0, sizeof(decoded));
        memset(&font, 0, sizeof(font));
        rc = nexus_v1_font_s2d_decode(data, (int)size, &decoded);
        CHECK(rc == 1 && decoded.valid,
              "real FONT256.S2D decodes through the S2D region parser");
        CHECK(decoded.section_count == 5,
              "real FONT256.S2D retains the five bounded SCR regions");
        CHECK(decoded.character_generator_offset == 0x2130U &&
              decoded.character_generator_size == 0x3c90U,
              "real FONT256.S2D character-generator region is 0x2130/0x3c90");
        CHECK(decoded.palette_offset == 0x5dc0U &&
              decoded.palette_size == 0x210U,
              "real FONT256.S2D palette region is 0x5dc0/0x210");
        CHECK(nexus_v1_font_s2d_copy_character_generator_tile(
                  data, (int)size, &decoded, 0, tile) == 0 &&
              nexus_v1_font_s2d_copy_character_generator_tile(
                  data, (int)size, &decoded, 241, tile) == 0 &&
              nexus_v1_font_s2d_copy_character_generator_tile(
                  data, (int)size, &decoded, 242, tile) != 0,
              "real FONT256.S2D exposes exactly 242 bounded CG tiles");
        rc = nexus_v1_font_load_from_s2d(&font, data, (int)size, &decoded);
        CHECK(rc == NEXUS_V1_FONT_S2D_REAL_TILE_COUNT,
              "real FONT256.S2D source handoff loads exactly 242 CG tiles");
        for (i = 0; i < NEXUS_V1_FONT_S2D_REAL_TILE_COUNT; ++i) {
            const uint8_t *glyph = nexus_v1_font_get_glyph(&font, i);
            int j;
            if (!glyph) {
                CHECK(0, "real FONT256.S2D CG tile lookup stays bounded");
                break;
            }
            for (j = 0; j < 64; ++j) {
                hash_a ^= glyph[j];
                hash_a *= UINT64_C(0x100000001b3);
            }
        }
        for (i = 0; i < NEXUS_V1_FONT_S2D_REAL_TILE_COUNT; ++i) {
            const uint8_t *glyph = nexus_v1_font_get_glyph(&font, i);
            int j;
            for (j = 0; glyph && j < 64; ++j) {
                hash_b ^= glyph[j];
                hash_b *= UINT64_C(0x100000001b3);
            }
        }
        CHECK(hash_a == hash_b,
              "real FONT256.S2D CG tile bytes are deterministic");
        nexus_v1_font_free(&font);
        free(data);
    }
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    fprintf(stderr, "=== Nexus V1 S2D glyph-byte-window decode probe ===\n");

    run_map_only_gate();
    run_optional_real_asset_gate();

    fprintf(stderr, "\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
