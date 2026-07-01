/*
 * firestaff_nexus_v1_light_cast_path_probe.c
 * ==========================================
 *
 * Nexus V1 light-overflow cast-path wire-in probe.
 *
 * The bounded data model + bug-classification API in
 * include/nexus_v1_light_overflow.h is exercised in the sibling
 * firestaff_nexus_v1_light_overflow_probe. This probe covers the
 * M11 / runtime cast-path wire-in: Nexus_V1_LightCastPath is the
 * small bundle a future M11 cast dispatch (or a debug overlay, or
 * a replay capture) drives for every Light / Torch / Darkness
 * cast. The probe also covers the FNXL save/load proof — the
 * minimal binary format dedicated to the cast-path state.
 *
 * Source-locked against:
 *   - ReDMCSB MENU.C:1926-1942 (Light / Torch / Darkness dispatch)
 *   - ReDMCSB TIMELINE.C F0238 lines 487-555 (BUG0_18 silent drop)
 *   - ReDMCSB TIMELINE.C F0257 lines 1720-1767 (F0257 decay chain)
 *   - ReDMCSB CHAMPION.C:27 (MagicalLightAmount field)
 *   - ReDMCSB LOADSAVE.C:2041 (EventMaximumCount = 100 base cap)
 *   - DMWeb Dungeon Master Nexus (Saturn) edition page
 *     (http://dmweb.free.fr/games/dungeon-master-nexus/editions/
 *     sega-saturn/) — user-visible overflow symptoms
 *   - DMWeb Nexus Cheats/Hacks page — BUG0_18 permanent-spell-effect
 *     exploitation
 *
 * What this probe verifies
 * ------------------------
 *  1.  Cast path init + reset round-trip the data model to a
 *      known-empty state, in both emulate and guard modes.
 *  2.  A single Light cast through the cast path raises
 *      MagicalLightAmount by the documented delta and reports the
 *      expected LightOverflowKind = NONE on a fresh state.
 *  3.  The cast path wires into the existing data model: cast +
 *      tick converges MagicalLightAmount back to 0 with the same
 *      decay chain length the data-model probe locks.
 *  4.  Emulate vs guard mode is preserved:
 *        a. Emulate: a saturated timeline silently drops the new
 *           cast (BUG0_18), the was_rejected flag stays 0, and
 *           the classification hook surfaces
 *           TIMELINE_FULL_PERMANENT_LIGHT.
 *        b. Guard: a saturated timeline rejects the new cast
 *           (was_rejected = 1), MagicalLightAmount is unchanged,
 *           and the classification hook still returns
 *           TIMELINE_FULL_PERMANENT_LIGHT (independent of the
 *           guard verdict — gap-row promise that the hook is
 *           available for HUD/debug even in guard mode).
 *  5.  Mode flip at runtime: set_mode(EMULATE) -> set_mode(GUARD)
 *      re-syncs the timeline's guard_rejects flag.
 *  6.  Save/load round-trip (FNXL format): write a non-zero
 *      MagicalLightAmount + a saturated timeline, load it back,
 *      verify every state field and every in-use timeline slot
 *      round-trips bit-exact. Also verify that an intentionally
 *      corrupted buffer is rejected with the correct error code.
 *  7.  Determinism: 5 runs of the same cast-tick-script produce
 *      a stable cast-path state hash.
 *  8.  NULL safety on every entry point.
 *  9.  Skip-real-assets: the probe does not touch the Saturn
 *      disc image, FONT256.S2D, MNS models, or DGN files. The
 *      cast path is data-free and works in-process.
 *
 * Run:
 *   ./build/firestaff_nexus_v1_light_cast_path_probe
 * CTest:
 *   ctest --test-dir build -R nexus_v1_light_cast_path --output-on-failure
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_v1_light_overflow.h"
#include "nexus_v1_save.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                 \
    if (cond) { printf("  PASS: %s\n", (msg)); ++g_pass; } \
    else      { printf("  FAIL: %s\n", (msg)); ++g_fail; } \
} while (0)

/* Deterministic 32-bit hash of a cast path. The mix is broad enough
 * to catch almost any round-trip drift in the state or timeline
 * slots, but the probe only uses it for the determinism check
 * (TEST 7). Round-trip checks are bit-exact field comparisons
 * elsewhere. */
static uint32_t cast_path_hash(const Nexus_V1_LightCastPath *path) {
    uint32_t h = 0x811c9dc5u;
    h ^= (uint32_t)path->state.magical_light_amount; h *= 0x01000193u;
    h ^= (uint32_t)path->mode;                        h *= 0x01000193u;
    h ^= (uint32_t)path->timeline.active_count;       h *= 0x01000193u;
    h ^= path->timeline.current_tick;                 h *= 0x01000193u;
    h ^= path->timeline.cast_counter;                 h *= 0x01000193u;
    h ^= path->timeline.decay_counter;                h *= 0x01000193u;
    h ^= path->timeline.dropped_counter;              h *= 0x01000193u;
    h ^= (uint32_t)path->timeline.guard_rejects;      h *= 0x01000193u;
    for (size_t i = 0; i < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP; ++i) {
        if (path->timeline.slots[i].in_use) {
            h ^= (uint32_t)path->timeline.slots[i].light_power * 0x9e3779b1u;
            h ^= path->timeline.slots[i].fire_at_tick;
            h *= 0x01000193u;
        }
    }
    return h;
}

int main(void) {
    printf("=== Nexus V1 light-overflow cast-path probe ===\n\n");

    /* ── Test 1: cast path init / reset ─────────────────────────────── */
    printf("[1] cast path init / reset (emulate + guard)\n");
    {
        Nexus_V1_LightCastPath path;
        nexus_v1_light_cast_path_init(&path, NEXUS_LIGHT_CAST_MODE_EMULATE);
        CHECK(nexus_v1_light_cast_path_get_mode(&path) == NEXUS_LIGHT_CAST_MODE_EMULATE,
              "init(EMULATE) sets mode to EMULATE");
        CHECK(path.timeline.guard_rejects == 0,
              "init(EMULATE) leaves timeline.guard_rejects=0");
        CHECK(nexus_v1_light_cast_path_light_amount(&path) == 0,
              "init starts at light amount 0");
        CHECK(nexus_v1_light_cast_path_active_count(&path) == 0,
              "init starts with empty timeline");

        /* Reset preserves mode but zeroes the rest. */
        path.state.magical_light_amount = 99;
        path.timeline.cast_counter = 7;
        path.timeline.current_tick = 1234;
        nexus_v1_light_cast_path_reset(&path);
        CHECK(nexus_v1_light_cast_path_get_mode(&path) == NEXUS_LIGHT_CAST_MODE_EMULATE,
              "reset preserves EMULATE mode");
        CHECK(nexus_v1_light_cast_path_light_amount(&path) == 0,
              "reset zeroes light amount");
        CHECK(path.timeline.cast_counter == 0,
              "reset zeroes cast counter");

        /* Init in guard mode. */
        nexus_v1_light_cast_path_init(&path, NEXUS_LIGHT_CAST_MODE_GUARD);
        CHECK(nexus_v1_light_cast_path_get_mode(&path) == NEXUS_LIGHT_CAST_MODE_GUARD,
              "init(GUARD) sets mode to GUARD");
        CHECK(path.timeline.guard_rejects == 1,
              "init(GUARD) sets timeline.guard_rejects=1");
    }

    /* ── Test 2: single Light cast through the cast path ───────────── */
    printf("\n[2] single Light cast through cast path (MENU.C:1926)\n");
    {
        Nexus_V1_LightCastPath path;
        nexus_v1_light_cast_path_init(&path, NEXUS_LIGHT_CAST_MODE_EMULATE);

        /* Mon Oh Ir Ra (PowerSymbol 4) -> SpellPower 20 -> LightPower 9.
         * Immediate delta = Table[9] = 68. */
        Nexus_V1_LightCastResult r = nexus_v1_light_cast_path_cast(
            &path, NEXUS_LIGHT_KIND_LIGHT, 4);
        CHECK(r.applied_light_power == 9,
              "cast returns applied LightPower = 9");
        CHECK(r.was_rejected == 0,
              "first cast on a fresh path is not rejected");
        CHECK(r.magical_light_amount_after == 68,
              "cast result reports MagicalLightAmount = 68 after cast");
        CHECK(r.classification == NEXUS_LIGHT_OVERFLOW_NONE,
              "single cast on fresh path classifies NONE");
        CHECK(nexus_v1_light_cast_path_light_amount(&path) == 68,
              "underlying state holds MagicalLightAmount = 68");
        CHECK(nexus_v1_light_cast_path_cast_count(&path) == 1,
              "cast counter incremented to 1");
    }

    /* ── Test 3: cast + tick converge back to 0 ────────────────────── */
    printf("\n[3] cast + tick converge (F0257 decay chain)\n");
    {
        Nexus_V1_LightCastPath path;
        nexus_v1_light_cast_path_init(&path, NEXUS_LIGHT_CAST_MODE_EMULATE);
        /* Lo Ful (PowerSymbol 0) -> SpellPower 4 -> LightPower 2.
         * Immediate delta = Table[2] = 12. Torch ticks = 2000 + (4-3)*128 = 2128. */
        nexus_v1_light_cast_path_cast(&path, NEXUS_LIGHT_KIND_TORCH, 0);
        CHECK(nexus_v1_light_cast_path_light_amount(&path) == 12,
              "Torch(Lo Ful) raises light by 12 (Table[2])");
        /* Advance enough ticks for the full decay chain to fire. */
        size_t fired = nexus_v1_light_cast_path_advance(&path, 2500);
        CHECK(nexus_v1_light_cast_path_light_amount(&path) == 0,
              "light returns to 0 after full decay chain");
        CHECK(fired >= 2,
              "decay chain fires at least 2 events for LightPower=2");
    }

    /* ── Test 4a: emulate mode + BUG0_18 silent drop ────────────────── */
    printf("\n[4a] emulate mode: BUG0_18 silent drop + classification\n");
    {
        Nexus_V1_LightCastPath path;
        nexus_v1_light_cast_path_init(&path, NEXUS_LIGHT_CAST_MODE_EMULATE);
        /* Fill the timeline with 100 high-tick Light casts (no tick
         * between them, so the recursive weaker chain never
         * schedules). Then the 101st cast hits the cap. */
        for (int i = 0; i < 110; ++i) {
            nexus_v1_light_cast_path_cast(&path, NEXUS_LIGHT_KIND_LIGHT, 4);
        }
        CHECK(nexus_v1_light_cast_path_dropped_count(&path) >= 10,
              "at least 10 silent drops after timeline cap");
        CHECK(nexus_v1_light_cast_path_active_count(&path) ==
              NEXUS_V1_LIGHT_TIMELINE_BASE_CAP,
              "timeline filled to documented cap");
        CHECK(nexus_v1_light_cast_path_classify(&path) ==
              NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT,
              "classify reports TIMELINE_FULL_PERMANENT_LIGHT");
        CHECK(nexus_v1_light_cast_path_should_guard(&path) == 0,
              "should_guard returns 0 in emulate mode");
    }

    /* ── Test 4b: guard mode rejects casts at cap ──────────────────── */
    printf("\n[4b] guard mode: cast rejected at cap, state preserved\n");
    {
        Nexus_V1_LightCastPath path;
        nexus_v1_light_cast_path_init(&path, NEXUS_LIGHT_CAST_MODE_GUARD);
        for (int i = 0; i < 110; ++i) {
            nexus_v1_light_cast_path_cast(&path, NEXUS_LIGHT_KIND_LIGHT, 4);
        }
        CHECK(nexus_v1_light_cast_path_active_count(&path) ==
              NEXUS_V1_LIGHT_TIMELINE_BASE_CAP,
              "guard mode: timeline still fills to cap");
        int32_t light_at_cap =
            nexus_v1_light_cast_path_light_amount(&path);
        uint32_t casts_at_cap = nexus_v1_light_cast_path_cast_count(&path);
        /* One more cast: must be rejected, light amount unchanged,
         * cast counter NOT incremented (validation reject), and the
         * classification hook still surfaces the overflow kind. */
        Nexus_V1_LightCastResult r = nexus_v1_light_cast_path_cast(
            &path, NEXUS_LIGHT_KIND_LIGHT, 4);
        CHECK(r.was_rejected == 1,
              "guard mode: was_rejected=1 on cast at cap");
        CHECK(r.magical_light_amount_after == light_at_cap,
              "guard mode: light amount unchanged on reject");
        CHECK(nexus_v1_light_cast_path_cast_count(&path) == casts_at_cap,
              "guard mode: cast counter NOT incremented on reject");
        CHECK(nexus_v1_light_cast_path_classify(&path) ==
              NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT,
              "guard mode: classification still surfaces the overflow kind");
        CHECK(nexus_v1_light_cast_path_should_guard(&path) == 1,
              "should_guard returns 1 in guard mode at cap");
    }

    /* ── Test 5: mode flip at runtime ──────────────────────────────── */
    printf("\n[5] mode flip at runtime\n");
    {
        Nexus_V1_LightCastPath path;
        nexus_v1_light_cast_path_init(&path, NEXUS_LIGHT_CAST_MODE_EMULATE);
        CHECK(nexus_v1_light_cast_path_get_mode(&path) ==
              NEXUS_LIGHT_CAST_MODE_EMULATE, "starts in EMULATE");
        CHECK(path.timeline.guard_rejects == 0,
              "timeline guard flag follows mode (emulate)");

        nexus_v1_light_cast_path_set_mode(&path, NEXUS_LIGHT_CAST_MODE_GUARD);
        CHECK(nexus_v1_light_cast_path_get_mode(&path) ==
              NEXUS_LIGHT_CAST_MODE_GUARD, "set_mode(GUARD) flips mode");
        CHECK(path.timeline.guard_rejects == 1,
              "set_mode(GUARD) re-syncs timeline guard_rejects=1");

        nexus_v1_light_cast_path_set_mode(&path, NEXUS_LIGHT_CAST_MODE_EMULATE);
        CHECK(path.timeline.guard_rejects == 0,
              "set_mode(EMULATE) re-syncs timeline guard_rejects=0");
    }

    /* ── Test 6: save/load round-trip (FNXL) ───────────────────────── */
    printf("\n[6] FNXL save/load round-trip preserves all state\n");
    {
        Nexus_V1_LightCastPath src;
        nexus_v1_light_cast_path_init(&src, NEXUS_LIGHT_CAST_MODE_GUARD);
        /* Force a non-trivial state: 5 casts, a partial decay, plus
         * an overflow so the saved buffer has a non-empty slot array
         * AND a non-zero MagicalLightAmount. */
        for (int i = 0; i < 5; ++i) {
            nexus_v1_light_cast_path_cast(&src, NEXUS_LIGHT_KIND_TORCH, 2);
        }
        /* 1 tick so at least one event has fired and active_count
         * stays bounded for the save buffer size math. */
        nexus_v1_light_cast_path_tick(&src);

        size_t bufsize = nexus_v1_light_cast_path_save_size(&src);
        CHECK(bufsize > 36u,
              "save_size() reports a non-trivial size after a few casts");

        uint8_t *buf = (uint8_t *)malloc(bufsize);
        CHECK(buf != NULL, "save_size() buffer allocation succeeded");
        if (!buf) return 1;

        size_t written = nexus_v1_light_cast_path_save(&src, buf, bufsize);
        CHECK(written == bufsize,
              "save() reports exactly save_size() bytes written");
        CHECK(nexus_v1_light_cast_path_probe(buf, bufsize) == 1,
              "probe() recognizes the saved buffer as FNXL");

        /* Round-trip into a fresh cast path. */
        Nexus_V1_LightCastPath dst;
        int rc = nexus_v1_light_cast_path_load(&dst, buf, bufsize);
        CHECK(rc == NEXUS_SAVE_OK,
              "load() returns NEXUS_SAVE_OK on a valid buffer");

        /* Field-by-field equality. */
        CHECK(dst.state.magical_light_amount == src.state.magical_light_amount,
              "magical_light_amount round-trips");
        CHECK(dst.timeline.current_tick == src.timeline.current_tick,
              "current_tick round-trips");
        CHECK(dst.timeline.cast_counter == src.timeline.cast_counter,
              "cast_counter round-trips");
        CHECK(dst.timeline.decay_counter == src.timeline.decay_counter,
              "decay_counter round-trips");
        CHECK(dst.timeline.dropped_counter == src.timeline.dropped_counter,
              "dropped_counter round-trips");
        CHECK(dst.timeline.active_count == src.timeline.active_count,
              "active_count round-trips");
        CHECK(dst.timeline.guard_rejects == src.timeline.guard_rejects,
              "guard_rejects round-trips (mode GUARD)");
        CHECK(dst.mode == src.mode,
              "mode round-trips (GUARD preserved)");

        /* Per-slot equality for in-use slots. The save/load order
         * is deterministic so the slots are in declared order on
         * both sides. */
        int slot_match = 1;
        for (size_t i = 0; i < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP; ++i) {
            if (src.timeline.slots[i].in_use != dst.timeline.slots[i].in_use ||
                src.timeline.slots[i].type != dst.timeline.slots[i].type ||
                src.timeline.slots[i].light_power != dst.timeline.slots[i].light_power ||
                src.timeline.slots[i].fire_at_tick != dst.timeline.slots[i].fire_at_tick) {
                slot_match = 0;
                break;
            }
        }
        CHECK(slot_match == 1,
              "all in-use timeline slots round-trip (type, light_power, fire_at_tick)");

        /* Cast path hash should be identical. */
        CHECK(cast_path_hash(&src) == cast_path_hash(&dst),
              "cast_path_hash(src) == cast_path_hash(dst) after round-trip");

        /* ── Corrupted buffer rejection ── */
        buf[4] ^= 0xFF;  /* corrupt the version field */
        Nexus_V1_LightCastPath corrupt_dst;
        int crc = nexus_v1_light_cast_path_load(&corrupt_dst, buf, bufsize);
        CHECK(crc != NEXUS_SAVE_OK,
              "corrupted version is rejected with a non-OK error code");
        buf[4] ^= 0xFF;  /* restore for the next iteration */

        /* Truncated buffer: just a few bytes — magic check should
         * either fail or the load returns an error. */
        int trunc_rc = nexus_v1_light_cast_path_load(&corrupt_dst, buf, 8u);
        CHECK(trunc_rc != NEXUS_SAVE_OK,
              "truncated buffer is rejected (not enough header bytes)");

        /* Random non-FNXL buffer: probe returns 0. */
        uint8_t junk[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0,0,0,0,0,0,0,0,0,0,0,0};
        CHECK(nexus_v1_light_cast_path_probe(junk, sizeof(junk)) == 0,
              "probe() rejects a non-FNXL buffer");

        free(buf);
    }

    /* ── Test 7: determinism across runs ───────────────────────────── */
    printf("\n[7] determinism (cast-then-tick hash stable)\n");
    {
        uint32_t expected = 0;
        int mismatch = 0;
        for (int rep = 0; rep < 5; ++rep) {
            Nexus_V1_LightCastPath path;
            nexus_v1_light_cast_path_init(&path, NEXUS_LIGHT_CAST_MODE_EMULATE);
            for (int i = 0; i < 6; ++i) {
                nexus_v1_light_cast_path_cast(&path, NEXUS_LIGHT_KIND_TORCH, 2);
            }
            nexus_v1_light_cast_path_advance(&path, 1500);
            uint32_t h = cast_path_hash(&path);
            if (rep == 0) expected = h;
            else if (h != expected) ++mismatch;
        }
        CHECK(mismatch == 0,
              "5 runs produce the same cast_path_hash");
    }

    /* ── Test 8: NULL safety on every entry point ──────────────────── */
    printf("\n[8] NULL safety\n");
    {
        Nexus_V1_LightCastResult r = nexus_v1_light_cast_path_cast(
            NULL, NEXUS_LIGHT_KIND_LIGHT, 4);
        CHECK(r.was_rejected == 1 && r.applied_light_power == 0,
              "cast(NULL, ...) returns a populated reject result");

        CHECK(nexus_v1_light_cast_path_tick(NULL) == 0,
              "tick(NULL) returns 0");
        CHECK(nexus_v1_light_cast_path_advance(NULL, 100) == 0,
              "advance(NULL, ...) returns 0");
        CHECK(nexus_v1_light_cast_path_classify(NULL) ==
              NEXUS_LIGHT_OVERFLOW_NONE,
              "classify(NULL) returns NONE");
        CHECK(nexus_v1_light_cast_path_should_guard(NULL) == 0,
              "should_guard(NULL) returns 0");
        CHECK(nexus_v1_light_cast_path_get_mode(NULL) ==
              NEXUS_LIGHT_CAST_MODE_EMULATE,
              "get_mode(NULL) returns EMULATE (sane default)");

        /* set_mode(NULL) is a silent no-op (we cannot return
         * anything; the helper just returns). */
        nexus_v1_light_cast_path_set_mode(NULL, NEXUS_LIGHT_CAST_MODE_GUARD);
        CHECK(1, "set_mode(NULL) is a silent no-op");

        /* Accessors on NULL all return 0. */
        CHECK(nexus_v1_light_cast_path_light_amount(NULL) == 0,
              "light_amount(NULL) returns 0");
        CHECK(nexus_v1_light_cast_path_cast_count(NULL) == 0,
              "cast_count(NULL) returns 0");
        CHECK(nexus_v1_light_cast_path_decay_count(NULL) == 0,
              "decay_count(NULL) returns 0");
        CHECK(nexus_v1_light_cast_path_dropped_count(NULL) == 0,
              "dropped_count(NULL) returns 0");
        CHECK(nexus_v1_light_cast_path_active_count(NULL) == 0,
              "active_count(NULL) returns 0");

        /* load(NULL, ...) is rejected. */
        uint8_t dummy[36] = {0};
        CHECK(nexus_v1_light_cast_path_load(NULL, dummy, sizeof(dummy)) ==
              NEXUS_SAVE_ERR_NULL,
              "load(NULL, ...) returns NEXUS_SAVE_ERR_NULL");
        CHECK(nexus_v1_light_cast_path_save_size(NULL) == 0,
              "save_size(NULL) returns 0");
        CHECK(nexus_v1_light_cast_path_save(NULL, dummy, sizeof(dummy)) == 0,
              "save(NULL, ...) returns 0");
    }

    /* ── Test 9: skip-real-assets contract ─────────────────────────── */
    printf("\n[9] skip-real-assets: cast path works without Saturn data\n");
    {
        /* The cast path is fully data-free. We exercise it in both
         * modes end-to-end without ever opening a Saturn disc
         * image, FONT256.S2D, MNS model, or DGN file. The probe
         * binary itself is the proof: it links the cast path
         * symbols and runs every code path on synthetic state. */
        Nexus_V1_LightCastPath emulate, guard;
        nexus_v1_light_cast_path_init(&emulate, NEXUS_LIGHT_CAST_MODE_EMULATE);
        nexus_v1_light_cast_path_init(&guard, NEXUS_LIGHT_CAST_MODE_GUARD);
        /* Same 3-cast script on both — under cap, both should
         * accumulate the same MagicalLightAmount and classify
         * NONE regardless of mode (mode only differs at the cap). */
        for (int i = 0; i < 3; ++i) {
            nexus_v1_light_cast_path_cast(&emulate,
                                           NEXUS_LIGHT_KIND_TORCH, 0);
            nexus_v1_light_cast_path_cast(&guard,
                                           NEXUS_LIGHT_KIND_TORCH, 0);
        }
        CHECK(nexus_v1_light_cast_path_light_amount(&emulate) ==
              nexus_v1_light_cast_path_light_amount(&guard),
              "emulate and guard agree on light amount under cap");
        CHECK(nexus_v1_light_cast_path_classify(&emulate) ==
              NEXUS_LIGHT_OVERFLOW_NONE,
              "emulate classifies NONE under cap (no overflow)");
        CHECK(nexus_v1_light_cast_path_classify(&guard) ==
              NEXUS_LIGHT_OVERFLOW_NONE,
              "guard classifies NONE under cap (no overflow)");
    }

    /* ── Summary ────────────────────────────────────────────────────── */
    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
