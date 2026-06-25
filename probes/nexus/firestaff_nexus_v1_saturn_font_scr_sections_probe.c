/*
 * firestaff_nexus_v1_saturn_font_scr_sections_probe.c
 * =====================================================
 *
 * Nexus V1 Saturn-font SEGA SATURN SCR section-table probe.
 *
 * Exercises the asset-backed `nexus_v1_font_load_sections()` parser in
 * src/nexus/nexus_v1_saturn_font.c against a small synthetic fixture and
 * the verified local FONT256.S2D (25,012 bytes). The SCR section table
 * is what the existing flat 1bpp loader skipped, so this probe is the
 * bounded next step that proves we can walk the real on-disk layout
 * without committing asset bytes or claiming full text-layout parity.
 *
 * What this probe locks (data-free):
 *   - nexus_v1_font_load_sections() rejects NULL inputs
 *   - rejects too-small buffers (header + table must fit)
 *   - rejects invalid magic
 *   - populates section_count with the count of non-empty entries
 *   - skips reserved/zero entries and preserves original table index
 *   - bounds-checks every section's [offset, offset+size) window
 *   - rejects a section that overlaps the section table itself
 *   - rejects a section window that escapes the file size
 *   - nexus_v1_font_section_in_bounds() returns 1/0/-1 correctly
 *   - nexus_v1_font_section_count(), nexus_v1_font_get_section(),
 *     and nexus_v1_font_section_table_index() round-trip parsed data
 *
 * What this probe locks (real FONT256.S2D, optional, skipped if absent):
 *   - real 25,012-byte asset walks to exactly four populated sections
 *     (indices 0, 2, 4, 6; the remaining 28 entries are reserved)
 *   - real section windows are well-formed and stay inside the file
 *   - section offsets form a contiguous non-overlapping partition that
 *     covers the entire payload after the section table
 *
 * Source-lock:
 *   - SEGA SATURN SCR format (16-byte "SEGA SATURN SCR\0" header)
 *   - src/nexus/nexus_v1_saturn_font.c (load_sections + helpers)
 *   - include/nexus_v1_saturn_font.h (section-table constants and
 *     Nexus_V1_FontSections / Nexus_V1_FontSection public types)
 *
 * Run:
 *   ./build/firestaff_nexus_v1_saturn_font_scr_sections_probe
 *
 * This probe does NOT claim full text layout/render parity; it proves
 * only that the bounded section-table parser and the local FONT256.S2D
 * asset-backed handoff agree on layout. Calling the S2D gap FIXED still
 * requires runtime text layout + an actual Nexus screen capture using
 * the real font.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nexus_v1_saturn_font.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }      \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }      \
} while (0)

static uint8_t *read_entire_file(const char *path, long *out_size) {
    FILE *fp;
    long size;
    uint8_t *data;

    if (out_size) *out_size = 0;
    if (!path) return NULL;

    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }

    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(fp);
        return NULL;
    }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    if (out_size) *out_size = size;
    return data;
}

/* Build a 32-byte SEGA SATURN SCR header followed by N section-table
 * entries (each 8 bytes: offset u32 BE + size u32 BE), then optional
 * section data. The caller specifies which table indices are populated
 * via `populate[i]`. Reserved/zero entries get offset=0, size=0. */
static uint8_t *build_scr(const uint32_t *offsets,
                          const uint32_t *sizes,
                          int *out_size) {
    /* 32-byte header + 32 * 8 = 256 bytes section table = 288 bytes
     * minimum. Section data starts at offset 0x120. */
    const int header = 32;
    const int table = 32 * 8;
    const int data_offset = header + table;
    int data_total = 0;
    int i;
    uint8_t *buf;
    int cur;

    /* Compute total section-data size first so we can allocate. */
    for (i = 0; i < 32; ++i) {
        if (sizes[i] == 0 && offsets[i] == 0) continue;
        /* Caller may pass sizes that sum to anything; we just append. */
        data_total += (int)sizes[i];
    }

    buf = (uint8_t *)calloc(1, (size_t)(data_offset + data_total));
    if (!buf) {
        if (out_size) *out_size = 0;
        return NULL;
    }

    memcpy(buf, "SEGA SATURN SCR", 15);
    /* char_count u32 BE at offset 0x10 (low 16 bits = 256). */
    buf[0x10] = 0; buf[0x11] = 0;
    buf[0x12] = 0x01; buf[0x13] = 0x00;
    /* header descriptor u32 BE at offset 0x14 (0x12 for FONT256). */
    buf[0x14] = 0; buf[0x15] = 0; buf[0x16] = 0; buf[0x17] = 0x12;

    cur = data_offset;
    for (i = 0; i < 32; ++i) {
        uint8_t *entry = buf + header + i * 8;
        uint32_t off = offsets[i];
        uint32_t sz = sizes[i];
        if (off == 0 && sz == 0) continue;
        /* Wire absolute offsets/sizes into the section table. */
        entry[0] = (uint8_t)((off >> 24) & 0xFF);
        entry[1] = (uint8_t)((off >> 16) & 0xFF);
        entry[2] = (uint8_t)((off >> 8) & 0xFF);
        entry[3] = (uint8_t)(off & 0xFF);
        entry[4] = (uint8_t)((sz >> 24) & 0xFF);
        entry[5] = (uint8_t)((sz >> 16) & 0xFF);
        entry[6] = (uint8_t)((sz >> 8) & 0xFF);
        entry[7] = (uint8_t)(sz & 0xFF);
        /* Touch section-data bytes so the buffer is not all zero. */
        if (sz > 0 && cur + (int)sz <= data_offset + data_total) {
            memset(buf + cur, (uint8_t)(i & 0xFF), sz);
            cur += (int)sz;
        }
    }

    if (out_size) *out_size = data_offset + data_total;
    return buf;
}

static void run_synthetic_section_table_gate(void) {
    uint32_t offsets[32] = {0};
    uint32_t sizes[32] = {0};
    uint8_t *scr;
    int scr_size = 0;
    Nexus_V1_FontSections sections;
    int rc;
    const Nexus_V1_FontSection *s;

    printf("\n-- synthetic SCR section-table gate --\n");

    /* 1. NULL inputs rejected. */
    rc = nexus_v1_font_load_sections(NULL, 1024, &sections);
    CHECK(rc == -1, "load_sections(NULL data, ...) rejected");
    scr = build_scr(offsets, sizes, &scr_size);
    rc = nexus_v1_font_load_sections(scr, scr_size, NULL);
    CHECK(rc == -1, "load_sections(..., NULL out) rejected");
    free(scr);

    /* 2. Too-small buffer rejected (header alone is 32 bytes; parser
     * also requires the 256-byte section table to fit). */
    {
        uint8_t tiny[64] = {0};
        memcpy(tiny, "SEGA SATURN SCR", 15);
        rc = nexus_v1_font_load_sections(tiny, 64, &sections);
        CHECK(rc == -1, "load_sections rejects too-small buffer");
    }

    /* 3. Invalid magic rejected even when the buffer is large enough. */
    {
        uint8_t *fake = (uint8_t *)calloc(1, 512);
        memcpy(fake, "NOT_A_SATURN_FNT", 15);
        rc = nexus_v1_font_load_sections(fake, 512, &sections);
        CHECK(rc == -1, "load_sections rejects invalid magic");
        free(fake);
    }

    /* 4. Empty section table: all reserved/zero. */
    memset(offsets, 0, sizeof(offsets));
    memset(sizes, 0, sizeof(sizes));
    scr = build_scr(offsets, sizes, &scr_size);
    rc = nexus_v1_font_load_sections(scr, scr_size, &sections);
    CHECK(rc == 0, "all-reserved section table parses successfully");
    CHECK(sections.section_count == 0,
          "all-reserved section table reports zero populated entries");
    CHECK(sections.char_count == 256,
          "header char_count parsed as 256");
    CHECK(sections.header_descriptor == 0x12u,
          "header descriptor parsed as 0x12");
    CHECK(nexus_v1_font_section_count(&sections) == 0,
          "section_count() reports 0 for empty table");
    s = nexus_v1_font_get_section(&sections, 0);
    CHECK(s == NULL, "get_section(0) is NULL on empty table");
    CHECK(nexus_v1_font_section_table_index(&sections, 0) == -1,
          "section_table_index(0) is -1 on empty table");
    free(scr);

    /* 5. One populated section at index 3. */
    memset(offsets, 0, sizeof(offsets));
    memset(sizes, 0, sizeof(sizes));
    offsets[3] = 0x120;
    sizes[3] = 0x10;
    scr = build_scr(offsets, sizes, &scr_size);
    rc = nexus_v1_font_load_sections(scr, scr_size, &sections);
    CHECK(rc == 0, "single-populated section table parses");
    CHECK(sections.section_count == 1,
          "single populated entry reports section_count = 1");
    CHECK(nexus_v1_font_section_table_index(&sections, 0) == 3,
          "preserves original table index 3 across skip");
    s = nexus_v1_font_get_section(&sections, 0);
    CHECK(s != NULL && s->file_offset == 0x120u && s->size == 0x10u,
          "section window matches (offset=0x120, size=0x10)");
    CHECK(nexus_v1_font_section_in_bounds(s, scr_size) == 1,
          "section window is in bounds for fixture");
    CHECK(nexus_v1_font_get_section(&sections, 1) == NULL,
          "get_section(1) is NULL when only one entry is populated");
    CHECK(nexus_v1_font_get_section(&sections, -1) == NULL,
          "get_section(-1) is NULL");
    free(scr);

    /* 6. Section whose declared size exceeds its actual window is
     * skipped (parser stays bounded by skipping rather than
     * rejecting). Build a 288-byte SCR (header + table only, no
     * section data); a section at offset 0x120 with size 0x100
     * would extend past the 288-byte buffer. */
    {
        uint8_t *tiny = (uint8_t *)calloc(1, 288);
        memset(tiny, 0, 288);
        memcpy(tiny, "SEGA SATURN SCR", 15);
        tiny[0x12] = 0x01; tiny[0x13] = 0x00;
        tiny[0x17] = 0x12;
        /* table entry [0] at offset 0x20..0x27: offset=0x120, size=0x100 */
        tiny[0x20 + 0] = 0x00; tiny[0x20 + 1] = 0x01; tiny[0x20 + 2] = 0x20; tiny[0x20 + 3] = 0x00;
        tiny[0x20 + 4] = 0x00; tiny[0x20 + 5] = 0x01; tiny[0x20 + 6] = 0x00; tiny[0x20 + 7] = 0x00;
        rc = nexus_v1_font_load_sections(tiny, 288, &sections);
        CHECK(rc == 0,
              "out-of-bounds section entry is skipped, parser succeeds");
        CHECK(sections.section_count == 0,
              "out-of-bounds section is not reported as populated");
        free(tiny);
    }

    /* 7. Section whose window overlaps the section table itself is
     * skipped (parser protects against self-modifying reads but
     * stays bounded by skipping rather than rejecting). */
    {
        memset(offsets, 0, sizeof(offsets));
        memset(sizes, 0, sizeof(sizes));
        offsets[0] = 0x10;
        sizes[0] = 0x200;  /* covers header + table */
        scr = build_scr(offsets, sizes, &scr_size);
        rc = nexus_v1_font_load_sections(scr, scr_size, &sections);
        CHECK(rc == 0,
              "section overlapping the section table is skipped, not rejected");
        CHECK(sections.section_count == 0,
              "no populated sections when only overlap found");
        free(scr);
    }

    /* 8. Four populated sections at indices 0, 2, 4, 6 (the
     * FONT256.S2D layout). */
    {
        memset(offsets, 0, sizeof(offsets));
        memset(sizes, 0, sizeof(sizes));
        offsets[0] = 0x120; sizes[0] = 0x2010;
        offsets[2] = 0x2130; sizes[2] = 0x3c90;
        offsets[4] = 0x5dc0; sizes[4] = 0x0210;
        offsets[6] = 0x5fd0; sizes[6] = 0x01e4;
        scr = build_scr(offsets, sizes, &scr_size);
        rc = nexus_v1_font_load_sections(scr, scr_size, &sections);
        CHECK(rc == 0, "four-populated synthetic table parses");
        CHECK(sections.section_count == 4,
              "four-populated synthetic table reports section_count = 4");
        CHECK(nexus_v1_font_section_table_index(&sections, 0) == 0 &&
              nexus_v1_font_section_table_index(&sections, 1) == 2 &&
              nexus_v1_font_section_table_index(&sections, 2) == 4 &&
              nexus_v1_font_section_table_index(&sections, 3) == 6,
              "synthetic table preserves indices 0, 2, 4, 6");
        s = nexus_v1_font_get_section(&sections, 1);
        CHECK(s != NULL && s->file_offset == 0x2130u && s->size == 0x3c90u,
              "synthetic section [1] window matches (offset=0x2130, size=0x3c90)");
        CHECK(nexus_v1_font_section_in_bounds(s, scr_size) == 1,
              "synthetic section [1] is in bounds");
        CHECK(nexus_v1_font_get_section(&sections, 4) == NULL,
              "get_section(4) is NULL when only 4 entries are populated");
        free(scr);
    }

    /* 9. nexus_v1_font_section_in_bounds() edge cases. */
    {
        Nexus_V1_FontSection empty = {0};
        Nexus_V1_FontSection good  = {0};
        Nexus_V1_FontSection bad   = {0};
        CHECK(nexus_v1_font_section_in_bounds(NULL, 100) == -1,
              "in_bounds(NULL, ...) rejected");
        CHECK(nexus_v1_font_section_in_bounds(&good, 0) == -1,
              "in_bounds with non-positive file size rejected");
        empty.file_offset = 0;
        empty.size = 0;
        CHECK(nexus_v1_font_section_in_bounds(&empty, 100) == 0,
              "empty (offset=0, size=0) reports unpopulated");
        good.file_offset = 16;
        good.size = 32;
        CHECK(nexus_v1_font_section_in_bounds(&good, 64) == 1,
              "in-bounds 16+32<=64 returns 1");
        CHECK(nexus_v1_font_section_in_bounds(&good, 32) == -1,
              "out-of-bounds 16+32>32 returns -1");
        bad.file_offset = 0;
        bad.size = 1;
        CHECK(nexus_v1_font_section_in_bounds(&bad, 100) == 1,
              "offset=0, size=1 is a valid in-bounds window (file_size>=1)");
        bad.file_offset = 50;
        bad.size = 100;
        CHECK(nexus_v1_font_section_in_bounds(&bad, 100) == -1,
              "offset=50, size=100 escapes a 100-byte file");
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

    printf("\n-- optional real FONT256.S2D section-table gate --\n");
    data = read_entire_file(path, &size);
    if (!data) {
        printf("  SKIP: no local FONT256.S2D at %s\n",
               path ? path : "(unset)");
        return;
    }

    {
        Nexus_V1_FontSections sections;
        int rc;
        const Nexus_V1_FontSection *s0;
        const Nexus_V1_FontSection *s1;
        const Nexus_V1_FontSection *s2;
        const Nexus_V1_FontSection *s3;

        CHECK(size == 25012,
              "local FONT256.S2D matches verified 25,012-byte asset size");

        rc = nexus_v1_font_load_sections(data, (int)size, &sections);
        CHECK(rc == 0, "real FONT256.S2D section table parses");
        CHECK(sections.section_count == 4,
              "real FONT256.S2D reports four populated sections");
        CHECK(sections.char_count == 256,
              "real FONT256.S2D header char_count = 256");
        CHECK(sections.header_descriptor == 0x12u,
              "real FONT256.S2D header descriptor = 0x12");
        CHECK(nexus_v1_font_section_count(&sections) == 4,
              "section_count() agrees with parser for real asset");
        CHECK(nexus_v1_font_section_table_index(&sections, 0) == 0 &&
              nexus_v1_font_section_table_index(&sections, 1) == 2 &&
              nexus_v1_font_section_table_index(&sections, 2) == 4 &&
              nexus_v1_font_section_table_index(&sections, 3) == 6,
              "real asset populated entries sit at indices 0, 2, 4, 6");

        s0 = nexus_v1_font_get_section(&sections, 0);
        s1 = nexus_v1_font_get_section(&sections, 1);
        s2 = nexus_v1_font_get_section(&sections, 2);
        s3 = nexus_v1_font_get_section(&sections, 3);

        CHECK(s0 && s0->file_offset == 0x0120u && s0->size == 0x2010u,
              "real asset section [0] window = (0x0120, 0x2010)");
        CHECK(s1 && s1->file_offset == 0x2130u && s1->size == 0x3c90u,
              "real asset section [1] window = (0x2130, 0x3c90)");
        CHECK(s2 && s2->file_offset == 0x5dc0u && s2->size == 0x0210u,
              "real asset section [2] window = (0x5dc0, 0x0210)");
        CHECK(s3 && s3->file_offset == 0x5fd0u && s3->size == 0x01e4u,
              "real asset section [3] window = (0x5fd0, 0x01e4)");

        CHECK(s0 && nexus_v1_font_section_in_bounds(s0, (int)size) == 1,
              "real asset section [0] is in bounds");
        CHECK(s1 && nexus_v1_font_section_in_bounds(s1, (int)size) == 1,
              "real asset section [1] is in bounds");
        CHECK(s2 && nexus_v1_font_section_in_bounds(s2, (int)size) == 1,
              "real asset section [2] is in bounds");
        CHECK(s3 && nexus_v1_font_section_in_bounds(s3, (int)size) == 1,
              "real asset section [3] is in bounds");

        /* Sections must form a contiguous non-overlapping partition
         * that lands exactly at file_size when chained. */
        if (s0 && s1 && s2 && s3) {
            uint64_t chain_end;
            CHECK(s0->file_offset + s0->size == s1->file_offset,
                  "real asset sections [0]->[1] chain is contiguous");
            CHECK(s1->file_offset + s1->size == s2->file_offset,
                  "real asset sections [1]->[2] chain is contiguous");
            CHECK(s2->file_offset + s2->size == s3->file_offset,
                  "real asset sections [2]->[3] chain is contiguous");
            chain_end = (uint64_t)s3->file_offset + (uint64_t)s3->size;
            CHECK(chain_end <= (uint64_t)size,
                  "real asset section chain ends inside file");
        }

        CHECK(nexus_v1_font_get_section(&sections, 4) == NULL,
              "get_section(4) is NULL on real four-section asset");
    }

    free(data);
}

int main(void) {
    printf("=== Nexus V1 Saturn-font SCR section-table probe ===\n");

    run_synthetic_section_table_gate();
    run_optional_real_asset_gate();

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
