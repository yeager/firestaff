#ifndef FIRESTAFF_CSB_V1_DUNGEON_WORLD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_DUNGEON_WORLD_PC34_COMPAT_H

/*
 * CSB V1 Phase 4 -- Mechanics Parity: Sensors, Actuators, Teleporters,
 * Pits, Doors, Pressure Plates, End Conditions, and Dungeon Logic.
 *
 * Source-locked to ReDMCSB TIMELINE.C (F0248/F0249), MOVESENS.C (F0276),
 * DUNGEON.C (door table), ENDGAME.C (F0666), GROUP.C (F0175), DEFS.H.
 *
 * Phase 4 elements that diverge from DM1:
 *   - C009_VERSION_CHECKER  (CSB-only floor sensor, CHANGE7_23)
 *   - C018_END_GAME         (CSB-only wall sensor, CHANGE7_21)
 *   - F0249 group-first teleporter/pit processing (CHANGE7_22_FIX, BUG0_22)
 *   - Door defense-point table (portcullis=110, wooden=42, iron=230, Ra=255 HP)
 *   - CHANGE7_17_FIX (BUG0_09): sensor squares skipped during thing discard
 *   - CHANGE7_18_FIX (BUG0_10): bit-15 always cleared before type check
 *   - CHANGE7_19_FIX (BUG0_69): Lord Chaos teleporter direction
 *   - G0302_B_GameWon flag + F0666_endgame() sequence
 *
 * CSB 2.0 engine version: 20  (CHANGE8_06 / MEDIA337)
 * CSB 2.1 engine version: 21  (CHANGE8_06 / MEDIA342)
 *
 * Reference: docs/source-lock/csb_v1_phase4_mechanics_parity_H2239.md
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 *  Core dungeon world types
 * ================================================================ */

#define CSB_MAX_LEVELS 16
#define CSB_MAX_WIDTH  64
#define CSB_MAX_HEIGHT 64

enum CSB_TileType {
    CSB_TILE_EMPTY       = 0,
    CSB_TILE_WALL        = 1,
    CSB_TILE_FLOOR       = 2,
    CSB_TILE_PIT         = 3,
    CSB_TILE_STAIRS_UP   = 4,
    CSB_TILE_STAIRS_DOWN = 5,
    CSB_TILE_DOOR        = 6,
    CSB_TILE_TELEPORTER  = 7,
    CSB_TILE_FAKE_WALL   = 8,
    CSB_TILE_COUNT
};

typedef struct CSB_Tile_s {
    uint8_t  type;
    uint8_t  flags;
    uint8_t  wallN;
    uint8_t  wallE;
    uint8_t  wallS;
    uint8_t  wallW;
    uint8_t  ornamentN;
    uint8_t  ornamentE;
    uint8_t  ornamentS;
    uint8_t  ornamentW;
    int16_t  thingList;
    int16_t  creatureList;
} CSB_Tile;

typedef struct CSB_Level_s {
    CSB_Tile tiles[CSB_MAX_HEIGHT][CSB_MAX_WIDTH];
    int     width;
    int     height;
    int     levelIndex;
    int     lightLevel;
} CSB_Level;

typedef struct CSB_DungeonWorld_s {
    CSB_Level levels[CSB_MAX_LEVELS];
    int       levelCount;
    int       currentLevel;
} CSB_DungeonWorld;

/* ================================================================
 *  Thing type helpers -- M012_TYPE / DEFS.H:399
 *
 *  THING handle layout (16-bit):
 *    [15:14 unused] [13:10 type, 0..15] [9:0 index]
 * ================================================================ */

#define CSB_THING_TYPE(thing)  (((thing) & 0x3C00u) >> 10)

enum {
    CSB_THING_TYPE_DOOR        = 0,
    CSB_THING_TYPE_TEXTSTRING  = 2,
    CSB_THING_TYPE_SENSOR      = 3,
    CSB_THING_TYPE_GROUP       = 4,
    CSB_THING_TYPE_WEAPON      = 5,
    CSB_THING_TYPE_ARMOUR      = 6,
    CSB_THING_TYPE_JUNK       = 10,
    CSB_THING_TYPE_PROJECTILE = 14
};

#define CSB_THING_ENDOFLIST  0xFFFEu
#define CSB_THING_PARTY      0xFFFFu

/* ================================================================
 *  Sensor type constants -- DEFS.H:1202-1283
 *
 *  Sensor Remote.Type_Data layout:  [15:7 data] [6:0 type]
 *    M039_TYPE:  Remote.Type_Data & 0x007F   (7-bit type)
 *    M040_DATA:  Remote.Type_Data >> 7        (9-bit data)
 *
 *  CSB-only types:
 *    C009 = VERSION_CHECKER  (floor sensor -- CHANGE7_23)
 *    C018 = END_GAME         (wall sensor  -- CHANGE7_21)
 * ================================================================ */

enum {
    CSB_SENSOR_DISABLED = 0,

    /* Floor sensors -- F0276 context (MOVESENS.C:1553+) */
    CSB_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT = 1,
    CSB_SENSOR_FLOOR_THERON_PARTY_CREATURE        = 2,
    CSB_SENSOR_FLOOR_PARTY                        = 3,
    CSB_SENSOR_FLOOR_OBJECT                       = 4,
    CSB_SENSOR_FLOOR_PARTY_ON_STAIRS              = 5,
    CSB_SENSOR_FLOOR_GROUP_GENERATOR              = 6,
    CSB_SENSOR_FLOOR_CREATURE                    = 7,
    CSB_SENSOR_FLOOR_PARTY_POSSESSION             = 8,
    CSB_SENSOR_FLOOR_VERSION_CHECKER             = 9,

    /* Wall sensors -- F0248 context (TIMELINE.C:1136+) */
    CSB_SENSOR_WALL_ORNAMENT_CLICK                                     =  1,
    CSB_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT                     =  2,
    CSB_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT                =  3,
    CSB_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED       =  4,
    CSB_SENSOR_WALL_AND_OR_GATE                                        =  5,
    CSB_SENSOR_WALL_COUNTDOWN                                         =  6,
    CSB_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_NEW_OBJECT             =  7,
    CSB_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_EXPLOSION               =  8,
    CSB_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_NEW_OBJECT             =  9,
    CSB_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION             = 10,
    CSB_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE                           = 11,
    CSB_SENSOR_WALL_OBJECT_GENERATOR_ROTATE                           = 12,
    CSB_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE                      = 13,
    CSB_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT         = 14,
    CSB_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT        = 15,
    CSB_SENSOR_WALL_OBJECT_EXCHANGER                                   = 16,
    CSB_SENSOR_WALL_CLICK_OBJ_REMOVED_REMOVE_SENSOR                   = 17,
    CSB_SENSOR_WALL_END_GAME                                          = 18,
    CSB_SENSOR_WALL_CHAMPION_PORTRAIT                                = 127
};

/* Effect constants -- DEFS.H:1288-1295 */
enum {
    CSB_EFFECT_NONE                  = -1,
    CSB_EFFECT_SET                    =  0,
    CSB_EFFECT_CLEAR                 =  1,
    CSB_EFFECT_TOGGLE                =  2,
    CSB_EFFECT_HOLD                  =  3,
    CSB_EFFECT_ADD_300XP_STEAL_SKILL = 10
};

/* Door types -- DUNGEON.C:560-565 (pass563 compat) */
enum CSB_DoorType {
    CSB_DOOR_PORTCULLIS = 0,
    CSB_DOOR_WOODEN     = 1,
    CSB_DOOR_IRON       = 2,
    CSB_DOOR_RA         = 3
};

/* Door flag masks -- DEFS.H:1570-1572 */
#define CSB_MASK_DOOR_CREATURES_CAN_SEE_THROUGH    0x0001u
#define CSB_MASK_DOOR_PROJECTILES_CAN_PASS_THROUGH 0x0002u
#define CSB_MASK_DOOR_ANIMATED                     0x0004u

typedef struct {
    uint16_t flags;
    uint16_t defensePoints;
} CSB_DoorInfo;

extern const CSB_DoorInfo CSB_doorInfo[4];

/* ================================================================
 *  End-game result structure -- G0302 / F0666 context
 * ================================================================ */

typedef struct {
    int gameWon;          /* G0302_B_GameWon — set before calling F0666 */
    int restartAllowed;   /* G0524_B_RestartGameAllowed — cleared before credits */
    int delayTicks;       /* CHANGE8_02: 60 * delaySeconds */
    int paletteSetCurtain;/* MEDIA671: palette curtain */
    int endgameFnCalled;  /* F0666_endgame() — checks G0302_B_GameWon guard */
    int startendEndgameFn;/* F0444_STARTEND_Endgame(TRUE) — draws credits */
} CSB_EndgameResult;

/* ================================================================
 *  Teleporter/pit result structure -- F0249 CHANGE7_22_FIX
 * ================================================================ */

typedef struct {
    int movingGroup;   /* 1 if this was a GROUP thing */
    int thingType;     /* M012_TYPE extracted from thing */
    int moved;         /* 1 if move was attempted */
    int oldMapX;       /* square coordinate before move */
    int oldMapY;
    int newMapX;       /* square coordinate after move (set by move_fn) */
    int newMapY;
} CSB_TeleporterPitResult;

/* ================================================================
 *  Dungeon-layer accessor stubs (M10 integration points)
 *
 *  These are the integration points between CSB compat layer and
 *  the M10 dungeon layer.  The default stubs return ENDOF / 0;
 *  replace them by wiring real F0161/F0159/F0175/F0156/F0267
 *  from M10 once those accessors are available.
 *
 *  ReDMCSB: GROUP.C:52 (F0175), DUNGEON.C:?? (F0161/F0159),
 *           DUNGEON.C:?? (F0156), MOVE.C:?? (F0267)
 * ================================================================ */

/*
 * csb_dungeon_get_group -- F0175_GROUP_GetThing equivalent.
 *
 *   Walk the square's thing list via F0161/F0159.
 *   Return the first THING with type GROUP (type index 4) or ENDOF.
 *
 * ReDMCSB: GROUP.C:52-70 (F0175_GROUP_GetThing)
 */
uint16_t csb_dungeon_get_group(
    uint16_t (*fn_get_first)(int mapX, int mapY),
    uint16_t (*fn_get_next)(uint16_t thing),
    int mapX, int mapY);

/* ================================================================
 *  Door helpers -- DUNGEON.C:561-565 (pass563 compat)
 * ================================================================ */

int csb_door_get_defense_points(int doorType);
int csb_door_minimum_attack_power(int doorType);

/* ================================================================
 *  Sensor segment accessors -- M039_TYPE / M040_DATA
 *
 *  CHANGE7_18_FIX (BUG0_10): bit 15 always cleared from raw
 *  Type_Data before type extraction (DUNGEON.C:2099 / MEDIA291).
 *
 *  Type_Data layout: [15:7 data] [6:0 type]
 *    M039_TYPE = Type_Data & 0x7F  (after bit 15 cleared)
 *    M040_DATA = Type_Data >> 7
 *
 * ReDMCSB: DEFS.H:1295-1296, DUNGEON.C:2099-2101 (MEDIA291)
 * ================================================================ */

uint8_t  csb_sensor_get_type(uint16_t typeData);
uint16_t csb_sensor_get_data(uint16_t typeData);
int      csb_sensor_is_csb_version_checker(uint16_t typeData);
int      csb_sensor_is_csb_end_game(uint16_t typeData);

/*
 * csb_version_checker_triggered -- version gate in C009 sensor.
 *
 *   condition: sensorData <= csbEngineVersion  -->  trigger fires
 *
 *  CSB 2.0: version 20 (CHANGE8_06 / MEDIA337)
 *  CSB 2.1: version 21 (CHANGE8_06 / MEDIA342)
 *
 *  Trigger guard (MOVESENS.C:1716-1750):
 *    - P0590_T_Thing must be THING_PARTY
 *    - P0592_B_AddThing must be TRUE
 *    - P0591_B_PartySquare must be FALSE
 *
 * ReDMCSB: MOVESENS.C:1716-1750 (F0276 switch case C009,
 *           CHANGE7_23, CHANGE8_06)
 */
int csb_version_checker_triggered(uint16_t sensorData, int csbEngineVersion);

/* ================================================================
 *  End-game sequence -- TIMELINE.C:1319-1340 (F0248 C018 branch)
 * ================================================================ */

/*
 * csb_endgame_trigger -- execute C018 END_GAME trigger sequence.
 *
 * Sequence (TIMELINE.C:1319-1340):
 *   1. F1012_PALETTE_SetCurtain(C0_BLACK_PALETTE)   [MEDIA671]
 *   2. F0694_SetMultipleColorsInPalette(blackIdx)  [MEDIA671]
 *   3. F0022_MAIN_Delay(60 * delaySeconds)         [CHANGE8_02]
 *   4. G0524_B_RestartGameAllowed = FALSE           [MEDIA277]
 *   5. G0302_B_GameWon = TRUE
 *   6. F0666_endgame()                             [MEDIA629]
 *   7. F0444_STARTEND_Endgame(TRUE)               [MEDIA277]
 *
 * delaySeconds: sensor->Remote.Value (0 = no delay, original dungeons)
 *
 * BUG0_00 guard: F0666_endgame() checks G0302_B_GameWon and returns
 * immediately if not set.  Step 5 must precede step 6.
 *
 * ReDMCSB: TIMELINE.C:1319-1340, ENDGAME.C:984-1002,
 *          DEFS.H:1202_comment, BugsAndChanges.htm:CHANGE7_21,CHANGE8_02
 */
void csb_endgame_trigger(int delaySeconds, CSB_EndgameResult* pOut);

/* ================================================================
 *  Teleporter / pit processing -- F0249 CHANGE7_22_FIX (BUG0_22)
 *
 *  Bug (DM1): arbitrary linked-list order causes infinite loops or
 *  missed things when >=2 things on a closed square open.
 *
 *  Fix (TIMELINE.C:1398-1476):
 *    1. Group processed FIRST via F0175_GROUP_GetThing().
 *    2. Non-group things counted: M012_TYPE > GROUP(4).
 *    3. while(thing && ThingsToMoveCount) { ... ThingsToMoveCount--; }
 *    4. NextThing read BEFORE call to moveFn.
 *
 * ReDMCSB: TIMELINE.C:1398-1476, BugsAndChanges.htm:CHANGE7_22,BUG0_22
 */

/*
 * csb_teleporter_pit_process -- apply CHANGE7_22_FIX on a square.
 *
 *  Dungeon-layer accessors are caller-supplied:
 *    get_group_fn  -- F0175_GROUP_GetThing equivalent
 *    get_first_fn   -- F0161_DUNGEON_GetSquareFirstThing equivalent
 *    get_next_fn    -- F0159_DUNGEON_GetNextThing equivalent
 *    move_fn        -- F0267_MOVE_GetMoveResult_CPSCE equivalent
 *
 *  Until M10 integration: use csb_dungeon_get_group() with stub
 *  accessors (csb_dungeon_get_first_thing_default, etc.) for testing.
 *
 * Returns: 0 on success, -1 if tile null or no moveFn.
 *
 * ReDMCSB: TIMELINE.C:1398-1476 (full F0249 body)
 */
int csb_teleporter_pit_process(
    const CSB_Tile*   tile,
    int               mapX,
    int               mapY,
    uint16_t        (*get_group_fn)(int mapX, int mapY),
    uint16_t        (*get_first_fn)(int mapX, int mapY),
    uint16_t        (*get_next_fn)(uint16_t thing),
    void             (*move_fn)(uint16_t thing, int oldMapX, int oldMapY,
                                int* newMapX, int* newMapY, void* ctxOpaque),
    void*            ctxOpaque,
    CSB_TeleporterPitResult* pOut,
    int              maxResults,
    int*             pResultCount);

/* ================================================================
 *  Dungeon bug-fix helpers
 * ================================================================ */

/*
 * csb_bugfix_is_sensor_square_clear_for_discard -- CHANGE7_17_FIX.
 *
 * Returns 1 if every sensor on the tile is disabled (M039_TYPE == 0),
 * meaning the square is safe for thing-discard during allocation.
 *
 * Problem (DM1 BUG0_09): discard search could land on a sensor square,
 * inadvertently triggering it (pressure plate closes door, etc.).
 *
 * Fix: squares with any enabled sensor are skipped in discard.
 *
 * ReDMCSB: DUNGEON.C:1996-2001 (CHANGE7_17_FIX / MEDIA278),
 *          BugsAndChanges.htm:CHANGE7_17,BUG0_09
 */
int csb_bugfix_is_sensor_square_clear_for_discard(
    int                          mapX,
    int                          mapY,
    uint16_t                   (*get_first_fn)(int mapX, int mapY),
    uint16_t                   (*get_next_fn)(uint16_t thing),
    uint16_t                   (*thing_data_fn)(uint16_t thing, int offset),
    int                         sensorTypeDataOffset);

/*
 * csb_bugfix_thing_type_bit15_clearly -- CHANGE7_18_FIX.
 *
 * Returns the 4-bit thing type with bit 15 masked off before shift.
 *
 * Problem (DM1 BUG0_10): bone-creation sets bit 15 in raw THING type
 * to indicate "use reserved thing pool".  Megamax compiler silently
 * drops this bit.  Other compilers need it explicitly cleared.
 *
 * Fix: raw &= ~0x8000 before M012_TYPE shift.
 *
 * ReDMCSB: DUNGEON.C:2099-2101 (CHANGE7_18_FIX / MEDIA291/MEDIA178),
 *          DEFS.H:399 comment, BugsAndChanges.htm:CHANGE7_18,BUG0_10
 */
uint8_t csb_bugfix_thing_type_bit15_clearly(uint16_t rawThingType);

/*
 * csb_bugfix_lord_chaos_teleport_dir -- CHANGE7_19_FIX (BUG0_69).
 *
 * Returns a properly initialized Lord Chaos teleport direction.
 *
 * Problem (DM1 BUG0_69): primary direction variable used as array
 * index without initialization, causing memory corruption.
 *
 * Fix: primaryDir = M004_RANDOM(4); secondaryDir = M017_NEXT(primaryDir).
 *
 * ReDMCSB: GROUP.C:2208-2215 (CHANGE7_19_FIX / MEDIA297),
 *          BugsAndChanges.htm:CHANGE7_19,BUG0_69
 */
int csb_bugfix_lord_chaos_teleport_dir(int (*random4)(void));

/* ================================================================
 *  Core dungeon world API
 * ================================================================ */

void         csb_world_init(CSB_DungeonWorld* w);
int          csb_world_add_level(CSB_DungeonWorld* w, int width, int height);
CSB_Tile*    csb_world_get_tile(CSB_DungeonWorld* w, int level, int x, int y);
const CSB_Tile* csb_world_get_tile_const(const CSB_DungeonWorld* w,
                                          int level, int x, int y);
int          csb_world_is_walkable(const CSB_DungeonWorld* w,
                                    int level, int x, int y);
int          csb_world_is_wall(const CSB_DungeonWorld* w,
                                 int level, int x, int y);
void         csb_world_set_tile_type(CSB_DungeonWorld* w,
                                       int level, int x, int y,
                                       uint8_t type);
void         csb_world_set_wall(CSB_DungeonWorld* w,
                                  int level, int x, int y,
                                  int dir, uint8_t wallType);
void         csb_world_set_ornament(CSB_DungeonWorld* w,
                                      int level, int x, int y,
                                      int dir, uint8_t ornament);
int          csb_world_get_level_count(const CSB_DungeonWorld* w);
void         csb_world_set_current_level(CSB_DungeonWorld* w, int level);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_DUNGEON_WORLD_PC34_COMPAT_H */