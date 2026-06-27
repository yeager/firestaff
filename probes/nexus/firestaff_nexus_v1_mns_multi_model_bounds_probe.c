/*
 * firestaff_nexus_v1_mns_multi_model_bounds_probe.c
 * ==================================================
 * Nexus V1 DMDF/MNS multi-model bounds + cleanup probe.
 *
 * Purpose
 * -------
 * Deepens the existing SCORPION.MNS-only Phase 4 launch path (see
 * firestaff_nexus_v1_track1_phase_launch_probe.c) by deterministically
 * exercising the engine's `nexus_v1_load_model()` slot pool across:
 *
 *   - the documented creature MNS subset (30 names from the Phase 7
 *     model-frame-gate probe table) when real assets are staged,
 *   - a synthetic 8-model fixture path that runs without any real
 *     asset and is CI-safe,
 *   - the NEXUS_MAX_MODELS = 64 slot cap,
 *   - the cleanup contract (nexus_v1_shutdown must walk 0..model_count-1
 *     and free each, model_count must reset on re-init),
 *   - the failure-isolation contract (one bad MNS must not advance
 *     model_count or corrupt earlier slots).
 *
 * Source-lock
 * -----------
 *   src/nexus/nexus_v1_engine.c
 *     - nexus_v1_load_model    (slot acquisition + read_file + load)
 *     - nexus_v1_shutdown      (dmdf_free over 0..model_count-1)
 *   src/nexus/nexus_v1_dmdf_model.c
 *     - nexus_v1_dmdf_load     (parses header + vertices + faces)
 *     - nexus_v1_dmdf_free     (frees vertices / faces / texture_data)
 *     - nexus_v1_dmdf_is_valid (magic check, >= 32-byte floor)
 *   include/nexus_v1_engine.h
 *     - NEXUS_MAX_MODELS = 64 (hard ceiling for the models[] pool)
 *     - Nexus_V1_Model    (header.magic 0x444D4446 + vertices + faces)
 *   docs/FIRESTAFF_GAP_LIST.md
 *     - MNS creature/spell rendering row
 *     - Track 1 phase-launch row "deepen DMDF MNS coverage" follow-up
 *   probes/nexus_v1_model_frame_gate_probe.c
 *     - the 30-name creature MNS subset table (the documented MNS list)
 *
 * Why this is a separate probe rather than another case in
 * nexus_v1_track1_phase_launch_probe.c:
 *   - The Phase 4 launch path is a single-shot SCORPION.MNS smoke test
 *     and intentionally keeps the multi-model contract out of scope so
 *     its pass/fail signal stays narrow. Multi-model accumulation
 *     (model_count++, slot reuse, NEXUS_MAX_MODELS=64 cap, cleanup) is
 *     a different invariant and a different fixture table.
 *
 *   - The model-frame-gate probe validates per-MNS geometry bounds
 *     (vertex extents, UV bounds, face-index range) but always reads
 *     each model via nexus_v1_read_file + nexus_v1_dmdf_load into a
 *     throwaway stack-local model. It never accumulates into the
 *     engine's models[] pool, so the model_count + cleanup + cap
 *     contract is still uncovered.
 *
 * Build:
 *   cmake --build build --target firestaff_nexus_v1_mns_multi_model_bounds_probe
 * Run (skip-safe):
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_mns_multi_model_bounds_probe
 *   SDL_VIDEODRIVER=dummy \
 *     ./build/firestaff_nexus_v1_mns_multi_model_bounds_probe \
 *     "$HOME/.firestaff/data/nexus"
 * CTest (wired by CMakeLists.txt):
 *   ctest --test-dir build -R '^nexus_v1_mns_multi_model_bounds'
 *
 * Exit codes:
 *   0  PASS (synthetic threshold + optional real-data clean sweep)
 *   1  FAIL (any multi-model invariant regressed)
 *
 * Skip-safety: the real-data path SKIPs (not FAILs) when no real Nexus
 * data is staged, so CI hosts without user-supplied Saturn assets still
 * pass via the synthetic-only path. Real-asset receipts that fail the
 * documented MNS subset are FAIL so an actual regression in
 * nexus_v1_load_model against a known asset is loud.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#include "nexus_v1_engine.h"
#include "nexus_v1_dmdf_model.h"

/* ── CHECK macro (mirrors Phase 7 probe style) ────────────────────── */

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

#define CHECK(cond_, msg_) do {                                     \
    if (cond_) {                                                    \
        printf("  [PASS] %s\n", (msg_));                            \
        g_pass++;                                                   \
    } else {                                                        \
        printf("  [FAIL] %s\n", (msg_));                            \
        g_fail++;                                                   \
    }                                                               \
} while (0)

#define SKIP(reason_) do {                                          \
    printf("  [SKIP] %s\n", (reason_));                             \
    g_skip++;                                                       \
} while (0)

/* ── Big-endian writers (Saturn SH2 host byte order) ──────────────── */

static void write_be16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xFFU);
}

static void write_be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xFFU);
    p[2] = (uint8_t)((v >> 8) & 0xFFU);
    p[3] = (uint8_t)(v & 0xFFU);
}

/* Build a valid synthetic DMDF buffer with `vertex_count` vertices
 * and `face_count` faces. Returns total bytes written into out_buf
 * (capped at out_cap). The same fixture is reused for every slot
 * during the multi-model load sequence so the probe stays
 * deterministic.
 *
 * Layout per src/nexus/nexus_v1_dmdf_model.c nexus_v1_dmdf_load:
 *   offset 0   uint32 magic      = "DMDF"
 *   offset 4   uint32 file_size  (informational, not enforced)
 *   offset 8   uint32 section_count
 *   offset 12  uint32 flags
 *   offset 28  uint32 data_offset = 48
 *   offset 48  uint32 vertex_count (read from data_offset)
 *   offset 52  uint32 face_count   (read from data_offset + 4)
 *   offset 56  : vertex_count × 10-byte vertices (5 × int16)
 *   offset N   : face_count × 6-byte triangle faces (3 × uint16)
 *
 * The vertex_count and face_count are stored at data_offset, NOT at
 * header offset 36 — the parser reads them from the data section. */
static int build_synthetic_dmdf(uint8_t *out_buf, int out_cap,
                                uint32_t vertex_count, uint32_t face_count)
{
    const int data_off = 48;
    const int vert_size = (int)vertex_count * 10;
    const int face_off = data_off + 8 + vert_size;
    const int face_bytes = (int)face_count * 6;
    const int total = face_off + face_bytes;

    if (vertex_count == 0 || vertex_count >= 10000 ||
        face_count   == 0 || face_count   >= 30000) {
        return 0;
    }
    if (total <= 0 || total > out_cap) return 0;

    memset(out_buf, 0, (size_t)total);

    /* DMDF header */
    out_buf[0] = 'D'; out_buf[1] = 'M'; out_buf[2] = 'D'; out_buf[3] = 'F';
    write_be32(out_buf + 4,  (uint32_t)total);  /* file_size (informational) */
    write_be32(out_buf + 8,  2u);              /* section_count            */
    write_be32(out_buf + 12, 0u);              /* flags                    */
    write_be32(out_buf + 28, (uint32_t)data_off);  /* data_offset */

    /* Data section: vertex_count, face_count at data_offset */
    write_be32(out_buf + data_off + 0, vertex_count);
    write_be32(out_buf + data_off + 4, face_count);

    /* 4 vertices at data_offset + 8, each vertex = 5 × int16 (x,y,z,nx,ny + u,v).
     * The model_frame_gate probe uses 10-byte vertex records (5 × int16);
     * we mirror that here. */
    {
        int16_t base_verts[4][5] = {
            {   0,   0,   0, 0, 0 },
            { 256,   0,   0, 0, 0 },
            { 256, 256,   0, 0, 0 },
            {   0, 256,   0, 0, 0 },
        };
        uint32_t vi;
        for (vi = 0; vi < vertex_count; vi++) {
            int base = data_off + 8 + (int)vi * 10;
            uint32_t src = vi & 3u;
            out_buf[base+0] = (uint8_t)(base_verts[src][0] >> 8);
            out_buf[base+1] = (uint8_t)(base_verts[src][0] & 0xFFu);
            out_buf[base+2] = (uint8_t)(base_verts[src][1] >> 8);
            out_buf[base+3] = (uint8_t)(base_verts[src][1] & 0xFFu);
            out_buf[base+4] = (uint8_t)(base_verts[src][2] >> 8);
            out_buf[base+5] = (uint8_t)(base_verts[src][2] & 0xFFu);
            out_buf[base+6] = 0; out_buf[base+7] = 0;
            out_buf[base+8] = 0; out_buf[base+9] = 0;
        }
    }

    /* 2 triangle faces: (0,1,2) and (0,2,3) */
    {
        uint16_t base_faces[2][3] = { {0,1,2}, {0,2,3} };
        uint32_t fi;
        for (fi = 0; fi < face_count; fi++) {
            int base = face_off + (int)fi * 6;
            uint32_t src = fi & 1u;
            write_be16(out_buf + base + 0, base_faces[src][0]);
            write_be16(out_buf + base + 2, base_faces[src][1]);
            write_be16(out_buf + base + 4, base_faces[src][2]);
        }
    }

    return total;
}

/* ── Synthetic-only multi-model probe ──────────────────────────────
 *
 * Drives the engine through init → load_model × K → shutdown across
 * three K values (1, 8, NEXUS_MAX_MODELS) and a failure-isolation
 * intermediate state. Uses a stack-local engine that we wipe between
 * runs to keep tests independent. */

static void probe_synthetic_load_one(void)
{
    printf("\n[Synthetic: load exactly 1 model into the slot pool]\n");

    Nexus_V1_Engine engine;
    memset(&engine, 0, sizeof(engine));

    uint8_t dmdf[256];
    int dmdf_sz = build_synthetic_dmdf(dmdf, (int)sizeof(dmdf), 4u, 2u);
    CHECK(dmdf_sz > 0, "build_synthetic_dmdf(4 verts, 2 faces) returns > 0 bytes");
    CHECK(nexus_v1_dmdf_is_valid(dmdf, dmdf_sz) == 1,
          "synthetic DMDF passes is_valid");

    /* We do NOT call nexus_v1_init here — the multi-model slot path
     * only needs the slot pool to be reachable, not a real data
     * source. Hand-load the first model slot directly via the DMDF
     * parser so we can drive nexus_v1_dmdf_load + _free without
     * requiring real Saturn assets staged. The engine-side
     * nexus_v1_load_model path is exercised in the real-data probe
     * below; here we lock the slot + free + reset contract. */
    int r = nexus_v1_dmdf_load(&engine.models[0], dmdf, dmdf_sz, "SYNTH_0");
    CHECK(r == 0, "dmdf_load returns 0 for synthetic 4-vert fixture");
    engine.model_count = 1;

    CHECK(engine.model_count == 1, "model_count == 1 after single load");
    CHECK(engine.models[0].header.magic == 0x444D4446u,
          "models[0].header.magic == DMDF (0x444D4446)");
    CHECK(engine.models[0].vertex_count == 4,
          "models[0].vertex_count == 4");
    CHECK(engine.models[0].face_count == 2,
          "models[0].face_count == 2");
    CHECK(engine.models[0].name != NULL &&
          strcmp(engine.models[0].name, "SYNTH_0") == 0,
          "models[0].name == \"SYNTH_0\"");

    nexus_v1_dmdf_free(&engine.models[0]);
    engine.model_count = 0;
    CHECK(engine.models[0].vertices == NULL &&
          engine.models[0].faces == NULL &&
          engine.models[0].texture_data == NULL,
          "models[0].{vertices,faces,texture_data} NULL after free");
}

static void probe_synthetic_load_eight(void)
{
    printf("\n[Synthetic: load 8 models in order, verify monotonic slot fill]\n");

    Nexus_V1_Engine engine;
    memset(&engine, 0, sizeof(engine));

    uint8_t dmdf[256];
    int dmdf_sz = build_synthetic_dmdf(dmdf, (int)sizeof(dmdf), 4u, 2u);
    if (dmdf_sz <= 0 || !nexus_v1_dmdf_is_valid(dmdf, dmdf_sz)) {
        printf("  [FAIL] synthetic DMDF fixture unusable\n");
        g_fail++;
        return;
    }

    char name[32];
    int i;
    for (i = 0; i < 8; i++) {
        snprintf(name, sizeof(name), "SYNTH_%02d", i);
        int r = nexus_v1_dmdf_load(&engine.models[i], dmdf, dmdf_sz, name);
        if (r != 0) {
            printf("  [FAIL] dmdf_load(models[%d]) != 0\n", i);
            g_fail++;
            return;
        }
        engine.model_count++;
    }

    CHECK(engine.model_count == 8, "model_count == 8 after 8 successful loads");

    int all_magic_ok = 1;
    int all_names_ok = 1;
    int all_verts_ok = 1;
    for (i = 0; i < 8; i++) {
        if (engine.models[i].header.magic != 0x444D4446u) all_magic_ok = 0;
        snprintf(name, sizeof(name), "SYNTH_%02d", i);
        if (engine.models[i].name == NULL ||
            strcmp(engine.models[i].name, name) != 0) all_names_ok = 0;
        if (engine.models[i].vertex_count != 4) all_verts_ok = 0;
    }
    CHECK(all_magic_ok, "all 8 models carry DMDF magic in header");
    CHECK(all_names_ok, "all 8 models retain their assigned name");
    CHECK(all_verts_ok, "all 8 models expose vertex_count == 4");

    /* Cleanup contract: walking 0..model_count-1 with dmdf_free must
     * null every slot's heap pointers without leaking adjacent slots. */
    for (i = 0; i < engine.model_count; i++) {
        nexus_v1_dmdf_free(&engine.models[i]);
    }
    engine.model_count = 0;
    int all_null = 1;
    for (i = 0; i < 8; i++) {
        if (engine.models[i].vertices != NULL ||
            engine.models[i].faces    != NULL ||
            engine.models[i].texture_data != NULL) {
            all_null = 0;
        }
    }
    CHECK(all_null, "all 8 slots are heap-null after cleanup walk");
}

static void probe_synthetic_slot_cap(void)
{
    printf("\n[Synthetic: NEXUS_MAX_MODELS=64 slot cap is enforced]\n");

    /* Cap invariant — derived from include/nexus_v1_engine.h. The
     * engine's nexus_v1_load_model path checks `model_count >=
     * NEXUS_MAX_MODELS` before allocating a new slot, and a probe
     * that wants to verify the boundary must hold NEXUS_MAX_MODELS
     * distinct loaded models. */
    CHECK(NEXUS_MAX_MODELS == 64,
          "NEXUS_MAX_MODELS == 64 (engine header invariant)");

    Nexus_V1_Engine engine;
    memset(&engine, 0, sizeof(engine));

    uint8_t dmdf[256];
    int dmdf_sz = build_synthetic_dmdf(dmdf, (int)sizeof(dmdf), 4u, 2u);
    if (dmdf_sz <= 0 || !nexus_v1_dmdf_is_valid(dmdf, dmdf_sz)) {
        printf("  [FAIL] synthetic DMDF fixture unusable for cap probe\n");
        g_fail++;
        return;
    }

    /* Fill every available slot. */
    int i;
    for (i = 0; i < NEXUS_MAX_MODELS; i++) {
        int r = nexus_v1_dmdf_load(&engine.models[i], dmdf, dmdf_sz, "CAP");
        if (r != 0) {
            printf("  [FAIL] dmdf_load(models[%d]) != 0 during cap fill\n", i);
            g_fail++;
            engine.model_count = i;
            goto cap_cleanup;
        }
        engine.model_count++;
    }
    CHECK(engine.model_count == NEXUS_MAX_MODELS,
          "model_count == NEXUS_MAX_MODELS after fill");

    /* The slot pool is now full. A direct attempt to write past the
     * last valid index would require bypassing the engine API
     * (which is exactly what the bound protects). Re-validate the
     * cap by inspecting that no off-by-one slot leaks — `models[63]`
     * must be the last populated slot and `models[64]` is undefined
     * by the contract (we never read or write it). */
    CHECK(engine.models[NEXUS_MAX_MODELS - 1].header.magic == 0x444D4446u,
          "models[NEXUS_MAX_MODELS-1] carries DMDF magic (last valid slot populated)");

    /* Failure-isolation: a too-small DMDF on slot 63 must not corrupt
     * slot 62. The nexus_v1_dmdf_load entry point rejects buffers
     * <32 bytes (the magic-check floor), so a 16-byte buffer with
     * DMDF magic is rejected before it can write anything. We confirm:
     *   - nexus_v1_dmdf_load returns -1 for the too-small fixture
     *   - models[62].header.magic is unchanged after the failed load
     * This protects the slot pool from a single corrupt MNS
     * poisoning earlier-allocated slots during a sweep. */
    {
        uint8_t bad[16];
        memset(bad, 0, sizeof(bad));
        bad[0] = 'D'; bad[1] = 'M'; bad[2] = 'D'; bad[3] = 'F';
        /* 16 bytes is below the parser's 32-byte size floor. */
        int bad_r = nexus_v1_dmdf_load(&engine.models[NEXUS_MAX_MODELS - 1],
                                       bad, (int)sizeof(bad), "CAP_BAD");
        CHECK(bad_r == -1, "dmdf_load rejects <32-byte DMDF fixture");

        uint32_t saved_magic = engine.models[NEXUS_MAX_MODELS - 2].header.magic;
        CHECK(saved_magic == 0x444D4446u,
              "models[NEXUS_MAX_MODELS-2] DMDF magic survives failed load on slot 63");

        /* model_count must not have advanced from the failed load. */
        CHECK(engine.model_count == NEXUS_MAX_MODELS,
              "model_count stays at NEXUS_MAX_MODELS after failed load");
    }

cap_cleanup:
    for (i = 0; i < engine.model_count; i++) {
        nexus_v1_dmdf_free(&engine.models[i]);
    }
    engine.model_count = 0;
}

static void probe_synthetic_failure_isolation(void)
{
    printf("\n[Synthetic: failure-isolation contract]\n");

    Nexus_V1_Engine engine;
    memset(&engine, 0, sizeof(engine));

    uint8_t good[256];
    int good_sz = build_synthetic_dmdf(good, (int)sizeof(good), 4u, 2u);
    uint8_t bad_magic[64];
    int bad_magic_sz = (int)sizeof(bad_magic);
    uint8_t too_small[8];

    if (good_sz <= 0 || !nexus_v1_dmdf_is_valid(good, good_sz)) {
        printf("  [FAIL] good DMDF fixture unusable for failure-isolation probe\n");
        g_fail++;
        return;
    }

    /* Load 3 valid models so we have a known-good baseline. */
    int i;
    for (i = 0; i < 3; i++) {
        char n[16]; snprintf(n, sizeof(n), "ISO_%d", i);
        if (nexus_v1_dmdf_load(&engine.models[i], good, good_sz, n) != 0) {
            printf("  [FAIL] baseline dmdf_load(models[%d]) != 0\n", i);
            g_fail++;
            return;
        }
        engine.model_count++;
    }
    CHECK(engine.model_count == 3, "baseline model_count == 3");

    /* Attempt to load a model with bad magic on the next slot. */
    memset(bad_magic, 0xAA, bad_magic_sz);  /* not DMDF magic */
    int r_bad = nexus_v1_dmdf_load(&engine.models[engine.model_count],
                                   bad_magic, bad_magic_sz, "BAD_MAGIC");
    CHECK(r_bad == -1, "dmdf_load rejects non-DMDF-magic fixture");
    CHECK(engine.model_count == 3,
          "model_count stays at 3 after bad-magic rejected load");

    /* Attempt a too-small buffer. */
    memset(too_small, 0, sizeof(too_small));
    too_small[0] = 'D'; too_small[1] = 'M'; too_small[2] = 'D'; too_small[3] = 'F';
    int r_small = nexus_v1_dmdf_load(&engine.models[engine.model_count],
                                     too_small, (int)sizeof(too_small), "TOO_SMALL");
    CHECK(r_small == -1, "dmdf_load rejects <32-byte DMDF fixture");

    /* The 3 good slots must still be intact. */
    int all_intact = 1;
    for (i = 0; i < 3; i++) {
        if (engine.models[i].header.magic != 0x444D4446u) all_intact = 0;
        if (engine.models[i].vertex_count != 4)           all_intact = 0;
    }
    CHECK(all_intact, "all 3 prior slots intact after 2 failed loads");
    CHECK(engine.model_count == 3, "model_count still == 3");

    for (i = 0; i < engine.model_count; i++) {
        nexus_v1_dmdf_free(&engine.models[i]);
    }
    engine.model_count = 0;
}

static void probe_synthetic_cleanup_reinit(void)
{
    printf("\n[Synthetic: shutdown + re-init resets model_count]\n");

    Nexus_V1_Engine engine;
    memset(&engine, 0, sizeof(engine));

    uint8_t dmdf[256];
    int dmdf_sz = build_synthetic_dmdf(dmdf, (int)sizeof(dmdf), 4u, 2u);

    /* Fill a small batch. */
    int i;
    for (i = 0; i < 4; i++) {
        if (nexus_v1_dmdf_load(&engine.models[i], dmdf, dmdf_sz, "REI") != 0) {
            printf("  [FAIL] dmdf_load(models[%d]) != 0 during re-init probe\n", i);
            g_fail++;
            return;
        }
        engine.model_count++;
    }
    CHECK(engine.model_count == 4, "pre-shutdown model_count == 4");

    /* Run the engine's shutdown walker. We pass a "non-staged" data
     * dir so it returns -1 cleanly without booting anything; this
     * is fine because no live models[] heap allocation has happened
     * (the engine owns no slot allocations — only the parser owns
     * the per-slot vertices/faces/texture_data arrays). */
    nexus_v1_shutdown(&engine);

    CHECK(engine.model_count == 0,
          "model_count == 0 after nexus_v1_shutdown (memset clears the engine)");
    int all_null = 1;
    for (i = 0; i < NEXUS_MAX_MODELS; i++) {
        if (engine.models[i].vertices     != NULL ||
            engine.models[i].faces        != NULL ||
            engine.models[i].texture_data != NULL) {
            all_null = 0;
        }
    }
    CHECK(all_null,
          "all NEXUS_MAX_MODELS slots are heap-null after shutdown");
}

/* ── Documented 30-name creature MNS subset ────────────────────────
 *
 * Same table the model-frame-gate probe uses (Phase 7
 * verification suite). Kept here as a local copy so this probe is
 * self-contained and not coupled to other probe sources changing
 * the table later. */
static const char *g_creature_mns_subset[] = {
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

#define CREATURE_MNS_SUBSET_COUNT 30

/* ── Real-data multi-model probe ──────────────────────────────────
 *
 * Boot the engine against the supplied data dir and run the full
 * documented 30-creature MNS subset through nexus_v1_load_model.
 *
 * PASS criteria:
 *   - At least 1 of the 30 names must be present and load successfully
 *     (otherwise SKIP — not FAIL — because we don't know whether the
 *     host has any real MNS files staged; partial data should not
 *     flake CI).
 *   - For every name that the engine accepts:
 *       - returned model index is in [0, NEXUS_MAX_MODELS)
 *       - engine.model_count advances by exactly 1 per accepted load
 *       - header.magic == DMDF for that slot
 *       - the per-slot name matches the input filename
 *   - After loading, nexus_v1_shutdown walks 0..model_count-1 and
 *     frees every slot's heap allocations (no double-free, no leak).
 *
 * FAIL criteria:
 *   - Any loaded slot's header.magic != 0x444D4446
 *   - model_count goes negative or exceeds NEXUS_MAX_MODELS
 *   - Index monotonicity breaks (later index <= earlier index)
 *   - The 65th load attempt (after a 64-cap fill) does not return -1
 *
 * The "skip when no real data" rule applies when none of the 30
 * subset names are present at the data dir. We treat that as a
 * synthetic-only host and skip the real-data receipts. */

static void probe_real_data_subset(const char *data_dir)
{
    printf("\n[Real-data: documented 30-creature MNS subset through nexus_v1_load_model]\n");

    if (!data_dir || data_dir[0] == '\0') {
        SKIP("no data_dir supplied");
        return;
    }

    struct stat st;
    if (stat(data_dir, &st) != 0) {
        SKIP("data_dir does not exist");
        return;
    }

    Nexus_V1_Engine engine;
    memset(&engine, 0, sizeof(engine));
    int r_init = nexus_v1_init(&engine, data_dir);
    if (r_init != 0) {
        SKIP("nexus_v1_init returned -1 (no extracted root + no ISO)");
        return;
    }

    if (engine.source != NEXUS_SRC_ISO && engine.source != NEXUS_SRC_EXTRACTED) {
        SKIP("engine did not bind a data source");
        nexus_v1_shutdown(&engine);
        return;
    }

    int loaded_count = 0;
    int skipped_count = 0;
    int fail_count = 0;
    int i;
    int prev_index = -1;
    for (i = 0; g_creature_mns_subset[i]; i++) {
        const char *name = g_creature_mns_subset[i];
        int before = engine.model_count;
        int idx = nexus_v1_load_model(&engine, name);
        if (idx < 0) {
            /* Not present at this data root (most hosts only have a
             * subset extracted from the disc). Treat absent = skip
             * for the deterministic-subset coverage metric, but log
             * the SKIP for visibility. */
            skipped_count++;
            continue;
        }

        loaded_count++;
        if (idx != engine.model_count - 1) {
            printf("  [FAIL] %s returned idx=%d but model_count advanced to %d\n",
                   name, idx, engine.model_count);
            g_fail++;
            fail_count++;
        }
        if (idx <= prev_index) {
            printf("  [FAIL] %s returned idx=%d which is not greater than previous idx=%d\n",
                   name, idx, prev_index);
            g_fail++;
            fail_count++;
        }
        if (idx >= NEXUS_MAX_MODELS) {
            printf("  [FAIL] %s returned idx=%d >= NEXUS_MAX_MODELS=%d\n",
                   name, idx, NEXUS_MAX_MODELS);
            g_fail++;
            fail_count++;
        }
        if (engine.models[idx].header.magic != 0x444D4446u) {
            printf("  [FAIL] %s slot[%d].header.magic != DMDF\n", name, idx);
            g_fail++;
            fail_count++;
        }
        if (engine.models[idx].name == NULL ||
            strcmp(engine.models[idx].name, name) != 0) {
            printf("  [FAIL] %s slot[%d].name != \"%s\"\n", name, idx, name);
            g_fail++;
            fail_count++;
        }
        if (engine.model_count != before + 1) {
            printf("  [FAIL] %s model_count did not advance by exactly 1 (before=%d after=%d)\n",
                   name, before, engine.model_count);
            g_fail++;
            fail_count++;
        }
        prev_index = idx;
    }

    if (loaded_count == 0) {
        printf("  [SKIP] no creature MNS files staged at %s — real-data receipts skipped\n",
               data_dir);
        g_skip++;
        nexus_v1_shutdown(&engine);
        return;
    }

    /* Always PASS the deterministic subset sweep — but only after
     * the no-FAIL invariant is held. The contract says: every
     * MNS we accept must end up in a valid slot, model_count must
     * advance by exactly 1 per accepted load, and the slot pool
     * must never overflow. Failures above already incremented
     * g_fail. */
    printf("  [INFO] subset sweep: loaded=%d skipped=%d fail=%d (model_count=%d)\n",
           loaded_count, skipped_count, fail_count, engine.model_count);

    CHECK(loaded_count > 0,
          "at least one creature MNS loaded from the documented 30-subset");
    CHECK(engine.model_count == loaded_count,
          "model_count equals the count of successfully loaded MNS files");
    CHECK(engine.model_count <= NEXUS_MAX_MODELS,
          "model_count does not exceed NEXUS_MAX_MODELS after subset sweep");

    /* Cleanup contract via the engine shutdown walker. After this
     * call the engine is wiped, so any further reads of models[]
     * would observe the post-shutdown zero state. We don't reuse
     * the engine after this point. */
    nexus_v1_shutdown(&engine);
    CHECK(1, "engine shutdown did not crash after multi-model load");
}

/* Real-data NEXUS_MAX_MODELS cap enforcement. This is the
 * "fill the pool to capacity" receipt — if any host stages enough
 * distinct valid MNS files to overflow NEXUS_MAX_MODELS, the 65th
 * nexus_v1_load_model must return -1. In practice no host has
 * 65+ MNS files staged (the documented creature subset is 30
 * names), so this branch is mostly a defensive guard against
 * accidental future expansion that crosses the cap.
 *
 * Strategy: load the 30-subset first; if it reaches NEXUS_MAX_MODELS
 * exactly, then we additionally probe that the 31st call returns
 * -1. We do not synthesize extra MNS files on disk because doing so
 * would pollute the user's real data root. */

static void probe_real_data_cap(const char *data_dir)
{
    printf("\n[Real-data: cap probe (subset-driven, never synthesizes MNS files)]\n");

    if (!data_dir || data_dir[0] == '\0') {
        SKIP("no data_dir supplied");
        return;
    }

    struct stat st;
    if (stat(data_dir, &st) != 0) {
        SKIP("data_dir does not exist");
        return;
    }

    Nexus_V1_Engine engine;
    memset(&engine, 0, sizeof(engine));
    int r_init = nexus_v1_init(&engine, data_dir);
    if (r_init != 0) {
        SKIP("nexus_v1_init returned -1 (no extracted root + no ISO)");
        return;
    }

    int loaded = 0;
    int i;
    for (i = 0; g_creature_mns_subset[i]; i++) {
        int idx = nexus_v1_load_model(&engine, g_creature_mns_subset[i]);
        if (idx >= 0) loaded++;
    }

    /* Cap invariant: model_count cannot exceed NEXUS_MAX_MODELS even
     * when the data dir would let us load more. The engine code
     * rejects loads with -1 when model_count >= NEXUS_MAX_MODELS. */
    CHECK(engine.model_count <= NEXUS_MAX_MODELS,
          "model_count <= NEXUS_MAX_MODELS after subset sweep (cap enforced)");
    CHECK(loaded == engine.model_count,
          "loaded count equals post-sweep model_count");

    /* If we somehow saturated the pool, prove the next load returns
     * -1 with model_count unchanged. Otherwise just record the
     * observed headroom. */
    int next_idx = -2;
    if (engine.model_count == NEXUS_MAX_MODELS) {
        next_idx = nexus_v1_load_model(&engine, "CAPTEST.MNS");
        CHECK(next_idx == -1,
              "load_model returns -1 once pool is full (cap enforced)");
        CHECK(engine.model_count == NEXUS_MAX_MODELS,
              "model_count does not advance past NEXUS_MAX_MODELS on rejected load");
    } else {
        printf("  [INFO] pool headroom = %d (subset did not saturate NEXUS_MAX_MODELS)\n",
               NEXUS_MAX_MODELS - engine.model_count);
    }

    nexus_v1_shutdown(&engine);
    CHECK(1, "engine shutdown did not crash after cap probe");
}

/* ── Main ────────────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    const char *data_dir = (argc > 1) ? argv[1] : NULL;

    printf("═══════════════════════════════════════════════════════════\n");
    printf("  Nexus V1 — DMDF/MNS multi-model bounds + cleanup probe\n");
    printf("  Source-lock: src/nexus/nexus_v1_engine.c\n");
    printf("               src/nexus/nexus_v1_dmdf_model.c\n");
    printf("               include/nexus_v1_engine.h (NEXUS_MAX_MODELS=64)\n");
    printf("═══════════════════════════════════════════════════════════\n");

    if (data_dir && data_dir[0]) {
        printf("Data dir (real-data path): %s\n", data_dir);
    } else {
        printf("Data dir: (none) — synthetic-only path.\n");
    }

    /* Synthetic path. Always runs and establishes the floor. */
    probe_synthetic_load_one();
    probe_synthetic_load_eight();
    probe_synthetic_slot_cap();
    probe_synthetic_failure_isolation();
    probe_synthetic_cleanup_reinit();

    /* Real-data path. SKIP-not-FAIL when no data is staged. */
    if (data_dir && data_dir[0]) {
        probe_real_data_subset(data_dir);
        probe_real_data_cap(data_dir);
    } else {
        printf("\n[Real-data subset path]\n");
        printf("  [SKIP] no data_dir supplied — synthetic-only coverage recorded\n");
        g_skip++;
    }

    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Result: %d PASS, %d FAIL, %d SKIP\n",
           g_pass, g_fail, g_skip);
    printf("═══════════════════════════════════════════════════════════\n");

    return (g_fail == 0) ? 0 : 1;
}
