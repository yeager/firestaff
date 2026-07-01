/*
 * test_csb_v1_combat_bugfix_helpers_pc34_compat.c
 *
 * CSB V1 Combat GAP 3 + GAP 4 source-lock fixture.
 *
 * Pins the source-locked behaviour of the three CSB dungeon-world
 * bug-fix helpers that gate CSB combat mechanics:
 *
 *   - csb_bugfix_is_sensor_square_clear_for_discard
 *     CHANGE7_17_FIX / BUG0_09 / MEDIA278
 *     (DUNGEON.C:1996-2001)
 *     -- "discard search must skip squares with enabled sensors"
 *     -- covers creature/group spawning into a sensor square that
 *        would have inadvertently closed a door / opened a pit
 *
 *   - csb_bugfix_thing_type_bit15_clearly
 *     CHANGE7_18_FIX / BUG0_10 / MEDIA291/MEDIA178
 *     (DUNGEON.C:2099-2101)
 *     -- "raw THING type must mask off bit 15 before M012_TYPE
 *        extraction"
 *     -- bone-creation path uses bit 15 to flag the reserved-thing
 *        pool; the Megamax C compiler silently dropped this bit but
 *        other compilers need it explicit
 *
 *   - csb_bugfix_lord_chaos_teleport_dir
 *     CHANGE7_19_FIX / BUG0_69 / MEDIA297
 *     (GROUP.C:2208-2215)
 *     -- "primary direction must be initialized before the array-
 *        index use that BUG0_69 flagged"
 *     -- covers Lord Chaos / Grey Lord teleport-out-of-danger
 *        dispatch on closing doors / poison clouds / 3+ fluxcages
 *
 * Plus the two sensor-segment helpers that the BUG0_09/10 helpers
 * depend on:
 *
 *   - csb_sensor_get_type (M039_TYPE after bit 15 cleared)
 *   - csb_sensor_get_data (M040_DATA)
 *
 * Source citations:
 *   ReDMCSB DUNGEON.C:1996-2001 (CHANGE7_17_FIX / MEDIA278)
 *   ReDMCSB DUNGEON.C:2099-2101 (CHANGE7_18_FIX / MEDIA291)
 *   ReDMCSB GROUP.C:2208-2215  (CHANGE7_19_FIX / MEDIA297)
 *   ReDMCSB DEFS.H:399 M012_TYPE comment
 *   ReDMCSB DEFS.H:1295-1296   M039_TYPE / M040_DATA layout
 *   ReDMCSB GROUP.C:2215 comment "BUG0_69 memory corruption"
 *   BugsAndChanges.htm:CHANGE7_17, CHANGE7_18, CHANGE7_19,
 *                       BUG0_09, BUG0_10, BUG0_69
 *
 * Companion probe:
 *   probes/csb/firestaff_csb_v1_combat_bugfix_helpers_probe.c
 *
 * This fixture is data-free and makes no original-payload claim.
 */

#include "csb_v1_dungeon_world_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* ----------------------------------------------------------
 * Synthetic sensor thing fixture for BUG0_09
 *
 * We build a 2-thing list per square.  Each "thing" is just a
 * 16-bit handle whose type field (bits 13:10 = CSB_THING_TYPE_*)
 * we populate directly so the bugfix helper's walk can inspect
 * it without needing a real dungeon.
 * ---------------------------------------------------------- */

/* Synthetic Type_Data storage addressed by the thing handle index. */
static uint16_t g_sensor_type_data[8] = {0};

/* Map "square" -> first thing index (or ENDOF). */
static uint16_t g_first_thing[4] = {CSB_THING_ENDOFLIST,
                                    CSB_THING_ENDOFLIST,
                                    CSB_THING_ENDOFLIST,
                                    CSB_THING_ENDOFLIST};

/* Per-thing "next" chain (or ENDOF). */
static uint16_t g_next_thing[8] = {CSB_THING_ENDOFLIST};

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
    /* Each thing's Type_Data lives in g_sensor_type_data[index]. */
    return g_sensor_type_data[index];
}

/* Pack a THING handle: type in bits 13:10, index in bits 9:0. */
static uint16_t pack_thing(int type, int index) {
    return (uint16_t)(((type & 0x0F) << 10) | (index & 0x03FF));
}

/* ----------------------------------------------------------
 * Random source for BUG0_69 Lord Chaos teleport direction
 *
 * Deterministic counter so the test never reads stale LCG state.
 * ---------------------------------------------------------- */
static int g_rng_seq[8] = {0, 2, 1, 3, 0, 1, 2, 3};
static int g_rng_idx = 0;
static int cb_random4(void) {
    int v = g_rng_seq[g_rng_idx & 7];
    g_rng_idx++;
    return v;
}

int main(void) {
    printf("=== CSB V1 Combat GAP 3+4 -- dungeon-world bug-fix helpers ===\n");

    /* ----------------------------------------------------------
     * Group A: csb_sensor_get_type / csb_sensor_get_data
     *
     * DEFS.H:1295-1296 M039_TYPE / M040_DATA layout.
     * Bit 15 sensitivity: csb_sensor_get_type clears it before
     * masking the 7-bit type field (CHANGE7_18_FIX).
     * ---------------------------------------------------------- */

    /* A.1 - clean sensor: type=1 (C001 floor party sensor),
     *       data=0. */
    CHECK(csb_sensor_get_type(0x0001u) == 1,
          "A.1 sensor_get_type(0x0001) == 1");
    CHECK(csb_sensor_get_data(0x0001u) == 0,
          "A.1 sensor_get_data(0x0001) == 0");

    /* A.2 - bit 15 set in raw Type_Data:
     *   0x8003 = bit15 | type=3 (C003 sensor generic) | data=0
     *   csb_sensor_get_type must mask bit 15 -> 3
     *   csb_sensor_get_data uses shift starting at bit 7, so
     *   bit 15 leaks into the data field: data = 0x8003 >> 7 = 0x0100.
     *   This documents the data-field bit-15 leak (deferred).
     */
    CHECK(csb_sensor_get_type(0x8003u) == 3,
          "A.2 sensor_get_type clears bit 15 (0x8003 -> type=3)");
    CHECK((csb_sensor_get_data(0x8003u) & 0xFF80u) == 0x0100u,
          "A.2 sensor_get_data uses bit-7 shift (data field leakage document)");

    /* A.3 - version-checker sensor (C009, CSB-only).
     *   0x0009 = type=9 | data=0
     */
    CHECK(csb_sensor_is_csb_version_checker(0x0009u) == 1,
          "A.3 version_checker test on raw type=9");
    CHECK(csb_sensor_is_csb_version_checker(0x8009u) == 1,
          "A.3 version_checker test on raw type=9 + bit 15");

    /* A.4 - end-game sensor (C018, CSB-only). */
    CHECK(csb_sensor_is_csb_end_game(0x0012u) == 1,
          "A.4 end_game test on raw type=18");
    CHECK(csb_sensor_is_csb_end_game(0x8012u) == 1,
          "A.4 end_game test on raw type=18 + bit 15");

    /* A.5 - disabled sensor (type=0).  Even with bit 15 set,
     *       the helper must report 0 (BUG0_10 contract). */
    CHECK(csb_sensor_get_type(0x8000u) == 0,
          "A.5 sensor_get_type(0x8000) == 0 after bit 15 clear");
    CHECK(csb_sensor_get_type(0xFFFFu) == 0x7F,
          "A.5 sensor_get_type(0xFFFF) == 0x7F (max type)");

    /* ----------------------------------------------------------
     * Group B: csb_bugfix_thing_type_bit15_clearly
     *
     * CHANGE7_18_FIX / BUG0_10: bone-creation path sets bit 15
     * to mark the reserved-thing pool.  Mask it off so M012_TYPE
     * returns the original creature/weapon/door type.
     *
     * THING layout: [13:10 type] [9:0 index].  Bit 15 is the
     * "reserved pool" flag.
     * ---------------------------------------------------------- */

    /* B.1 - clean type-3 sensor: 0x0C00. */
    CHECK(csb_bugfix_thing_type_bit15_clearly(0x0C00u) == 3,
          "B.1 type-3 sensor (0x0C00) -> 3");

    /* B.2 - type-3 sensor with reserved-pool bit: 0x8C00. */
    CHECK(csb_bugfix_thing_type_bit15_clearly(0x8C00u) == 3,
          "B.2 type-3 sensor + bit 15 (0x8C00) -> 3 (CHANGE7_18_FIX)");

    /* B.3 - type-4 group (creature group): 0x1000.
     *       csb_bugfix_thing_type_bit15_clearly returns the 4-bit
     *       CSB_THING_TYPE(...) value, so type 4 -> 4. */
    CHECK(csb_bugfix_thing_type_bit15_clearly(0x1000u) == 4,
          "B.3 type-4 group (0x1000) -> 4");

    /* B.4 - type-4 group with reserved-pool bit: 0x9000. */
    CHECK(csb_bugfix_thing_type_bit15_clearly(0x9000u) == 4,
          "B.4 type-4 group + bit 15 (0x9000) -> 4 (CHANGE7_18_FIX)");

    /* B.5 - type-5 weapon (Lord Chaos projectile thing source): 0x1400. */
    CHECK(csb_bugfix_thing_type_bit15_clearly(0x1400u) == 5,
          "B.5 type-5 weapon (0x1400) -> 5");

    /* B.6 - type-5 weapon with reserved-pool bit: 0x9400. */
    CHECK(csb_bugfix_thing_type_bit15_clearly(0x9400u) == 5,
          "B.6 type-5 weapon + bit 15 (0x9400) -> 5 (CHANGE7_18_FIX)");

    /* B.7 - type-0 door with reserved-pool bit: 0x8000. */
    CHECK(csb_bugfix_thing_type_bit15_clearly(0x8000u) == 0,
          "B.7 type-0 door + bit 15 (0x8000) -> 0 (CHANGE7_18_FIX)");

    /* B.8 - full 0xFFFF input: after mask -> 0x7FFF -> M012_TYPE
     *       (bits 13:10) = 0xF = 15 (max type). */
    CHECK(csb_bugfix_thing_type_bit15_clearly(0xFFFFu) == 0xF,
          "B.8 full 0xFFFF -> type 0xF (max type)");

    /* ----------------------------------------------------------
     * Group C: csb_bugfix_is_sensor_square_clear_for_discard
     *
     * CHANGE7_17_FIX / BUG0_09: discard search skips squares
     * with enabled sensors (M039_TYPE != 0).  Walk the synthetic
     * thing list and check every sensor's Type_Data.
     * ---------------------------------------------------------- */

    /* C.1 - empty square (ENDOF), no sensors -> clear. */
    CHECK(csb_bugfix_is_sensor_square_clear_for_discard(
              0, 0, cb_get_first, cb_get_next, cb_thing_data,
              /* sensorTypeDataOffset */ 0) == 1,
          "C.1 empty square -> clear");

    /* C.2 - square with no sensors (just a group + a weapon).
     *   thing[0] = GROUP (type=4, no sensor type)
     *   thing[1] = WEAPON (type=5, no sensor type)
     * Both have Type_Data = 0 but M039_TYPE only matches SENSOR type.
     */
    g_first_thing[1] = pack_thing(CSB_THING_TYPE_GROUP, 0);
    g_next_thing[0]  = pack_thing(CSB_THING_TYPE_WEAPON, 1);
    g_next_thing[1]  = CSB_THING_ENDOFLIST;
    g_sensor_type_data[0] = 0x0001u; /* group doesn't read as sensor */
    g_sensor_type_data[1] = 0x0001u; /* weapon doesn't read as sensor */
    CHECK(csb_bugfix_is_sensor_square_clear_for_discard(
              1, 0, cb_get_first, cb_get_next, cb_thing_data, 0) == 1,
          "C.2 square with group + weapon (no sensors) -> clear");

    /* C.3 - square with one enabled floor sensor (C003 type=3).
     *   thing[2] = SENSOR (type=3) with Type_Data type=3 (enabled).
     * Bugfix must return 0 (square NOT clear - skip during discard).
     */
    g_first_thing[2] = pack_thing(CSB_THING_TYPE_SENSOR, 2);
    g_next_thing[2]  = CSB_THING_ENDOFLIST;
    g_sensor_type_data[2] = 0x0003u; /* M039_TYPE = 3, enabled */
    CHECK(csb_bugfix_is_sensor_square_clear_for_discard(
              2, 0, cb_get_first, cb_get_next, cb_thing_data, 0) == 0,
          "C.3 square with enabled sensor (C003) -> NOT clear (BUG0_09 fix)");

    /* C.4 - same square but sensor DISABLED (M039_TYPE == 0).
     *   Type_Data = 0 -> M039_TYPE = 0 -> helper should return 1
     *   (clear, safe for discard).
     */
    g_sensor_type_data[2] = 0x0000u;
    CHECK(csb_bugfix_is_sensor_square_clear_for_discard(
              2, 0, cb_get_first, cb_get_next, cb_thing_data, 0) == 1,
          "C.4 square with disabled sensor (M039_TYPE=0) -> clear");

    /* C.5 - sensor with bit 15 set in Type_Data but real type=3.
     *   CHANGE7_18_FIX clears bit 15 in csb_sensor_get_type, so
     *   M039_TYPE = 3 still -> square NOT clear.
     */
    g_sensor_type_data[2] = 0x8003u;
    CHECK(csb_bugfix_is_sensor_square_clear_for_discard(
              2, 0, cb_get_first, cb_get_next, cb_thing_data, 0) == 0,
          "C.5 sensor with bit 15 (raw 0x8003) still triggers (CHANGE7_18+17 fix)");

    /* C.6 - square with group + sensor combo: sensor still gates. */
    g_first_thing[3] = pack_thing(CSB_THING_TYPE_GROUP, 3);
    g_next_thing[3]  = pack_thing(CSB_THING_TYPE_SENSOR, 4);
    g_next_thing[4]  = CSB_THING_ENDOFLIST;
    g_sensor_type_data[3] = 0x0000u; /* group doesn't read as sensor */
    g_sensor_type_data[4] = 0x0009u; /* C009 version-checker enabled */
    CHECK(csb_bugfix_is_sensor_square_clear_for_discard(
              3, 0, cb_get_first, cb_get_next, cb_thing_data, 0) == 0,
          "C.6 group + version-checker combo -> NOT clear (C009 enabled)");

    /* C.7 - NULL callbacks must default to "clear" (1) so the
     *       pre-M10 integration shape does not block test-only
     *       builds. */
    CHECK(csb_bugfix_is_sensor_square_clear_for_discard(
              0, 0, NULL, NULL, NULL, 0) == 1,
          "C.7 NULL callbacks default to clear (M10-stub safety)");

    /* ----------------------------------------------------------
     * Group D: csb_bugfix_lord_chaos_teleport_dir
     *
     * CHANGE7_19_FIX / BUG0_69: primary direction must be
     * initialized before the array-index use flagged by BUG0_69.
     * The helper passes the random4() result through `& 3`,
     * guaranteeing the value is always in [0..3].
     * ---------------------------------------------------------- */

    /* D.1 - sequence 0, 2, 1, 3 produces the documented
     *       primary direction.  Returns 0 first call. */
    g_rng_idx = 0;
    CHECK(csb_bugfix_lord_chaos_teleport_dir(cb_random4) == 0,
          "D.1 first random4() (0) -> primaryDir=0");
    CHECK(csb_bugfix_lord_chaos_teleport_dir(cb_random4) == 2,
          "D.2 next random4() (2) -> primaryDir=2");
    CHECK(csb_bugfix_lord_chaos_teleport_dir(cb_random4) == 1,
          "D.3 next random4() (1) -> primaryDir=1");
    CHECK(csb_bugfix_lord_chaos_teleport_dir(cb_random4) == 3,
          "D.4 next random4() (3) -> primaryDir=3");

    /* D.5 - never returns out-of-range even if random4 returns
     *       garbage > 3.  The helper must mask with & 3 to
     *       guarantee the BUG0_69 array-index contract. */
    g_rng_idx = 0;
    /* Override the sequence to expose a >3 value once. */
    g_rng_seq[0] = 17; /* would corrupt BUG0_69 index if not masked */
    CHECK(csb_bugfix_lord_chaos_teleport_dir(cb_random4) == 1,
          "D.5 random4()=17 still masked to (17 & 3) == 1 (BUG0_69 guard)");
    g_rng_seq[0] = 0; /* restore deterministic sequence */

    /* D.6 - NULL callback returns the "uninitialised" sentinel 0
     *       (pre-decrement / caller fallback).  This documents
     *       the contract: callers should treat 0 as "not yet
     *       randomised" and trigger their own RNG fallback. */
    CHECK(csb_bugfix_lord_chaos_teleport_dir(NULL) == 0,
          "D.6 NULL callback returns sentinel 0 (caller fallback contract)");

    /* ----------------------------------------------------------
     * Group E: door-defense table sanity (already source-locked
     * to DUNGEON.C:560-565 / pass563 compat).  These checks
     * protect the combat mechanic from accidentally regressing
     * when the dungeon-world helpers are touched.
     * ---------------------------------------------------------- */

    CHECK(csb_door_get_defense_points(CSB_DOOR_PORTCULLIS) == 110,
          "E.1 Portcullis defense == 110");
    CHECK(csb_door_get_defense_points(CSB_DOOR_WOODEN) == 42,
          "E.1 Wooden door defense == 42");
    CHECK(csb_door_get_defense_points(CSB_DOOR_IRON) == 230,
          "E.1 Iron door defense == 230");
    CHECK(csb_door_get_defense_points(CSB_DOOR_RA) == 255,
          "E.1 Ra door defense == 255");

    CHECK(csb_door_minimum_attack_power(CSB_DOOR_WOODEN) == 42,
          "E.2 wooden minimum attack power == 42");
    CHECK(csb_door_minimum_attack_power(CSB_DOOR_IRON) == 230,
          "E.2 iron minimum attack power == 230");
    CHECK(csb_door_minimum_attack_power(CSB_DOOR_RA) == -1,
          "E.2 Ra door minimum attack power == -1 (melee immune)");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
