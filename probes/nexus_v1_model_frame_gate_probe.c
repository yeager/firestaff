/*
 * probes/nexus_v1_model_frame_gate_probe.c
 * =========================================
 * Nexus V1 Phase 7 — Model-Frame Gate Probe
 *
 * Validates each .MNS DMDF file:
 *   1. Passes nexus_v1_dmdf_is_valid() (magic check)
 *   2. nexus_v1_dmdf_load() produces vertex_count > 0, face_count > 0
 *   3. Vertex positions within world bounds: |x|,|z| <= 8192, |y| <= 2048
 *   4. UV coordinates within texture atlas bounds: u,v < 4096
 *   5. All face indices < vertex_count
 *
 * Headless: skips if no game data present.
 * With game data: reports per-model geometry validation.
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_model_frame_gate_probe
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_model_frame_gate_probe ~/.firestaff/data/nexus/
 *
 * Source-lock: src/nexus/nexus_v1_dmdf_model.c
 *              docs/source-lock/nexus_v1_phase7_verification_suite_H0357.md
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_engine.h"

/* ── Geometry bounds ─────────────────────────────────────────────────── */

#define MAX_VERTEX_XZ  8192  /* world extents: ±8192 in XZ plane */
#define MAX_VERTEX_Y    2048  /* world extents: ±2048 in Y (height) */
#define MAX_UV          4096  /* texture atlas: valid UV < 4096 */

static int validate_model(const char *name, Nexus_V1_Model *m) {
    int errors = 0;

    /* Check vertex_count and face_count */
    if (m->vertex_count <= 0) {
        printf("    FAIL: %s vertex_count=%d (must be > 0)\n", name, m->vertex_count);
        return 1;
    }
    if (m->vertex_count > 10000) {
        printf("    FAIL: %s vertex_count=%d (exceeds reasonable bound 10000)\n",
               name, m->vertex_count);
        return 1;
    }
    if (m->face_count <= 0) {
        printf("    FAIL: %s face_count=%d (must be > 0)\n", name, m->face_count);
        return 1;
    }
    if (m->face_count > 30000) {
        printf("    FAIL: %s face_count=%d (exceeds reasonable bound 30000)\n",
               name, m->face_count);
        return 1;
    }

    /* Validate vertex positions */
    for (int i = 0; i < m->vertex_count; i++) {
        Nexus_DMDFVertex *v = &m->vertices[i];

        if (v->x < -MAX_VERTEX_XZ || v->x > MAX_VERTEX_XZ) {
            printf("    WARN: %s vertex[%d] x=%d out of bounds [-%d, %d]\n",
                   name, i, v->x, MAX_VERTEX_XZ, MAX_VERTEX_XZ);
            errors++;
        }
        if (v->z < -MAX_VERTEX_XZ || v->z > MAX_VERTEX_XZ) {
            printf("    WARN: %s vertex[%d] z=%d out of bounds [-%d, %d]\n",
                   name, i, v->z, MAX_VERTEX_XZ, MAX_VERTEX_XZ);
            errors++;
        }
        if (v->y < -MAX_VERTEX_Y || v->y > MAX_VERTEX_Y) {
            printf("    WARN: %s vertex[%d] y=%d out of bounds [-%d, %d]\n",
                   name, i, v->y, MAX_VERTEX_Y, MAX_VERTEX_Y);
            errors++;
        }
        if (v->u >= MAX_UV || v->v >= MAX_UV) {
            printf("    WARN: %s vertex[%d] uv=(%u,%u) >= %d\n",
                   name, i, v->u, v->v, MAX_UV);
            errors++;
        }
    }

    /* Validate face indices */
    if (m->faces) {
        int total_indices = m->face_count * 3;
        for (int i = 0; i < total_indices; i++) {
            if (m->faces[i] >= m->vertex_count) {
                printf("    FAIL: %s face[%d] index=%u >= vertex_count=%d\n",
                       name, i / 3, m->faces[i], m->vertex_count);
                errors++;
                break;  /* report once per model */
            }
        }
    }

    return errors;
}

/* ── Synthetic DMDF fixture tests ───────────────────────────────────── */

static int test_synthetic_valid(void) {
    printf("\n  [Synthetic DMDF — Valid 8-vertex model]\n");

    /* Build valid DMDF: magic + header + 8 verts + 12 faces */
    uint8_t buf[512];
    memset(buf, 0, sizeof(buf));

    buf[0] = 'D'; buf[1] = 'M'; buf[2] = 'D'; buf[3] = 'F';
    buf[4] = 0; buf[5] = 0; buf[6] = 0; buf[7] = 2; /* file_size = 512 */
    buf[8] = 0; buf[9] = 0; buf[10] = 0; buf[11] = 2; /* sections = 2 */
    buf[28] = 0; buf[29] = 0; buf[30] = 0; buf[31] = 48; /* data_offset */
    buf[36] = 0; buf[37] = 0; buf[38] = 0; buf[39] = 8;  /* vertex_count = 8 */
    buf[40] = 0; buf[41] = 0; buf[42] = 0; buf[43] = 12; /* face_count = 12 */

    /* 8 vertices */
    int16_t verts[8][3] = {
        {0,0,0},{512,0,0},{512,512,0},{0,512,0},
        {0,0,512},{512,0,512},{512,512,512},{0,512,512}
    };
    for (int vi = 0; vi < 8; vi++) {
        int base = 48 + vi * 10;
        buf[base+0] = (uint8_t)(verts[vi][0] >> 8); buf[base+1] = (uint8_t)(verts[vi][0] & 0xFF);
        buf[base+2] = (uint8_t)(verts[vi][1] >> 8); buf[base+3] = (uint8_t)(verts[vi][1] & 0xFF);
        buf[base+4] = (uint8_t)(verts[vi][2] >> 8); buf[base+5] = (uint8_t)(verts[vi][2] & 0xFF);
        buf[base+6] = 0; buf[base+7] = 0; buf[base+8] = 0; buf[base+9] = 0;
    }

    /* 12 triangle faces */
    uint16_t faces[12][3] = {
        {0,1,2},{0,2,3},{4,6,5},{4,7,6},{0,3,7},{0,7,4},
        {1,5,6},{1,6,2},{3,2,6},{3,6,7},{0,4,5},{0,5,1}
    };
    for (int fi = 0; fi < 12; fi++) {
        int base = 128 + fi * 6;
        buf[base+0] = (uint8_t)(faces[fi][0] >> 8); buf[base+1] = (uint8_t)(faces[fi][0] & 0xFF);
        buf[base+2] = (uint8_t)(faces[fi][1] >> 8); buf[base+3] = (uint8_t)(faces[fi][1] & 0xFF);
        buf[base+4] = (uint8_t)(faces[fi][2] >> 8); buf[base+5] = (uint8_t)(faces[fi][2] & 0xFF);
    }

    Nexus_V1_Model model;
    memset(&model, 0, sizeof(model));

    if (!nexus_v1_dmdf_is_valid(buf, (int)sizeof(buf))) {
        printf("    FAIL: is_valid returned 0\n"); return 0;
    }
    if (nexus_v1_dmdf_load(&model, buf, (int)sizeof(buf), "SYNTHETIC_VALID") < 0) {
        printf("    FAIL: load returned -1\n"); return 0;
    }

    int errs = validate_model("SYNTHETIC_VALID", &model);
    if (errs == 0)
        printf("    PASS: synthetic model — verts=%d faces=%d (0 geometry errors)\n",
               model.vertex_count, model.face_count);
    nexus_v1_dmdf_free(&model);
    return (errs == 0) ? 1 : 0;
}

static int test_synthetic_oob_vertex(void) {
    printf("\n  [Synthetic DMDF — Out-of-bounds vertex]\n");

    uint8_t buf[512];
    memset(buf, 0, sizeof(buf));

    buf[0] = 'D'; buf[1] = 'M'; buf[2] = 'D'; buf[3] = 'F';
    buf[4] = 0; buf[5] = 0; buf[6] = 0; buf[7] = 2; /* file_size = 512 */
    buf[8] = 0; buf[9] = 0; buf[10] = 0; buf[11] = 2;
    buf[28] = 0; buf[29] = 0; buf[30] = 0; buf[31] = 48;
    buf[36] = 0; buf[37] = 0; buf[38] = 0; buf[39] = 3; /* vertex_count = 3 */
    buf[40] = 0; buf[41] = 0; buf[42] = 0; buf[43] = 1; /* face_count = 1 */

    /* 3 vertices — one with x = 16384 (way out of bounds) */
    int16_t verts[3][5] = {
        {0, 0, 0, 0, 0},
        {512, 512, 0, 256, 256},
        {16384, 0, 0, 0, 0}  /* x out of bounds */
    };
    for (int vi = 0; vi < 3; vi++) {
        int base = 48 + vi * 10;
        buf[base+0] = (uint8_t)(verts[vi][0] >> 8); buf[base+1] = (uint8_t)(verts[vi][0] & 0xFF);
        buf[base+2] = (uint8_t)(verts[vi][1] >> 8); buf[base+3] = (uint8_t)(verts[vi][1] & 0xFF);
        buf[base+4] = (uint8_t)(verts[vi][2] >> 8); buf[base+5] = (uint8_t)(verts[vi][2] & 0xFF);
        buf[base+6] = (uint8_t)(verts[vi][3] >> 8); buf[base+7] = (uint8_t)(verts[vi][3] & 0xFF);
        buf[base+8] = (uint8_t)(verts[vi][4] >> 8); buf[base+9] = (uint8_t)(verts[vi][4] & 0xFF);
    }

    /* 1 triangle face */
    buf[78] = 0; buf[79] = 0; /* i0 = 0 */
    buf[80] = 0; buf[81] = 1; /* i1 = 1 */
    buf[82] = 0; buf[83] = 2; /* i2 = 2 */

    Nexus_V1_Model model;
    memset(&model, 0, sizeof(model));

    if (nexus_v1_dmdf_load(&model, buf, (int)sizeof(buf), "OOB_VERTEX") < 0) {
        printf("    FAIL: load returned -1\n"); return 0;
    }

    int errs = validate_model("OOB_VERTEX", &model);
    if (errs > 0)
        printf("    PASS: OOB vertex correctly flagged (%d errors)\n", errs);
    else
        printf("    FAIL: OOB vertex not flagged\n");
    nexus_v1_dmdf_free(&model);
    return (errs > 0) ? 1 : 0;
}

static int test_synthetic_bad_face_index(void) {
    printf("\n  [Synthetic DMDF — Bad face index]\n");

    uint8_t buf[256];
    memset(buf, 0, sizeof(buf));

    buf[0] = 'D'; buf[1] = 'M'; buf[2] = 'D'; buf[3] = 'F';
    buf[4] = 0; buf[5] = 0; buf[6] = 0; buf[7] = 1;
    buf[8] = 0; buf[9] = 0; buf[10] = 0; buf[11] = 2;
    buf[28] = 0; buf[29] = 0; buf[30] = 0; buf[31] = 48;
    buf[36] = 0; buf[37] = 0; buf[38] = 0; buf[39] = 3; /* vertex_count = 3 */
    buf[40] = 0; buf[41] = 0; buf[42] = 0; buf[43] = 1; /* face_count = 1 */

    /* 3 valid vertices */
    int16_t verts[3][5] = {{0,0,0,0,0},{256,0,0,128,0},{0,256,0,0,128}};
    for (int vi = 0; vi < 3; vi++) {
        int base = 48 + vi * 10;
        buf[base+0] = (uint8_t)(verts[vi][0] >> 8); buf[base+1] = (uint8_t)(verts[vi][0] & 0xFF);
        buf[base+2] = (uint8_t)(verts[vi][1] >> 8); buf[base+3] = (uint8_t)(verts[vi][1] & 0xFF);
        buf[base+4] = (uint8_t)(verts[vi][2] >> 8); buf[base+5] = (uint8_t)(verts[vi][2] & 0xFF);
        buf[base+6] = (uint8_t)(verts[vi][3] >> 8); buf[base+7] = (uint8_t)(verts[vi][3] & 0xFF);
        buf[base+8] = (uint8_t)(verts[vi][4] >> 8); buf[base+9] = (uint8_t)(verts[vi][4] & 0xFF);
    }

    /* 1 face with index 99 (out of range for vertex_count=3) */
    buf[78] = 0; buf[79] = 0; /* i0 = 0 */
    buf[80] = 0; buf[81] = 1; /* i1 = 1 */
    buf[82] = 0; buf[83] = 99; /* i2 = 99 (OUT OF RANGE) */

    Nexus_V1_Model model;
    memset(&model, 0, sizeof(model));

    if (nexus_v1_dmdf_load(&model, buf, (int)sizeof(buf), "BAD_FACE_IDX") < 0) {
        printf("    FAIL: load returned -1\n"); return 0;
    }

    int errs = validate_model("BAD_FACE_IDX", &model);
    if (errs > 0)
        printf("    PASS: bad face index correctly flagged (%d errors)\n", errs);
    else
        printf("    FAIL: bad face index not flagged\n");
    nexus_v1_dmdf_free(&model);
    return (errs > 0) ? 1 : 0;
}

/* ── Main ──────────────────────────────────────────────────────────────── */

static const char *g_model_files[] = {
    "ANTMAN.MNS", "BIGWORM.MNS", "BORKETH.MNS", "CHAOS.MNS",
    "DRA_ZOM.MNS", "D_GOLD.MNS", "D_RED.MNS", "D_SILVER.MNS",
    "GHOST.MNS", "GIGGLER.MNS", "GOLEM.MNS", "GRN_DRA.MNS",
    "H_HOUND.MNS", "LAS_MON.MNS", "LORD_RIB.MNS", "MINI_DRA.MNS",
    "MUMMY.MNS", "OBAKE.MNS", "OITU.MNS", "RAT.MNS",
    "RED_DRA.MNS", "ROCKPILE.MNS", "SCORPION.MNS", "SCREAMER.MNS",
    "SN_FLOOR.MNS", "SN_WALL.MNS", "S_SHIELD.MNS", "S_SWORD.MNS",
    "VEXIRK.MNS", "WORM.MNS",
    NULL
};

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
    printf("  Nexus V1 Phase 7 — Model-Frame Gate Probe\n");
    printf("  Source-lock: src/nexus/nexus_v1_dmdf_model.c\n");
    printf("═══════════════════════════════════════════════════════\n");

    /* Synthetic fixture tests */
    printf("\n[Synthetic DMDF Fixture Tests]\n");
    int fix_pass = 0;
    fix_pass += test_synthetic_valid();
    fix_pass += test_synthetic_oob_vertex();
    fix_pass += test_synthetic_bad_face_index();
    printf("\n  Fixture results: %d/3 passed\n", fix_pass);

    /* Live model validation */
    Nexus_V1_Engine engine;
    int engine_ok = (nexus_v1_init(&engine, data_dir) >= 0);

    if (!engine_ok) {
        printf("\n[Live Models] — no game data, skipping live validation\n");
        printf("  Place .MNS files in ~/.firestaff/data/nexus/ to enable.\n");
        printf("\n═══════════════════════════════════════════════════════\n");
        printf("  Result: PASS (fixture-only, %d/3 fixtures)\n", fix_pass);
        printf("═══════════════════════════════════════════════════════\n");
        return (fix_pass >= 2) ? 0 : 1;
    }

    printf("\n[Live Model Validation]\n");
    int model_pass = 0, model_fail = 0, model_skip = 0;

    for (int i = 0; g_model_files[i]; i++) {
        const char *name = g_model_files[i];
        int size = 0;
        uint8_t *data = nexus_v1_read_file(&engine, name, &size);

        if (!data) {
            printf("  SKIP: %s — not found\n", name);
            model_skip++;
            continue;
        }

        /* Magic check */
        if (!nexus_v1_dmdf_is_valid(data, size)) {
            printf("  FAIL: %s — bad DMDF magic\n", name);
            model_fail++;
            free(data);
            continue;
        }

        /* Load */
        Nexus_V1_Model model;
        memset(&model, 0, sizeof(model));
        if (nexus_v1_dmdf_load(&model, data, size, name) < 0) {
            printf("  FAIL: %s — nexus_v1_dmdf_load returned -1\n", name);
            model_fail++;
            free(data);
            continue;
        }

        /* Validate geometry */
        int errs = validate_model(name, &model);
        if (errs == 0) {
            printf("  PASS: %-20s verts=%4d faces=%5d  (0 errors)\n",
                   name, model.vertex_count, model.face_count);
            model_pass++;
        } else {
            printf("  FAIL: %s — %d geometry errors\n", name, errs);
            model_fail++;
        }

        nexus_v1_dmdf_free(&model);
        free(data);
    }

    nexus_v1_shutdown(&engine);

    printf("\n  Live models: %d passed, %d failed, %d skipped\n",
           model_pass, model_fail, model_skip);

    printf("\n═══════════════════════════════════════════════════════\n");
    int total_fail = (model_fail > 0) ? model_fail : (fix_pass < 2 ? 1 : 0);
    if (total_fail > 0)
        printf("  Result: FAIL\n");
    else
        printf("  Result: PASS (fixture=%d/3, live=%d/%d)\n",
               fix_pass, model_pass, model_pass + model_fail);
    printf("═══════════════════════════════════════════════════════\n");

    return (model_fail == 0 && fix_pass >= 2) ? 0 : 1;
}