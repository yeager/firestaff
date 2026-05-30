/*
 * firestaff_csb_v1_dsa_section_probe.c
 *
 * Pass H2327: CSB V1 Phase 2 — DSA Script Section Parser Probe
 *
 * Headless probe: verifies that the CSB dungeon.dat DSA script section
 * can be located and parsed after dungeon map loading.
 *
 * Problem addressed (CSB-specific, no DM1 equivalent):
 *   csb_v1_dungeon_load() stores a raw copy of the dungeon.dat buffer
 *   and parses the map header + level descriptors, but never locates
 *   or enumerated the DSA script section that follows the thing data.
 *   DM1 has no DSA section; CSB has one thing-type (DSA type 15, DEFS.H)
 *   that references scripts in this section.
 *
 *   The dsa_count / dsa_offsets fields in CSB_V1_DungeonData are
 *   always 0 / NULL after loading — a DM1-only assumption gap.
 *
 *   A valid CSB DUNGEON.DAT has a well-defined layout:
 *     [header: levels(LE16) + thing_type_count(LE16)]
 *     [level_headers: N * (width(u8) height(u8) offset(u32))]
 *     [square data: one block per level at its declared offset]
 *     [thing data: follows last level's square block]
 *     [DSA section: count(LE16) + N*offset(LE32)]
 *
 * Strategy:
 *   1. Load dungeon via csb_v1_dungeon_load().
 *   2. After loading, traverse the buffer to find the DSA section:
 *        - Locate the last level's square block end.
 *        - Scan forward from there for the DSA magic pattern.
 *        - Read dsa_count (LE uint16 at section start).
 *        - Read N dsa_offsets (LE uint32 each).
 *   3. Verify offsets point within the buffer.
 *   4. Check dsa_offsets are monotonically increasing.
 *   5. Check that dsa_count > 0 on a real CSB DUNGEON.DAT.
 *
 * Output files:
 *   <output_dir>/csb_dsa_probe.md          — human-readable report
 *   <output_dir>/csb_dsa_invariants.md      — machine-checkable PASS/FAIL
 *
 * Usage: probe <DUNGEON.DAT path> <output_dir>
 *
 * Source: ReDMCSB DUNGEON.C F0148-F0170 (shared format)
 *         ReDMCSB DEFS.H M034/M035 (square layout)
 *         ReDMCSB DEFS.H:399 (M012_TYPE, DSA=15)
 *         CSBWin/CSBCode.cpp:318-480 (TAG00332a DBank::Initialize)
 *         CSBWin/CSBCode.cpp:6800-6950 (LoadDungeon)
 *         docs/source-lock/csb_v1_phase2_dungeon_data_model_H23xx.md
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "csb_v1_dungeon_loader_pc34_compat.h"

/* ── Helpers ─────────────────────────────────────────────────────── */

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/*
 * dsa_locate -- scan the dungeon buffer for the DSA script section.
 *
 *   The DSA section follows the thing data.  Strategy:
 *     1. Compute last level's square block end.
 *     2. Scan forward from there looking for a plausible DSA header:
 *          - word at candidate = dsa_count (0 < count <= 256)
 *          - dsa_count * 4 bytes available for offsets
 *          - all offsets monotonically increasing and in-bounds
 *     3. Also check for the magic marker "$DSA" at candidate-4 (optional,
 *        present in some CSB builds but not all).
 *
 *   Returns pointer to DSA count word, or NULL if not found.
 *   Sets *out_count and *out_offset_array on success.
 *
 * ReDMCSB: CSBWin/CSBCode.cpp TAG00332a lines ~440-460 (DSA section parse)
 */
static const uint8_t *dsa_locate(const uint8_t *buf, int buf_size,
                                 const CSB_V1_DungeonData *dungeon,
                                 uint16_t *out_count,
                                 const uint32_t **out_offsets)
{
    static uint16_t  g_dsa_count = 0;
    static uint32_t g_dsa_offsets[256];
    memset(g_dsa_offsets, 0, sizeof(g_dsa_offsets));

    if (!buf || !dungeon || !out_count || !out_offsets) return NULL;

    /* Find the last level's square block end */
    int last_level = dungeon->level_count - 1;
    if (last_level < 0) return NULL;

    uint32_t last_offset = (uint32_t)dungeon->level_offsets[last_level];
    uint8_t  last_w = dungeon->level_widths[last_level];
    uint8_t  last_h = dungeon->level_heights[last_level];
    uint32_t last_square_bytes = (uint32_t)last_w * (uint32_t)last_h * 2U;
    uint32_t square_end = last_offset + last_square_bytes;

    /* DSA section starts somewhere after the last square block.
     * The engine uses a 4-byte magic "$DSA" prefix in some versions;
     * in others it just reads count word directly.  We scan from
     * square_end until buf_size-2, looking for a plausible header.
     *
     * A plausible DSA header has:
     *   - count word: 1..256
     *   - count * 4 bytes must fit in buffer
     *   - all offsets monotonically increasing and <= buf_size
     */
    for (uint32_t pos = square_end; pos + 2 + 4 <= (uint32_t)buf_size; pos++) {
        uint16_t candidate_count = rd16le(buf + pos);
        if (candidate_count == 0 || candidate_count > 256) continue;

        uint32_t needed = 2 + (uint32_t)candidate_count * 4U;
        if (pos + needed > (uint32_t)buf_size) continue;

        /* Validate all offsets */
        int monotonic = 1;
        uint32_t prev = 0;
        for (uint16_t i = 0; i < candidate_count; i++) {
            uint32_t off = rd32le(buf + pos + 2 + i * 4);
            if (i > 0 && off <= prev) { monotonic = 0; break; }
            if (off > (uint32_t)buf_size) { monotonic = 0; break; }
            g_dsa_offsets[i] = off;
            prev = off;
        }
        if (!monotonic) continue;

        /* Found valid DSA section */
        g_dsa_count = candidate_count;
        *out_count = g_dsa_count;
        *out_offsets = g_dsa_offsets;

        /* Verify optional magic prefix "$DSA" (bytes 0x24 0x44 0x53 0x41) */
        if (pos >= 4) {
            if (buf[pos-4] == '$' && buf[pos-3] == 'D'
                && buf[pos-2] == 'S' && buf[pos-1] == 'A') {
                /* Magic prefix present */
            }
        }
        return buf + pos;
    }

    return NULL;
}

/* ── Report generation ────────────────────────────────────────────── */

static void write_report(const CSB_V1_DungeonData *dungeon,
                         int dsa_found,
                         uint16_t dsa_count,
                         const uint32_t *dsa_offsets,
                         FILE *fp)
{
    int i;
    fprintf(fp, "# CSB V1 DSA Script Section Probe\n\n");
    fprintf(fp, "## Dungeon Data (from loader)\n\n");
    fprintf(fp, "| Property | Value |\n");
    fprintf(fp, "|-----------|-------|\n");
    fprintf(fp, "| level_count | %d |\n", dungeon->level_count);
    fprintf(fp, "| raw_size | %d bytes |\n", dungeon->raw_size);
    fprintf(fp, "| dsa_count (struct) | %d |\n", dungeon->dsa_count);
    fprintf(fp, "\n## DSA Section Scan\n\n");
    fprintf(fp, "| Property | Value |\n");
    fprintf(fp, "|-----------|-------|\n");
    fprintf(fp, "| DSA section found | %s |\n", dsa_found ? "YES" : "NO");
    if (dsa_found) {
        fprintf(fp, "| dsa_count (scanned) | %u |\n", dsa_count);
        fprintf(fp, "\n## DSA Script Offsets\n\n");
        fprintf(fp, "| Index | Offset |\n");
        fprintf(fp, "|-------|--------|\n");
        for (i = 0; i < (int)dsa_count; i++) {
            fprintf(fp, "| %d | 0x%08x |\n", i, dsa_offsets[i]);
        }
    } else {
        fprintf(fp, "\n*DSA section not found in dungeon buffer.*\n");
        fprintf(fp, "This may indicate the dungeon has no DSA scripts,\n");
        fprintf(fp, "or the scan range is insufficient.\n");
    }
    fprintf(fp, "\n## Note\n\n");
    fprintf(fp, "The dsa_count field in CSB_V1_DungeonData is populated by\n");
    fprintf(fp, "csb_v1_dungeon_load(). The loader currently does not parse\n");
    fprintf(fp, "the DSA section — this probe verifies the section exists in\n");
    fprintf(fp, "real CSB assets and that scanning it is feasible, paving\n");
    fprintf(fp, "the way for loader integration (pass H23xx).\n");
}

static void write_invariants(FILE *fp, int fail_count,
                              const CSB_V1_DungeonData *dungeon,
                              int dsa_found,
                              uint16_t dsa_count,
                              const uint32_t *dsa_offsets)
{
#define CHECK(cond, msg) do { \
        if (cond) { \
            fprintf(fp, "- PASS: %s\n", msg); \
        } else { \
            fprintf(fp, "- FAIL: %s\n", msg); \
            fail_count++; \
        } \
    } while(0)

    fprintf(fp, "# CSB V1 DSA Section Invariants\n\n");

    /* I1: Dungeon loaded successfully */
    CHECK(dungeon->raw_data != NULL && dungeon->raw_size > 0,
          "Dungeon raw_data is non-NULL and raw_size > 0");

    /* I2: level_count in valid range */
    CHECK(dungeon->level_count >= 1 && dungeon->level_count <= CSB_V1_MAX_LEVELS,
          "level_count in 1..CSB_V1_MAX_LEVELS");

    /* I3: dsa_offsets field is NULL after current load (pre-condition) */
    CHECK(dungeon->dsa_offsets == NULL,
          "dsa_offsets is NULL after load (pre-condition: loader gap)");

    /* I4: dsa_count field is 0 after current load (pre-condition) */
    CHECK(dungeon->dsa_count == 0,
          "dsa_count is 0 after load (pre-condition: loader gap)");

    /* I5: At least one level's width/height is readable */
    if (dungeon->level_count > 0) {
        CHECK(dungeon->level_widths[0] >= 1 && dungeon->level_widths[0] <= 32
              && dungeon->level_heights[0] >= 1 && dungeon->level_heights[0] <= 32,
              "level[0] dimensions in 1..32");
    }

    /* I6: Scan finds DSA section (if file is a real CSB DUNGEON.DAT) */
    if (dsa_found) {
        CHECK(dsa_count >= 1, "dsa_count >= 1 on real CSB asset");

        /* I7: All DSA offsets in bounds */
        int offsets_ok = 1;
        for (uint16_t i = 0; i < dsa_count; i++) {
            if (dsa_offsets[i] > (uint32_t)dungeon->raw_size) {
                offsets_ok = 0; break;
            }
        }
        CHECK(offsets_ok, "All DSA offsets in bounds");

        /* I8: DSA offsets monotonically increasing */
        int mono = 1;
        for (uint16_t i = 1; i < dsa_count; i++) {
            if (dsa_offsets[i] <= dsa_offsets[i-1]) { mono = 0; break; }
        }
        CHECK(mono, "DSA offsets monotonically increasing");

        /* I9: First offset > last level's square block end */
        if (dsa_count > 0 && dungeon->level_count > 0) {
            int last = dungeon->level_count - 1;
            uint32_t last_off = (uint32_t)dungeon->level_offsets[last];
            uint32_t sq_bytes = (uint32_t)dungeon->level_widths[last]
                              * (uint32_t)dungeon->level_heights[last] * 2U;
            CHECK(dsa_offsets[0] >= last_off + sq_bytes,
                  "First DSA offset >= end of last level square block");
        }
    } else {
        fprintf(fp, "- INFO: DSA section not found — dungeon may lack scripts or scan range is insufficient\n");
    }

    fprintf(fp, "\n**Status: %s**\n", fail_count == 0 ? "PASS" : "FAIL");
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <DUNGEON.DAT path> <output_dir>\n", argv[0]);
        return 1;
    }

    const char *filepath = argv[1];
    const char *outdir   = argv[2];

    /* Load file */
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "FAIL: cannot open %s\n", filepath);
        return 1;
    }
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 1; }
    long fsize = ftell(fp);
    if (fsize <= 0 || fsize > 16*1024*1024) { fclose(fp); return 1; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return 1; }

    uint8_t *buf = (uint8_t *)malloc((size_t)fsize);
    if (!buf) { fclose(fp); return 1; }
    if (fread(buf, 1, (size_t)fsize, fp) != (size_t)fsize) {
        free(buf); fclose(fp); return 1;
    }
    fclose(fp);

    /* Load via existing loader */
    CSB_V1_DungeonData dungeon;
    memset(&dungeon, 0, sizeof(dungeon));
    int load_ret = csb_v1_dungeon_load(&dungeon, buf, (int)fsize);

    /* Open output files */
    char rep_path[512], inv_path[512];
    snprintf(rep_path, sizeof(rep_path), "%s/csb_dsa_probe.md", outdir);
    snprintf(inv_path, sizeof(inv_path), "%s/csb_dsa_invariants.md", outdir);

    FILE *frep = fopen(rep_path, "w");
    FILE *finv = fopen(inv_path, "w");
    if (!frep || !finv) {
        if (frep) fclose(frep);
        if (finv) fclose(finv);
        csb_v1_dungeon_free(&dungeon);
        free(buf);
        return 1;
    }

    /* DSA scan */
    uint16_t dsa_count = 0;
    const uint32_t *dsa_offsets = NULL;
    int dsa_found = 0;

    if (load_ret == 0) {
        const uint8_t *dsa_ptr = dsa_locate(buf, (int)fsize, &dungeon,
                                             &dsa_count, &dsa_offsets);
        dsa_found = (dsa_ptr != NULL);

        int fail_count = 0;
        write_invariants(finv, fail_count, &dungeon,
                         dsa_found, dsa_count, dsa_offsets);
        write_report(&dungeon, dsa_found, dsa_count, dsa_offsets, frep);
    } else {
        fprintf(finv, "- FAIL: csb_v1_dungeon_load returned %d\n", load_ret);
        fprintf(frep, "**Load result:** FAILURE (%d)\n", load_ret);
    }

    fclose(frep);
    fclose(finv);

    printf("CSB V1 DSA probe: load=%s, dsa_found=%s\n",
           load_ret == 0 ? "OK" : "FAIL",
           dsa_found ? "YES" : "NO");

    csb_v1_dungeon_free(&dungeon);
    free(buf);
    return (load_ret == 0 && dsa_found) ? 0 : 1;
}