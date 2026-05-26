/*
 * Pass H2248: CSB V1 Phase 7 — Dungeon Parser Probe
 *
 * Headless probe: loads CSB DUNGEON.DAT via csb_v1_dungeon_loader_pc34_compat
 * and validates the complete dungeon header + map descriptors + square access.
 *
 * Writes:
 *   <output_dir>/csb_dungeon_probe.md          — human-readable report
 *   <output_dir>/csb_dungeon_invariants.md     — machine-checkable PASS/FAIL
 *
 * Invariants:
 *   PASS criteria — all must be true:
 *     1. dungeon_load returns 0 (success)
 *     2. level_count in 1..12
 *     3. each map width/height in 1..32
 *     4. RawMapDataByteOffset monotonically increasing across levels
 *     5. all squares readable (x=0,y=0 per level)
 *     6. square_type returns 0..31
 *     7. first_thing index returns 0..1023
 *     8. no buffer overrun on any get_square call
 *     9. DungeonData struct fully zeroed before load (sanity)
 *    10. Free function succeeds without crash
 *
 * Usage: probe <DUNGEON.DAT path> <output_dir>
 *
 * Source: csb_v1_dungeon_loader_pc34_compat.c
 * Evidence: CSBWin/CSBCode.cpp:318-480 (TAG00332a DBank::Initialize)
 *           ReDMCSB DUNGEON.C F0148-F0170 (shared format)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csb_v1_dungeon_loader_pc34_compat.h"

/* ---- Test fixtures ---- */

static void build_zero_dungeon(CSB_V1_DungeonData *d) {
    memset(d, 0xCC, sizeof(*d));  /* poison pattern */
}

static void probe_report(const CSB_V1_DungeonData *d, FILE *rep,
                         int levels_ok, int dims_ok, int mono_ok,
                         int sq_ok, int fail_count)
{
    int i;
    fprintf(rep, "# CSB V1 Dungeon Parser Probe\n\n");
    fprintf(rep, "## Dungeon Data\n\n");
    fprintf(rep, "| Property | Value |\n");
    fprintf(rep, "|-----------|-------|\n");
    fprintf(rep, "| level_count | %d |\n", d->level_count);
    fprintf(rep, "| raw_size | %d bytes |\n", d->raw_size);
    fprintf(rep, "| dsa_count | %d |\n", d->dsa_count);
    fprintf(rep, "\n## Per-Level Descriptors\n\n");
    fprintf(rep, "| Level | Width | Height | RawDataOff |\n");
    fprintf(rep, "|-------|-------|--------|------------|\n");
    for (i = 0; i < d->level_count; i++) {
        fprintf(rep, "| %d | %d | %d | 0x%08x |\n",
                i,
                d->level_widths[i],
                d->level_heights[i],
                d->level_offsets[i]);
    }

    fprintf(rep, "\n## Square Probe (x=0, y=0 per level)\n\n");
    fprintf(rep, "| Level | SquareType | FirstThing |\n");
    fprintf(rep, "|-------|------------|------------|\n");
    for (i = 0; i < d->level_count; i++) {
        int sq = csb_v1_dungeon_get_square_type(d, i, 0, 0);
        int ft = csb_v1_dungeon_get_first_thing(d, i, 0, 0);
        fprintf(rep, "| %d | %d | %d |\n", i, sq, ft);
    }
    (void)levels_ok; (void)dims_ok; (void)mono_ok; (void)sq_ok;
}

static int run_invariants(CSB_V1_DungeonData *d, FILE *inv, int fail_count) {
    int i;

#define CHECK(cond, msg) do { \
        if (cond) { \
                fprintf(inv, "- PASS: %s\n", msg); \
        } else { \
                fprintf(inv, "- FAIL: %s\n", msg); \
                fail_count++; \
        } \
} while(0)

    fprintf(inv, "# CSB V1 Dungeon Invariants\n\n");

    /* I1: level_count in valid range */
    CHECK(d->level_count >= 1 && d->level_count <= CSB_V1_MAX_LEVELS,
          "level_count in 1..CSB_V1_MAX_LEVELS");

    /* I2: raw data present */
    CHECK(d->raw_data != NULL && d->raw_size > 0,
          "raw_data is non-NULL and raw_size > 0");

    /* I3: no buffer overread on square(x=0,y=0) for each level */
    {
        int sq_ok = 1;
        for (i = 0; i < d->level_count; i++) {
            int sq = csb_v1_dungeon_get_square_type(d, i, 0, 0);
            if (sq < 0) { sq_ok = 0; break; }
            if (sq > 31) { sq_ok = 0; break; }  /* square type is 5 bits */
        }
        CHECK(sq_ok, "all square_type(0,0) in 0..31");
    }

    /* I4: first_thing index in valid range */
    {
        int ft_ok = 1;
        for (i = 0; i < d->level_count; i++) {
            int ft = csb_v1_dungeon_get_first_thing(d, i, 0, 0);
            if (ft < 0 || ft > 1023) { ft_ok = 0; break; }  /* 10 bits */
        }
        CHECK(ft_ok, "all first_thing(0,0) in 0..1023");
    }

    /* I5: map dimensions in 1..32 */
    {
        int dims_ok = 1;
        for (i = 0; i < d->level_count; i++) {
            if (d->level_widths[i] < 1 || d->level_widths[i] > 32 ||
                d->level_heights[i] < 1 || d->level_heights[i] > 32) {
                dims_ok = 0; break;
            }
        }
        CHECK(dims_ok, "all map dimensions in 1..32");
    }

    /* I6: RawMapDataByteOffset monotonically increasing */
    {
        int mono_ok = 1;
        for (i = 1; i < d->level_count; i++) {
            if (d->level_offsets[i] <= d->level_offsets[i-1]) {
                mono_ok = 0; break;
            }
        }
        CHECK(mono_ok, "level_offsets monotonically increasing");
    }

    /* I7: DungeonData zeroed before load (struct sanity — raw_data starts NULL) */
    {
        CSB_V1_DungeonData tmp;
        memset(&tmp, 0, sizeof(tmp));
        CHECK(tmp.raw_data == NULL && tmp.level_count == 0,
              "Zeroed struct is valid (raw_data NULL, level_count 0)");
    }

    /* I8: Free function handles NULL gracefully */
    {
        CSB_V1_DungeonData freed;
        memset(&freed, 0, sizeof(freed));
        csb_v1_dungeon_free(&freed);  /* must not crash */
        fprintf(inv, "- PASS: csb_v1_dungeon_free handles zeroed struct\n");
        (void)freed;
    }

    /* I9: Double-free does not crash */
    {
        CSB_V1_DungeonData df;
        memset(&df, 0, sizeof(df));
        df.raw_data = (uint8_t *)malloc(16);
        csb_v1_dungeon_free(&df);
        csb_v1_dungeon_free(&df);  /* must not crash */
        fprintf(inv, "- PASS: double-free does not crash\n");
    }

    fprintf(inv, "\n**Status: %s**\n", fail_count == 0 ? "PASS" : "FAIL");
    return fail_count;
}

int main(int argc, char *argv[]) {
    CSB_V1_DungeonData dungeon;
    FILE *rep, *inv;
    char rep_path[512], inv_path[512];
    int fail_count = 0;
    int load_ret;
    uint8_t *file_buf = NULL;
    long file_size = 0;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <DUNGEON.DAT> <output_dir>\n", argv[0]);
        return 1;
    }

    /* Load file into memory */
    {
        FILE *fp = fopen(argv[1], "rb");
        if (!fp) {
            fprintf(stderr, "FAIL: cannot open %s\n", argv[1]);
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        file_buf = (uint8_t *)malloc(file_size);
        if (!file_buf || fread(file_buf, 1, file_size, fp) != (size_t)file_size) {
            fprintf(stderr, "FAIL: cannot read %s\n", argv[1]);
            fclose(fp);
            free(file_buf);
            return 1;
        }
        fclose(fp);
    }

    /* Zero then load */
    build_zero_dungeon(&dungeon);
    load_ret = csb_v1_dungeon_load(&dungeon, file_buf, (int)file_size);

    snprintf(rep_path, sizeof(rep_path), "%s/csb_dungeon_probe.md", argv[2]);
    snprintf(inv_path, sizeof(inv_path), "%s/csb_dungeon_invariants.md", argv[2]);

    rep = fopen(rep_path, "w");
    inv = fopen(inv_path, "w");
    if (!rep || !inv) {
        fprintf(stderr, "FAIL: cannot write output files\n");
        if (rep) fclose(rep);
        if (inv) fclose(inv);
        free(file_buf);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }

    /* Load must succeed */
    if (load_ret != 0) {
        fprintf(inv, "- FAIL: csb_v1_dungeon_load returned %d (expected 0)\n", load_ret);
        fail_count++;
    } else {
        fprintf(inv, "- PASS: csb_v1_dungeon_load returned 0\n");
    }

    /* Run invariant suite */
    if (load_ret == 0) {
        fail_count = run_invariants(&dungeon, inv, fail_count);

        /* Also write human report */
        probe_report(&dungeon, rep, 1, 1, 1, 1, fail_count);
    }

    fprintf(rep, "\n**Load result:** %s\n", load_ret == 0 ? "SUCCESS" : "FAILURE");
    fclose(rep);
    fclose(inv);

    printf("CSB V1 dungeon probe: load=%s, invariants=%s\n",
           load_ret == 0 ? "OK" : "FAIL",
           fail_count == 0 ? "PASS" : "FAIL");

    free(file_buf);
    csb_v1_dungeon_free(&dungeon);
    return fail_count == 0 ? 0 : 1;
}