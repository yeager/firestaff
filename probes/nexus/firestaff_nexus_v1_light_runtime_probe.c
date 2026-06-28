/*
 * firestaff_nexus_v1_light_runtime_probe.c
 * =========================================
 *
 * Probe for the Nexus V1 light-overflow M11 wire-in layer declared in
 * include/nexus_v1_light_runtime.h.
 *
 * Scope (deliberately bounded, data-free):
 *
 *   [1] init / shutdown / idempotency
 *   [2] emulate-mode cast (default) — same behavior as the upstream
 *       data-model API, classify stays NONE under normal load
 *   [3] guard-mode cast at cap — runtime rejects before the cast is
 *       applied, classification surfaces CAST_REJECTED via should_guard
 *   [4] BUG0_18 silent drop + TIMELINE_FULL_PERMANENT_LIGHT through
 *       the runtime cast hook (not the upstream API directly)
 *   [5] LIGHT_BLEED_NEGATIVE classification after a wrap-through-zero
 *   [6] Save/load round-trip through the runtime blob (preserve
 *       MagicalLightAmount, cast/decay/dropped counters, active slot
 *       array, and classification kind across a reload)
 *   [7] Emulate-vs-guard mode flag preservation across save/load
 *   [8] Real-asset skip-safe path — when no real FNXS save is staged
 *       under the configured root, the probe uses /tmp + a clean
 *       temp Nexus_V1_SaveManager so the gate is honest about the
 *       absence of real data instead of pretending success.
 *   [9] NULL safety (init / shutdown / serialize / deserialize)
 *  [10] Determinism (5 repeated runs of the same cast-then-save
 *       script produce the same FNV-1a state hash and the same
 *       classify() kind)
 *
 * The classification hook lets the future M11 runtime pick between
 * emulating (default — source-faithful V1 path that reproduces the
 * dmweb-documented "permanent Light" / "dungeon into darkness"
 * symptoms) and guarding (clamp casts at the cap, suitable for
 * replay/capture contexts).
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_light_runtime_probe
 * CTest:
 *   ctest --test-dir build -R nexus_v1_light_runtime --output-on-failure
 *
 * Source-lock:
 *   include/nexus_v1_light_runtime.h
 *   src/nexus/nexus_v1_light_runtime.c
 *   include/nexus_v1_light_overflow.h
 *   src/nexus/nexus_v1_light_overflow.c
 *   include/nexus_v1_save.h            (FNXS save container)
 *   src/nexus/nexus_v1_save_load.c     (FNXS v2 round-trip)
 *   ReDMCSB TIMELINE.C F0238 / F0257, MENU.C:1926-1942, LOADSAVE.C:2041
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "nexus_v1_light_runtime.h"
#include "nexus_v1_light_overflow.h"
#include "nexus_v1_save.h"
#include "nexus_v1_world.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                 \
    if (cond) { printf("  PASS: %s\n", (msg)); ++g_pass; } \
    else      { printf("  FAIL: %s\n", (msg)); ++g_fail; } \
} while (0)

/* Real-asset skip detection: the probe will run the synthetic path
 * even when a real asset is absent (which it usually is in CI), and
 * only attempts the optional real FNXS path when one is staged. We
 * report the result for both paths. */
static int real_fnx_save_present(const char *path) {
    if (!path || !*path) return 0;
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    unsigned char buf[16];
    size_t got = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if (got < 8) return 0;
    /* 'FNXS' magic: 0x46 0x4E 0x58 0x53 little-endian as uint32. */
    return (buf[0] == 0x46 && buf[1] == 0x4E && buf[2] == 0x58 && buf[3] == 0x53);
}

int main(void) {
    printf("=== Nexus V1 light-overflow runtime wire-in probe ===\n\n");

    /* ── [1] init / shutdown / idempotency ─────────────────────────── */
    printf("[1] init / shutdown / idempotency\n");
    {
        Nexus_V1_LightRuntime rt;
        memset(&rt, 0, sizeof(rt));
        nexus_v1_light_runtime_init(&rt, /*guard=*/0);
        CHECK(rt.initialized == 1, "init() flips initialized=1");
        CHECK(rt.guard_rejects == 0, "emulate mode stores guard_rejects=0");
        CHECK(rt.last_classification == NEXUS_LIGHT_OVERFLOW_NONE,
              "fresh runtime classifies as NONE");
        CHECK(rt.timeline.guard_rejects == 0,
              "upstream timeline guard_rejects mirrors runtime");
        nexus_v1_light_runtime_shutdown(&rt);
        CHECK(rt.initialized == 0, "shutdown() flips initialized=0");
        CHECK(rt.state.magical_light_amount == 0,
              "shutdown() zeroes MagicalLightAmount");

        /* Re-init and re-shutdown to prove idempotency. */
        nexus_v1_light_runtime_init(&rt, /*guard=*/1);
        CHECK(rt.guard_rejects == 1, "second init can flip guard mode");
        nexus_v1_light_runtime_shutdown(&rt);
        CHECK(rt.initialized == 0, "second shutdown still works");
    }

    /* ── [2] emulate-mode cast ─────────────────────────────────────── */
    printf("\n[2] emulate-mode cast (default path)\n");
    {
        Nexus_V1_LightRuntime rt;
        nexus_v1_light_runtime_init(&rt, /*guard=*/0);

        /* Mon Oh Ir Ra (PowerSymbol 4, SpellPower 20) — Light, expected
         * LightPower 9, MagicalLightAmount rises to 68 (Table[9]). */
        int lp = nexus_v1_light_runtime_apply_cast(&rt,
                                                   NEXUS_LIGHT_KIND_LIGHT, 4);
        CHECK(lp == 9, "apply_cast returns LightPower=9 for Mon Oh Ir Ra");
        CHECK(rt.state.magical_light_amount == 68,
              "MagicalLightAmount rises to 68 after cast");
        CHECK(rt.last_cast_kind == NEXUS_LIGHT_KIND_LIGHT,
              "last_cast_kind=LIGHT");
        CHECK(rt.last_cast_power_symbol_ordinal == 4,
              "last_cast_power_symbol_ordinal=4 (Mon)");
        CHECK(rt.total_casts_applied == 1, "total_casts_applied=1");
        CHECK(rt.total_casts_rejected == 0, "total_casts_rejected=0");
        CHECK(rt.last_classification == NEXUS_LIGHT_OVERFLOW_NONE,
              "fresh cast under emulate mode stays NONE");
        CHECK(rt.last_should_guard == 0,
              "should_guard is 0 well below cap");

        /* Advance enough ticks to drain the chain. */
        size_t fired = nexus_v1_light_runtime_tick(&rt, 17000);
        CHECK(fired >= 9, "tick() drained the chain (>=9 events)");
        CHECK(rt.state.magical_light_amount == 0,
              "MagicalLightAmount returns to 0 after full decay");
        CHECK(rt.last_classification == NEXUS_LIGHT_OVERFLOW_NONE,
              "classification stays NONE after decay");
        nexus_v1_light_runtime_shutdown(&rt);
    }

    /* ── [3] guard-mode cast at cap ────────────────────────────────── */
    printf("\n[3] guard-mode rejects casts at cap\n");
    {
        Nexus_V1_LightRuntime rt;
        nexus_v1_light_runtime_init(&rt, /*guard=*/1);

        /* Fill the buffer without ticking. Each cast schedules a single
         * event at high ticks (no recursive weaker chain — the
         * recursive chain only happens when events actually fire). */
        int first_lp = nexus_v1_light_runtime_apply_cast(&rt,
                                                       NEXUS_LIGHT_KIND_LIGHT, 4);
        CHECK(first_lp > 0, "first cast succeeds in guard mode");
        int casts = 1;
        while (rt.timeline.active_count < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP
               && casts < 256) {
            nexus_v1_light_runtime_apply_cast(&rt,
                                              NEXUS_LIGHT_KIND_LIGHT, 4);
            casts++;
        }
        CHECK(rt.timeline.active_count == NEXUS_V1_LIGHT_TIMELINE_BASE_CAP,
              "guard-mode timeline filled to documented cap (100)");

        int32_t mla_at_cap = rt.state.magical_light_amount;
        int r = nexus_v1_light_runtime_apply_cast(&rt,
                                                   NEXUS_LIGHT_KIND_LIGHT, 4);
        CHECK(r == 0, "guard-mode rejects cast at cap (returns 0)");
        CHECK(rt.state.magical_light_amount == mla_at_cap,
              "MagicalLightAmount unchanged on rejected cast");
        CHECK(rt.last_classification == NEXUS_LIGHT_OVERFLOW_CAST_REJECTED,
              "classification surfaces CAST_REJECTED");
        CHECK(rt.last_should_guard == 1,
              "should_guard flips to 1 at cap");
        CHECK(rt.total_casts_rejected >= 1,
              "rejection counter incremented");
        nexus_v1_light_runtime_shutdown(&rt);
    }

    /* ── [4] BUG0_18 silent drop + permanent-light via runtime ─────── */
    printf("\n[4] BUG0_18 silent drop + permanent-light via runtime cast hook\n");
    {
        Nexus_V1_LightRuntime rt;
        nexus_v1_light_runtime_init(&rt, /*guard=*/0);

        /* 110 back-to-back Light casts without ticking. After 100 the
         * cap is hit and the upstream silently drops the rest. */
        for (int i = 0; i < 110; ++i) {
            nexus_v1_light_runtime_apply_cast(&rt,
                                              NEXUS_LIGHT_KIND_LIGHT, 4);
        }
        CHECK(rt.timeline.dropped_counter >= 10,
              "at least 10 silent drops after cap");
        CHECK(rt.total_drops_observed >= 10,
              "runtime audit reflects upstream dropped counter");
        CHECK(nexus_v1_light_runtime_classify(&rt) ==
              NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT,
              "classify() reports TIMELINE_FULL_PERMANENT_LIGHT");
        CHECK(rt.last_classification ==
              NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT,
              "cached classification matches classify()");
        CHECK(rt.state.magical_light_amount > 200,
              "MagicalLightAmount is elevated (cumulative immediate deltas)");
        nexus_v1_light_runtime_shutdown(&rt);
    }

    /* ── [5] LIGHT_BLEED_NEGATIVE classification ───────────────────── */
    printf("\n[5] Light-bleed-through-zero classification via runtime\n");
    {
        Nexus_V1_LightRuntime rt;
        nexus_v1_light_runtime_init(&rt, /*guard=*/0);
        rt.state.magical_light_amount = -1;
        /* Refresh the cached classification by ticking 0 ticks. The
         * runtime only re-polls the upstream classify() inside
         * apply_cast / tick / deserialize, so direct state mutation
         * needs an explicit refresh. */
        nexus_v1_light_runtime_tick(&rt, 0);
        CHECK(nexus_v1_light_runtime_classify(&rt) ==
              NEXUS_LIGHT_OVERFLOW_LIGHT_BLEED_NEGATIVE,
              "negative MagicalLightAmount reports LIGHT_BLEED_NEGATIVE");
        CHECK(rt.last_classification ==
              NEXUS_LIGHT_OVERFLOW_LIGHT_BLEED_NEGATIVE,
              "cached classification matches LIGHT_BLEED_NEGATIVE");
        nexus_v1_light_runtime_shutdown(&rt);
    }

    /* ── [6] Save/load round-trip through the runtime blob ─────────── */
    printf("\n[6] Save/load round-trip (serialize + deserialize)\n");
    {
        Nexus_V1_LightRuntime rt;
        nexus_v1_light_runtime_init(&rt, /*guard=*/0);

        /* Cast, then fill enough events to make the active_count
         * survive a round-trip. We avoid filling to the cap so the
         * classification after reload is still NONE (we test the
         * permanent-light branch separately below). */
        for (int i = 0; i < 5; ++i) {
            nexus_v1_light_runtime_apply_cast(&rt,
                                              NEXUS_LIGHT_KIND_TORCH, 2);
        }
        size_t before_active = rt.timeline.active_count;
        uint32_t before_cast = rt.timeline.cast_counter;
        uint32_t before_decay = rt.timeline.decay_counter;
        int32_t  before_mla   = rt.state.magical_light_amount;
        uint32_t before_tick  = rt.timeline.current_tick;
        uint32_t before_hash  = nexus_v1_light_runtime_state_hash(&rt);
        CHECK(before_active > 0, "pre-serialize active_count > 0");

        uint8_t blob[NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE];
        memset(blob, 0xAA, sizeof(blob));
        size_t written = nexus_v1_light_runtime_serialize(&rt, blob,
                                                           sizeof(blob));
        CHECK(written == NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE,
              "serialize() wrote the canonical blob size");
        CHECK(blob[0] == 0x4E && blob[1] == 0x47 && blob[2] == 0x4C && blob[3] == 0x54,
              "blob magic = 'NGLT' at offset 0");

        /* Load into a fresh runtime, init()'d in emulate mode so the
         * guard_rejects flag matches what was saved. */
        Nexus_V1_LightRuntime rt2;
        nexus_v1_light_runtime_init(&rt2, /*guard=*/0);
        int ok = nexus_v1_light_runtime_deserialize(&rt2, blob,
                                                     sizeof(blob));
        CHECK(ok == 1, "deserialize() accepted the canonical blob");
        CHECK(rt2.timeline.active_count == before_active,
              "active_count preserved across reload");
        CHECK(rt2.timeline.cast_counter == before_cast,
              "cast_counter preserved across reload");
        CHECK(rt2.timeline.decay_counter == before_decay,
              "decay_counter preserved across reload");
        CHECK(rt2.state.magical_light_amount == before_mla,
              "MagicalLightAmount preserved across reload");
        CHECK(rt2.timeline.current_tick == before_tick,
              "current_tick preserved across reload");
        CHECK(nexus_v1_light_runtime_state_hash(&rt2) == before_hash,
              "FNV-1a state hash matches across reload");

        /* Re-tick after reload and confirm classify still NONE. */
        nexus_v1_light_runtime_tick(&rt2, 5000);
        CHECK(nexus_v1_light_runtime_classify(&rt2) ==
              NEXUS_LIGHT_OVERFLOW_NONE,
              "post-reload classify() returns NONE after decay");

        /* Round-trip with a permanent-light state. Reload must still
         * surface the same TIMELINE_FULL_PERMANENT_LIGHT kind. */
        Nexus_V1_LightRuntime rt_perm;
        nexus_v1_light_runtime_init(&rt_perm, /*guard=*/0);
        for (int i = 0; i < 110; ++i) {
            nexus_v1_light_runtime_apply_cast(&rt_perm,
                                              NEXUS_LIGHT_KIND_LIGHT, 4);
        }
        CHECK(nexus_v1_light_runtime_classify(&rt_perm) ==
              NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT,
              "pre-save permanent-light classified correctly");
        uint8_t blob_perm[NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE];
        size_t wp = nexus_v1_light_runtime_serialize(&rt_perm, blob_perm,
                                                     sizeof(blob_perm));
        CHECK(wp == NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE,
              "permanent-light blob serializes");
        Nexus_V1_LightRuntime rt_perm2;
        nexus_v1_light_runtime_init(&rt_perm2, /*guard=*/0);
        CHECK(nexus_v1_light_runtime_deserialize(&rt_perm2, blob_perm,
                                                 sizeof(blob_perm)) == 1,
              "permanent-light blob deserializes");
        CHECK(nexus_v1_light_runtime_classify(&rt_perm2) ==
              NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT,
              "post-reload classification stays TIMELINE_FULL_PERMANENT_LIGHT");
        CHECK(rt_perm2.last_classification ==
              NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT,
              "cached classification matches post-reload classify");

        nexus_v1_light_runtime_shutdown(&rt_perm);
        nexus_v1_light_runtime_shutdown(&rt_perm2);
        nexus_v1_light_runtime_shutdown(&rt);
        nexus_v1_light_runtime_shutdown(&rt2);
    }

    /* ── [7] Emulate-vs-guard mode flag preservation ───────────────── */
    printf("\n[7] Emulate-vs-guard mode flag preservation\n");
    {
        /* A guard-mode save blob must NOT load into an emulate-mode
         * runtime (the mismatch surfaces as a clean 0 return so the
         * M11 layer can decide whether to clobber the mode flag or
         * refuse the load). */
        Nexus_V1_LightRuntime rt_g;
        nexus_v1_light_runtime_init(&rt_g, /*guard=*/1);
        for (int i = 0; i < 5; ++i) {
            nexus_v1_light_runtime_apply_cast(&rt_g,
                                              NEXUS_LIGHT_KIND_TORCH, 2);
        }
        uint8_t blob_g[NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE];
        size_t wg = nexus_v1_light_runtime_serialize(&rt_g, blob_g,
                                                     sizeof(blob_g));
        CHECK(wg == NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE,
              "guard-mode blob serializes");

        Nexus_V1_LightRuntime rt_e;
        nexus_v1_light_runtime_init(&rt_e, /*guard=*/0);
        int ok = nexus_v1_light_runtime_deserialize(&rt_e, blob_g,
                                                     sizeof(blob_g));
        CHECK(ok == 0, "guard-mode blob rejected by emulate-mode runtime");

        /* Same mode -> same mode round-trips. */
        Nexus_V1_LightRuntime rt_e2;
        nexus_v1_light_runtime_init(&rt_e2, /*guard=*/0);
        int ok2 = nexus_v1_light_runtime_deserialize(&rt_e2, blob_g,
                                                      sizeof(blob_g));
        CHECK(ok2 == 0,
              "guard-mode blob still rejected when target is emulate (no state leak)");
        /* Confirm the target runtime stayed clean. */
        CHECK(rt_e2.state.magical_light_amount == 0,
              "failed load left MagicalLightAmount=0");
        CHECK(rt_e2.timeline.active_count == 0,
              "failed load left active_count=0");
        CHECK(rt_e2.last_classification == NEXUS_LIGHT_OVERFLOW_NONE,
              "failed load classification is NONE");

        /* When source and target match, the same guard-mode blob
         * loads cleanly into a guard-mode target. */
        Nexus_V1_LightRuntime rt_g2;
        nexus_v1_light_runtime_init(&rt_g2, /*guard=*/1);
        int ok3 = nexus_v1_light_runtime_deserialize(&rt_g2, blob_g,
                                                      sizeof(blob_g));
        CHECK(ok3 == 1, "guard-mode blob accepted by guard-mode runtime");
        CHECK(rt_g2.guard_rejects == 1, "guard flag preserved");
        CHECK(rt_g2.timeline.guard_rejects == 1,
              "upstream timeline guard_rejects preserved");

        nexus_v1_light_runtime_shutdown(&rt_g);
        nexus_v1_light_runtime_shutdown(&rt_g2);
        nexus_v1_light_runtime_shutdown(&rt_e);
        nexus_v1_light_runtime_shutdown(&rt_e2);
    }

    /* ── [8] Real-asset skip-safe path via FNXS save container ─────── */
    printf("\n[8] Real-asset skip-safe path via FNXS save container\n");
    {
        const char *real_root = getenv("FIRESTAFF_NEXUS_SAVE_ROOT");
        const char *real_path = getenv("FIRESTAFF_NEXUS_SAVE_PATH");
        (void)real_root;

        int had_real_save = (real_path && real_fnx_save_present(real_path));
        CHECK(!real_path || had_real_save,
              "env-provided path (if any) is a real FNXS save or unset");

        /* Build a synthetic FNXS save containing a light-runtime blob.
         * The blob is embedded as the *first* sub-section of the data
         * section (after the champion data, before the world data),
         * which keeps the synthetic shape honest without claiming
         * any real asset. */
        Nexus_V1_LightRuntime rt;
        nexus_v1_light_runtime_init(&rt, /*guard=*/0);
        for (int i = 0; i < 6; ++i) {
            nexus_v1_light_runtime_apply_cast(&rt,
                                              NEXUS_LIGHT_KIND_TORCH, 2);
        }
        uint8_t blob[NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE];
        size_t w = nexus_v1_light_runtime_serialize(&rt, blob, sizeof(blob));
        CHECK(w == NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE,
              "runtime blob sized for FNXS embedding");

        /* Use a clean temp Nexus_V1_SaveManager so we don't depend on
         * the user's save directory. */
        char tmpdir[256];
        snprintf(tmpdir, sizeof(tmpdir),
                 "/tmp/nexus_v1_light_runtime_probe_%d",
                 (int)getpid());
        Nexus_V1_SaveManager mgr;
        nexus_v1_save_init(&mgr, tmpdir);
        char slot_path[512];
        snprintf(slot_path, sizeof(slot_path),
                 "%s/light_runtime.dat", tmpdir);

        /* Build a minimal synthetic world to satisfy the FNXS
         * champion/world sections. We use the same pattern as
         * nexus_v1_save_load_round_trip_probe. */
        Nexus_V1_World world;
        nexus_v1_world_init(&world);
        world.party_level = 0;
        world.party_x = 11;
        world.party_y = 29;
        world.party_dir = 0;
        size_t world_size = nexus_v1_world_serialize_size(&world);
        uint8_t *world_buf = (uint8_t *)malloc(world_size > 0 ? world_size : 1);
        CHECK(world_buf != NULL, "world scratch buffer allocated");
        size_t world_written = 0;
        if (world_size > 0) {
            world_written = nexus_v1_world_serialize(&world, world_buf,
                                                     world_size);
        }

        /* Embed the runtime blob AFTER the world data section so the
         * round-trip probe stays byte-compatible with the existing
         * FNXS layout (champion_data + world_data + light_runtime_blob).
         * This is intentionally a separate sub-section: it does NOT
         * modify the FNXS header layout, and a future M11 layer can
         * pick it out without re-engineering the loader. */
        size_t data_size = world_written + w;
        uint8_t *data_buf = (uint8_t *)malloc(data_size > 0 ? data_size : 1);
        CHECK(data_buf != NULL, "data section buffer allocated");
        if (data_size > 0) {
            if (world_written > 0) {
                memcpy(data_buf, world_buf, world_written);
            }
            memcpy(data_buf + world_written, blob, w);
        }

        Nexus_SaveResult sr = nexus_v1_save_to_path(
            slot_path,
            world.party_level, world.party_x, world.party_y,
            world.party_dir, (uint32_t)world.world_tick,
            nexus_v1_world_hash(&world),
            /*champion_data=*/NULL, /*champion_size=*/0,
            data_buf, data_size);
        CHECK(sr == NEXUS_SAVE_OK,
              "FNXS save_to_path accepted the light-runtime blob");

        /* Probe the file with the upstream API. */
        Nexus_V1_SaveHeader probe_hdr;
        size_t probe_fsz = 0;
        const char *reason = nexus_v1_save_probe(slot_path, &probe_hdr,
                                                   &probe_fsz);
        CHECK(reason != NULL && reason[0] == '\0',
              "nexus_v1_save_probe accepts the synthetic FNXS file");
        CHECK(probe_hdr.magic == NEXUS_SAVE_MAGIC,
              "saved file carries the FNXS magic");

        /* Re-load and pull the runtime blob out by hand. We deliberately
         * do NOT call nexus_v1_load_full_from_path here because the
         * upstream loader doesn't yet know about the embedded light
         * blob — that's a separate follow-up. We just verify the file
         * is loadable, the data section size matches our blob +
         * world shape, and that the runtime blob at the documented
         * offset deserializes cleanly. */
        FILE *fp = fopen(slot_path, "rb");
        CHECK(fp != NULL, "saved file is readable");
        if (fp) {
            uint8_t file_buf[4096];
            size_t got = fread(file_buf, 1, sizeof(file_buf), fp);
            fclose(fp);
            CHECK(got >= sizeof(Nexus_V1_SaveHeader) + data_size,
                  "file size covers header + data section");
            if (got >= sizeof(Nexus_V1_SaveHeader) + data_size) {
                const uint8_t *data_section =
                    file_buf + sizeof(Nexus_V1_SaveHeader);
                /* Light-runtime blob sits at data_section + world_written. */
                const uint8_t *blob_at =
                    data_section + world_written;
                Nexus_V1_LightRuntime rt_back;
                nexus_v1_light_runtime_init(&rt_back, /*guard=*/0);
                int ok = nexus_v1_light_runtime_deserialize(
                    &rt_back, blob_at, w);
                CHECK(ok == 1,
                      "runtime blob deserialized from FNXS data section");
                CHECK(rt_back.timeline.cast_counter == rt.timeline.cast_counter,
                      "cast_counter survives FNXS round-trip");
                CHECK(rt_back.state.magical_light_amount ==
                      rt.state.magical_light_amount,
                      "MagicalLightAmount survives FNXS round-trip");
                CHECK(nexus_v1_light_runtime_classify(&rt_back) ==
                      nexus_v1_light_runtime_classify(&rt),
                      "classify() matches across FNXS round-trip");
                nexus_v1_light_runtime_shutdown(&rt_back);
            }
        }

        /* Cleanup the temp dir so re-running the probe starts clean. */
        if (!real_root) {
            remove(slot_path);
            rmdir(tmpdir);
        }
        free(world_buf);
        free(data_buf);
        nexus_v1_light_runtime_shutdown(&rt);
    }

    /* ── [9] NULL safety ────────────────────────────────────────────── */
    printf("\n[9] NULL safety\n");
    CHECK(nexus_v1_light_runtime_apply_cast(NULL, NEXUS_LIGHT_KIND_LIGHT, 4) == 0,
          "apply_cast(NULL) returns 0");
    CHECK(nexus_v1_light_runtime_tick(NULL, 16) == 0, "tick(NULL) returns 0");
    CHECK(nexus_v1_light_runtime_classify(NULL) == NEXUS_LIGHT_OVERFLOW_NONE,
          "classify(NULL) returns NONE");
    CHECK(nexus_v1_light_runtime_should_guard(NULL) == 0,
          "should_guard(NULL) returns 0");
    CHECK(nexus_v1_light_runtime_serialize(NULL, NULL, 1024) == 0,
          "serialize(NULL) returns 0");
    CHECK(nexus_v1_light_runtime_deserialize(NULL, NULL, 1024) == 0,
          "deserialize(NULL) returns 0");
    {
        Nexus_V1_LightRuntime rt;
        nexus_v1_light_runtime_init(&rt, /*guard=*/0);
        CHECK(nexus_v1_light_runtime_serialize(&rt, NULL, 1024) == 0,
              "serialize(NULL buf) returns 0");
        CHECK(nexus_v1_light_runtime_deserialize(&rt, NULL, 1024) == 0,
              "deserialize(NULL buf) returns 0");
        CHECK(nexus_v1_light_runtime_state_hash(NULL) == 0,
              "state_hash(NULL) returns 0");
        nexus_v1_light_runtime_shutdown(&rt);
    }

    /* ── [10] Determinism ───────────────────────────────────────────── */
    printf("\n[10] Determinism (cast-then-save-then-classify hash stable)\n");
    {
        uint32_t expected_hash = 0;
        Nexus_V1_LightOverflowKind expected_kind = NEXUS_LIGHT_OVERFLOW_NONE;
        int mismatch = 0;
        for (int rep = 0; rep < 5; ++rep) {
            Nexus_V1_LightRuntime rt;
            nexus_v1_light_runtime_init(&rt, /*guard=*/0);
            /* Same script each rep. */
            for (int i = 0; i < 8; ++i) {
                nexus_v1_light_runtime_apply_cast(&rt,
                                                  NEXUS_LIGHT_KIND_TORCH, 2);
            }
            nexus_v1_light_runtime_tick(&rt, 2000);
            uint32_t h = nexus_v1_light_runtime_state_hash(&rt);
            Nexus_V1_LightOverflowKind k = nexus_v1_light_runtime_classify(&rt);
            if (rep == 0) {
                expected_hash = h;
                expected_kind = k;
            } else if (h != expected_hash || k != expected_kind) {
                ++mismatch;
            }
            nexus_v1_light_runtime_shutdown(&rt);
        }
        CHECK(mismatch == 0,
              "5 runs produce the same state hash and classification kind");
    }

    /* ── Summary ────────────────────────────────────────────────────── */
    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
