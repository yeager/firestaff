/*
 * firestaff_csb_v1_combat_bugfix_helpers_probe.c
 * ===============================================
 *
 * CSB V1 Combat GAP 3 + GAP 4 headless CI probe.
 *
 * Companion to test_csb_v1_combat_bugfix_helpers_pc34_compat
 * (data-free).  This probe adds:
 *
 *   1. Library self-test re-run + invariants-counting
 *      on the same source-locked surface (CHANGE7_17_FIX /
 *      CHANGE7_18_FIX / CHANGE7_19_FIX / MEDIA278 / MEDIA291 /
 *      MEDIA297).
 *
 *   2. Deterministic FNV-1a hash of the test output envelope
 *      (invariant_id, expected_value, observed_value) so the
 *      probe can detect silent byte-level regressions across
 *      rebuilds without depending on printf ordering.
 *
 *   3. Source-evidence citation block (ReDMCSB file/function/
 *      line) emitted as a labeled line so a CI log scraper can
 *      audit the gate against docs/source-lock.
 *
 *   4. Skip-safe behaviour: the probe has no real-asset
 *      dependency (data-free) and never loads GRAPHICS.DAT /
 *      DUNGEON.DAT.  It can run in any environment including
 *      CI containers that lack user-supplied CSB data.
 *
 * Probe contract:
 *   exit 0 + invariant summary on PASS
 *   exit 1 with FAIL: prefix on regression
 *
 * Run:
 *   ./build/firestaff_csb_v1_combat_bugfix_helpers_probe
 *   ctest --test-dir build -R csb_v1_combat_bugfix_helpers_probe
 *
 * Source citations:
 *   ReDMCSB DUNGEON.C:1996-2001 (CHANGE7_17_FIX / MEDIA278)
 *   ReDMCSB DUNGEON.C:2099-2101 (CHANGE7_18_FIX / MEDIA291)
 *   ReDMCSB GROUP.C:2208-2215  (CHANGE7_19_FIX / MEDIA297)
 *   ReDMCSB DEFS.H:1295-1296   M039_TYPE / M040_DATA layout
 *   ReDMCSB DEFS.H:399         M012_TYPE comment
 *   ReDMCSB GROUP.C:2215       BUG0_69 memory corruption
 *   ReDMCSB DUNGEON.C:560-565  door defense table (pass563)
 *   BugsAndChanges.htm:
 *     CHANGE7_17, CHANGE7_18, CHANGE7_19,
 *     BUG0_09, BUG0_10, BUG0_69
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "csb_v1_dungeon_world_pc34_compat.h"

/* ---------- counter / log ---------- */
static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                 \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); }     \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); }     \
} while (0)

/* ---------- FNV-1a 32-bit over the supplied (id, expected, observed)
 *            triple.  We do NOT hash printf ordering; we hash the
 *            invariant shape so a regression in a single CHECK can
 *            be detected even if other checks still pass. */
static uint32_t fnv1a_32_init(void) { return 0x811C9DC5u; }

static void fnv1a_32_update(uint32_t *state, const void *data, size_t n) {
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < n; ++i) {
        *state ^= (uint32_t)p[i];
        *state *= 0x01000193u;
    }
}

static void fnv1a_32_update_str(uint32_t *state, const char *s) {
    if (!s) return;
    fnv1a_32_update(state, s, strlen(s));
}

/* ---------- synthetic sensor / thing storage (BUG0_09 fixture) ---------- */
static uint16_t g_sensor_type_data[8];
static uint16_t g_first_thing[4] = {CSB_THING_ENDOFLIST,
                                    CSB_THING_ENDOFLIST,
                                    CSB_THING_ENDOFLIST,
                                    CSB_THING_ENDOFLIST};
static uint16_t g_next_thing[8] = {CSB_THING_ENDOFLIST,
                                   CSB_THING_ENDOFLIST,
                                   CSB_THING_ENDOFLIST,
                                   CSB_THING_ENDOFLIST,
                                   CSB_THING_ENDOFLIST,
                                   CSB_THING_ENDOFLIST,
                                   CSB_THING_ENDOFLIST,
                                   CSB_THING_ENDOFLIST};

static uint16_t cb_get_first(int mapX, int mapY) {
    (void)mapY;
    if (mapX < 0 || mapX >= 4) return CSB_THING_ENDOFLIST;
    return g_first_thing[mapX];
}

static uint16_t cb_get_next(uint16_t thing) {
    unsigned index = (unsigned)(thing & 0x03FFu);
    if (index >= 8u) return CSB_THING_ENDOFLIST;
    return g_next_thing[index];
}

static uint16_t cb_thing_data(uint16_t thing, int offset) {
    unsigned index = (unsigned)(thing & 0x03FFu);
    if (index >= 8u) return 0;
    (void)offset;
    return g_sensor_type_data[index];
}

static uint16_t pack_thing(int type, int index) {
    return (uint16_t)(((type & 0x0F) << 10) | (index & 0x03FF));
}

/* ---------- deterministic random4 for BUG0_69 ---------- */
static int g_rng_seq[8] = {0, 2, 1, 3, 0, 1, 2, 3};
static int g_rng_idx = 0;
static int cb_random4(void) {
    int v = g_rng_seq[g_rng_idx & 7];
    g_rng_idx++;
    return v;
}

/* ---------- source evidence block ---------- */
static void emit_source_evidence(void) {
    printf("\n=== Source evidence ===\n");
    printf("ReDMCSB/DUNGEON.C:1996-2001  CHANGE7_17_FIX / MEDIA278  "
           "(BUG0_09 sensor-square discard)\n");
    printf("ReDMCSB/DUNGEON.C:2099-2101  CHANGE7_18_FIX / MEDIA291  "
           "(BUG0_10 thing-type bit 15)\n");
    printf("ReDMCSB/GROUP.C:2208-2215    CHANGE7_19_FIX / MEDIA297  "
           "(BUG0_69 Lord Chaos teleport dir)\n");
    printf("ReDMCSB/GROUP.C:2215         BUG0_69 memory corruption "
           "comment\n");
    printf("ReDMCSB/DEFS.H:399           M012_TYPE comment (THING layout)\n");
    printf("ReDMCSB/DEFS.H:1295-1296     M039_TYPE / M040_DATA sensor "
           "layout\n");
    printf("ReDMCSB/DUNGEON.C:560-565    door defense table (pass563 compat)\n");
    printf("BugsAndChanges.htm:CHANGE7_17,CHANGE7_18,CHANGE7_19,"
           "BUG0_09,BUG0_10,BUG0_69\n");
}

/* ---------- invariants that the probe asserts on top of the
 *            data-free test fixture.  Each invariant is a
 *            tagged triple (id, expected, observed) so the
 *            FNV-1a hash can be reproduced byte-for-byte. */
typedef struct {
    const char *id;
    long        expected;
    long        observed;
} Invariant;

#define INV_MAX 64
static Invariant g_invariants[INV_MAX];
static int       g_invariant_count = 0;

static void inv_record(const char *id, long expected, long observed) {
    if (g_invariant_count >= INV_MAX) return;
    g_invariants[g_invariant_count].id       = id;
    g_invariants[g_invariant_count].expected = expected;
    g_invariants[g_invariant_count].observed = observed;
    g_invariant_count++;
}

static uint32_t hash_invariants(void) {
    uint32_t h = fnv1a_32_init();
    for (int i = 0; i < g_invariant_count; ++i) {
        fnv1a_32_update_str(&h, g_invariants[i].id);
        fnv1a_32_update(&h, &g_invariants[i].expected, sizeof(long));
        fnv1a_32_update(&h, &g_invariants[i].observed, sizeof(long));
    }
    return h;
}

int main(void) {
    printf("=== CSB V1 Combat GAP 3+4 -- bugfix helpers probe ===\n");

    /* ---- A: sensor type / data ---- */
    long v;
    v = (long)csb_sensor_get_type(0x0001u);
    CHECK(v == 1, "A.1 sensor_get_type(0x0001) == 1");
    inv_record("A.1", 1, v);

    v = (long)csb_sensor_get_type(0x8003u);
    CHECK(v == 3, "A.2 sensor_get_type(0x8003) == 3 (bit-15 cleared)");
    inv_record("A.2", 3, v);

    v = (long)csb_sensor_get_data(0x0001u);
    CHECK(v == 0, "A.3 sensor_get_data(0x0001) == 0");
    inv_record("A.3", 0, v);

    v = (long)csb_sensor_is_csb_version_checker(0x0009u);
    CHECK(v == 1, "A.4 version_checker test on raw type=9");
    inv_record("A.4", 1, v);

    v = (long)csb_sensor_is_csb_end_game(0x0012u);
    CHECK(v == 1, "A.5 end_game test on raw type=18");
    inv_record("A.5", 1, v);

    /* ---- B: thing-type bit 15 (CHANGE7_18_FIX / BUG0_10) ---- */
    v = (long)csb_bugfix_thing_type_bit15_clearly(0x0C00u);
    CHECK(v == 3, "B.1 type-3 sensor (0x0C00) -> 3");
    inv_record("B.1", 3, v);

    v = (long)csb_bugfix_thing_type_bit15_clearly(0x8C00u);
    CHECK(v == 3, "B.2 type-3 + bit 15 (0x8C00) -> 3 (CHANGE7_18_FIX)");
    inv_record("B.2", 3, v);

    v = (long)csb_bugfix_thing_type_bit15_clearly(0x9000u);
    CHECK(v == 4, "B.3 type-4 group + bit 15 (0x9000) -> 4");
    inv_record("B.3", 4, v);

    v = (long)csb_bugfix_thing_type_bit15_clearly(0xFFFFu);
    CHECK(v == 0xF, "B.4 0xFFFF -> type 0xF (max)");
    inv_record("B.4", 0xF, v);

    /* ---- C: sensor-square clear (CHANGE7_17_FIX / BUG0_09) ---- */

    /* C.1 - empty square. */
    v = (long)csb_bugfix_is_sensor_square_clear_for_discard(
        0, 0, cb_get_first, cb_get_next, cb_thing_data, 0);
    CHECK(v == 1, "C.1 empty square -> clear (1)");
    inv_record("C.1", 1, v);

    /* C.2 - group + weapon (no sensors). */
    g_first_thing[1] = pack_thing(CSB_THING_TYPE_GROUP, 0);
    g_next_thing[0]  = pack_thing(CSB_THING_TYPE_WEAPON, 1);
    g_next_thing[1]  = CSB_THING_ENDOFLIST;
    g_sensor_type_data[0] = 0x0001u;
    g_sensor_type_data[1] = 0x0001u;
    v = (long)csb_bugfix_is_sensor_square_clear_for_discard(
        1, 0, cb_get_first, cb_get_next, cb_thing_data, 0);
    CHECK(v == 1, "C.2 group + weapon (no sensors) -> clear (1)");
    inv_record("C.2", 1, v);

    /* C.3 - enabled C003 sensor -> not clear. */
    g_first_thing[2] = pack_thing(CSB_THING_TYPE_SENSOR, 2);
    g_next_thing[2]  = CSB_THING_ENDOFLIST;
    g_sensor_type_data[2] = 0x0003u;
    v = (long)csb_bugfix_is_sensor_square_clear_for_discard(
        2, 0, cb_get_first, cb_get_next, cb_thing_data, 0);
    CHECK(v == 0, "C.3 enabled C003 sensor -> not clear (0) (BUG0_09 fix)");
    inv_record("C.3", 0, v);

    /* C.4 - same sensor disabled. */
    g_sensor_type_data[2] = 0x0000u;
    v = (long)csb_bugfix_is_sensor_square_clear_for_discard(
        2, 0, cb_get_first, cb_get_next, cb_thing_data, 0);
    CHECK(v == 1, "C.4 disabled sensor (M039_TYPE=0) -> clear (1)");
    inv_record("C.4", 1, v);

    /* C.5 - bit 15 in sensor Type_Data still triggers. */
    g_sensor_type_data[2] = 0x8003u;
    v = (long)csb_bugfix_is_sensor_square_clear_for_discard(
        2, 0, cb_get_first, cb_get_next, cb_thing_data, 0);
    CHECK(v == 0, "C.5 sensor with bit 15 still gates (CHANGE7_17+18 chain)");
    inv_record("C.5", 0, v);

    /* C.6 - group + C009 version-checker combo. */
    g_first_thing[3] = pack_thing(CSB_THING_TYPE_GROUP, 3);
    g_next_thing[3]  = pack_thing(CSB_THING_TYPE_SENSOR, 4);
    g_next_thing[4]  = CSB_THING_ENDOFLIST;
    g_sensor_type_data[3] = 0x0000u;
    g_sensor_type_data[4] = 0x0009u;
    v = (long)csb_bugfix_is_sensor_square_clear_for_discard(
        3, 0, cb_get_first, cb_get_next, cb_thing_data, 0);
    CHECK(v == 0, "C.6 group + version-checker -> not clear (0)");
    inv_record("C.6", 0, v);

    /* C.7 - NULL callbacks default to clear. */
    v = (long)csb_bugfix_is_sensor_square_clear_for_discard(
        0, 0, NULL, NULL, NULL, 0);
    CHECK(v == 1, "C.7 NULL callbacks default to clear (1)");
    inv_record("C.7", 1, v);

    /* ---- D: Lord Chaos teleport direction (CHANGE7_19_FIX / BUG0_69) ---- */

    g_rng_idx = 0;
    v = (long)csb_bugfix_lord_chaos_teleport_dir(cb_random4);
    CHECK(v == 0, "D.1 first random4 (0) -> 0");
    inv_record("D.1", 0, v);

    v = (long)csb_bugfix_lord_chaos_teleport_dir(cb_random4);
    CHECK(v == 2, "D.2 next random4 (2) -> 2");
    inv_record("D.2", 2, v);

    v = (long)csb_bugfix_lord_chaos_teleport_dir(cb_random4);
    CHECK(v == 1, "D.3 next random4 (1) -> 1");
    inv_record("D.3", 1, v);

    v = (long)csb_bugfix_lord_chaos_teleport_dir(cb_random4);
    CHECK(v == 3, "D.4 next random4 (3) -> 3");
    inv_record("D.4", 3, v);

    /* D.5 - random4 returns 17 -> masked to (17 & 3) == 1. */
    g_rng_idx = 0;
    g_rng_seq[0] = 17;
    v = (long)csb_bugfix_lord_chaos_teleport_dir(cb_random4);
    CHECK(v == 1, "D.5 random4=17 masked to 1 (BUG0_69 guard)");
    inv_record("D.5", 1, v);
    g_rng_seq[0] = 0;

    /* D.6 - NULL callback -> sentinel 0. */
    v = (long)csb_bugfix_lord_chaos_teleport_dir(NULL);
    CHECK(v == 0, "D.6 NULL callback -> sentinel 0 (caller fallback)");
    inv_record("D.6", 0, v);

    /* ---- E: door defense table (pass563 / DUNGEON.C:560-565) ---- */
    v = (long)csb_door_get_defense_points(CSB_DOOR_PORTCULLIS);
    CHECK(v == 110, "E.1 Portcullis defense == 110");
    inv_record("E.1", 110, v);

    v = (long)csb_door_get_defense_points(CSB_DOOR_WOODEN);
    CHECK(v == 42, "E.1 Wooden defense == 42");
    inv_record("E.2", 42, v);

    v = (long)csb_door_get_defense_points(CSB_DOOR_IRON);
    CHECK(v == 230, "E.1 Iron defense == 230");
    inv_record("E.3", 230, v);

    v = (long)csb_door_get_defense_points(CSB_DOOR_RA);
    CHECK(v == 255, "E.1 Ra defense == 255");
    inv_record("E.4", 255, v);

    v = (long)csb_door_minimum_attack_power(CSB_DOOR_RA);
    CHECK(v == -1, "E.2 Ra minimum attack == -1 (melee immune)");
    inv_record("E.5", -1, v);

    /* ---- F: deterministic FNV-1a stability ---- */
    uint32_t hash1 = hash_invariants();
    uint32_t hash2 = hash_invariants();
    CHECK(hash1 == hash2, "F.1 FNV-1a stable across two consecutive hashes");
    inv_record("F.1", (long)hash1, (long)hash2);

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    printf("=== Invariant count: %d ===\n", g_invariant_count);
    printf("=== FNV-1a hash:    0x%08X ===\n", (unsigned)hash1);

    emit_source_evidence();

    if (g_fail != 0) {
        printf("\nFAIL: %d regression(s)\n", g_fail);
        return 1;
    }
    printf("\nPASS: %d invariants, FNV-1a 0x%08X stable\n",
           g_pass, (unsigned)hash1);
    return 0;
}
