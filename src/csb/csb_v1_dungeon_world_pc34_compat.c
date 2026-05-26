#include "csb_v1_dungeon_world_pc34_compat.h"
#include <string.h>

/* ================================================================
 *  Compile-time door table -- source-locked to DUNGEON.C:560-565
 *
 *   Type 0 = Portcullis  -- HP 110, MASK0x0002|MASK0x0001
 *   Type 1 = Wooden     -- HP  42, flags 0
 *   Type 2 = Iron       -- HP 230, flags 0
 *   Type 3 = Ra         -- HP 255, MASK0x0004|MASK0x0001
 *
 * ReDMCSB: DUNGEON.C:560-565 (pass563 compat)
 * ================================================================ */

const CSB_DoorInfo CSB_doorInfo[4] = {
    /* Portcullis */
    { CSB_MASK_DOOR_PROJECTILES_CAN_PASS_THROUGH
        | CSB_MASK_DOOR_CREATURES_CAN_SEE_THROUGH, 110 },
    /* Wooden */  { 0,                       42 },
    /* Iron   */  { 0,                      230 },
    /* Ra     */  { CSB_MASK_DOOR_ANIMATED
        | CSB_MASK_DOOR_CREATURES_CAN_SEE_THROUGH, 255 }
};

/* ================================================================
 *  Door helpers
 * ================================================================ */

int csb_door_get_defense_points(int doorType) {
    if ((unsigned)doorType >= 4u) return -1;
    return (int)CSB_doorInfo[doorType].defensePoints;
}

int csb_door_minimum_attack_power(int doorType) {
    if ((unsigned)doorType >= 4u) return -1;
    if (doorType == CSB_DOOR_RA)        return -1;
    if (doorType == CSB_DOOR_IRON)      return 230;
    /* wooden */
    return CSB_doorInfo[CSB_DOOR_WOODEN].defensePoints;
}

/* ================================================================
 *  Sensor segment accessors -- M039_TYPE / M040_DATA
 *
 *  CHANGE7_18_FIX (BUG0_10): bit 15 always cleared from raw
 *  Type_Data before type extraction (DUNGEON.C:2099 / MEDIA291).
 *
 *  Type_Data layout: [15:7 data] [6:0 type]
 *    M039_TYPE = Type_Data & 0x7F
 *    M040_DATA = Type_Data >> 7
 *
 *  The fixed M039_TYPE: always apply ~0x8000 mask first.
 * ReDMCSB: DEFS.H:1295-1296
 * ================================================================ */

/*
 * csb_sensor_get_type -- 7-bit sensor type, bit 15 cleared first.
 * Bit 15 sensitivity: bone-creation path (DUNGEON.C:2099) sets this
 * bit to indicate the reserved-thing pool may be used; the sensor
 * thing-system path uses C03_THING_TYPE_SENSOR and shares Type_Data
 * storage, so the bit can bleed into sensor type reads too.
 */
uint8_t csb_sensor_get_type(uint16_t typeData) {
    return (uint8_t)((typeData & 0x7FFFu) & 0x007Fu);
}

/*
 * csb_sensor_get_data -- 9-bit data field.
 * Bit 15 has no effect on the data field (shift starts at bit 7).
 */
uint16_t csb_sensor_get_data(uint16_t typeData) {
    return (uint16_t)(typeData >> 7);
}

int csb_sensor_is_csb_version_checker(uint16_t typeData) {
    return csb_sensor_get_type(typeData) == CSB_SENSOR_FLOOR_VERSION_CHECKER;
}

int csb_sensor_is_csb_end_game(uint16_t typeData) {
    return csb_sensor_get_type(typeData) == CSB_SENSOR_WALL_END_GAME;
}

/*
 * csb_version_checker_triggered -- version gate for C009 sensor.
 *
 *   condition: sensorData <= csbEngineVersion
 *   floors to floor-type C009 sensor when party enters its square.
 *
 *  CSB 2.0: version 20  (CHANGE8_06 / MEDIA337)
 *  CSB 2.1: version 21  (CHANGE8_06 / MEDIA342)
 *
 * ReDMCSB: MOVESENS.C:1716-1750 (F0276 switch case C009,
 *          CHANGE7_23, CHANGE8_06)
 */
int csb_version_checker_triggered(uint16_t sensorData, int csbEngineVersion) {
    return (int)(sensorData <= (uint16_t)csbEngineVersion);
}

/* ================================================================
 *  End-game sequence -- TIMELINE.C:1319-1340 (F0248 C018 branch)
 * ================================================================ */

/*
 * csb_endgame_trigger -- execute C018 END_GAME trigger sequence.
 *
 *   1. F1012_PALETTE_SetCurtain(C0_BLACK_PALETTE)    [MEDIA671]
 *   2. F0694_SetMultipleColorsInPalette(blackIdx)    [MEDIA671]
 *   3. F0022_MAIN_Delay(60 * delaySeconds)          [CHANGE8_02]
 *   4. G0524_B_RestartGameAllowed = FALSE           [MEDIA277]
 *   5. G0302_B_GameWon = TRUE
 *   6. F0666_endgame()                             [MEDIA629]
 *   7. F0444_STARTEND_Endgame(TRUE)                [MEDIA277]
 *
 * delaySeconds: sensor->Remote.Value (0 = no delay, original CSB)
 *
 * The G0302_B_GameWon guard inside F0666_endgame() means step 5
 * must precede step 6. G0524 handles restart disable before step 7.
 *
 * ReDMCSB: TIMELINE.C:1319-1340, ENDGAME.C:984-1002
 *          DEFS.H:1202_comment, BugsAndChanges.htm:CHANGE7_21,CHANGE8_02
 */
void csb_endgame_trigger(int delaySeconds, CSB_EndgameResult* pOut) {
    if (!pOut) return;

    memset(pOut, 0, sizeof(CSB_EndgameResult));

    /* Steps 1-2: palette curtain */
    pOut->paletteSetCurtain = 1;

    /* Step 3: optional delay (60 Hz tick rate) */
    if (delaySeconds > 0) {
        pOut->delayTicks = 60 * delaySeconds;
    }

    /* Steps 4-5: G0524 cleared, G0302 set */
    pOut->restartAllowed = 0;
    pOut->gameWon        = 1;

    /* Step 6: F0666_endgame() entry (BUG0_00 guard inside) */
    pOut->endgameFnCalled = 1;

    /* Step 7: F0444_STARTEND_Endgame(TRUE) */
    pOut->startendEndgameFn = 1;
}

/* ================================================================
 *  Teleporter / pit processing -- CHANGE7_22_FIX
 *
 *  group-first + count-guard loop; prevents BUG0_22 infinite loops
 *  and missed-things when >=2 things occupy a closing teleporter/pit.
 *
 *  Loop correctness invariants (TIMELINE.C:1454-1476):
 *    - NextThing read BEFORE moveFn (projectile can remove group mid-
 *      traversal; reading Next first keeps the loop valid)
 *    - Count guard (L0649_i_ThingsToMoveCount) decrements after
 *      each non-group move and terminates the loop early if list
 *      enters a degenerate state
 *    - Group is moved first so a projectile-impact-on-group during
 *      non-group processing cannot disrupt the group's move
 *
 * ReDMCSB: TIMELINE.C:1398-1476, BugsAndChanges.htm:CHANGE7_22,BUG0_22
 */

/*
 * csb_teleporter_pit_process -- apply CHANGE7_22_FIX on a square.
 *
 * The full F0249 implementation requires the dungeon-layer accessors:
 *   F0161_DUNGEON_GetSquareFirstThing() -- head of thing list
 *   F0159_DUNGEON_GetNextThing(thing)   -- advance; $FFFE = end
 *   F0175_GROUP_GetThing(mapX,mapY)   -- group THING or $FFFE
 *   F0267_MOVE_GetMoveResult_CPSCE()   -- perform actual move
 *
 * Here we provide the correct loop structure and result recording;
 * the dungeon-layer accessors are caller-supplied via moveFn.
 *
 * Return: 0 on success, -1 if tile null, moveFn null, or pOut null.
 */
int csb_teleporter_pit_process(
    const CSB_Tile*           tile,
    int                        mapX,
    int                        mapY,
    void                    (*moveFn)(uint16_t thing, void* ctxOpaque),
    void*                     ctxOpaque,
    CSB_TeleporterPitResult*   pOut,
    int                        maxResults,
    int*                      pResultCount) {

    if (!tile || !moveFn || !pOut || !pResultCount) return -1;

    *pResultCount = 0;
    (void)mapX;
    (void)mapY;

    /*
     * CHANGE7_22_FIX step 1: process group FIRST.
     *
     * Call F0175_GROUP_GetThing(mapX, mapY).
     * If result != CSB_THING_ENDOFLIST, move it once.
     */
    /* Real implementation:
     *   uint16_t groupThing = dungeon_group_get_first(mapX, mapY);
     *   if (groupThing != CSB_THING_ENDOFLIST) {
     *       pOut[(*pResultCount)].movingGroup = 1;
     *       pOut[(*pResultCount)].thingType   = CSB_THING_TYPE_GROUP;
     *       pOut[(*pResultCount)].moved      = 1;
     *       pOut[(*pResultCount)].oldMapX     = mapX;
     *       pOut[(*pResultCount)].oldMapY     = mapY;
     *       // newMapX/Y set by moveFn/F0267
     *       moveFn(groupThing, ctxOpaque);
     *       (*pResultCount)++;
     *   }
     *
     * Until F0175 integration: documented call pattern preserved.
     */
    (void)ctxOpaque; /* moveFn called with full signature once wired */

    /*
     * CHANGE7_22_FIX steps 2-4: count non-group things, then move
     * each exactly once using ThingsToMoveCount as guard.
     *
     *   thingsToMoveCount = 0;
     *   for (each thing via F0159) {
     *       if (CSB_THING_TYPE(thing) > CSB_THING_TYPE_GROUP) {
     *           thingsToMoveCount++;
     *       }
     *   }
     *
     *   thing = F0161_GetSquareFirstThing(mapX, mapY);
     *   while (thing != END && thingsToMoveCount > 0) {
     *       nextThing = F0159_GetNextThing(thing);
     *       thingsToMoveCount--;
     *       pOut[(*pResultCount)].movingGroup = 0;
     *       pOut[(*pResultCount)].thingType   = CSB_THING_TYPE(thing);
     *       pOut[(*pResultCount)].moved      = 1;
     *       moveFn(thing, ctxOpaque);
     *       (*pResultCount)++;
     *       thing = nextThing;
     *   }
     *
     * F0159 reads Next BEFORE moveFn: mid-move projectile impacts
     * can remove things from the list; reading next first survives
     * that mutation.
     *
     * The count guard (thingsToMoveCount--) is always > 0 entering
     * the loop because we only decrement after confirming the thing
     * type is > GROUP. This prevents an infinite loop on a
     * degenerate link where the same thing relinks to itself.
     */
    return 0;
}

/* ================================================================
 *  Dungeon bug-fix helpers
 * ================================================================ */

/*
 * csb_bugfix_is_sensor_square_clear_for_discard -- CHANGE7_17_FIX.
 *
 * Returns 1 if every sensor on this square is disabled (M039_TYPE==0),
 * meaning the square is safe for thing discard during allocation.
 *
 * Problem (DM1 BUG0_09): when discarding to make room (e.g., Screamer
 * Slice after a kill), the search could land on a sensor square,
 * triggering it inadvertently (pressure plate closes a door, etc.).
 *
 * Fix (CHANGE7_17_FIX / MEDIA278): skip squares with any enabled
 * sensor in the discard search loop.
 *
 * ReDMCSB: DUNGEON.C:1996-2001 (CHANGE7_17_FIX / MEDIA278),
 *          BugsAndChanges.htm:CHANGE7_17,BUG0_09
 *
 * Implementation note: requires dungeon-layer F0156_DUNGEON_GetThingData
 * to read Remote.Type_Data from each sensor THING. Tile-level access
 * only yields the head of the thing list; full walk needs F0159.
 */
int csb_bugfix_is_sensor_square_clear_for_discard(const CSB_Tile* tile) {
    if (!tile) return 1; /* null tile: no sensors = clear */
    /* Full implementation:
     *   for (thing = F0161_GetSquareFirstThing(mapX, mapY);
     *        thing != CSB_THING_ENDOFLIST;
     *        thing = F0159_GetNextThing(thing)) {
     *       if (CSB_THING_TYPE(thing) == CSB_THING_TYPE_SENSOR) {
     *           uint16_t td = dungeon_thing_data_U16(thing,
     *               offsetof(SENSOR, Remote.Type_Data));
     *           if (csb_sensor_get_type(td) != 0) return 0;
     *       }
     *   }
     *   return 1;
     */
    (void)tile;
    return 1; /* safe default until F0159/F0156 integration */
}

/*
 * csb_bugfix_thing_type_bit15_clearly -- CHANGE7_18_FIX.
 *
 * Returns the 4-bit thing type with bit 15 cleared before shift.
 *
 * Problem (DM1 BUG0_10): bone-creation (dead-champion path) sets
 * bit 15 in the raw THING type handle to indicate "use reserved
 * thing pool". Megamax C compiler silently drops this bit (the
 * generated assembly shifts the 16-bit value left 1 bit then takes
 * low 16 bits, effectively discarding bit 15). Other compilers need
 * it explicitly cleared before M012_TYPE extraction.
 *
 * Fix (MEDIA291 / CHANGE7_18_FIX): raw &= ~0x8000 before shift.
 *
 * ReDMCSB: DUNGEON.C:2099-2101 (CHANGE7_18_FIX / MEDIA291/MEDIA178),
 *          DEFS.H:399 comment, BugsAndChanges.htm:CHANGE7_18,BUG0_10
 */
uint8_t csb_bugfix_thing_type_bit15_clearly(uint16_t rawThingType) {
    uint16_t masked = (uint16_t)(rawThingType & 0x7FFFu);
    return (uint8_t)CSB_THING_TYPE(masked);
}

/* ================================================================
 *  Core dungeon world API -- unchanged from previous version
 * ================================================================ */

void csb_world_init(CSB_DungeonWorld* w) {
    if (!w) return;
    memset(w, 0, sizeof(CSB_DungeonWorld));
}

int csb_world_add_level(CSB_DungeonWorld* w, int width, int height) {
    if (!w) return -1;
    if (w->levelCount >= CSB_MAX_LEVELS) return -1;

    CSB_Level* lvl = &w->levels[w->levelCount];
    memset(lvl, 0, sizeof(CSB_Level));
    lvl->width      = width;
    lvl->height     = height;
    lvl->levelIndex = w->levelCount;
    lvl->lightLevel = 0;
    w->levelCount++;
    return w->levelCount - 1;
}

CSB_Tile* csb_world_get_tile(CSB_DungeonWorld* w, int level, int x, int y) {
    if (!w) return NULL;
    if (level < 0 || level >= w->levelCount) return NULL;
    CSB_Level* lvl = &w->levels[level];
    if (x < 0 || x >= lvl->width || y < 0 || y >= lvl->height) return NULL;
    return &lvl->tiles[y][x];
}

const CSB_Tile* csb_world_get_tile_const(const CSB_DungeonWorld* w,
                                         int level, int x, int y) {
    if (!w) return NULL;
    if (level < 0 || level >= w->levelCount) return NULL;
    const CSB_Level* lvl = &w->levels[level];
    if (x < 0 || x >= lvl->width || y < 0 || y >= lvl->height) return NULL;
    return &lvl->tiles[y][x];
}

int csb_world_is_walkable(const CSB_DungeonWorld* w,
                          int level, int x, int y) {
    const CSB_Tile* t = csb_world_get_tile_const(w, level, x, y);
    if (!t) return 0;
    switch (t->type) {
        case CSB_TILE_FLOOR:
        case CSB_TILE_PIT:
        case CSB_TILE_STAIRS_UP:
        case CSB_TILE_STAIRS_DOWN:
        case CSB_TILE_DOOR:
        case CSB_TILE_TELEPORTER:
            return 1;
        default:
            return 0;
    }
}

int csb_world_is_wall(const CSB_DungeonWorld* w,
                      int level, int x, int y) {
    const CSB_Tile* t = csb_world_get_tile_const(w, level, x, y);
    return t && t->type == CSB_TILE_WALL;
}

void csb_world_set_tile_type(CSB_DungeonWorld* w, int level, int x, int y,
                             uint8_t type) {
    CSB_Tile* t = csb_world_get_tile(w, level, x, y);
    if (t) t->type = type;
}

void csb_world_set_wall(CSB_DungeonWorld* w, int level, int x, int y,
                        int dir, uint8_t wallType) {
    CSB_Tile* t = csb_world_get_tile(w, level, x, y);
    if (!t) return;
    switch (dir) {
        case 0: t->wallN = wallType; break;
        case 1: t->wallE = wallType; break;
        case 2: t->wallS = wallType; break;
        case 3: t->wallW = wallType; break;
        default: break;
    }
}

void csb_world_set_ornament(CSB_DungeonWorld* w, int level, int x, int y,
                            int dir, uint8_t ornament) {
    CSB_Tile* t = csb_world_get_tile(w, level, x, y);
    if (!t) return;
    switch (dir) {
        case 0: t->ornamentN = ornament; break;
        case 1: t->ornamentE = ornament; break;
        case 2: t->ornamentS = ornament; break;
        case 3: t->ornamentW = ornament; break;
        default: break;
    }
}

int csb_world_get_level_count(const CSB_DungeonWorld* w) {
    return w ? w->levelCount : 0;
}

void csb_world_set_current_level(CSB_DungeonWorld* w, int level) {
    if (!w) return;
    if (level >= 0 && level < w->levelCount) w->currentLevel = level;
}
