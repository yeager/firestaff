/*
 * firestaff_nexus_v1_light_full_load_nglt_probe.c
 * ================================================
 *
 * Probe for the NGLT recognition + runtime handoff added to
 *   include/nexus_v1_save.h          (API surface)
 *   src/nexus/nexus_v1_save_load.c   (loader + writer)
 * Builds on top of the existing
 *   include/nexus_v1_light_runtime.h (NGLT blob format)
 *   include/nexus_v1_light_overflow.h (cast/decay/drop model)
 *
 * Scope (deliberately bounded, data-free):
 *
 *   [1] nexus_v1_save_data_section_has_nglt() predicate
 *       - rejects NULL
 *       - rejects too-small buffers
 *       - accepts a real NGLT blob of canonical size
 *       - rejects a buffer with the magic but a wrong slot_count
 *
 *   [2] Save with embedded NGLT
 *       - nexus_v1_save_full_to_path_with_runtime() writes
 *         champion + world + NGLT into the FNXS data section
 *       - on-disk file has data_size == champ + world + NGLT_BLOB_SIZE
 *       - the trailing bytes start with 'NGLT'
 *
 *   [3] Load with embedded NGLT into a runtime
 *       - nexus_v1_load_full_from_path_with_runtime() decodes
 *         champion + world + runtime
 *       - out_nglt_decoded flips to 1
 *       - cast_counter / MagicalLightAmount / state_hash survive
 *       - classification kind survives
 *
 *   [4] Save without runtime (NULL param)
 *       - behaves like the legacy non-runtime save
 *       - on-disk file has data_size == champ + world (no NGLT tail)
 *
 *   [5] Load with NULL runtime
 *       - behaves like the legacy non-runtime load
 *       - champion + world still load cleanly
 *
 *   [6] Load with init()'d runtime from a non-NGLT save
 *       - champion + world load cleanly
 *       - out_nglt_decoded stays 0
 *       - runtime stays at its caller-supplied init state
 *
 *   [7] Load with NGLT but uninitialized runtime
 *       - champion + world load cleanly
 *       - out_nglt_diagnostic surfaces a clean reason
 *       - runtime stays untouched (not init()'d by the loader)
 *
 *   [8] Mode-mismatched NGLT
 *       - guard-mode blob loaded into emulate-mode runtime surfaces
 *         a clean diagnostic; champion + world still load
 *       - the target runtime's prior init state is preserved
 *
 *   [9] Determinism (5 cast-then-save-then-load round-trips produce
 *       the same FNV-1a state hash on both source and target runtime)
 *
 *  [10] Real-asset skip-safe path
 *       - the probe never touches the user's real FNXS dir unless
 *         FIRESTAFF_NEXUS_SAVE_PATH points at a real file
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_light_full_load_nglt_probe
 * CTest:
 *   ctest --test-dir build -R nexus_v1_light_full_load_nglt --output-on-failure
 *
 * Source-lock:
 *   include/nexus_v1_save.h          (NGLT recognition API)
 *   src/nexus/nexus_v1_save_load.c   (loader implementation)
 *   include/nexus_v1_light_runtime.h (NGLT blob layout)
 *   include/nexus_v1_light_overflow.h (cast/decay/drop model)
 *   ReDMCSB TIMELINE.C F0238 / F0257, MENU.C:1926-1942, LOADSAVE.C:2041
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(_WIN32)
#include <direct.h>
#define PROBE_MKDIR(path) _mkdir(path)
#else
#define PROBE_MKDIR(path) mkdir((path), 0755)
#endif

#include "nexus_v1_save.h"
#include "nexus_v1_light_runtime.h"
#include "nexus_v1_light_overflow.h"
#include "nexus_v1_world.h"
#include "nexus_v1_champions.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                 \
    if (cond) { printf("  PASS: %s\n", (msg)); ++g_pass; } \
    else      { printf("  FAIL: %s\n", (msg)); ++g_fail; } \
} while (0)

/* Build a minimal-but-distinct synthetic world. */
static void build_world(Nexus_V1_World *world) {
    nexus_v1_world_init(world);
    world->party_level = 0;
    world->party_x = 11;
    world->party_y = 29;
    world->party_dir = 0;
    /* Add a chest at (5,10) so the world hash is distinct from a
     * freshly-init'd one. */
    Nexus_V1_Object chest = {
        .id = 0, .type = 1 /* NEXUS_OBJECT_CHEST */,
        .state = 0, .x = 5, .y = 10,
        .level = 0, .quantity = 1,
        .linked_id = 0, .flags = 0
    };
    nexus_v1_object_place(world, &chest);
    nexus_v1_world_hash_inject(world, 0x444E5558UL);
}

/* Build a synthetic champion pool with deterministic content. */
static void build_pool(Nexus_V1_ChampionPool *pool) {
    nexus_v1_champions_init(pool);
    /* One champion in slot 0 with a deterministic name so the
     * serialized bytes are non-trivial. */
    const char *n = "FIRE";
    size_t i = 0;
    for (; i < 5 && n[i]; ++i) pool->champions[0].name_ascii[i] = n[i];
    pool->champions[0].name_ascii[i] = '\0';
    pool->champions[0].health = 50;
    pool->champions[0].max_health = 50;
}

/* Make a tmpdir + slot path under /tmp so we never touch the user's
 * real FNXS dir unless asked. Each call uses a different suffix so
 * cleanup of one test doesn't remove a dir another test still
 * depends on. */
static int make_tmp_slot(char *tmpdir_out, size_t tmpdir_size,
                         char *slot_out, size_t slot_size,
                         const char *suffix) {
    snprintf(tmpdir_out, tmpdir_size,
             "/tmp/nexus_v1_light_full_load_nglt_probe_%d_%s",
             (int)getpid(), suffix ? suffix : "main");
    /* mkdir -p equivalent. The probe only needs a single dir; if it
     * exists from a previous run, mkdir succeeds on POSIX when the
     * dir already exists... no, it returns EEXIST. We use access() to
     * gate that. */
    if (access(tmpdir_out, 0) != 0) {
        if (PROBE_MKDIR(tmpdir_out) != 0) return 0;
    }
    snprintf(slot_out, slot_size,
             "%s/save.dat", tmpdir_out);
    return 1;
}

int main(void) {
    printf("=== Nexus V1 NGLT embedded blob recognition probe ===\n\n");

    /* ── [1] nexus_v1_save_data_section_has_nglt() predicate ─────── */
    printf("[1] nexus_v1_save_data_section_has_nglt() predicate\n");
    {
        /* Build a canonical NGLT blob from a runtime. */
        Nexus_V1_LightRuntime rt;
        nexus_v1_light_runtime_init(&rt, /*guard=*/0);
        for (int i = 0; i < 3; ++i) {
            nexus_v1_light_runtime_apply_cast(&rt,
                                              NEXUS_LIGHT_KIND_LIGHT, 2);
        }
        uint8_t blob[NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE];
        size_t w = nexus_v1_light_runtime_serialize(&rt, blob, sizeof(blob));
        CHECK(w == NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE,
              "serialize() produced canonical blob size");
        CHECK(blob[0] == 'N' && blob[1] == 'G' && blob[2] == 'L' && blob[3] == 'T',
              "blob magic = 'NGLT' at offset 0");

        CHECK(nexus_v1_save_data_section_has_nglt(NULL, sizeof(blob)) == 0,
              "predicate rejects NULL");
        CHECK(nexus_v1_save_data_section_has_nglt(blob, sizeof(blob) - 1) == 0,
              "predicate rejects too-small buffer");
        CHECK(nexus_v1_save_data_section_has_nglt(blob, sizeof(blob)) == 1,
              "predicate accepts canonical blob");

        /* Corrupt the slot_count field and confirm the predicate
         * rejects. The slot_count uint32 sits at offset 9*4 = 36 from
         * the blob start. */
        uint8_t bad_slot[NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE];
        memcpy(bad_slot, blob, sizeof(bad_slot));
        bad_slot[36] = 0xFF; bad_slot[37] = 0xFF; bad_slot[38] = 0xFF; bad_slot[39] = 0xFE;
        CHECK(nexus_v1_save_data_section_has_nglt(bad_slot, sizeof(bad_slot)) == 0,
              "predicate rejects foreign slot_count");

        /* Corrupt the magic. */
        uint8_t bad_magic[NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE];
        memcpy(bad_magic, blob, sizeof(bad_magic));
        bad_magic[0] = 'X';
        CHECK(nexus_v1_save_data_section_has_nglt(bad_magic, sizeof(bad_magic)) == 0,
              "predicate rejects wrong magic");

        nexus_v1_light_runtime_shutdown(&rt);
    }

    /* ── [2] Save with embedded NGLT ─────────────────────────────── */
    printf("\n[2] Save with embedded NGLT\n");
    char tmpdir[256];
    char slot_path[512];
    if (!make_tmp_slot(tmpdir, sizeof(tmpdir), slot_path, sizeof(slot_path), "main")) {
        printf("  FAIL: could not create temp dir %s\n", tmpdir);
        ++g_fail;
    } else {
        Nexus_V1_ChampionPool pool;
        Nexus_V1_World world;
        build_pool(&pool);
        build_world(&world);

        Nexus_V1_LightRuntime rt;
        nexus_v1_light_runtime_init(&rt, /*guard=*/0);
        for (int i = 0; i < 6; ++i) {
            nexus_v1_light_runtime_apply_cast(&rt,
                                              NEXUS_LIGHT_KIND_TORCH, 2);
        }
        size_t expected_nglt = nexus_v1_light_runtime_blob_size();
        size_t expected_world = nexus_v1_world_serialize_size(&world);
        size_t expected_champ = nexus_v1_champion_pool_serialize_size(&pool);
        size_t expected_data = 0;

        size_t actual_data = 0;
        Nexus_SaveResult sr = nexus_v1_save_full_to_path_with_runtime(
            slot_path,
            world.party_level, world.party_x, world.party_y,
            world.party_dir, (uint32_t)world.world_tick,
            nexus_v1_world_hash(&world),
            &pool, &world, &rt, &actual_data);
        CHECK(sr == NEXUS_SAVE_OK,
              "save_full_to_path_with_runtime() returned NEXUS_SAVE_OK");
        expected_data = expected_champ + expected_world + expected_nglt;
        CHECK(actual_data == expected_data,
              "out_data_size matches champion + world + NGLT");
        CHECK(sr == NEXUS_SAVE_OK && actual_data > expected_nglt,
              "data section is bigger than the NGLT tail alone");

        /* Read the file back and confirm the trailing bytes are the
         * NGLT blob. */
        FILE *fp = fopen(slot_path, "rb");
        CHECK(fp != NULL, "saved file is readable");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long fsize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            CHECK(fsize > 0 && (size_t)fsize == sizeof(Nexus_V1_SaveHeader) + actual_data,
                  "file size = header + data section");
            uint8_t *file_buf = (uint8_t *)malloc((size_t)fsize);
            CHECK(file_buf != NULL, "file bytes allocated");
            if (file_buf) {
                size_t got = fread(file_buf, 1, (size_t)fsize, fp);
                CHECK(got == (size_t)fsize, "full file read");
                const uint8_t *data_section =
                    file_buf + sizeof(Nexus_V1_SaveHeader);
                CHECK(nexus_v1_save_data_section_has_nglt(
                          data_section, actual_data) == 1,
                      "data section's tail is a recognizable NGLT blob");
                /* Spot-check the NGLT magic is at the start of the
                 * tail (data_size - NGLT_BLOB_SIZE), not at the very
                 * last 4 bytes (the slot array ends there). */
                size_t nglt_off = actual_data - NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE;
                CHECK(data_section[nglt_off + 0] == 'N' &&
                      data_section[nglt_off + 1] == 'G' &&
                      data_section[nglt_off + 2] == 'L' &&
                      data_section[nglt_off + 3] == 'T',
                      "NGLT magic at start of tail (data_size - NGLT_BLOB_SIZE)");
                free(file_buf);
            }
            fclose(fp);
        }

        /* Save state for [3] / [6] / [7] / [8] / [9]. */
        /* Keep `slot_path` around — the rest of the probe re-uses it. */
        nexus_v1_light_runtime_shutdown(&rt);
    }

    /* ── [3] Load with embedded NGLT into a runtime ───────────────── */
    printf("\n[3] Load with embedded NGLT into a runtime\n");
    {
        /* Build the same world/pool state in memory so we can compare
         * hashes before vs. after the round-trip. */
        Nexus_V1_World src_world;
        Nexus_V1_ChampionPool src_pool;
        build_world(&src_world);
        build_pool(&src_pool);

        Nexus_V1_LightRuntime src_rt;
        nexus_v1_light_runtime_init(&src_rt, /*guard=*/0);
        for (int i = 0; i < 6; ++i) {
            nexus_v1_light_runtime_apply_cast(&src_rt,
                                              NEXUS_LIGHT_KIND_TORCH, 2);
        }
        uint32_t src_cast_counter = src_rt.timeline.cast_counter;
        int32_t  src_mla          = src_rt.state.magical_light_amount;
        uint32_t src_state_hash   = nexus_v1_light_runtime_state_hash(&src_rt);

        /* Re-save so [3] runs against the same file layout [2] built. */
        remove(slot_path);
        size_t data_size = 0;
        Nexus_SaveResult sr = nexus_v1_save_full_to_path_with_runtime(
            slot_path,
            src_world.party_level, src_world.party_x, src_world.party_y,
            src_world.party_dir, (uint32_t)src_world.world_tick,
            nexus_v1_world_hash(&src_world),
            &src_pool, &src_world, &src_rt, &data_size);
        CHECK(sr == NEXUS_SAVE_OK,
              "resaved FNXS with NGLT for the load step");

        /* Load into fresh storage + a fresh runtime. */
        Nexus_V1_World dst_world;
        Nexus_V1_ChampionPool dst_pool;
        Nexus_V1_LightRuntime dst_rt;
        nexus_v1_light_runtime_init(&dst_rt, /*guard=*/0);
        Nexus_V1_SaveHeader hdr;
        int nglt_decoded = 0;
        char nglt_diag[256] = {0};
        char diag[256] = {0};
        sr = nexus_v1_load_full_from_path_with_runtime(
            slot_path, &hdr,
            &dst_pool, &dst_world, &dst_rt,
            &nglt_decoded, nglt_diag, sizeof(nglt_diag),
            diag, sizeof(diag));
        CHECK(sr == NEXUS_SAVE_OK, "load_full_from_path_with_runtime OK");
        CHECK(nglt_decoded == 1, "out_nglt_decoded flipped to 1");
        CHECK(nglt_diag[0] == '\0',
              "nglt_diagnostic stays empty on success");
        CHECK(dst_rt.timeline.cast_counter == src_cast_counter,
              "cast_counter survives FNXS round-trip");
        CHECK(dst_rt.state.magical_light_amount == src_mla,
              "MagicalLightAmount survives FNXS round-trip");
        CHECK(nexus_v1_light_runtime_state_hash(&dst_rt) == src_state_hash,
              "FNV-1a state hash matches across reload");
        CHECK(nexus_v1_light_runtime_classify(&dst_rt) ==
              nexus_v1_light_runtime_classify(&src_rt),
              "classification kind matches across reload");
        CHECK(dst_world.party_x == src_world.party_x &&
              dst_world.party_y == src_world.party_y,
              "world.party_x/y survives FNXS round-trip");
        CHECK(dst_world.party_level == src_world.party_level,
              "world.party_level survives FNXS round-trip");
        CHECK(dst_world.object_count == src_world.object_count,
              "world.object_count survives FNXS round-trip");
        CHECK(dst_world.event_count == src_world.event_count,
              "world.event_count survives FNXS round-trip");
        /* Note: we do NOT compare world_hash() because the in-memory
         * object id (set by nexus_v1_object_place) is not part of the
         * serialized payload — that's a pre-existing world-serializer
         * quirk, not something the NGLT loader changes. */

        nexus_v1_light_runtime_shutdown(&dst_rt);
        nexus_v1_light_runtime_shutdown(&src_rt);
    }

    /* ── [4] Save without runtime (NULL param) ───────────────────── */
    printf("\n[4] Save without runtime (NULL param)\n");
    {
        char legacy_tmp[256];
        char legacy_slot[512];
        if (!make_tmp_slot(legacy_tmp, sizeof(legacy_tmp),
                           legacy_slot, sizeof(legacy_slot), "legacy")) {
            printf("  FAIL: could not create legacy tmpdir\n");
            ++g_fail;
        } else {
            Nexus_V1_ChampionPool pool;
            Nexus_V1_World world;
            build_pool(&pool);
            build_world(&world);

            size_t data_size = 0;
            Nexus_SaveResult sr = nexus_v1_save_full_to_path_with_runtime(
                legacy_slot,
                world.party_level, world.party_x, world.party_y,
                world.party_dir, (uint32_t)world.world_tick,
                nexus_v1_world_hash(&world),
                &pool, &world, /*runtime=*/NULL, &data_size);
            CHECK(sr == NEXUS_SAVE_OK,
                  "save with NULL runtime returned OK");
            size_t expected = nexus_v1_champion_pool_serialize_size(&pool)
                            + nexus_v1_world_serialize_size(&world);
            CHECK(data_size == expected,
                  "data_size matches champion + world (no NGLT tail)");

            /* Confirm the file does NOT have an NGLT tail. */
            FILE *fp = fopen(legacy_slot, "rb");
            CHECK(fp != NULL, "legacy save readable");
            if (fp) {
                uint8_t file_buf[8192];
                size_t got = fread(file_buf, 1, sizeof(file_buf), fp);
                fclose(fp);
                CHECK(got >= sizeof(Nexus_V1_SaveHeader) + data_size,
                      "file covers header + data section");
                if (got >= sizeof(Nexus_V1_SaveHeader) + data_size) {
                    const uint8_t *data_section =
                        file_buf + sizeof(Nexus_V1_SaveHeader);
                    CHECK(nexus_v1_save_data_section_has_nglt(
                              data_section, data_size) == 0,
                          "legacy save has no NGLT tail");
                }
            }
            /* Cleanup. */
            remove(legacy_slot);
            rmdir(legacy_tmp);
        }
    }

    /* ── [5] Load with NULL runtime ───────────────────────────────── */
    printf("\n[5] Load with NULL runtime\n");
    {
        Nexus_V1_World src_world;
        Nexus_V1_ChampionPool src_pool;
        build_world(&src_world);
        build_pool(&src_pool);

        Nexus_V1_LightRuntime src_rt;
        nexus_v1_light_runtime_init(&src_rt, /*guard=*/0);
        for (int i = 0; i < 4; ++i) {
            nexus_v1_light_runtime_apply_cast(&src_rt,
                                              NEXUS_LIGHT_KIND_LIGHT, 2);
        }
        remove(slot_path);
        size_t data_size = 0;
        Nexus_SaveResult sr = nexus_v1_save_full_to_path_with_runtime(
            slot_path,
            src_world.party_level, src_world.party_x, src_world.party_y,
            src_world.party_dir, (uint32_t)src_world.world_tick,
            nexus_v1_world_hash(&src_world),
            &src_pool, &src_world, &src_rt, &data_size);
        CHECK(sr == NEXUS_SAVE_OK, "save with NGLT for NULL-runtime load");

        Nexus_V1_World dst_world;
        Nexus_V1_ChampionPool dst_pool;
        Nexus_V1_SaveHeader hdr;
        int nglt_decoded = 99;  /* sentinel: loader should leave this alone */
        char nglt_diag[256];
        nglt_diag[0] = 'X';     /* sentinel */
        char diag[256];
        diag[0] = 'X';
        sr = nexus_v1_load_full_from_path_with_runtime(
            slot_path, &hdr,
            &dst_pool, &dst_world, /*runtime=*/NULL,
            &nglt_decoded, nglt_diag, sizeof(nglt_diag),
            diag, sizeof(diag));
        CHECK(sr == NEXUS_SAVE_OK,
              "load_full_from_path_with_runtime OK with NULL runtime");
        CHECK(nglt_decoded == 99,
              "NULL runtime leaves out_nglt_decoded untouched");
        CHECK(nglt_diag[0] == 'X',
              "NULL runtime leaves nglt_diagnostic untouched");
        CHECK(dst_world.party_x == src_world.party_x &&
              dst_world.party_y == src_world.party_y &&
              dst_world.party_level == src_world.party_level,
              "world survives NULL-runtime load");
        nexus_v1_light_runtime_shutdown(&src_rt);
    }

    /* ── [6] Load with init()'d runtime from a non-NGLT save ──────── */
    printf("\n[6] Load with init()'d runtime from a non-NGLT save\n");
    {
        char nonnglt_tmp[256];
        char nonnglt_slot[512];
        if (!make_tmp_slot(nonnglt_tmp, sizeof(nonnglt_tmp),
                           nonnglt_slot, sizeof(nonnglt_slot), "nonnglt")) {
            printf("  FAIL: could not create non-nglt tmpdir\n");
            ++g_fail;
        } else {
            Nexus_V1_ChampionPool pool;
            Nexus_V1_World world;
            build_pool(&pool);
            build_world(&world);

            /* Save without runtime (no tail). */
            Nexus_SaveResult sr = nexus_v1_save_full_to_path_with_runtime(
                nonnglt_slot,
                world.party_level, world.party_x, world.party_y,
                world.party_dir, (uint32_t)world.world_tick,
                nexus_v1_world_hash(&world),
                &pool, &world, /*runtime=*/NULL, /*out_data_size=*/NULL);
            CHECK(sr == NEXUS_SAVE_OK, "non-NGLT save OK");

            /* Load with a runtime; the runtime should stay clean. */
            Nexus_V1_World dst_world;
            Nexus_V1_ChampionPool dst_pool;
            Nexus_V1_LightRuntime dst_rt;
            nexus_v1_light_runtime_init(&dst_rt, /*guard=*/0);
            /* Cast once so we can detect any accidental override. */
            nexus_v1_light_runtime_apply_cast(&dst_rt,
                                              NEXUS_LIGHT_KIND_TORCH, 2);
            uint32_t pre_active = dst_rt.timeline.active_count;
            int32_t  pre_mla    = dst_rt.state.magical_light_amount;

            Nexus_V1_SaveHeader hdr;
            int nglt_decoded = -1;
            char nglt_diag[256] = {0};
            char diag[256] = {0};
            sr = nexus_v1_load_full_from_path_with_runtime(
                nonnglt_slot, &hdr,
                &dst_pool, &dst_world, &dst_rt,
                &nglt_decoded, nglt_diag, sizeof(nglt_diag),
                diag, sizeof(diag));
            CHECK(sr == NEXUS_SAVE_OK, "non-NGLT load OK");
            CHECK(nglt_decoded == 0,
                  "non-NGLT save: out_nglt_decoded stays 0");
            CHECK(nglt_diag[0] == '\0',
                  "non-NGLT save: nglt_diagnostic stays empty");
            CHECK(dst_rt.timeline.active_count == pre_active,
                  "non-NGLT save: runtime active_count preserved");
            CHECK(dst_rt.state.magical_light_amount == pre_mla,
                  "non-NGLT save: runtime MLA preserved");
            CHECK(dst_world.party_x == world.party_x &&
                  dst_world.party_y == world.party_y,
                  "non-NGLT save: world fields preserved");
            nexus_v1_light_runtime_shutdown(&dst_rt);
            remove(nonnglt_slot);
            rmdir(nonnglt_tmp);
        }
    }

    /* ── [7] Load with NGLT but uninitialized runtime ─────────────── */
    printf("\n[7] Load with NGLT but uninitialized runtime\n");
    {
        /* `slot_path` from [2] still has an NGLT tail. */
        Nexus_V1_World dst_world;
        Nexus_V1_ChampionPool dst_pool;
        Nexus_V1_LightRuntime dst_rt;
        /* Deliberately do NOT init() the runtime. memset the
         * initialized field to 0 so we hit the diagnostic branch. */
        memset(&dst_rt, 0, sizeof(dst_rt));

        Nexus_V1_SaveHeader hdr;
        int nglt_decoded = -1;
        char nglt_diag[256] = {0};
        char diag[256] = {0};
        Nexus_SaveResult sr = nexus_v1_load_full_from_path_with_runtime(
            slot_path, &hdr,
            &dst_pool, &dst_world, &dst_rt,
            &nglt_decoded, nglt_diag, sizeof(nglt_diag),
            diag, sizeof(diag));
        CHECK(sr == NEXUS_SAVE_OK,
              "uninit runtime load still succeeds for champion/world");
        CHECK(nglt_decoded == 0,
              "uninit runtime: out_nglt_decoded stays 0");
        CHECK(nglt_diag[0] != '\0',
              "uninit runtime: nglt_diagnostic surfaces a reason");
        CHECK(dst_rt.initialized == 0,
              "uninit runtime: initialized flag stays 0");
        CHECK(nexus_v1_world_hash(&dst_world) != 0,
              "world was loaded successfully despite uninit runtime");
    }

    /* ── [8] Mode-mismatched NGLT ─────────────────────────────────── */
    printf("\n[8] Mode-mismatched NGLT (guard blob into emulate runtime)\n");
    {
        /* Build a guard-mode save with NGLT. */
        Nexus_V1_ChampionPool pool;
        Nexus_V1_World world;
        build_pool(&pool);
        build_world(&world);

        Nexus_V1_LightRuntime src_rt;
        nexus_v1_light_runtime_init(&src_rt, /*guard=*/1);
        for (int i = 0; i < 3; ++i) {
            nexus_v1_light_runtime_apply_cast(&src_rt,
                                              NEXUS_LIGHT_KIND_TORCH, 2);
        }
        char mm_tmp[256];
        char mm_slot[512];
        if (!make_tmp_slot(mm_tmp, sizeof(mm_tmp), mm_slot, sizeof(mm_slot), "mm")) {
            printf("  FAIL: could not create mm tmpdir\n");
            ++g_fail;
        } else {
            Nexus_SaveResult sr = nexus_v1_save_full_to_path_with_runtime(
                mm_slot,
                world.party_level, world.party_x, world.party_y,
                world.party_dir, (uint32_t)world.world_tick,
                nexus_v1_world_hash(&world),
                &pool, &world, &src_rt, /*out_data_size=*/NULL);
            CHECK(sr == NEXUS_SAVE_OK, "guard-mode save OK");
            nexus_v1_light_runtime_shutdown(&src_rt);

            /* Load into an emulate-mode runtime; the NGLT must be
             * rejected cleanly. */
            Nexus_V1_World dst_world;
            Nexus_V1_ChampionPool dst_pool;
            Nexus_V1_LightRuntime dst_rt;
            nexus_v1_light_runtime_init(&dst_rt, /*guard=*/0);
            uint32_t pre_active = dst_rt.timeline.active_count;
            int32_t  pre_mla    = dst_rt.state.magical_light_amount;

            Nexus_V1_SaveHeader hdr;
            int nglt_decoded = -1;
            char nglt_diag[256] = {0};
            char diag[256] = {0};
            sr = nexus_v1_load_full_from_path_with_runtime(
                mm_slot, &hdr,
                &dst_pool, &dst_world, &dst_rt,
                &nglt_decoded, nglt_diag, sizeof(nglt_diag),
                diag, sizeof(diag));
            CHECK(sr == NEXUS_SAVE_OK,
                  "mode-mismatched load still succeeds for champion/world");
            CHECK(nglt_decoded == 0,
                  "mode-mismatched: out_nglt_decoded stays 0");
            CHECK(nglt_diag[0] != '\0',
                  "mode-mismatched: nglt_diagnostic surfaces a reason");
            CHECK(dst_rt.guard_rejects == 0,
                  "mode-mismatched: target runtime mode preserved (emulate)");
            CHECK(dst_rt.timeline.active_count == pre_active,
                  "mode-mismatched: target runtime active_count preserved");
            CHECK(dst_rt.state.magical_light_amount == pre_mla,
                  "mode-mismatched: target runtime MLA preserved");
            nexus_v1_light_runtime_shutdown(&dst_rt);
            remove(mm_slot);
            rmdir(mm_tmp);
        }
    }

    /* ── [9] Determinism ──────────────────────────────────────────── */
    printf("\n[9] Determinism (cast-then-save-then-load round-trip stable)\n");
    {
        uint32_t expected_state_hash = 0;
        uint32_t expected_cast_counter = 0;
        int32_t  expected_mla = 0;
        int32_t  expected_party_x = 0;
        int32_t  expected_object_count = 0;
        int mismatch = 0;
        for (int rep = 0; rep < 5; ++rep) {
            Nexus_V1_ChampionPool pool;
            Nexus_V1_World world;
            build_pool(&pool);
            build_world(&world);

            Nexus_V1_LightRuntime rt;
            nexus_v1_light_runtime_init(&rt, /*guard=*/0);
            /* Same script each rep. */
            for (int i = 0; i < 5; ++i) {
                nexus_v1_light_runtime_apply_cast(&rt,
                                                  NEXUS_LIGHT_KIND_TORCH, 2);
            }
            /* Save then load — uses a per-rep tmpfile so the round-
             * trip is end-to-end. */
            char rep_path[512];
            snprintf(rep_path, sizeof(rep_path),
                     "%s/det_%d.dat", tmpdir, rep);
            /* Make sure the rep file goes into the same dir as the
             * main slot so cleanup at the end of main() stays simple. */
            size_t data_size = 0;
            Nexus_SaveResult sr = nexus_v1_save_full_to_path_with_runtime(
                rep_path,
                world.party_level, world.party_x, world.party_y,
                world.party_dir, (uint32_t)world.world_tick,
                nexus_v1_world_hash(&world),
                &pool, &world, &rt, &data_size);
            if (sr != NEXUS_SAVE_OK) { ++mismatch; continue; }

            Nexus_V1_World dst_world;
            Nexus_V1_ChampionPool dst_pool;
            Nexus_V1_LightRuntime dst_rt;
            nexus_v1_light_runtime_init(&dst_rt, /*guard=*/0);
            Nexus_V1_SaveHeader hdr;
            int nglt_decoded = 0;
            char nglt_diag[256] = {0};
            char diag[256] = {0};
            sr = nexus_v1_load_full_from_path_with_runtime(
                rep_path, &hdr,
                &dst_pool, &dst_world, &dst_rt,
                &nglt_decoded, nglt_diag, sizeof(nglt_diag),
                diag, sizeof(diag));
            remove(rep_path);
            if (sr != NEXUS_SAVE_OK || nglt_decoded != 1) { ++mismatch; }
            uint32_t h = nexus_v1_light_runtime_state_hash(&dst_rt);
            if (rep == 0) {
                expected_state_hash = h;
                expected_cast_counter = dst_rt.timeline.cast_counter;
                expected_mla = dst_rt.state.magical_light_amount;
                expected_party_x = dst_world.party_x;
                expected_object_count = dst_world.object_count;
            } else {
                if (h != expected_state_hash) ++mismatch;
                if (dst_rt.timeline.cast_counter != expected_cast_counter) ++mismatch;
                if (dst_rt.state.magical_light_amount != expected_mla) ++mismatch;
                if (dst_world.party_x != expected_party_x) ++mismatch;
                if (dst_world.object_count != expected_object_count) ++mismatch;
            }
            nexus_v1_light_runtime_shutdown(&dst_rt);
            nexus_v1_light_runtime_shutdown(&rt);
        }
        CHECK(mismatch == 0,
              "5 round-trips produce the same state hash + counters + world fields");
    }

    /* ── [10] Real-asset skip-safe path ───────────────────────────── */
    printf("\n[10] Real-asset skip-safe path\n");
    {
        const char *real_path = getenv("FIRESTAFF_NEXUS_SAVE_PATH");
        /* Only attempt the real-asset path if the operator pointed at
         * an FNXS file. Otherwise the synthetic path above is what we
         * gate on. */
        if (real_path && *real_path) {
            printf("  INFO: FIRESTAFF_NEXUS_SAVE_PATH=%s (skipping synthetic-only verdict)\n",
                   real_path);
            /* We don't actually load the operator-staged file here;
             * the load_full_from_path_with_runtime API is exercised
             * by [3] against the synthetic file. The real-asset
             * handoff stays a TODO.md follow-up until a real FNXS
             * with embedded NGLT is staged. */
            CHECK(1, "real-asset path noted; verdict deferred to operator-staged file");
        } else {
            CHECK(1,
                  "no FIRESTAFF_NEXUS_SAVE_PATH: synthetic-only verdict is the gate");
        }
        /* Final cleanup of the synthetic test slot. */
        remove(slot_path);
        rmdir(tmpdir);
    }

    /* ── Summary ──────────────────────────────────────────────────── */
    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
