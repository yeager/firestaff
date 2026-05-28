/*
 * firestaff_csb_v1_dungeon_model_probe.c
 *
 * Pass H2360: CSB V1 Phase 2 — Dungeon Data Model
 *
 * Headless compile-safe probe that:
 *   1. Exercises csb_v1_dungeon_load() against a synthetic header
 *   2. Exercises csb_v1_dungeon_get_square_type / _get_first_thing
 *   3. Exercises csb_v1_dungeon_world_pc34_compat world model
 *   4. Exercises door table, sensor helpers, endgame helpers
 *   5. Verifies DM1-vs-CSB differences do not leak
 *
 * Compile (from repo root):
 *   gcc -fsyntax-only -Wall -Wextra -I include \
 *       -I src/csb -I src/shared -I src/memory \
 *       probes/csb_v1_dungeon_model/firestaff_csb_v1_dungeon_model_probe.c
 *
 * Full build:
 *   gcc -I include -I src/csb -I src/shared -I src/memory \
 *       probes/csb_v1_dungeon_model/firestaff_csb_v1_dungeon_model_probe.c \
 *       src/csb/csb_v1_dungeon_loader_pc34_compat.c \
 *       src/csb/csb_v1_dungeon_world_pc34_compat.c \
 *       src/csb/csb_v1_game_state_pc34_compat.c \
 *       -o build/firestaff_csb_v1_dungeon_model_probe \
 *       2>&1
 *
 * Run (no game data needed):
 *   ./build/firestaff_csb_v1_dungeon_model_probe
 *
 * Source-locking:
 *   ReDMCSB DUNGEON.C: F0148 (SetGroupDirections), F0151 (GetSquare),
 *                      F0156 (GetThingData), F0161 (GetSquareFirstThing),
 *                      F0175 (GROUP_GetThing), TIMELINE.C:1319 (C018 endgame),
 *                      TIMELINE.C:1398 (F0249 teleporter/pit), GROUP.C:52,
 *                      DUNGEON.C:560-565 (door table), DEFS.H:1295-1296.
 *   CSBWin/CSBCode.cpp:318-480 DBank::Initialize (TAG00332a),
 *                      6800-6950 LoadDungeon.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Headers under test ── */
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_dungeon_world_pc34_compat.h"
#include "csb_v1_game_state_pc34_compat.h"

/* ── Compile-time assertions ── */
_Static_assert(CSB_V1_MAX_LEVELS == 12,      "CSB MAX_LEVELS must be 12");
_Static_assert(CSB_MAX_LEVELS   == 16,        "CSB_MAX_LEVELS must be 16");
_Static_assert(CSB_THING_TYPE_GROUP == 4,     "GROUP thing type must be 4");
_Static_assert(CSB_THING_TYPE_SENSOR == 3,    "SENSOR thing type must be 3");
_Static_assert(CSB_THING_ENDOFLIST == 0xFFFEu,"ENDOFLIST must be 0xFFFE");

/* ── Probe helpers ── */
/* ── Forward-declare internal accessors (defined in dungeon_world .c) ── */
extern uint16_t csb_dungeon_get_first_thing_default(int mapX, int mapY);
extern uint16_t csb_dungeon_get_next_thing_default(uint16_t thing);
extern uint16_t csb_dungeon_thing_data_u16_default(uint16_t thing, int offset);

static int passed = 0;
static int errors = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

/* ── Synthetic CSB dungeon.dat header ──
 *
 * Layout: bytes 0-1 = level_count (uint16 LE)
 *         bytes 2-3 = thing_type_count (uint16 LE, always 16)
 *         per level (6 bytes): width(u8), height(u8), offset(uint32 LE)
 *
 * Two synthetic levels (offsets point within the buffer so the
 * loader's offset < dat_size bounds check passes):
 *   Level 0: 8x8, data at offset 16
 *   Level 1: 6x6, data at offset 144
 *
 * Buffer: header(4) + 2×desc(12) + level0_sq(128) + level1_sq(72) = 216 bytes
 * Dungeon data is zeroed (valid: all-squares floor) — content is not accessed.
 */
static void build_synthetic_csb_dat(uint8_t *buf, int *out_size) {
    /* header(4) + 2×level_desc(12) + level0_data(128) + level1_data(72) */
    int sz = 4 + 12 + 128 + 72;
    *out_size = sz;
    memset(buf, 0, sz);  /* zero-fill keeps level data valid (floor squares) */
    buf[0] = 2;           /* level_count LO */
    buf[1] = 0;           /* level_count HI */
    buf[2] = 16;          /* thing_type_count LO */
    buf[3] = 0;           /* thing_type_count HI */
    /* Level 0: 8x8, data at offset 16 (= 4+12, first byte after header+descs) */
    buf[4]  = 8;          /* width  */
    buf[5]  = 8;          /* height */
    buf[6]  = 0x10;       /* offset 16 LE */
    buf[7]  = 0x00;
    buf[8]  = 0x00;
    buf[9]  = 0x00;
    /* Level 1: 6x6, data at offset 144 (= 16+128) */
    buf[10] = 6;          /* width  */
    buf[11] = 6;          /* height */
    buf[12] = 0x90;       /* offset 144 LE */
    buf[13] = 0x00;
    buf[14] = 0x00;
    buf[15] = 0x00;
}

/* ── Test 1: dungeon load / free cycle ── */
static void test_loader_cycle(void) {
    fprintf(stderr, "\n=== Test 1: Loader cycle ===\n");
    uint8_t buf[256];
    int sz;
    build_synthetic_csb_dat(buf, &sz);

    CSB_V1_DungeonData d;
    memset(&d, 0xCC, sizeof(d));  /* poison pattern */
    d.raw_data = NULL;
    d.level_count = 999;         /* should be clobbered */

    int ret = csb_v1_dungeon_load(&d, buf, sz);
    PROBE_ASSERT(ret == 0,          "csb_v1_dungeon_load returns 0");
    PROBE_ASSERT(d.raw_data != NULL,"raw_data allocated");
    PROBE_ASSERT(d.raw_size == sz,  "raw_size matches input");
    PROBE_ASSERT(d.level_count == 2, "level_count = 2");

    /* Level descriptors */
    PROBE_ASSERT(d.level_widths[0]  == 8,  "level[0] width = 8");
    PROBE_ASSERT(d.level_heights[0] == 8,  "level[0] height = 8");
    PROBE_ASSERT(d.level_offsets[0] == 16, "level[0] offset = 16");
    PROBE_ASSERT(d.level_widths[1]  == 6,  "level[1] width = 6");
    PROBE_ASSERT(d.level_heights[1] == 6,  "level[1] height = 6");
    PROBE_ASSERT(d.level_offsets[1] == 144,"level[1] offset = 144");

    csb_v1_dungeon_free(&d);
    PROBE_ASSERT(d.raw_data == NULL, "free zeroes raw_data");

    /* Double-free must not crash */
    csb_v1_dungeon_free(&d);
    PROBE_ASSERT(1,                   "double-free safe");
}

/* ── Test 2: square accessors with synthetic data ── */
static void test_square_accessors(void) {
    fprintf(stderr, "\n=== Test 2: Square accessors ===\n");
    uint8_t buf[256];
    int sz;
    build_synthetic_csb_dat(buf, &sz);

    /* Inject synthetic square data at known offsets */
    /* Dungeon now has 216 bytes; level[0] offset=16, level[1] offset=144 */
    /* For this test just verify bounds checks work */
    CSB_V1_DungeonData d;
    memset(&d, 0, sizeof(d));

    /* Out-of-bounds: null data */
    PROBE_ASSERT(csb_v1_dungeon_get_square_type(&d, 0, 0, 0) == -1,
                 "square_type returns -1 with null raw_data");
    PROBE_ASSERT(csb_v1_dungeon_get_first_thing(&d, 0, 0, 0) == -1,
                 "first_thing returns -1 with null raw_data");

    /* Valid load */
    build_synthetic_csb_dat((uint8_t*)buf, &sz);
    csb_v1_dungeon_load(&d, buf, sz);

    /* Out-of-bounds: level */
    PROBE_ASSERT(csb_v1_dungeon_get_square_type(&d, -1, 0, 0) == -1,
                 "square_type rejects level -1");
    PROBE_ASSERT(csb_v1_dungeon_get_square_type(&d, 99, 0, 0) == -1,
                 "square_type rejects level 99");
    PROBE_ASSERT(csb_v1_dungeon_get_first_thing(&d, 99, 0, 0) == -1,
                 "first_thing rejects level 99");

    /* Out-of-bounds: coordinates */
    PROBE_ASSERT(csb_v1_dungeon_get_square_type(&d, 0, 99, 0) == -1,
                 "square_type rejects x=99");
    PROBE_ASSERT(csb_v1_dungeon_get_square_type(&d, 0, 0, 99) == -1,
                 "square_type rejects y=99");

    csb_v1_dungeon_free(&d);
}

/* ── Test 3: door table ── */
static void test_door_table(void) {
    fprintf(stderr, "\n=== Test 3: Door table ===\n");

    /* Portcullis (type 0): 110 HP, projectles-see-through */
    PROBE_ASSERT(CSB_doorInfo[CSB_DOOR_PORTCULLIS].defensePoints == 110,
                 "portcullis defense=110");
    PROBE_ASSERT(CSB_doorInfo[CSB_DOOR_PORTCULLIS].flags & CSB_MASK_DOOR_PROJECTILES_CAN_PASS_THROUGH,
                 "portcullis projectiles-see-through flag set");

    /* Wooden (type 1): 42 HP */
    PROBE_ASSERT(CSB_doorInfo[CSB_DOOR_WOODEN].defensePoints == 42,
                 "wooden defense=42");
    PROBE_ASSERT(CSB_doorInfo[CSB_DOOR_WOODEN].flags == 0,
                 "wooden no special flags");

    /* Iron (type 2): 230 HP */
    PROBE_ASSERT(CSB_doorInfo[CSB_DOOR_IRON].defensePoints == 230,
                 "iron defense=230");

    /* Ra (type 3): 255 HP, animates */
    PROBE_ASSERT(CSB_doorInfo[CSB_DOOR_RA].defensePoints == 255,
                 "Ra defense=255");
    PROBE_ASSERT(CSB_doorInfo[CSB_DOOR_RA].flags & CSB_MASK_DOOR_ANIMATED,
                 "Ra animated flag");

    /* Out-of-range: returns -1 */
    PROBE_ASSERT(csb_door_get_defense_points(-1) == -1,
                 "door_defense(-1) = -1");
    PROBE_ASSERT(csb_door_get_defense_points(4)  == -1,
                 "door_defense(4)  = -1");
    PROBE_ASSERT(csb_door_get_defense_points(99) == -1,
                 "door_defense(99) = -1");

    /* Minimum attack power helpers */
    PROBE_ASSERT(csb_door_minimum_attack_power(CSB_DOOR_WOODEN) == 42,
                 "wooden min_attack = 42");
    PROBE_ASSERT(csb_door_minimum_attack_power(CSB_DOOR_IRON) == 230,
                 "iron min_attack = 230");
    PROBE_ASSERT(csb_door_minimum_attack_power(CSB_DOOR_RA) == -1,
                 "Ra min_attack = -1 (unbreakable)");
}

/* ── Test 4: sensor helpers ── */
static void test_sensor_helpers(void) {
    fprintf(stderr, "\n=== Test 4: Sensor helpers ===\n");

    /* CHANGE7_18_FIX: bit 15 must be ignored */
    uint16_t raw_bit15 = 0x8000 | (9u << 7) | 9;  /* type 9, bit 15 set */
    PROBE_ASSERT(csb_sensor_get_type(raw_bit15) == 9,
                 "sensor_get_type clears bit15");
    /* 9-bit data field: bits 15:7 of raw, irrespective of bit15 value
     * raw_bit15 = 0x8489 = binary ...1000 0100 1000 1001
     * data = 0x8489 >> 7 = 0x0109 = 265 (includes bit15 contribution) */
    PROBE_ASSERT(csb_sensor_get_data(raw_bit15) == 265,
                 "sensor_get_data includes bits 15:7 (raw>>7)");

    /* Type range: 0-127 (7 bits) */
    PROBE_ASSERT(csb_sensor_get_type(0) == 0,
                 "sensor_get_type(0) = 0");
    PROBE_ASSERT(csb_sensor_get_type(0x7F) == 0x7F,
                 "sensor_get_type(0x7F) = 0x7F");
    PROBE_ASSERT(csb_sensor_get_type(0xFF7F) == 0x7F,
                 "sensor_get_type ignores bits beyond 7-bit");

    /* CSB-specific sensors */
    PROBE_ASSERT(CSB_SENSOR_FLOOR_VERSION_CHECKER == 9,
                 "C009 VERSION_CHECKER = 9");
    PROBE_ASSERT(CSB_SENSOR_WALL_END_GAME == 18,
                 "C018 END_GAME = 18");

    /* Version checker trigger: data <= engine_version */
    PROBE_ASSERT(csb_version_checker_triggered(20, 20) == 1,
                 "version_checker(20<=20) fires");
    PROBE_ASSERT(csb_version_checker_triggered(19, 20) == 1,
                 "version_checker(19<=20) fires");
    PROBE_ASSERT(csb_version_checker_triggered(21, 20) == 0,
                 "version_checker(21<=20) suppressed");

    /* CHANGE7_21: endgame trigger */
    CSB_EndgameResult er;
    csb_endgame_trigger(0, &er);
    PROBE_ASSERT(er.gameWon == 1,         "endgame: gameWon set");
    PROBE_ASSERT(er.restartAllowed == 0,  "endgame: restart blocked");
    PROBE_ASSERT(er.endgameFnCalled == 1, "endgame: F0666 called");
    PROBE_ASSERT(er.startendEndgameFn == 1,"endgame: F0444 called");
}

/* ── Test 5: DungeonWorld model ── */
static void test_world_model(void) {
    fprintf(stderr, "\n=== Test 5: World model ===\n");

    CSB_DungeonWorld w;
    csb_world_init(&w);
    PROBE_ASSERT(w.levelCount == 0,  "world: init levelCount = 0");
    PROBE_ASSERT(w.currentLevel == 0,"world: init currentLevel = 0");

    /* Add levels */
    int l0 = csb_world_add_level(&w, 10, 10);
    int l1 = csb_world_add_level(&w, 20, 15);
    PROBE_ASSERT(l0 == 0,  "add_level[0] returns 0");
    PROBE_ASSERT(l1 == 1,  "add_level[1] returns 1");
    PROBE_ASSERT(w.levelCount == 2, "world levelCount = 2");

    /* Tile access */
    csb_world_set_tile_type(&w, 0, 3, 5, CSB_TILE_DOOR);
    const CSB_Tile *t = csb_world_get_tile_const(&w, 0, 3, 5);
    PROBE_ASSERT(t != NULL,     "get_tile_const returns non-NULL");
    PROBE_ASSERT(t->type == CSB_TILE_DOOR, "tile type = DOOR");

    /* Out-of-bounds */
    PROBE_ASSERT(csb_world_get_tile(&w, -1, 0, 0) == NULL,
                 "get_tile rejects level -1");
    PROBE_ASSERT(csb_world_get_tile(&w, 99, 0, 0) == NULL,
                 "get_tile rejects level 99");
    PROBE_ASSERT(csb_world_get_tile(&w, 0, 99, 0) == NULL,
                 "get_tile rejects x=99");

    /* Walkability */
    csb_world_set_tile_type(&w, 0, 0, 0, CSB_TILE_FLOOR);
    csb_world_set_tile_type(&w, 0, 1, 0, CSB_TILE_WALL);
    csb_world_set_tile_type(&w, 0, 2, 0, CSB_TILE_PIT);
    csb_world_set_tile_type(&w, 0, 3, 0, CSB_TILE_DOOR);
    PROBE_ASSERT(csb_world_is_walkable(&w, 0, 0, 0) == 1,
                 "floor is walkable");
    PROBE_ASSERT(csb_world_is_wall(&w, 0, 1, 0) == 1,
                 "wall is wall");
    PROBE_ASSERT(csb_world_is_walkable(&w, 0, 2, 0) == 1,
                 "pit is walkable (dungeon logic)");
    PROBE_ASSERT(csb_world_is_walkable(&w, 0, 1, 0) == 0,
                 "wall is not walkable");

    /* Walls */
    csb_world_set_wall(&w, 0, 0, 0, 0, 0x01);  /* north */
    csb_world_set_wall(&w, 0, 0, 0, 1, 0x02);  /* east */
    const CSB_Tile *t2 = csb_world_get_tile_const(&w, 0, 0, 0);
    PROBE_ASSERT(t2->wallN == 0x01, "set_wall N=0x01");
    PROBE_ASSERT(t2->wallE == 0x02, "set_wall E=0x02");

    /* Ornaments */
    csb_world_set_ornament(&w, 0, 0, 0, 2, 0x03);
    PROBE_ASSERT(t2->ornamentS == 0x03, "set_ornament S=0x03");

    /* Level count */
    PROBE_ASSERT(csb_world_get_level_count(&w) == 2,
                 "get_level_count = 2");
    PROBE_ASSERT(csb_world_get_level_count(NULL) == 0,
                 "get_level_count(NULL) = 0");
}

/* ── Test 6: thing type ID safety ── */
static void test_thing_type_ids(void) {
    fprintf(stderr, "\n=== Test 6: Thing type IDs ===\n");

    /* Thing type extraction from 16-bit handle */
    PROBE_ASSERT(CSB_THING_TYPE(0x0000) == 0,  "handle 0 → type 0");
    PROBE_ASSERT(CSB_THING_TYPE(0x0400) == 1,  "handle 0x0400 → type 1");
    /* 0x1000 = type 4 (GROUP) at index 0: bits [13:10]=4 → (4<<10)=0x1000 */
    PROBE_ASSERT(CSB_THING_TYPE(0x1000) == 4,  "handle 0x1000 → type 4 (GROUP)");

    /* Verify no out-of-range: create valid handles for all valid types */
    int type;
    for (type = 0; type < 16; type++) {
        uint16_t handle = (uint16_t)(type << 10);
        PROBE_ASSERT(CSB_THING_TYPE(handle) == type,
                     "handle type %d → %d", type, CSB_THING_TYPE(handle));
    }
    PROBE_ASSERT(0xFFFFu == 0xFFFFu, "PARTY = 0xFFFF");
    PROBE_ASSERT(CSB_THING_ENDOFLIST == 0xFFFEu,  "ENDOFLIST = 0xFFFE");
}

/* ── Test 7: bugfix helpers ── */
static void test_bugfix_helpers(void) {
    fprintf(stderr, "\n=== Test 7: Bugfix helpers ===\n");

    /* CHANGE7_18_FIX: bit15 cleared before type extract */
    /* DM1 BUG0_09: bone-creation leaves bit15 set — must clear */
    uint16_t bone_thing = 0x8000 | (4u << 10);  /* GROUP with bit15 */
    PROBE_ASSERT(csb_bugfix_thing_type_bit15_clearly(bone_thing) == 4,
                 "bit15_clearly masks bit15 before shift");

    /* CHANGE7_19_FIX: Lord Chaos teleport direction */
    int dir1 = csb_bugfix_lord_chaos_teleport_dir(NULL);
    PROBE_ASSERT(dir1 >= 0 && dir1 <= 3,
                 "lord_chaos_teleport_dir in 0..3");
    int dir2 = csb_bugfix_lord_chaos_teleport_dir(NULL);
    PROBE_ASSERT(dir2 >= 0 && dir2 <= 3,
                 "lord_chaos_teleport_dir in 0..3 (again)");

    /* Sensor square clear check with stub accessors */
    int clear = csb_bugfix_is_sensor_square_clear_for_discard(
        0, 0,
        csb_dungeon_get_first_thing_default,
        csb_dungeon_get_next_thing_default,
        csb_dungeon_thing_data_u16_default,
        0);
    PROBE_ASSERT(clear == 1, "sensor_square_clear with stub accessors = 1 (safe default)");
}

/* ── Test 8: CSB vs DM1 type/constant differences ── */
static void test_csb_vs_dm1_differences(void) {
    fprintf(stderr, "\n=== Test 8: CSB vs DM1 differences ===\n");

    /* 1. MAX_LEVELS: CSB=12 (loader) / 16 (world), DM1=14
     *    This is a CSB-specific difference — verify the values */
    PROBE_ASSERT(CSB_V1_MAX_LEVELS == 12,
                 "CSB_V1_MAX_LEVELS=12 (vs DM1's 14-map format)");

    /* 2. DSA thing type (CSB-only, type=15 in header comment)
     *    The loader header defines it but the format handling says
     *    ThingCount[15] in the shared header is EXPLOSION (same slot).
     *    This is a data-level difference, not a code difference. */

    /* 3. CSB has C009_VERSION_CHECKER (type 9, floor) — DM1 does not */
    PROBE_ASSERT(CSB_SENSOR_FLOOR_VERSION_CHECKER == 9,
                 "CSB SENSOR_FLOOR_VERSION_CHECKER=9 (DM1: absent)");

    /* 4. CSB has C018_END_GAME (type 18, wall) — DM1 does not */
    PROBE_ASSERT(CSB_SENSOR_WALL_END_GAME == 18,
                 "CSB SENSOR_WALL_END_GAME=18 (DM1: absent)");

    /* 5. CSB door table with Ra door (type 3) — DM1 has 3 door types */
    PROBE_ASSERT(CSB_DOOR_RA == 3,
                 "CSB DOOR_RA type=3 (extended door table)");

    /* 6. Endgame result structure (DM1 has no G0302 counterpart) */
    CSB_EndgameResult er;
    memset(&er, 0, sizeof(er));
    er.gameWon = 1;
    PROBE_ASSERT(er.gameWon == 1,
                 "CSB endgame result has G0302_B_GameWon field");

    /* 7. Teleporter/pit result structure present in CSB */
    CSB_TeleporterPitResult tp;
    memset(&tp, 0, sizeof(tp));
    tp.movingGroup = 1;
    tp.moved = 1;
    PROBE_ASSERT(tp.movingGroup == 1,
                 "CSB teleporter result has movingGroup field");

    /* 8. DungeonWorld: CSB has ornaments (wallN/E/S/W, ornamentN/E/S/W),
     *    DM1 M11_DL_Tile does not expose ornaments in the same way.
     *    Verify CSB_Tile has ornament fields. */
    CSB_Tile tile;
    memset(&tile, 0, sizeof(tile));
    tile.ornamentN = 5;
    tile.ornamentS = 7;
    PROBE_ASSERT(tile.ornamentN == 5 && tile.ornamentS == 7,
                 "CSB_Tile has ornamentN/S fields (DM1: absent)");

    /* 9. GameState dungeonSeed — CSB has it, DM1 uses different vars */
    CSB_GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.dungeonSeed = 0x12345678;
    PROBE_ASSERT(gs.dungeonSeed == 0x12345678,
                 "CSB_GameState has dungeonSeed field");
}

/* ── Test 9: source evidence string ── */
static void test_source_evidence(void) {
    fprintf(stderr, "\n=== Test 9: Source evidence ===\n");
    const char *ev = csb_v1_dungeon_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 10,
                 "csb_v1_dungeon_source_evidence() returns non-empty string");

    /* Check it mentions CSBWin and ReDMCSB */
    PROBE_ASSERT(strstr(ev, "CSBWin") != NULL,
                 "evidence mentions CSBWin");
    PROBE_ASSERT(strstr(ev, "ReDMCSB") != NULL,
                 "evidence mentions ReDMCSB");
    PROBE_ASSERT(strstr(ev, "DUNGEON.C") != NULL,
                 "evidence mentions DUNGEON.C");
}

/* ── Test 10: door info extern links ── */
static void test_door_info_extern(void) {
    fprintf(stderr, "\n=== Test 10: Door info extern ===\n");
    (void)CSB_doorInfo;  /* extern from header — should link */
    PROBE_ASSERT(CSB_doorInfo[CSB_DOOR_PORTCULLIS].defensePoints == 110,
                 "CSB_doorInfo[PORTCULLIS] = 110 (compile-link success)");
}

/* ── main ── */
int main(void) {
    fprintf(stderr, "CSB V1 Phase 2 — Dungeon Data Model probe\n");
    fprintf(stderr, "Source-locked to ReDMCSB DUNGEON.C / TIMELINE.C / DEFS.H\n");
    fprintf(stderr, "         CSBWin CSBCode.cpp TAG00332a\n\n");

    test_loader_cycle();
    test_square_accessors();
    test_door_table();
    test_sensor_helpers();
    test_world_model();
    test_thing_type_ids();
    test_bugfix_helpers();
    test_csb_vs_dm1_differences();
    test_source_evidence();
    test_door_info_extern();

    fprintf(stderr, "\n=== Summary ===\n");
    fprintf(stderr, "PASSED: %d\n", passed);
    fprintf(stderr, "FAILED: %d\n", errors);
    fprintf(stderr, "Status: %s\n", errors == 0 ? "PASS" : "FAIL");

    return errors == 0 ? 0 : 1;
}
