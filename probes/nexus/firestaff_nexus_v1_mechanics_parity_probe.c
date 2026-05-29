/*
 * Nexus V1 Mechanics Parity Probe
 * ===============================
 * Headless compile/link verification of the Nexus V1 mechanics APIs.
 *
 * Verifies API signatures are correct and callable without requiring
 * any game data or SDL initialization.  Exercises all major systems:
 *
 *   1. Dungeon loading   — DGN Structure1B (64x64, 8 bytes/cell)
 *   2. Movement          — nexus_v1_movement.h API
 *   3. Combat            — nexus_v1_combat.h API
 *   4. Save/Load         — nexus_v1_save.h API signatures
 *   5. World state       — nexus_v1_world.h API
 *   6. Engine lifecycle  — nexus_v1_init() / nexus_v1_shutdown()
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_mechanics_parity_probe
 * Or:
 *   cmake --build build --parallel
 *   ctest --test-dir build -R "nexus_v1_mechanics" -j4 --output-on-failure
 *
 * Source-lock references:
 *   DMWeb DGN — http://dmweb.free.fr/ ("Dungeon Master Nexus DGN files",
 *             fetched 2026-05-28) for DGN Structure1B format.
 *   ReDMCSB — Starcraft's decompilation WIP20210206/Toolchains/Common/Source/
 *             DUNGEON.C, COMMAND.C, MOVESENS.C, CHAMPION.C, CLIKMENU.C,
 *             CREATURE.C, LOADSAVE.C, SAVEHEAD.C.
 *             Primary source lock for movement/combat/save lifecycle.
 *   SDDRVS  — SDDRVS.TSK script opcodes documented in docs/nexus_triggers.md.
 */

/* ── System headers ─────────────────────────────────────────────────── */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* ── Nexus V1 headers ──────────────────────────────────────────────── */
#include "nexus_v1_engine.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_combat.h"
#include "nexus_v1_save.h"
#include "nexus_v1_world.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_game.h"

/* ═══════════════════════════════════════════════════════════════════════
 * CHECK macro — accumulate pass/fail counts and print result
 * ═══════════════════════════════════════════════════════════════════════ */
static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond_, msg_) do { \
    if (cond_) { \
        printf("  [PASS] %s\n", (msg_)); \
        g_pass++; \
    } else { \
        printf("  [FAIL] %s\n", (msg_)); \
        g_fail++; \
    } \
} while (0)

/* ── Utility helpers for synthetic DGN fixture ──────────────────────── */

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

/*
 * Build a minimal synthetic LEV01.DGN (2048-byte block container).
 * Structure1B: 64×64 grid at offset rel=0x40, 8 bytes per cell.
 *
 * Layout of the block container (per DMWeb "DGN files" 2026-05-28):
 *   Block 0: unused header region
 *   Block 1..17: Structure1 container (block header + grid data)
 *     0x00 (uint16): block number of grid  = 1
 *     0x02 (uint16): block count            = 18
 *     0x04 (uint32): end offset from start  = 0x40 + 0x8000
 *     0x08..0x3F: padding
 *     0x40..: grid data (64×64×8 = 0x8000 bytes)
 *
 * Grid cell at [y][x] = grid[(y*64+x)*8]:
 *   byte[0] = thing id lo
 *   byte[1] = thing id hi
 *   byte[2] = floor texture
 *   byte[3] = wall texture (N/E info)
 *   byte[4] = wall texture (S/W info)
 *   byte[5] = thing id mid
 *   byte[6] = square_type (0=wall, 1=floor, 2=pit, etc.)
 *   byte[7] = extended flags / 3D geometry flag
 */
static void build_synthetic_dgn(uint8_t *buf, size_t bufsz)
{
    const int BLOCK  = 2048;
    const int GRID_rel = 0x40;
    const int GRID_bytes = NEXUS_MAX_MAP_SIZE * NEXUS_MAX_MAP_SIZE
                           * NEXUS_DGN_STRUCTURE1B_CELL_BYTES; /* 0x8000 */

    memset(buf, 0, bufsz);

    /* Block 1 header (Structure1) */
    uint8_t *s1 = buf + BLOCK;                  /* block 1 in container */
    write_be16(s1 + 0x00, 1);                   /* grid at block 1 */
    write_be16(s1 + 0x02, 18);                  /* 18 blocks total */
    write_be32(s1 + 0x08, (uint32_t)GRID_rel); /* offset to grid data */
    write_be32(s1 + 0x0C, (uint32_t)(GRID_rel + GRID_bytes)); /* end */

    /* Grid data: default floor, walls on all four edges */
    uint8_t *grid = s1 + GRID_rel;
    memset(grid, 0x01, GRID_bytes);             /* default: floor */
    int xy;
    for (xy = 0; xy < 64; xy++) {
        grid[(0  * 64 + xy) * 8] = 0;            /* north edge: wall */
        grid[(63 * 64 + xy) * 8] = 0;            /* south edge: wall */
        grid[(xy * 64 + 0)  * 8] = 0;            /* west edge:  wall */
        grid[(xy * 64 + 63) * 8] = 0;            /* east edge:  wall */
    }
    /* Special squares */
    grid[(20 * 64 + 10) * 8] = NEXUS_SQUARE_STAIRS_DN;
    grid[(21 * 64 + 11) * 8] = NEXUS_SQUARE_TELEPORT;
    grid[(22 * 64 + 12) * 8] = NEXUS_SQUARE_PIT;
    grid[(23 * 64 + 13) * 8] = NEXUS_SQUARE_EXIT;
    /* Boss chamber — 3D geometry flag in byte[7] */
    grid[(30 * 64 + 30) * 8] = NEXUS_SQUARE_FLOOR;
    grid[(30 * 64 + 30) * 8 + 7] = NEXUS_SQF_3D_ONLY;
    grid[(30 * 64 + 30) * 8 + 6] = 0x0F;
}

/* ═══════════════════════════════════════════════════════════════════════
 * 1. Dungeon loading — verify DGN Structure1B loads correctly
 * Source: DMWeb DGN format (2026-05-28);
 *         nexus_v1_dungeon.h, src/nexus/nexus_v1_dungeon.c
 * ═══════════════════════════════════════════════════════════════════════ */
static void probe_dungeon(void)
{
    printf("\n[Probe 1: Dungeon Loading -- DGN Structure1B]\n");
    printf("  Source: DMWeb DGN format (64x64 grid, 8 bytes/cell, 0x8000 bytes)\n");
    printf("  Lock:   nexus_v1_dungeon.h; src/nexus/nexus_v1_dungeon.c\n");

    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    build_synthetic_dgn(dgn, sizeof(dgn));

    Nexus_V1_Level level;
    memset(&level, 0xAA, sizeof(level));

    int r = nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1);
    CHECK(r == 0,                       "nexus_v1_level_load returns 0 on valid DGN");
    CHECK(level.width  == 64,           "width  = 64");
    CHECK(level.height == 64,           "height = 64");
    CHECK(level.squares[0][0]   == 0,   "wall at corner (0,0)");
    CHECK(level.squares[1][1]   == 1,   "floor at (1,1)");
    CHECK(level.squares[20][10] == NEXUS_SQUARE_STAIRS_DN,
          "stairs-down at (10,20)");
    CHECK(level.squares[21][11] == NEXUS_SQUARE_TELEPORT,
          "teleporter at (11,21)");
    CHECK(level.squares[22][12] == NEXUS_SQUARE_PIT,
          "pit at (12,22)");
    CHECK(level.squares[23][13] == NEXUS_SQUARE_EXIT,
          "exit at (13,23)");
    CHECK(level.squares[30][30] == NEXUS_SQUARE_FLOOR,
          "boss chamber at (30,30) is floor type");

    CHECK(nexus_v1_level_get_square(&level, 0,  0)  == 0,
          "get_square(0,0):    wall");
    CHECK(nexus_v1_level_get_square(&level, 1,  1)  == 1,
          "get_square(1,1):    floor");
    CHECK(nexus_v1_level_get_square(&level, 99, 99) == 0,
          "get_square(99,99): OOB returns wall");
    CHECK(nexus_v1_level_get_square(&level, 20, 10) == NEXUS_SQUARE_STAIRS_DN,
          "get_square(10,20): stairs-down");
}

/* ═══════════════════════════════════════════════════════════════════════
 * 2. Movement API — verify movement types and functions callable
 * Source: src/nexus/nexus_v1_movement.c;
 *         ReDMCSB COMMAND.C F0380, CLIKMENU.C F0366, MOVESENS.C F0267
 * ═══════════════════════════════════════════════════════════════════════ */
static void probe_movement(void)
{
    printf("\n[Probe 2: Movement API -- nexus_v1_movement.h]\n");
    printf("  Source: ReDMCSB COMMAND.C F0380, CLIKMENU.C F0366,\n");
    printf("          MOVESENS.C F0267; nexus_v1_movement.c\n");

    /* Direction constants */
    CHECK(NEXUS_DIR_NORTH == 0, "NEXUS_DIR_NORTH = 0");
    CHECK(NEXUS_DIR_EAST  == 1, "NEXUS_DIR_EAST  = 1");
    CHECK(NEXUS_DIR_SOUTH == 2, "NEXUS_DIR_SOUTH = 2");
    CHECK(NEXUS_DIR_WEST  == 3, "NEXUS_DIR_WEST  = 3");

    /* Command constants */
    CHECK(NEXUS_CMD_NONE        == 0,  "NEXUS_CMD_NONE        = 0");
    CHECK(NEXUS_CMD_FORWARD     == 1,  "NEXUS_CMD_FORWARD     = 1");
    CHECK(NEXUS_CMD_BACKWARD    == 2,  "NEXUS_CMD_BACKWARD    = 2");
    CHECK(NEXUS_CMD_TURN_LEFT   == 3,  "NEXUS_CMD_TURN_LEFT   = 3");
    CHECK(NEXUS_CMD_TURN_RIGHT  == 4,  "NEXUS_CMD_TURN_RIGHT  = 4");
    CHECK(NEXUS_CMD_STRAFE_LEFT == 5,  "NEXUS_CMD_STRAFE_LEFT = 5");
    CHECK(NEXUS_CMD_STRAFE_RIGHT== 6,  "NEXUS_CMD_STRAFE_RIGHT= 6");
    CHECK(NEXUS_CMD_COUNT == 11,       "NEXUS_CMD_COUNT = 11");

    /* Movement result codes */
    CHECK(NEXUS_MOVE_OK            == 0, "NEXUS_MOVE_OK            = 0");
    CHECK(NEXUS_MOVE_BLOCKED_WALL  == 1, "NEXUS_MOVE_BLOCKED_WALL = 1");

    /* Square type constants */
    CHECK(NEXUS_SQUARE_WALL      == 0,  "NEXUS_SQUARE_WALL       = 0");
    CHECK(NEXUS_SQUARE_FLOOR     == 1,  "NEXUS_SQUARE_FLOOR      = 1");
    CHECK(NEXUS_SQUARE_STAIRS_DN == 2,  "NEXUS_SQUARE_STAIRS_DN  = 2");
    CHECK(NEXUS_SQUARE_STAIRS_UP == 3,  "NEXUS_SQUARE_STAIRS_UP  = 3");
    CHECK(NEXUS_SQUARE_DOOR      == 8,  "NEXUS_SQUARE_DOOR       = 8");
    CHECK(NEXUS_SQUARE_TELEPORT  == 9,  "NEXUS_SQUARE_TELEPORT   = 9");
    CHECK(NEXUS_SQUARE_EXIT      == 14, "NEXUS_SQUARE_EXIT       = 14");
    CHECK(NEXUS_SQUARE_WATER     == 21, "NEXUS_SQUARE_WATER      = 21");
    CHECK(NEXUS_SQUARE_FIRE      == 22, "NEXUS_SQUARE_FIRE       = 22");

    /* Movement state struct */
    Nexus_MovementState ms;
    memset(&ms, 0, sizeof(ms));
    nexus_movement_init(&ms, 11, 29, NEXUS_DIR_NORTH);
    CHECK(ms.party_x     == 11,   "movement_init: party_x = 11 (DM1 entrance)");
    CHECK(ms.party_y     == 29,   "movement_init: party_y = 29");
    CHECK(ms.party_dir   == 0,     "movement_init: party_dir = North");
    CHECK(ms.disabled_ticks == 0, "movement_init: disabled_ticks = 0");

    /* Input queue */
    Nexus_InputQueue q;
    memset(&q, 0, sizeof(q));
    nexus_input_queue_init(&q);
    CHECK(q.count == 0, "input_queue_init: count = 0");
    int ok = nexus_input_queue_push(&q, NEXUS_CMD_FORWARD);
    CHECK(ok == 1,             "input_queue_push: returns 1 on success");
    CHECK(q.count == 1,        "input_queue_push: count = 1");
    int cmd = -1;
    ok = nexus_input_queue_pop(&q, &cmd);
    CHECK(ok == 1,             "input_queue_pop:  returns 1");
    CHECK(cmd == NEXUS_CMD_FORWARD, "input_queue_pop: cmd = FORWARD");
    ok = nexus_input_queue_pop(&q, &cmd);
    CHECK(ok == 0,             "input_queue_pop:  returns 0 when empty");

    /* Direction deltas */
    int dx = 999, dy = 999;
    nexus_dir_deltas(NEXUS_DIR_NORTH, &dx, &dy);
    CHECK(dx ==  0 && dy == -1, "dir_deltas(North):  dx=0,  dy=-1");
    nexus_dir_deltas(NEXUS_DIR_EAST,  &dx, &dy);
    CHECK(dx ==  1 && dy ==  0, "dir_deltas(East):   dx=1,  dy=0");
    nexus_dir_deltas(NEXUS_DIR_SOUTH, &dx, &dy);
    CHECK(dx ==  0 && dy ==  1, "dir_deltas(South): dx=0,  dy=1");
    nexus_dir_deltas(NEXUS_DIR_WEST,  &dx, &dy);
    CHECK(dx == -1 && dy ==  0, "dir_deltas(West):   dx=-1, dy=0");

    /* Normalize / dir_diff */
    CHECK(nexus_normalize_dir(0)  == 0, "normalize_dir(0) = 0");
    CHECK(nexus_normalize_dir(4)  == 0, "normalize_dir(4) = 0");
    CHECK(nexus_normalize_dir(-1) == 3, "normalize_dir(-1) = 3");
    CHECK(nexus_dir_diff(0, 2) ==  2,  "dir_diff(N->S) =  2");
    CHECK(nexus_dir_diff(2, 0) == -2,   "dir_diff(S->N) = -2");

    /* Target square computation */
    int tx = 999, ty = 999;
    nexus_target_square(10, 10, NEXUS_DIR_NORTH, 1, 0, 0, &tx, &ty);
    CHECK(tx == 10 && ty == 9,  "target_square: forward N  -> (10,9)");
    nexus_target_square(10, 10, NEXUS_DIR_NORTH, 0, 0, 0, &tx, &ty);
    CHECK(tx == 10 && ty == 11, "target_square: backward N -> (10,11)");
    nexus_target_square(10, 10, NEXUS_DIR_NORTH, 1, 1, 1, &tx, &ty);
    CHECK(tx == 9  && ty == 9,   "target_square: strafe left  N -> (9,9)");
    nexus_target_square(10, 10, NEXUS_DIR_NORTH, 1, 1, 0, &tx, &ty);
    CHECK(tx == 11 && ty == 9,   "target_square: strafe right N -> (11,9)");

    /* Square helpers */
    CHECK(nexus_square_is_stairs(NEXUS_SQUARE_STAIRS_DN) == 1,
          "square_is_stairs(STAIRS_DN)");
    CHECK(nexus_square_is_pit(NEXUS_SQUARE_PIT) == 1,
          "square_is_pit(PIT)");
    CHECK(nexus_square_is_teleporter(NEXUS_SQUARE_TELEPORT) == 1,
          "square_is_teleporter(TELEPORT)");
    CHECK(nexus_square_is_door(NEXUS_SQUARE_DOOR) == 1,
          "square_is_door(DOOR)");
    CHECK(nexus_square_is_passable(NEXUS_SQUARE_WALL) == 0,
          "square_is_passable(WALL) = false");
    CHECK(nexus_square_is_passable(NEXUS_SQUARE_FLOOR) == 1,
          "square_is_passable(FLOOR) = true");

    /* Stamina cost */
    CHECK(nexus_movement_stamina_cost(40, 100) >= 0,
          "movement_stamina_cost: callable, non-negative");

    /* Turn */
    int dir = NEXUS_DIR_NORTH;
    Nexus_MoveResultData res;
    memset(&res, 0, sizeof(res));
    nexus_turn(&dir, 1 /* right */);
    CHECK(dir == NEXUS_DIR_EAST,  "turn right: N->E");
    nexus_turn(&dir, 1);
    CHECK(dir == NEXUS_DIR_SOUTH, "turn right: E->S");
    nexus_turn(&dir, 0 /* left */);
    CHECK(dir == NEXUS_DIR_EAST,  "turn left:  S->E");

    /* Build a test dungeon grid for nexus_try_move */
    uint8_t squares64[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    int i;
    for (i = 0; i < NEXUS_MAX_MAP_SIZE; i++)
        memset(squares64[i], NEXUS_SQUARE_FLOOR, NEXUS_MAX_MAP_SIZE);
    for (i = 0; i < NEXUS_MAX_MAP_SIZE; i++) {
        squares64[0][i]   = NEXUS_SQUARE_WALL;
        squares64[63][i]  = NEXUS_SQUARE_WALL;
        squares64[i][0]   = NEXUS_SQUARE_WALL;
        squares64[i][63]  = NEXUS_SQUARE_WALL;
    }

    /* Free forward step */
    int mx = 10, my = 10;
    int move_result = 999;
    int nmx = 999, nmy = 999;
    int ok2 = nexus_try_move(NEXUS_DIR_NORTH, 1, squares64,
                              &mx, &my, &move_result, &nmx, &nmy);
    CHECK(ok2 == 1,             "nexus_try_move: forward step succeeds");
    CHECK(nmx == 10 && nmy == 9,
          "nexus_try_move: moved north to (10,9)");
    CHECK(move_result == NEXUS_MOVE_OK,
          "nexus_try_move: result = MOVE_OK");

    /* Blocked by wall */
    mx = 0; my = 0;
    ok2 = nexus_try_move(NEXUS_DIR_NORTH, 1, squares64,
                          &mx, &my, &move_result, &nmx, &nmy);
    CHECK(ok2 == 1,                  "nexus_try_move: blocked returns 1");
    CHECK(move_result == NEXUS_MOVE_BLOCKED_WALL,
          "nexus_try_move: result = BLOCKED_WALL");

    /* Square getter */
    CHECK(nexus_get_square(squares64, 10, 10) == NEXUS_SQUARE_FLOOR,
          "get_square(10,10) = floor");
    CHECK(nexus_get_square(squares64, 0, 5) == NEXUS_SQUARE_WALL,
          "get_square(0,5) = wall");
}

/* ═══════════════════════════════════════════════════════════════════════
 * 3. Combat API — verify combat functions callable
 * Source: src/nexus/nexus_v1_combat.c;
 *         ReDMCSB CREATURE.C, CHAMPION.C F0309
 * ═══════════════════════════════════════════════════════════════════════ */
static void probe_combat(void)
{
    printf("\n[Probe 3: Combat API -- nexus_v1_combat.h]\n");
    printf("  Source: ReDMCSB CREATURE.C, CHAMPION.C F0309;\n");
    printf("          nexus_v1_combat.c\n");

    /* Nexus_CombatResult */
    Nexus_CombatResult cr;
    memset(&cr, 0, sizeof(cr));
    cr.attack_type = 0;
    cr.damage     = 0;
    cr.hit        = 0;
    cr.critical   = 0;
    cr.experience_gained = 0;
    CHECK(cr.attack_type == 0, "Nexus_CombatResult fields are writable");

    /* Nexus_V1_Champion (layout from nexus_v1_champions.h) */
    Nexus_V1_Champion champ;
    memset(&champ, 0, sizeof(champ));
    champ.health      = 50;
    champ.max_health  = 100;
    champ.stamina    = 100;
    champ.max_stamina = 100;
    champ.load       = 30;
    champ.max_load   = 80;
    champ.alive      = 1;
    champ.primary_class = NEXUS_CLASS_FIGHTER;
    CHECK(champ.health == 50,  "Nexus_V1_Champion.health is writable");
    CHECK(champ.alive == 1,    "Nexus_V1_Champion.alive is writable");

    /* Call combat functions — link verification only; return values
     * are discarded since full combat requires game data. */
    (void)nexus_v1_attack(&champ, 5, 3);
    (void)nexus_v1_take_damage(&champ, 10);
    (void)nexus_v1_gain_experience(&champ, NEXUS_CLASS_PRIEST, 100);

    CHECK(champ.alive == 1 || champ.health >= 0,
          "nexus_v1_attack/take_damage/gain_xp are callable without linker error");
}

/* ═══════════════════════════════════════════════════════════════════════
 * 4. Save/Load API — verify save/load function signatures
 * Source: nexus_v1_save.h, src/nexus/nexus_v1_save_load.c;
 *         ReDMCSB LOADSAVE.C F0433/F0434, SAVEHEAD.C F0429/F0430
 * ═══════════════════════════════════════════════════════════════════════ */
static void probe_save_load(void)
{
    printf("\n[Probe 4: Save/Load API -- nexus_v1_save.h]\n");
    printf("  Source: ReDMCSB LOADSAVE.C F0433/F0434,\n");
    printf("          SAVEHEAD.C F0429/F0430; nexus_v1_save_load.c\n");

    /* Magic and version constants */
    CHECK(NEXUS_SAVE_MAGIC == 0x53584E46U,
          "NEXUS_SAVE_MAGIC = 'FNXS'");
    CHECK(NEXUS_SAVE_VERSION == 2,
          "NEXUS_SAVE_VERSION = 2");
    CHECK(NEXUS_SAVE_MAX_SLOTS == 8,
          "NEXUS_SAVE_MAX_SLOTS = 8");

    /* Nexus_V1_SaveHeader */
    Nexus_V1_SaveHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic         = NEXUS_SAVE_MAGIC;
    hdr.version       = NEXUS_SAVE_VERSION;
    hdr.data_size     = 0;
    hdr.game_time     = 0;
    hdr.crc32         = 0;
    hdr.current_level = 0;
    hdr.party_x       = 11;
    hdr.party_y       = 29;
    hdr.party_dir     = 0;
    hdr.state_hash    = 0;
    CHECK(hdr.magic == NEXUS_SAVE_MAGIC,     "SaveHeader.magic is writable");
    CHECK(hdr.version == 2,                 "SaveHeader.version is writable");

    /* Nexus_V1_SaveSlot */
    Nexus_V1_SaveSlot slot;
    memset(&slot, 0, sizeof(slot));
    slot.occupied   = 0;
    slot.slot_index = 0;
    CHECK(slot.occupied == 0,                 "Nexus_V1_SaveSlot.occupied is writable");

    /* Nexus_SaveResult enum values */
    Nexus_SaveResult res = NEXUS_SAVE_OK;
    CHECK(res == 0,                     "NEXUS_SAVE_OK = 0");
    res = NEXUS_SAVE_ERR_NULL;
    CHECK(res != 0,                     "NEXUS_SAVE_ERR_NULL != 0");
    res = NEXUS_SAVE_ERR_OPEN;
    CHECK(res != 0,                     "NEXUS_SAVE_ERR_OPEN != 0");
    res = NEXUS_SAVE_ERR_MAGIC;
    CHECK(res != 0,                     "NEXUS_SAVE_ERR_MAGIC != 0");
    res = NEXUS_SAVE_ERR_VERSION;
    CHECK(res != 0,                     "NEXUS_SAVE_ERR_VERSION != 0");
    res = NEXUS_SAVE_ERR_DATA_DIR;
    CHECK(res != 0,                     "NEXUS_SAVE_ERR_DATA_DIR != 0");

    /* nexus_v1_save_strerror() */
    const char *s = nexus_v1_save_strerror(NEXUS_SAVE_OK);
    CHECK(s != NULL && s[0] != '\0',
          "nexus_v1_strerror(OK) returns non-empty string");
    s = nexus_v1_save_strerror(NEXUS_SAVE_ERR_MAGIC);
    CHECK(s != NULL, "nexus_v1_strerror(ERR_MAGIC) returns non-NULL");

    /* Nexus_V1_SaveManager */
    Nexus_V1_SaveManager mgr;
    memset(&mgr, 0, sizeof(mgr));
    mgr.initialized = 0;
    CHECK(mgr.initialized == 0, "SaveManager.initialized is writable");

    /* Verify functions compile by calling with dummy args.
     * No files are opened; this only checks symbol visibility. */
    (void)nexus_v1_save_init(&mgr, "/tmp");
    (void)nexus_v1_save_scan(&mgr);
    (void)nexus_v1_save_delete(&mgr, 0);

    /* Build an in-memory FNXS v2 header and verify field access */
    {
        uint8_t buf[256];
        memset(buf, 0, sizeof(buf));
        Nexus_V1_SaveHeader *h = (Nexus_V1_SaveHeader *)buf;
        h->magic         = NEXUS_SAVE_MAGIC;
        h->version       = NEXUS_SAVE_VERSION;
        h->header_size   = (uint16_t)sizeof(Nexus_V1_SaveHeader);
        h->data_size     = 0;
        h->crc32         = 0;
        h->current_level = 5;
        h->party_x       = 31;
        h->party_y       = 31;
        h->party_dir     = NEXUS_DIR_SOUTH;
        CHECK(h->magic == NEXUS_SAVE_MAGIC,      "in-memory hdr magic");
        CHECK(h->party_x == 31,                   "in-memory hdr party_x");
        CHECK(h->party_dir == NEXUS_DIR_SOUTH,    "in-memory hdr party_dir");
    }

    /* nexus_v1_save_probe() with non-existent path */
    Nexus_V1_SaveHeader probe_hdr;
    size_t file_size = 0;
    s = nexus_v1_save_probe("/nonexistent/path/LEV00.DGN", &probe_hdr, &file_size);
    CHECK(s != NULL, "nexus_v1_save_probe returns non-NULL for non-existent path");
}

/* ═══════════════════════════════════════════════════════════════════════
 * 5. World State API — verify world/objects/events/timers API
 * Source: nexus_v1_world.h, src/nexus/nexus_v1_world.c;
 *         ReDMCSB DUNGEON.C F0029/F0044, MOVESENS.C F0067/F0071
 * ═══════════════════════════════════════════════════════════════════════ */
static void probe_world(void)
{
    printf("\n[Probe 5: World State API -- nexus_v1_world.h]\n");
    printf("  Source: ReDMCSB DUNGEON.C F0029/F0044,\n");
    printf("          MOVESENS.C F0067/F0071; nexus_v1_world.c\n");

    /* World struct */
    Nexus_V1_World world;
    nexus_v1_world_init(&world);
    CHECK(world.party_level   == 0,   "world.party_level = 0");
    CHECK(world.party_x       == 11,  "world.party_x = 11 (DM1 entrance)");
    CHECK(world.party_y       == 29,  "world.party_y = 29");
    CHECK(world.party_dir     == 0,    "world.party_dir = North");
    CHECK(world.world_tick    == 0,    "world.world_tick = 0");
    CHECK(world.object_count  == 0,   "world.object_count = 0");
    CHECK(world.event_count   == 0,   "world.event_count = 0");
    CHECK(world.timer_count   == 0,   "world.timer_count = 0");

    /* Opcode enum */
    CHECK(NEXUS_OP_NONE == 0,               "NEXUS_OP_NONE = 0");
    CHECK(NEXUS_OP_WHEN_PARTY_ON_XY == 1,   "NEXUS_OP_WHEN_PARTY_ON_XY = 1");
    CHECK(NEXUS_OP_TELEPORT == 10,          "NEXUS_OP_TELEPORT = 10");
    CHECK(NEXUS_OP_COUNT > NEXUS_OP_DISPLAY_MESSAGE,
          "NEXUS_OP_COUNT exceeds DISPLAY_MESSAGE");

    /* Event type enum */
    CHECK(NEXUS_EVT_NONE         == 0,   "NEXUS_EVT_NONE = 0");
    CHECK(NEXUS_EVT_PARTY_STEP   == 1,   "NEXUS_EVT_PARTY_STEP = 1");
    CHECK(NEXUS_EVT_LEVEL_LOADED == 7, "NEXUS_EVT_LEVEL_LOADED = 7");
    CHECK(NEXUS_EVT_SCRIPT_TRIGGER == 10, "NEXUS_EVT_SCRIPT_TRIGGER = 10");

    /* Timer kind enum */
    CHECK(NEXUS_TIMER_ONESHOT    == 0, "NEXUS_TIMER_ONESHOT = 0");
    CHECK(NEXUS_TIMER_REPEAT     == 1, "NEXUS_TIMER_REPEAT = 1");
    CHECK(NEXUS_TIMER_COUNTDOWN  == 2, "NEXUS_TIMER_COUNTDOWN = 2");

    /* Object placement */
    Nexus_V1_Object obj = {
        .id = 0, .type = NEXUS_OBJECT_CHEST, .state = 0,
        .x = 5, .y = 10, .level = 0, .quantity = 1,
        .linked_id = 0, .flags = 0
    };
    int id1 = nexus_v1_object_place(&world, &obj);
    CHECK(id1 > 0, "object_place returns positive id");

    obj.type = NEXUS_OBJECT_DOOR;
    obj.x = 7; obj.y = 12;
    int id2 = nexus_v1_object_place(&world, &obj);
    CHECK(id2 > 0 && id2 != id1, "second object gets distinct id");

    Nexus_V1_Object *found = nexus_v1_object_by_id(&world, id1);
    CHECK(found != NULL && found->type == NEXUS_OBJECT_CHEST,
          "object_by_id finds placed object");

    found = nexus_v1_object_at(&world, 0, 5, 10);
    CHECK(found != NULL && found->id == id1,
          "object_at finds object by position");

    found = nexus_v1_object_at(&world, 0, 99, 99);
    CHECK(found == NULL, "object_at returns NULL for empty cell");

    int r = nexus_v1_object_set_state(&world, id1, 2);
    CHECK(r == 0, "object_set_state succeeds");
    found = nexus_v1_object_by_id(&world, id1);
    CHECK(found->state == 2, "object state updated");

    r = nexus_v1_object_set_flag(&world, id1, NEXUS_OBJ_F_OPENED);
    CHECK(r == 0, "object_set_flag succeeds");
    r = nexus_v1_object_clear_flag(&world, id1, NEXUS_OBJ_F_OPENED);
    CHECK(r == 0, "object_clear_flag succeeds");

    r = nexus_v1_object_remove(&world, id1);
    CHECK(r == 0, "object_remove succeeds");
    CHECK(world.object_count == 1, "one object remains after removal");

    /* Event system */
    r = nexus_v1_event_fire(&world, NEXUS_EVT_PARTY_STEP, 0, 5, 10, 0, 0);
    CHECK(r == 0, "event_fire returns 0");
    CHECK(world.event_count == 1, "event recorded");
    CHECK(world.events[0].type == NEXUS_EVT_PARTY_STEP,   "event type is PARTY_STEP");
    CHECK(world.events[0].x == 5 && world.events[0].y == 10,
          "event position recorded");

    nexus_v1_events_clear_level(&world, 1);
    CHECK(world.event_count == 1,        "clear_level(1) preserves level-0 events");
    nexus_v1_events_clear_all(&world);
    CHECK(world.event_count == 0,        "clear_all clears all events");

    /* Timer system */
    int tid1 = nexus_v1_timer_add(&world, NEXUS_TIMER_REPEAT, 0, 10, 10, NULL);
    CHECK(tid1 > 0, "timer_add returns positive id");

    int tid2 = nexus_v1_timer_add(&world, NEXUS_TIMER_ONESHOT, 1, 5, 0, NULL);
    CHECK(tid2 > 0, "second timer added");

    nexus_v1_timer_pause(&world, tid1);
    nexus_v1_timer_resume(&world, tid1);
    nexus_v1_timer_remove(&world, tid2);

    /* Level transitions */
    nexus_v1_party_place(&world, 3, 15, 15, NEXUS_DIR_EAST);
    CHECK(world.party_level == 3,  "party_place: level = 3");
    CHECK(world.party_x     == 15, "party_place: x = 15");
    CHECK(world.party_y     == 15, "party_place: y = 15");
    CHECK(world.party_dir   == 1,  "party_place: dir = East");

    r = nexus_v1_transition_queue(&world, 4, 20, 20);
    CHECK(r == 0,                  "transition_queue succeeds");
    CHECK(world.transition_pending == 1,    "transition_pending = 1");
    CHECK(world.transition_target  == 4,    "transition_target = 4");
    CHECK(world.transition_x == 20,          "transition_x recorded");
    CHECK(world.transition_y == 20,          "transition_y recorded");

    nexus_v1_transition_execute(&world);
    CHECK(world.party_level == 4,  "transition_execute: level = 4");
    CHECK(world.party_x     == 20, "transition_execute: x = 20");
    CHECK(world.party_y     == 20, "transition_execute: y = 20");

    /* Serializer */
    size_t ser_sz = nexus_v1_world_serialize_size(&world);
    CHECK(ser_sz > 0, "world_serialize_size returns non-zero");
    uint8_t *ser_buf = malloc(ser_sz + 16);
    if (ser_buf) {
        memset(ser_buf, 0xAA, ser_sz + 16);
        size_t written = nexus_v1_world_serialize(&world, ser_buf, ser_sz);
        CHECK(written > 0 && written <= ser_sz,
              "world_serialize writes positive bytes");
        Nexus_V1_World w2;
        memset(&w2, 0xAA, sizeof(w2));
        int dr = nexus_v1_world_deserialize(&w2, ser_buf, ser_sz);
        CHECK(dr == 0,                   "world_deserialize returns 0 on success");
        CHECK(w2.party_level == 4,       "deserialized world preserves party_level");
        free(ser_buf);
    }

    /* Hash stability */
    uint64_t h1 = nexus_v1_world_hash(&world);
    uint64_t h2 = nexus_v1_world_hash(&world);
    CHECK(h1 == h2, "world_hash is stable (idempotent calls)");

    nexus_v1_world_init(&world);
    uint64_t h3 = nexus_v1_world_hash(&world);
    CHECK(h3 != h1, "hash differs after world_init (state mutated)");

    /* Seeded hash */
    nexus_v1_world_hash_inject(&world, NEXUS_HASH_SEED_LEVEL00);
    CHECK(world.state_hash != 0, "hash_inject seeds state_hash away from zero");
}

/* ═══════════════════════════════════════════════════════════════════════
 * 6. Engine Lifecycle — verify nexus_v1_init/shutdown signatures
 * Source: nexus_v1_engine.h, src/nexus/nexus_v1_engine.c
 * ═══════════════════════════════════════════════════════════════════════ */
static void probe_engine_lifecycle(void)
{
    printf("\n[Probe 6: Engine Lifecycle -- nexus_v1_engine.h]\n");
    printf("  Source: nexus_v1_engine.c, nexus_v1_mechanics.c\n");
    printf("  Note:   SDL/file I/O skipped; no game data required.\n");

    /* Nexus_V1_Engine struct layout */
    Nexus_V1_Engine engine;
    memset(&engine, 0, sizeof(engine));
    CHECK(engine.initialized == 0,  "engine.initialized = 0 after zero");
    CHECK(engine.level_loaded == 0, "engine.level_loaded = 0 after zero");
    CHECK(engine.mechanics == NULL,
          "engine.mechanics = NULL before init");

    /* Nexus_V1_GameState fields (embedded in engine) */
    engine.game.game_started = 0;
    engine.game.tick_count   = 0;
    CHECK(engine.game.game_started == 0, "engine.game.game_started writable");
    CHECK(engine.game.tick_count   == 0, "engine.game.tick_count   writable");

    /* nexus_v1_init — returns -1 when no data found (no crash, no SDL) */
    int init_r = nexus_v1_init(&engine, "/nonexistent/path");
    CHECK(init_r == -1,             "nexus_v1_init returns -1 for missing data dir");
    CHECK(engine.initialized == 0, "engine still uninitialized without data");

    /* nexus_v1_shutdown — always safe on uninitialized engine */
    nexus_v1_shutdown(&engine);
    CHECK(engine.initialized == 0,  "engine still uninitialized after shutdown");
}

/* ═══════════════════════════════════════════════════════════════════════
 * Main entry point
 * ═══════════════════════════════════════════════════════════════════════ */
int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    printf("=================================================================\n");
    printf("  Nexus V1 Mechanics Parity Probe\n");
    printf("  Source: DMWeb DGN format (2026-05-28);\n");
    printf("          ReDMCSB WIP20210206 (\n");
    printf("          COMMAND.C F0380, CLIKMENU.C F0366,\n");
    printf("          MOVESENS.C F0267/F0276, DUNGEON.C F0029/F0044,\n");
    printf("          CHAMPION.C F0309/F0325, CREATURE.C,\n");
    printf("          LOADSAVE.C F0433/F0434,\n");
    printf("          SDDRVS.TSK (docs/nexus_triggers.md)\n");
    printf("  Probe:  compile/link verification; no game data required\n");
    printf("=================================================================\n");

    probe_dungeon();
    probe_movement();
    probe_combat();
    probe_save_load();
    probe_world();
    probe_engine_lifecycle();

    printf("\n=================================================================\n");
    printf("  Results: %d PASS, %d FAIL  (%s)\n",
           g_pass, g_fail,
           g_fail == 0 ? "ALL CHECKS PASSED" : "ONE OR MORE CHECKS FAILED");
    printf("=================================================================\n");

    return g_fail > 0 ? 1 : 0;
}
