#ifndef FIRESTAFF_CSB_V1_DUNGEON_WORLD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_DUNGEON_WORLD_PC34_COMPAT_H

/*
 * CSB V1 Phase 4 -- Mechanics Parity: Sensors, Actuators, Teleporters,
 * Pits, Doors, Pressure Plates, End Conditions, and Dungeon Logic.
 *
 * Source-locked to ReDMCSB TIMELINE.C (F0248/F0249), MOVESENS.C (F0276),
 * DUNGEON.C (door table), ENDGAME.C (F0666), DEFS.H.
 *
 * Phase 4 elements that diverge from DM1:
 *   - C009_VERSION_CHECKER  (CSB-only floor sensor, CHANGE7_23)
 *   - C018_END_GAME         (CSB-only wall sensor, CHANGE7_21)
 *   - F0249 group-first teleporter/pit processing (CHANGE7_22_FIX)
 *   - Door defense-point table (wooden=42, iron=230, Ra=255 HP)
 *   - CHANGE7_17_FIX: sensor squares skipped during thing discard
 *   - CHANGE7_18_FIX: bit-15 always cleared before type check
 *   - CHANGE7_19_FIX: Lord Chaos teleporter direction (GROUP.C)
 *   - G0302_B_GameWon flag + F0666_endgame() sequence
 *
 * CSB 2.0 engine version: 20  (CHANGE8_06)
 * CSB 2.1 engine version: 21  (CHANGE8_06)
 *
 * Reference: docs/
 * source-lock/csb_v1_phase4_mechanics_parity_H2239.md
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
    int      levelCount;
    int      currentLevel;
} CSB_DungeonWorld;

/* ================================================================
 *  Thing type helpers -- M012_TYPE equivalent
 *  Source: DEFS.H:399
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

    /* Floor sensors -- F0276 context */
    CSB_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT = 1,
    CSB_SENSOR_FLOOR_THERON_PARTY_CREATURE        = 2,
    CSB_SENSOR_FLOOR_PARTY                        = 3,
    CSB_SENSOR_FLOOR_OBJECT                       = 4,
    CSB_SENSOR_FLOOR_PARTY_ON_STAIRS              = 5,
    CSB_SENSOR_FLOOR_GROUP_GENERATOR              = 6,
    CSB_SENSOR_FLOOR_CREATURE                    = 7,
    CSB_SENSOR_FLOOR_PARTY_POSSESSION             = 8,
    CSB_SENSOR_FLOOR_VERSION_CHECKER             = 9,

    /* Wall sensors -- F0248/F0275 context */
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
    CSB_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT          = 15,
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
    int gameWon;
    int restartAllowed;
    int delayTicks;
    int paletteSetCurtain;
    int endgameFnCalled;
    int startendEndgameFn;
} CSB_EndgameResult;

/* ================================================================
 *  Teleporter/pit result structure -- F0249 CHANGE7_22_FIX
 * ================================================================ */

typedef struct {
    int movingGroup;
    int thingType;
    int moved;
    int oldMapX;
    int oldMapY;
    int newMapX;
    int newMapY;
} CSB_TeleporterPitResult;

/* ================================================================
 *  Door helpers -- source-locked to DUNGEON.C:560-565
 * ================================================================ */

int csb_door_get_defense_points(int doorType);
int csb_door_minimum_attack_power(int doorType);

/* ================================================================
 *  Sensor segment accessors -- M039_TYPE / M040_DATA
 *
 *  CHANGE7_18_FIX (BUG0_10): bit 15 always cleared from raw
 *  Type_Data before type extraction and M012_TYPE shift.
 *  DUNGEON.C:2099 (MEDIA291 compat)
 * ================================================================ */

uint8_t  csb_sensor_get_type(uint16_t typeData);
uint16_t csb_sensor_get_data(uint16_t typeData);
int      csb_sensor_is_csb_version_checker(uint16_t typeData);
int      csb_sensor_is_csb_end_game(uint16_t typeData);

/*
 * csb_version_checker_triggered -- version gate in C009 sensor.
 *
 *   sensorData <= csbEngineVersion  -->  trigger fires
 *
 *  CSB 2.0: version 20 (CHANGE8_06 / MEDIA337)
 *  CSB 2.1: version 21 (CHANGE8_06 / MEDIA342)
 *
 *  ReDMCSB: MOVESENS.C:1716-1750 (F0276 switch case C009,
 *           CHANGE7_23, CHANGE8_06)
 */
int csb_version_checker_triggered(uint16_t sensorData, int csbEngineVersion);

/* ================================================================
 *  End-game sequence -- TIMELINE.C:1319-1340 (F0248 C018 branch)
 * ================================================================ */

/*
 * csb_endgame_trigger -- execute C018 END_GAME trigger sequence.
 *
 * Sequence mirrors TIMELINE.C:1319-1340:
 *   1. F1012_PALETTE_SetCurtain(C0_BLACK_PALETTE)   [MEDIA671]
 *   2. F0694_SetMultipleColorsInPalette(blackIdx)  [MEDIA671]
 *   3. F0022_MAIN_Delay(60 * delaySeconds)         [CHANGE8_02]
 *   4. G0524_B_RestartGameAllowed = FALSE          [MEDIA277]
 *   5. G0302_B_GameWon = TRUE
 *   6. F0666_endgame()                             [MEDIA629]
 *   7. F0444_STARTEND_Endgame(TRUE)               [MEDIA277]
 *
 * delaySeconds: sensor->Remote.Value (0 = no delay, original dungeons)
 *
 * ReDMCSB: TIMELINE.C:1319-1340, ENDGAME.C:984-1002, DEFS.H:1202_comment
 */
void csb_endgame_trigger(int delaySeconds, CSB_EndgameResult* pOut);

/* ================================================================
 *  Teleporter / pit processing -- F0249 CHANGE7_22_FIX
 *
 *  Bug (DM1 BUG0_22): arbitrary linked-list order causes infinite
 *  loops or missed things when >=2 things on a closed square open.
 *
 *  Fix (TIMELINE.C:1398-1476):
 *    1. Group processed FIRST via F0175_GROUP_GetThing().
 *    2. Non-group things counted:  M012_TYPE > GROUP(4).
 *    3. while(thing && ThingsToMoveCount) { ... ThingsToMoveCount--; }
 *       Count guard prevents infinite loops.
 *    4. NextThing read BEFORE call to moveFn (mid-traversal removal
 *       resilience: projectile impacts group, group goes away, loop
 *       continues with valid NextThing).
 *
 * ReDMCSB: TIMELINE.C:1398-1476, BugsAndChanges.htm:CHANGE7_22
 */

/*
 * csb_teleporter_pit_process -- apply CHANGE7_22_FIX on a square.
 *
 *  - Group (if any) moved first.
 *  - Non-group things moved exactly once, count-guarded.
 *  - Next link read before each move.
 *
 *  Parameters:
 *    tile        -- tile being processed (non-null)
 *    mapX, mapY  -- coordinates of this square
 *    moveFn      -- void moveFn(uint16_t thing, void* ctx)
 *    ctx         -- forwarded to moveFn
 *    pOut        -- result array (non-null)
 *    maxResults  -- array capacity
 *    pResultCount -- receives written count (non-null)
 *
 *  Returns: 0 on success, -1 if tile null or no moveFn.
 *
 * ReDMCSB: TIMELINE.C:1398-1476 (full F0249 body)
 */
int csb_teleporter_pit_process(
    const CSB_Tile*              tile,
    int                          mapX,
    int                          mapY,
    void                       (*moveFn)(uint16_t thing, void* ctxOpaque),
    void*                        ctxOpaque,
    CSB_TeleporterPitResult*     pOut,
    int                          maxResults,
    int*                         pResultCount);

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
 * inadvertently triggering it (e.g., closing a door under a pressure
 * plate, opening a pit under a champion).
 *
 * Fix: squares with any enabled sensor are skipped in discard.
 *
 * ReDMCSB: DUNGEON.C:1996-2001 (CHANGE7_17_FIX / MEDIA278),
 *          BugsAndChanges.htm:CHANGE7_17,BUG0_09
 */
int csb_bugfix_is_sensor_square_clear_for_discard(const CSB_Tile* tile);

/*
 * csb_bugfix_thing_type_bit15_clearly -- CHANGE7_18_FIX.
 *
 * Returns the 4-bit thing type with bit 15 masked off before shift.
 *
 * Problem (DM1 BUG0_10): bone-creation path sets bit 15 in the raw
 * THING type to indicate "use reserved thing pool". Megamax compiler
 * silently ignores this (signed right-shift discards bit 15). Other
 * compilers need it explicitly cleared before type comparisons.
 *
 * Fix: always apply (raw & ~0x8000) before M012_TYPE extraction.
 *
 * ReDMCSB: DUNGEON.C:2099-2101 (CHANGE7_18_FIX / MEDIA291/MEDIA178),
 *          BugsAndChanges.htm:CHANGE7_18,BUG0_10
 */
uint8_t csb_bugfix_thing_type_bit15_clearly(uint16_t rawThingType);

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
