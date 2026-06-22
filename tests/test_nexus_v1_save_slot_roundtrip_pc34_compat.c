/*
 * tests/test_nexus_v1_save_slot_roundtrip_pc34_compat.c
 * =====================================================
 *
 * Nexus V1 save slot round-trip regression — one world field, one slot.
 *
 * Scope: Narrow, intentional, source-locked.
 *   - Exercises the manager-level API: nexus_v1_save_full() -> nexus_v1_load_full()
 *     (the existing test_nexus_v1_save_header_rejection.c only covers the
 *     malformed-magic rejection path; the existing round_trip_probe writes a
 *     synthetic FNXS file by hand instead of using the high-level API).
 *   - Proves ONE world field round-trips through slot 0:
 *       world.party_x  (the single surviving gate field).
 *   - Verifies the slot file path + manager cache marker are consistent with
 *     the saved header (party_x at the same coordinate).
 *
 * Headless: requires no Saturn game data; uses a deterministic synthetic
 * world + empty champion pool written to a private temp directory.
 *
 * Source-lock reference:
 *   src/nexus/nexus_v1_save_load.c — nexus_v1_save_full() / nexus_v1_load_full()
 *   src/nexus/nexus_v1_world.c     — nexus_v1_world_serialize() /
 *                                     nexus_v1_world_deserialize() (party_x
 *                                     preserved across the round trip)
 *   ReDMCSB LOADSAVE.C: F0433 save, F0434 load (DM1 lineage for save
 *     structure; the Nexus-specific bytes live in the
 *     `Nexus_V1_SaveHeader` shape above the data section).
 *
 * Out of scope (deliberate non-claims):
 *   - Champion pool round-trip (not exercised here)
 *   - Object / event / timer round-trip (not exercised here)
 *   - Multiple slots (only slot 0 is exercised)
 *   - Original Saturn memory card format (Firestaff-native only)
 *   - Pixel/runtime parity with original hardware
 */

#include "nexus_v1_save.h"
#include "nexus_v1_world.h"
#include "nexus_v1_champions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#include <process.h>
#define NSR_MKDIR(path) _mkdir(path)
#define NSR_RMDIR(path) _rmdir(path)
#define NSR_UNLINK(path) _unlink(path)
#define NSR_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define NSR_MKDIR(path) mkdir((path), 0700)
#define NSR_RMDIR(path) rmdir(path)
#define NSR_UNLINK(path) unlink(path)
#define NSR_GETPID() getpid()
#endif

static int g_failures;

static void expect(int cond, const char *msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        ++g_failures;
    }
}

static int make_temp_root(char *out, size_t out_size) {
    const char *base = getenv("TMPDIR");
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)NSR_GETPID();
    int i;

#ifdef _WIN32
    if (!base || !base[0]) base = getenv("TEMP");
#endif
    if (!base || !base[0]) base = "/tmp";

    for (i = 0; i < 64; ++i) {
        int n = snprintf(out, out_size,
                         "%s/firestaff-nexus-save-roundtrip-%u-%d",
                         base, seed, i);
        if (n < 0 || (size_t)n >= out_size) return 0;
        if (NSR_MKDIR(out) == 0) return 1;
    }
    return 0;
}

/* Build a deterministic world with a non-default party_x so the round-trip
 * check is meaningful (the world_init() default party_x is 11). */
static void build_world(Nexus_V1_World *world, int party_x_marker) {
    nexus_v1_world_init(world);
    /* Place the party at the marker coordinate via the documented API. */
    nexus_v1_party_place(world, /*level=*/0,
                         /*x=*/party_x_marker,
                         /*y=*/22,
                         /*dir=*/1 /* East */);
    /* Tick once so world_tick != 0 — also exercises the tick path that
     * lives outside the serialized object/event section but inside the
     * world serialize block (world_tick is written after timers). */
    nexus_v1_world_tick(world);
}

int main(void) {
    char root[512];
    char save_dir[512];
    char slot_path[512];
    Nexus_V1_SaveManager mgr;
    Nexus_V1_World world_before;
    Nexus_V1_World world_after;
    Nexus_V1_ChampionPool pool_before;
    Nexus_V1_ChampionPool pool_after;
    Nexus_V1_SaveHeader out_header;
    char diagnostic[256];
    Nexus_SaveResult save_result;
    Nexus_SaveResult load_result;
    const int PARTY_X_MARKER = 17;
    const uint8_t SLOT_INDEX = 0;

    if (!make_temp_root(root, sizeof(root))) {
        fprintf(stderr, "FAIL: could not create temporary test root\n");
        return 1;
    }

    /* ── Arrange: deterministic world + empty champion pool ────────── */
    nexus_v1_champions_init(&pool_before);
    expect(pool_before.champion_count > 0,
           "roster is seeded from g_nexus_roster so champion_count > 0");
    /* Drop the party to empty to keep the champion blob tiny — the gate
     * here is about the world field, not the pool serialization. */
    pool_before.party_count = 0;
    pool_before.leader_index = -1;

    build_world(&world_before, PARTY_X_MARKER);
    expect(world_before.party_x == PARTY_X_MARKER,
           "world_before.party_x set to PARTY_X_MARKER");

    /* ── Save through the manager-level API to slot 0 ──────────────── */
    snprintf(save_dir, sizeof(save_dir), "%s/saves", root);
    nexus_v1_save_init(&mgr, save_dir);
    expect(mgr.initialized == 1,
           "save manager initializes with the temp save directory");

    save_result = nexus_v1_save_full(&mgr, SLOT_INDEX,
                                      /*current_level=*/world_before.party_level,
                                      /*party_x=*/world_before.party_x,
                                      /*party_y=*/world_before.party_y,
                                      /*party_dir=*/world_before.party_dir,
                                      /*game_time=*/(uint32_t)world_before.world_tick,
                                      /*state_hash=*/world_before.state_hash,
                                      /*champion_pool=*/&pool_before,
                                      /*world=*/&world_before);
    expect(save_result == NEXUS_SAVE_OK,
           "nexus_v1_save_full returns NEXUS_SAVE_OK for slot 0");
    if (save_result == NEXUS_SAVE_OK) {
        const Nexus_V1_SaveSlot *slot = nexus_v1_save_get_slot(&mgr, SLOT_INDEX);
        expect(slot != NULL && slot->occupied == 1,
               "manager slot cache marks slot 0 occupied after save");
        if (slot != NULL) {
            expect(slot->header.magic == NEXUS_SAVE_MAGIC,
                   "cached slot header carries the 'FNXS' magic");
            expect(slot->header.party_x == PARTY_X_MARKER,
                   "cached slot header.party_x matches the world marker");
        }
        snprintf(slot_path, sizeof(slot_path), "%s/nexus_save_%02u.dat", save_dir, SLOT_INDEX);
        expect(access(slot_path, 0) == 0,
               "slot 0 file exists on disk after save");
    }

    /* ── Reset the destination world, then load through the manager API ─ */
    /* The high-level load_full sizes its scratch buffers from the destination
     * structures' champion_count / object_count fields, so the destination
     * must already be initialized (callers cannot pass uninitialized
     * memory). Init both, then clear party_count for symmetry with the
     * saved pool. */
    nexus_v1_champions_init(&pool_after);
    pool_after.party_count = 0;
    pool_after.leader_index = -1;
    nexus_v1_world_init(&world_after);
    memset(&out_header, 0, sizeof(out_header));
    memset(diagnostic, 0, sizeof(diagnostic));

    load_result = nexus_v1_load_full(&mgr, SLOT_INDEX, &out_header,
                                      /*champion_pool=*/&pool_after,
                                      /*world=*/&world_after,
                                      diagnostic, sizeof(diagnostic));
    expect(load_result == NEXUS_SAVE_OK,
           "nexus_v1_load_full returns NEXUS_SAVE_OK for the just-saved slot");
    if (load_result == NEXUS_SAVE_OK) {
        /* The narrow gate: ONE world field round-trips. */
        expect(world_after.party_x == PARTY_X_MARKER,
               "world.party_x round-trips through slot 0 (single gate field)");
        /* Complementary header check: the slot marker field on the
         * loaded header agrees with the world field. */
        expect(out_header.party_x == world_after.party_x,
               "loaded save header.party_x agrees with rehydrated world.party_x");
        expect(out_header.current_level == world_after.party_level,
               "loaded save header.current_level agrees with rehydrated world.party_level");
        expect(out_header.magic == NEXUS_SAVE_MAGIC,
               "loaded save header.magic is the 'FNXS' marker");
        expect(out_header.version == NEXUS_SAVE_VERSION,
               "loaded save header.version matches NEXUS_SAVE_VERSION");
        expect(out_header.data_size > 0,
               "loaded save header.data_size reflects the serialized sections");
        /* CRC32 of the loaded sections is recomputed inside the load
         * path; if it disagreed, the load would have returned
         * NEXUS_SAVE_ERR_CRC, which the gate above already covered. */
    } else {
        fprintf(stderr, "load diagnostic: %s\n", diagnostic);
    }

    /* ── Cleanup ───────────────────────────────────────────────────── */
    snprintf(slot_path, sizeof(slot_path), "%s/nexus_save_%02u.dat", save_dir, SLOT_INDEX);
    NSR_UNLINK(slot_path);
    NSR_RMDIR(save_dir);
    NSR_RMDIR(root);

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_save_slot_roundtrip_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    puts("ok: nexus_v1_save_full -> nexus_v1_load_full round-trips world.party_x on slot 0");
    return 0;
}
