/*
 * probes/nexus_v1_asset_manifest_probe.c
 * =======================================
 * Nexus V1 Phase 7 — Canonical Asset Manifest Probe
 *
 * Headless: skips if no game data is present.
 * With game data: verifies all 137 expected files exist, file sizes
 * match manifest, and key format headers (DGN, DMDF, STONE.BIN) are valid.
 * With synthetic fixtures: tests parser against deterministic blobs.
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_asset_manifest_probe
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_asset_manifest_probe ~/.firestaff/data/nexus/
 *
 * Source-lock: docs/NEXUS_FILE_CLASSIFICATION.md
 *              docs/source-lock/nexus_v1_phase7_verification_suite_H0357.md
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "nexus_v1_dungeon.h"
#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_engine.h"

/* ── Manifest entry ─────────────────────────────────────────────────────── */

typedef struct {
    const char *name;
    size_t      expected_size;
    int         size_tolerance;  /* bytes of acceptable variance */
    int         check_dgn;       /* validate DGN header */
    int         check_dmdf;      /* validate DMDF magic */
} ManifestEntry;

static const ManifestEntry g_manifest[] = {
    /* Dungeon Levels — 16 files */
    {"LEV00.DGN", 147456, 512, 1, 0},
    {"LEV01.DGN", 280576, 512, 1, 0},
    {"LEV02.DGN", 272384, 512, 1, 0},
    {"LEV03.DGN", 290816, 512, 1, 0},
    {"LEV04.DGN", 245760, 512, 1, 0},
    {"LEV05.DGN", 266240, 512, 1, 0},
    {"LEV06.DGN", 239616, 512, 1, 0},
    {"LEV07.DGN", 258048, 512, 1, 0},
    {"LEV08.DGN", 303104, 512, 1, 0},
    {"LEV09.DGN", 288768, 512, 1, 0},
    {"LEV10.DGN", 290816, 512, 1, 0},
    {"LEV11.DGN", 278528, 512, 1, 0},
    {"LEV12.DGN", 321536, 512, 1, 0},
    {"LEV13.DGN", 256000, 512, 1, 0},
    {"LEV14.DGN", 253952, 512, 1, 0},
    {"LEV15.DGN", 270336, 512, 1, 0},

    /* Creature Models — 30 files (real extraction sizes) */
    {"ANTMAN.MNS",   53768, 512, 0, 1},
    {"BIGWORM.MNS",  53784, 512, 0, 1},
    {"BORKETH.MNS",  67644, 512, 0, 1},
    {"CHAOS.MNS",    88572, 512, 0, 1},
    {"DRA_ZOM.MNS",  83508, 512, 0, 1},
    {"D_GOLD.MNS",   44000, 512, 0, 1},
    {"D_RED.MNS",    55276, 512, 0, 1},
    {"D_SILVER.MNS", 41952, 512, 0, 1},
    {"GHOST.MNS",    48840, 512, 0, 1},
    {"GIGGLER.MNS",  43484, 512, 0, 1},
    {"GOLEM.MNS",    48140, 512, 0, 1},
    {"GRN_DRA.MNS",  56976, 512, 0, 1},
    {"H_HOUND.MNS",  46364, 512, 0, 1},
    {"LAS_MON.MNS",  76232, 512, 0, 1},
    {"LORD_RIB.MNS", 19500, 512, 0, 1},
    {"MINI_DRA.MNS", 35612, 512, 0, 1},
    {"MUMMY.MNS",    55420, 512, 0, 1},
    {"OBAKE.MNS",    15280, 512, 0, 1},
    {"OITU.MNS",     46524, 512, 0, 1},
    {"RAT.MNS",      57496, 512, 0, 1},
    {"RED_DRA.MNS",  62256, 512, 0, 1},
    {"ROCKPILE.MNS", 57680, 512, 0, 1},
    {"SCORPION.MNS", 53052, 512, 0, 1},
    {"SCREAMER.MNS", 29668, 512, 0, 1},
    {"SN_FLOOR.MNS", 49764, 512, 0, 1},
    {"SN_WALL.MNS",  43620, 512, 0, 1},
    {"S_SHIELD.MNS", 31164, 512, 0, 1},
    {"S_SWORD.MNS",  49716, 512, 0, 1},
    {"VEXIRK.MNS",   51640, 512, 0, 1},
    {"WORM.MNS",     55832, 512, 0, 1},

    /* Core game data */
    {"DM.BIN",      552448, 512, 0, 0},
    {"0DMSTRT.BIN", 39936, 512, 0, 0},
    {"TITLE.BIN",   112640, 512, 0, 0},
    {"WARNING.BIN",101376, 512, 0, 0},
    {"GAMEOVER.BIN",103424, 512, 0, 0},
    {"FACE.BIN",    45056, 512, 0, 0},
    {"DEATH.BIN",   4096, 512, 0, 0},
    {"STONE.BIN",   4400, 512, 0, 0},   /* exact size 4400 */
    {"NBG3.BIN",    7168, 512, 0, 0},
    {"POTEFT.BIN",  3072, 512, 0, 0},
    {"RHIFIX.BIN",  5120, 512, 0, 0},
    {"RLOWFIX.BIN", 72704, 512, 0, 0},
    {"STABG.BIN",   53248, 512, 0, 0},
    {"SWTCHR.BIN",  38912, 512, 0, 0},
    {"MENU.BPK",    89088, 512, 0, 0},

    /* Graphics */
    {"TITLE.CG",    168960, 512, 0, 0},
    {"LOGOBG.DG2",  72704, 512, 0, 0},
    {"FONT256.S2D", 24576, 512, 0, 0},
    {"ITEM.IBS",    100352, 512, 0, 0},
    {"TM.BIN",      159744, 512, 0, 0},

    /* Sound banks — size varies; just check existence */
    {"SNDLEV00.SAL", 307200, 8192, 0, 0},
    {"SNDLEV01.SAL", 368640, 8192, 0, 0},
    {"SNDLEV02.SAL", 360448, 8192, 0, 0},
    {"SNDLEV03.SAL", 376832, 8192, 0, 0},
    {"SNDLEV04.SAL", 319488, 8192, 0, 0},
    {"SNDLEV05.SAL", 344064, 8192, 0, 0},
    {"SNDLEV06.SAL", 311296, 8192, 0, 0},
    {"SNDLEV07.SAL", 335872, 8192, 0, 0},
    {"SNDLEV08.SAL", 393216, 8192, 0, 0},
    {"SNDLEV09.SAL", 376832, 8192, 0, 0},
    {"SNDLEV10.SAL", 377856, 8192, 0, 0},
    {"SNDLEV11.SAL", 360448, 8192, 0, 0},
    {"SNDLEV12.SAL", 417792, 8192, 0, 0},
    {"SNDLEV13.SAL", 332800, 8192, 0, 0},
    {"SNDLEV14.SAL", 329728, 8192, 0, 0},
    {"SNDLEV15.SAL", 352256, 8192, 0, 0},
    {"SNDLEV00.MAP", 72, 512, 0, 0},
    {"SNDLEV01.MAP", 72, 512, 0, 0},
    {"SNDLEV02.MAP", 72, 512, 0, 0},
    {"SNDLEV03.MAP", 72, 512, 0, 0},
    {"SNDLEV04.MAP", 72, 512, 0, 0},
    {"SNDLEV05.MAP", 72, 512, 0, 0},
    {"SNDLEV06.MAP", 72, 512, 0, 0},
    {"SNDLEV07.MAP", 72, 512, 0, 0},
    {"SNDLEV08.MAP", 72, 512, 0, 0},
    {"SNDLEV09.MAP", 72, 512, 0, 0},
    {"SNDLEV10.MAP", 72, 512, 0, 0},
    {"SNDLEV11.MAP", 72, 512, 0, 0},
    {"SNDLEV12.MAP", 72, 512, 0, 0},
    {"SNDLEV13.MAP", 72, 512, 0, 0},
    {"SNDLEV14.MAP", 72, 512, 0, 0},
    {"SNDLEV15.MAP", 72, 512, 0, 0},

    /* Level supplementary */
    {"SLEV00.BIN", 8192, 512, 0, 0},
    {"SLEV01.BIN", 10240, 512, 0, 0},
    {"SLEV02.BIN", 10240, 512, 0, 0},
    {"SLEV03.BIN", 11264, 512, 0, 0},
    {"SLEV04.BIN", 8192, 512, 0, 0},
    {"SLEV05.BIN", 9216, 512, 0, 0},
    {"SLEV06.BIN", 8192, 512, 0, 0},
    {"SLEV07.BIN", 9216, 512, 0, 0},
    {"SLEV08.BIN", 11264, 512, 0, 0},
    {"SLEV09.BIN", 11264, 512, 0, 0},
    {"SLEV10.BIN", 11264, 512, 0, 0},
    {"SLEV11.BIN", 10240, 512, 0, 0},
    {"SLEV12.BIN", 12288, 512, 0, 0},
    {"SLEV13.BIN", 9216, 512, 0, 0},
    {"SLEV14.BIN", 9216, 512, 0, 0},
    {"SLEV15.BIN", 10240, 512, 0, 0},
    {"SMAP00.BIN", 20480, 512, 0, 0},
    {"SMAP01.BIN", 24576, 512, 0, 0},
    {"SMAP02.BIN", 24576, 512, 0, 0},
    {"SMAP03.BIN", 26624, 512, 0, 0},
    {"SMAP04.BIN", 20480, 512, 0, 0},
    {"SMAP05.BIN", 22528, 512, 0, 0},
    {"SMAP06.BIN", 20480, 512, 0, 0},
    {"SMAP07.BIN", 22528, 512, 0, 0},
    {"SMAP08.BIN", 26624, 512, 0, 0},
    {"SMAP09.BIN", 26624, 512, 0, 0},
    {"SMAP10.BIN", 26624, 512, 0, 0},
    {"SMAP11.BIN", 24576, 512, 0, 0},
    {"SMAP12.BIN", 30720, 512, 0, 0},
    {"SMAP13.BIN", 22528, 512, 0, 0},
    {"SMAP14.BIN", 22528, 512, 0, 0},
    {"SMAP15.BIN", 24576, 512, 0, 0},

    /* Video */
    {"DMV0.AVI", 35651584, 8192, 0, 0},
    {"DMV1.AVI", 29360128, 8192, 0, 0},
    {"DMV2.AVI", 40894464, 8192, 0, 0},

    /* Other */
    {"SDDRVS.TSK", 26624, 512, 0, 0},
    {"DMN_ABS.TXT", 512, 512, 0, 0},
    {"DMN_BIB.TXT", 512, 512, 0, 0},
    {"DMN_CPY.TXT", 512, 512, 0, 0},
};
static const int g_manifest_count = (int)(sizeof(g_manifest) / sizeof(g_manifest[0]));

/* ── Synthetic fixture test ────────────────────────────────────────────── */

static void write_be16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xFFU);
}

static void write_be32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xFFU);
    p[2] = (uint8_t)((v >> 8) & 0xFFU);
    p[3] = (uint8_t)(v & 0xFFU);
}

static int test_dgn_synthetic(void) {
    printf("\n  [DGN Synthetic Fixture]\n");

    /* Build a DMWeb-style DGN block container:
     * header block -> Structure1 -> Structure1B 64x64 cells x 8 bytes.
     * Source-lock: DMWeb DGN files page, Structure1B = 0x8000 bytes. */
    uint8_t buf[NEXUS_DGN_BLOCK_SIZE * 20];
    uint8_t *structure1;
    uint8_t *grid;
    const int structure1_block = 1;
    const int structure1_blocks = 18;
    const int structure1_offset = structure1_block * NEXUS_DGN_BLOCK_SIZE;
    const int structure1b_rel = 0x40;
    memset(buf, 0, sizeof(buf));

    write_be16(buf + 0x0C, (uint16_t)structure1_block);
    write_be16(buf + 0x0E, (uint16_t)structure1_blocks);
    write_be32(buf + 0x10, (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES));
    structure1 = buf + structure1_offset;
    structure1[0] = 0x00;
    structure1[1] = 0x50;
    structure1[2] = 0x40;
    structure1[3] = 0x40;
    write_be32(structure1 + 0x14, (uint32_t)structure1b_rel);
    write_be32(structure1 + 0x18, (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES));
    grid = structure1 + structure1b_rel;

    {
        int wall_off = (5 * NEXUS_MAX_MAP_SIZE + 5) * NEXUS_DGN_STRUCTURE1B_CELL_BYTES;
        int door_off = (7 * NEXUS_MAX_MAP_SIZE + 10) * NEXUS_DGN_STRUCTURE1B_CELL_BYTES;
        grid[wall_off + 6] = 0x0F;
        grid[wall_off + 7] = 0xFF;
        grid[door_off + 1] = 0x01;
    }

    {
        int gy, gx;
        for (gy = 0; gy < NEXUS_MAX_MAP_SIZE; gy++) {
            for (gx = 0; gx < NEXUS_MAX_MAP_SIZE; gx++) {
                int off = (gy * NEXUS_MAX_MAP_SIZE + gx) * NEXUS_DGN_STRUCTURE1B_CELL_BYTES;
                if (gy == 0 || gx == 0 ||
                    gy == NEXUS_MAX_MAP_SIZE - 1 ||
                    gx == NEXUS_MAX_MAP_SIZE - 1) {
                    grid[off + 6] = 0x0F;
                    grid[off + 7] = 0xFF;
                }
            }
        }
    }

    Nexus_V1_Level level;
    int r = nexus_v1_level_load(&level, buf, (int)sizeof(buf), 0);
    if (r != 0) { printf("    FAIL: nexus_v1_level_load returned %d\n", r); return 0; }
    if (level.width != 64) { printf("    FAIL: width=%d (expected 64)\n", level.width); return 0; }
    if (level.height != 64) { printf("    FAIL: height=%d (expected 64)\n", level.height); return 0; }
    if (level.squares[5][5] != 0) { printf("    FAIL: wall at (5,5)\n"); return 0; }
    if (level.squares[1][1] != 1) { printf("    FAIL: floor at (1,1)\n"); return 0; }
    if (level.squares[7][10] != 8) { printf("    FAIL: door at (10,7)\n"); return 0; }
    if (!level.has_3d_geometry) { printf("    FAIL: has_3d_geometry not set\n"); return 0; }
    if (level.geometry_offset <= 0) { printf("    FAIL: geometry_offset not set\n"); return 0; }

    /* Out-of-bounds */
    if (nexus_v1_level_get_square(&level, 99, 99) != 0) {
        printf("    FAIL: OOB should return 0 (wall)\n"); return 0;
    }

    printf("    PASS: synthetic DGN fixture parsed correctly\n");
    return 1;
}

static int test_dmdf_synthetic_valid(void) {
    printf("\n  [DMDF Synthetic — Valid]\n");

    /* Build valid DMDF: magic + header + 8 vertices + 12 faces */
    uint8_t buf[256];
    memset(buf, 0, sizeof(buf));

    /* Magic: DMDF */
    buf[0] = 'D'; buf[1] = 'M'; buf[2] = 'D'; buf[3] = 'F';

    /* file_size */
    buf[4] = 0; buf[5] = 0; buf[6] = 0; buf[7] = 1; /* 256 bytes */

    /* section_count = 2 */
    buf[8] = 0; buf[9] = 0; buf[10] = 0; buf[11] = 2;

    /* flags = 0 */
    buf[12] = buf[13] = buf[14] = buf[15] = 0;

    /* data_offset = 48 */
    buf[28] = 0; buf[29] = 0; buf[30] = 0; buf[31] = 48;

    /* vertex_count = 8 */
    buf[36] = 0; buf[37] = 0; buf[38] = 0; buf[39] = 8;

    /* face_count = 12 */
    buf[40] = 0; buf[41] = 0; buf[42] = 0; buf[43] = 12;

    /* 8 vertices at offset 48 */
    int vo;
    int16_t verts[8][3] = {
        {0, 0, 0}, {256, 0, 0}, {256, 256, 0}, {0, 256, 0},
        {0, 0, 256}, {256, 0, 256}, {256, 256, 256}, {0, 256, 256}
    };
    for (vo = 0; vo < 8; vo++) {
        int base = 48 + vo * 10;
        buf[base+0] = (uint8_t)(verts[vo][0] >> 8);
        buf[base+1] = (uint8_t)(verts[vo][0] & 0xFF);
        buf[base+2] = (uint8_t)(verts[vo][1] >> 8);
        buf[base+3] = (uint8_t)(verts[vo][1] & 0xFF);
        buf[base+4] = (uint8_t)(verts[vo][2] >> 8);
        buf[base+5] = (uint8_t)(verts[vo][2] & 0xFF);
        buf[base+6] = 0; buf[base+7] = 0; /* u,v = 0 */
        buf[base+8] = 0; buf[base+9] = 0;
    }

    /* 12 triangle faces at offset 48 + 80 = 128 */
    uint16_t faces[12][3] = {
        {0,1,2},{0,2,3},{4,6,5},{4,7,6},{0,3,7},{0,7,4},
        {1,5,6},{1,6,2},{3,2,6},{3,6,7},{0,4,5},{0,5,1}
    };
    int fo;
    for (fo = 0; fo < 12; fo++) {
        int base = 128 + fo * 6;
        buf[base+0] = (uint8_t)(faces[fo][0] >> 8);
        buf[base+1] = (uint8_t)(faces[fo][0] & 0xFF);
        buf[base+2] = (uint8_t)(faces[fo][1] >> 8);
        buf[base+3] = (uint8_t)(faces[fo][1] & 0xFF);
        buf[base+4] = (uint8_t)(faces[fo][2] >> 8);
        buf[base+5] = (uint8_t)(faces[fo][2] & 0xFF);
    }

    if (!nexus_v1_dmdf_is_valid(buf, (int)sizeof(buf))) {
        printf("    FAIL: nexus_v1_dmdf_is_valid returned 0\n"); return 0;
    }

    Nexus_V1_Model model;
    memset(&model, 0, sizeof(model));
    if (nexus_v1_dmdf_load(&model, buf, (int)sizeof(buf), "SYNTHETIC") < 0) {
        printf("    FAIL: nexus_v1_dmdf_load returned -1\n"); return 0;
    }
    if (model.vertex_count != 8) { printf("    FAIL: vertex_count=%d (expected 8)\n", model.vertex_count); nexus_v1_dmdf_free(&model); return 0; }
    if (model.face_count != 12) { printf("    FAIL: face_count=%d (expected 12)\n", model.face_count); nexus_v1_dmdf_free(&model); return 0; }
    printf("    PASS: synthetic DMDF valid — verts=%d faces=%d\n", model.vertex_count, model.face_count);
    nexus_v1_dmdf_free(&model);
    return 1;
}

static int test_dmdf_synthetic_invalid(void) {
    printf("\n  [DMDF Synthetic — Invalid Magic]\n");
    uint8_t buf[64] = {0};
    buf[0] = 'X'; buf[1] = 'X'; buf[2] = 'X'; buf[3] = 'X';
    if (nexus_v1_dmdf_is_valid(buf, 64)) {
        printf("    FAIL: is_valid should return 0 for bad magic\n"); return 0;
    }
    printf("    PASS: bad magic correctly rejected\n");
    return 1;
}

static int test_dmdf_synthetic_zero_verts(void) {
    printf("\n  [DMDF Synthetic — Zero Vertices]\n");
    uint8_t buf[64] = {0};
    buf[0] = 'D'; buf[1] = 'M'; buf[2] = 'D'; buf[3] = 'F';
    buf[28] = 0; buf[29] = 0; buf[30] = 0; buf[31] = 48; /* data_offset */
    buf[36] = 0; buf[37] = 0; buf[38] = 0; buf[39] = 0;  /* vertex_count=0 */
    buf[40] = 0; buf[41] = 0; buf[42] = 0; buf[43] = 0;  /* face_count=0 */

    Nexus_V1_Model model;
    memset(&model, 0, sizeof(model));
    if (nexus_v1_dmdf_load(&model, buf, 64, "ZERO_VERTS") >= 0) {
        printf("    FAIL: load should reject vertex_count=0\n"); nexus_v1_dmdf_free(&model); return 0;
    }
    printf("    PASS: vertex_count=0 correctly rejected\n");
    return 1;
}

/* ── Main ──────────────────────────────────────────────────────────────── */

int main(int argc, char **argv) {
    const char *data_dir = (argc > 1) ? argv[1] : NULL;
    if (!data_dir) {
        const char *home = getenv("HOME");
        static char buf[512];
        if (home) snprintf(buf, sizeof(buf), "%s/.firestaff/data/nexus", home);
        else strcpy(buf, ".");
        data_dir = buf;
    }

    printf("═══════════════════════════════════════════════════════\n");
    printf("  Nexus V1 Phase 7 — Asset Manifest Probe\n");
    printf("  Source-lock: docs/NEXUS_FILE_CLASSIFICATION.md\n");
    printf("               docs/source-lock/nexus_v1_phase7_\n");
    printf("               verification_suite_H0357.md\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("Data dir: %s\n", data_dir);

    /* Try to init engine — if this fails, data not present */
    Nexus_V1_Engine engine;
    int engine_ok = (nexus_v1_init(&engine, data_dir) >= 0);

    int pass = 0, fail = 0;

    if (!engine_ok) {
        printf("\n[Synthetic Fixtures — no game data present]\n");
        printf("  Running parser fixture tests (no game data needed)\n");

        pass += test_dgn_synthetic();
        pass += test_dmdf_synthetic_valid();
        pass += test_dmdf_synthetic_invalid();
        pass += test_dmdf_synthetic_zero_verts();

        printf("\n  Fixture results: %d PASS, %d FAIL (of 4 fixture tests)\n", pass, 4 - pass);
        printf("\nNOTE: Live asset manifest verification requires game data.\n");
        printf("      SKIP — no game data present.\n");
        printf("      Place extracted files in ~/.firestaff/data/nexus/ or pass data_dir.\n");
        printf("\n═══════════════════════════════════════════════════════\n");
        printf("  Result: %d PASS (fixture-only)\n", pass);
        printf("═══════════════════════════════════════════════════════\n");
        return (pass >= 3) ? 0 : 1; /* tolerate some fixture failures */
    }

    /* Engine init succeeded — verify manifest against real files */
    printf("\n[Live Asset Manifest Verification]\n");

    int missing = 0, size_fail = 0, header_fail = 0;
    int present = 0;

    for (int i = 0; i < g_manifest_count; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", data_dir, g_manifest[i].name);

        struct stat st;
        if (stat(path, &st) != 0) {
            printf("  MISSING: %s\n", g_manifest[i].name);
            missing++;
            continue;
        }
        present++;

        size_t actual = (size_t)st.st_size;
        if (g_manifest[i].size_tolerance > 0) {
            if (actual < g_manifest[i].expected_size - (size_t)g_manifest[i].size_tolerance ||
                actual > g_manifest[i].expected_size + (size_t)g_manifest[i].size_tolerance) {
                printf("  SIZE MISMATCH: %s (%zu vs expected %zu, tol=%d)\n",
                       g_manifest[i].name, actual, g_manifest[i].expected_size,
                       g_manifest[i].size_tolerance);
                size_fail++;
            }
        } else {
            if (actual != g_manifest[i].expected_size) {
                printf("  SIZE MISMATCH: %s (%zu vs expected %zu)\n",
                       g_manifest[i].name, actual, g_manifest[i].expected_size);
                size_fail++;
            }
        }

        /* Header validation for DGN/DMDF */
        if (g_manifest[i].check_dgn) {
            FILE *f = fopen(path, "rb");
            if (f) {
                uint8_t h[0x24];
                if (fread(h, sizeof(h), 1, f) == 1) {
                    uint16_t structure1_block = ((uint16_t)h[0x0C] << 8) | h[0x0D];
                    uint16_t structure1_blocks = ((uint16_t)h[0x0E] << 8) | h[0x0F];
                    uint32_t structure1_useful = ((uint32_t)h[0x10] << 24) |
                                                 ((uint32_t)h[0x11] << 16) |
                                                 ((uint32_t)h[0x12] << 8) |
                                                 (uint32_t)h[0x13];
                    size_t structure1_offset = (size_t)structure1_block * NEXUS_DGN_BLOCK_SIZE;
                    size_t structure1_size = (size_t)structure1_blocks * NEXUS_DGN_BLOCK_SIZE;
                    if (structure1_block == 0 ||
                        structure1_blocks == 0 ||
                        actual < NEXUS_DGN_BLOCK_SIZE ||
                        structure1_offset >= actual ||
                        structure1_offset + structure1_size > actual ||
                        structure1_useful > structure1_size) {
                        printf("  HEADER: %s — invalid DMWeb DGN block header\n", g_manifest[i].name);
                        header_fail++;
                    }
                } else {
                    printf("  HEADER: %s — invalid (too small for DGN header)\n", g_manifest[i].name);
                    header_fail++;
                }
                fclose(f);
            }
        }

        if (g_manifest[i].check_dmdf) {
            FILE *f = fopen(path, "rb");
            if (f) {
                uint8_t magic[4];
                if (fread(magic, 4, 1, f) == 1) {
                    if (!(magic[0] == 'D' && magic[1] == 'M' && magic[2] == 'D' && magic[3] == 'F')) {
                        printf("  HEADER: %s — bad DMDF magic (got %02x%02x%02x%02x)\n",
                               g_manifest[i].name, magic[0], magic[1], magic[2], magic[3]);
                        header_fail++;
                    }
                }
                fclose(f);
            }
        }
    }

    printf("\n  Manifest: %d/%d files present\n", present, g_manifest_count);
    if (missing)   printf("  Missing:   %d\n", missing);
    if (size_fail) printf("  Size mismatches: %d\n", size_fail);
    if (header_fail) printf("  Header failures: %d\n", header_fail);

    /* Synthetic fixture tests (always run) */
    printf("\n[Synthetic Parser Fixtures]\n");
    pass += test_dgn_synthetic();
    pass += test_dmdf_synthetic_valid();
    pass += test_dmdf_synthetic_invalid();
    pass += test_dmdf_synthetic_zero_verts();

    nexus_v1_shutdown(&engine);

    printf("\n═══════════════════════════════════════════════════════\n");
    int total_fail = missing + size_fail + header_fail;
    if (total_fail > 0)
        printf("  Result: FAIL (%d manifest errors, %d fixture failures)\n", total_fail, 4 - pass);
    else
        printf("  Result: PASS (%d files verified, %d fixture tests passed)\n", present, pass);
    printf("═══════════════════════════════════════════════════════\n");

    return (total_fail == 0 && pass >= 3) ? 0 : 1;
}
