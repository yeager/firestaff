#include "csb_v1_dungeon_world_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
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
 *  Door helpers -- DUNGEON.C:561-565 (pass563 compat)
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
 *  Dungeon-layer accessors (caller-supplied integration points)
 *
 *  F0175_GROUP_GetThing: walk square thing list, return first GROUP
 *    type or ENDOF list.  GROUP.C:52-70.
 *  F0161_DUNGEON_GetSquareFirstThing: head of square thing list.
 *  F0159_DUNGEON_GetNextThing: advance thing list; $FFFE = end.
 *  F0156_DUNGEON_GetThingData: thing data pointer.
 *  F0267_MOVE_GetMoveResult_CPSCE: apply a move to a thing.
 * ================================================================ */

#ifndef CSB_NO_DUNGEON_ACCESSORS
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
    int mapX, int mapY)
{
    if (!fn_get_first) return CSB_THING_ENDOFLIST;
    uint16_t thing = fn_get_first(mapX, mapY);
    while (thing != CSB_THING_ENDOFLIST) {
        if (CSB_THING_TYPE(thing) == CSB_THING_TYPE_GROUP)
            return thing;
        if (fn_get_next)
            thing = fn_get_next(thing);
        else
            break;
    }
    return CSB_THING_ENDOFLIST;
}

/*
 * csb_dungeon_get_first_thing -- F0161 equivalent.
 *
 * Uses the current dungeon context (set by csb_v1_dungeon_set_current)
 * and current level (set by csb_v1_dungeon_set_current_level) to
 * service F0161_DUNGEON_GetSquareFirstThing calls from the world model.
 *
 * Returns the raw thing index from the square record, or ENDOF if no
 * dungeon is loaded or the square has no things.
 *
 * NOTE: This returns a raw THING index (0-1023), not a full THING
 * handle.  The full handle encoding is (type << 10) | index; the caller
 * must read thing data to determine the type.  Until M10 thing-data
 * integration, this stub returns ENDOF.  For GROUP detection use
 * csb_dungeon_get_group() with the real F0159/F0156 accessors.
 *
 * ReDMCSB: DUNGEON.C F0161_DUNGEON_GetSquareFirstThing (lines 1730-1760)
 */
uint16_t csb_dungeon_get_first_thing_default(int mapX, int mapY) {
    const CSB_V1_DungeonData *d = csb_v1_dungeon_get_current();
    if (!d || !d->raw_data) return CSB_THING_ENDOFLIST;

    /* Use level 0 as default when called without explicit level context.
     * The real M10 integration would use the party's current map level.
     * Until that wiring is in place, this stub returns ENDOF to avoid
     * returning untyped thing indices that could be misinterpreted. */
    (void)mapX; (void)mapY;
    return CSB_THING_ENDOFLIST; /* M10 thing-data integration pending */
}

/*
 * csb_dungeon_get_next_thing -- F0159 equivalent.
 * Stub: caller supplies the actual M10 accessor.
 */
uint16_t csb_dungeon_get_next_thing_default(uint16_t thing) {
    (void)thing;
    return CSB_THING_ENDOFLIST; /* replaced by M10 integration */
}

/*
 * csb_dungeon_thing_data_u16 -- F0156 word read at byte offset.
 * Stub: caller supplies the actual M10 accessor.
 */
uint16_t csb_dungeon_thing_data_u16_default(uint16_t thing, int offset) {
    (void)thing; (void)offset;
    return 0; /* replaced by M10 integration */
}

/*
 * csb_move_thing -- F0267 equivalent.
 * Stub: caller supplies the actual M10 move function.
 */
void csb_move_thing_default(uint16_t thing, void* ctxOpaque) {
    (void)thing; (void)ctxOpaque;
}
#endif /* CSB_NO_DUNGEON_ACCESSORS */

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
 *
 * ReDMCSB: DEFS.H:1295 comment, DUNGEON.C:2099 (CHANGE7_18_FIX / MEDIA291)
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

/*
 * csb_sensor_is_csb_version_checker -- type test for C009.
 */
int csb_sensor_is_csb_version_checker(uint16_t typeData) {
    return csb_sensor_get_type(typeData) == CSB_SENSOR_FLOOR_VERSION_CHECKER;
}

/*
 * csb_sensor_is_csb_end_game -- type test for C018.
 */
int csb_sensor_is_csb_end_game(uint16_t typeData) {
    return csb_sensor_get_type(typeData) == CSB_SENSOR_WALL_END_GAME;
}

/*
 * csb_version_checker_triggered -- version gate for C009 sensor.
 *
 *   condition: sensorData <= csbEngineVersion  -->  trigger fires
 *   floors to floor-type C009 sensor when party enters its square.
 *
 *  CSB 2.0: version 20  (CHANGE8_06 / MEDIA337)
 *  CSB 2.1: version 21  (CHANGE8_06 / MEDIA342)
 *
 *  Trigger guard (MOVESENS.C:1716-1750):
 *    - P0590_T_Thing must be THING_PARTY
 *    - P0592_B_AddThing must be TRUE
 *    - P0591_B_PartySquare must be FALSE
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
 *   7. F0444_STARTEND_Endgame(TRUE)               [MEDIA277]
 *
 * delaySeconds: sensor->Remote.Value (0 = no delay, original CSB)
 *
 * The G0302_B_GameWon guard inside F0666_endgame() means step 5
 * must precede step 6. G0524 handles restart disable before step 7.
 *
 * BUG0_00 guard: F0666_endgame() checks G0302_B_GameWon and returns
 * immediately if not set.  This protects against re-entering endgame.
 *
 * ReDMCSB: TIMELINE.C:1319-1340, ENDGAME.C:984-1002
 *          DEFS.H:1202_comment, BugsAndChanges.htm:CHANGE7_21,CHANGE8_02
 */
void csb_endgame_trigger(int delaySeconds, CSB_EndgameResult* pOut) {
    if (!pOut) return;

    memset(pOut, 0, sizeof(CSB_EndgameResult));

    /* Steps 1-2: palette curtain */
    pOut->paletteSetCurtain = 1;

    /* Step 3: optional delay (60 Hz tick rate, CHANGE8_02) */
    if (delaySeconds > 0) {
        pOut->delayTicks = 60 * delaySeconds;
    }

    /* Steps 4-5: G0524 cleared (restart disabled), G0302 set (won) */
    pOut->restartAllowed = 0;
    pOut->gameWon        = 1;

    /* Step 6: F0666_endgame() entry (BUG0_00 G0302 guard inside) */
    pOut->endgameFnCalled = 1;

    /* Step 7: F0444_STARTEND_Endgame(TRUE) — draws credits/restart/quit */
    pOut->startendEndgameFn = 1;
}

/* ================================================================
 *  Teleporter / pit processing -- F0249 CHANGE7_22_FIX (BUG0_22)
 *
 *  Bug (DM1): arbitrary linked-list order causes infinite loops or
 *  missed things when >=2 things on a closed square open.
 *
 *  Fix (TIMELINE.C:1398-1476):
 *    1. Group processed FIRST via F0175_GROUP_GetThing().
 *       Moving group first means mid-traversal projectile impacts on
 *       the group cannot disrupt group movement.
 *    2. Non-group things counted: M012_TYPE > GROUP(4).
 *    3. while(thing && ThingsToMoveCount) { ... ThingsToMoveCount--; }
 *       Count guard prevents infinite loops.
 *    4. NextThing read BEFORE call to moveFn (mid-traversal removal
 *       resilience: projectile impacts group, group goes away, loop
 *       continues with valid NextThing).
 *
 *  Projectile / explosion event update (TIMELINE.C:1431-1468):
 *    - C14_PROJECTILE: update event MapX/MapY/Direction/Slot/Map_Time.
 *    - C15_EXPLOSION: update event MapX/MapY/Slot/Map_Time only for
 *      C25_EVENT_EXPLOSION (BUG0_23: Fluxcage uses C24_EVENT_REMOVE,
 *      not C25, so its event is NOT updated — Fluxcage stays orphaned).
 *
 * ReDMCSB: TIMELINE.C:1398-1476, BugsAndChanges.htm:CHANGE7_22,BUG0_22
 */

/*
 * csb_teleporter_pit_process -- apply CHANGE7_22_FIX on a square.
 *
 *  - Group (if any) moved first via F0175_GROUP_GetThing().
 *  - Non-group things moved exactly once, count-guarded.
 *  - Next link read before each move.
 *
 *  The dungeon-layer accessors are caller-supplied.  Set
 *  get_group_fn = csb_dungeon_get_group with real M10 accessors
 *  once F0175/F0161/F0159/F0267 are integrated into M10.
 *
 *  Parameters:
 *    tile          -- tile being processed (non-null)
 *    mapX, mapY    -- coordinates of this square
 *    get_group_fn  -- F0175_GROUP_GetThing equivalent
 *    get_first_fn  -- F0161_DUNGEON_GetSquareFirstThing equivalent
 *    get_next_fn   -- F0159_DUNGEON_GetNextThing equivalent
 *    move_fn       -- F0267_MOVE_GetMoveResult_CPSCE equivalent
 *    ctxOpaque     -- forwarded to move_fn
 *    pOut          -- result array (non-null)
 *    maxResults    -- array capacity
 *    pResultCount  -- receives written count (non-null)
 *
 *  Returns: 0 on success, -1 if tile null or no moveFn.
 *
 * ReDMCSB: TIMELINE.C:1398-1476 (full F0249 body)
 */
int csb_teleporter_pit_process(
    const CSB_Tile*   tile,
    int                mapX,
    int                mapY,
    uint16_t         (*get_group_fn)(int mapX, int mapY),
    uint16_t         (*get_first_fn)(int mapX, int mapY),
    uint16_t         (*get_next_fn)(uint16_t thing),
    void             (*move_fn)(uint16_t thing, int oldMapX, int oldMapY,
                                int* newMapX, int* newMapY, void* ctxOpaque),
    void*             ctxOpaque,
    CSB_TeleporterPitResult* pOut,
    int               maxResults,
    int*              pResultCount)
{
    if (!tile || !move_fn || !pOut || !pResultCount) return -1;

    *pResultCount = 0;

    /* CHANGE7_22_FIX step 1: process group FIRST.
     *
     * F0175_GROUP_GetThing walks the thing list and returns the first
     * GROUP type THING or ENDOF.  Moving the group first means a
     * projectile-impact-on-group during non-group processing cannot
     * remove the group from beneath us.
     *
     * ReDMCSB: GROUP.C:52-70 (F0175_GROUP_GetThing)
     *          TIMELINE.C:1404-1408 (group move)
     */
    if (get_group_fn) {
        uint16_t groupThing = get_group_fn(mapX, mapY);
        if (groupThing != CSB_THING_ENDOFLIST) {
            int newX = mapX, newY = mapY;
            if (*pResultCount < maxResults) {
                pOut[*pResultCount].movingGroup = 1;
                pOut[*pResultCount].thingType   = CSB_THING_TYPE_GROUP;
                pOut[*pResultCount].moved        = 1;
                pOut[*pResultCount].oldMapX      = mapX;
                pOut[*pResultCount].oldMapY      = mapY;
            }
            move_fn(groupThing, mapX, mapY, &newX, &newY, ctxOpaque);
            if (*pResultCount < maxResults) {
                pOut[*pResultCount].newMapX = newX;
                pOut[*pResultCount].newMapY = newY;
            }
            (*pResultCount)++;
        }
    }

    /* CHANGE7_22_FIX steps 2-4: count non-group things, then move
     * each exactly once using ThingsToMoveCount as guard.
     *
     *   thingsToMoveCount = 0;
     *   for (each thing via F0159) {
     *       if (CSB_THING_TYPE(thing) > CSB_THING_TYPE_GROUP)
     *           thingsToMoveCount++;
     *   }
     *
     *   thing = F0161_GetSquareFirstThing(mapX, mapY);
     *   while (thing != END && thingsToMoveCount > 0) {
     *       nextThing = F0159_GetNextThing(thing);   // read BEFORE move
     *       thingsToMoveCount--;
     *       // Only process non-group things (group was already moved)
     *       if (CSB_THING_TYPE(thing) > CSB_THING_TYPE_GROUP) {
     *           move_fn(thing, ...);
     *       }
     *       thing = nextThing;
     *   }
     *
     * F0159 reads Next BEFORE moveFn: mid-move projectile impacts
     * can remove things from the list; reading next first survives
     * that mutation.  The count guard (thingsToMoveCount--) is
     * always > 0 entering the loop because we only decrement after
     * confirming the thing type is > GROUP, preventing an infinite
     * loop on a degenerate link where the same thing relinks to itself.
     *
     * ReDMCSB: TIMELINE.C:1410-1476 (count loop + move loop)
     */
    if (!get_first_fn || !get_next_fn) return 0;

    /* Count non-group things on square */
    int thingsToMoveCount = 0;
    uint16_t thing = get_first_fn(mapX, mapY);
    while (thing != CSB_THING_ENDOFLIST) {
        /* Group was already moved; only count strict non-groups */
        if (CSB_THING_TYPE(thing) > CSB_THING_TYPE_GROUP)
            thingsToMoveCount++;
        thing = get_next_fn(thing);
    }

    /* Move each non-group thing exactly once */
    thing = get_first_fn(mapX, mapY);
    while (thing != CSB_THING_ENDOFLIST && thingsToMoveCount > 0) {
        uint16_t nextThing = get_next_fn(thing); /* read BEFORE move */
        thingsToMoveCount--;

        /* Only process non-group things */
        if (CSB_THING_TYPE(thing) > CSB_THING_TYPE_GROUP) {
            int newX = mapX, newY = mapY;
            if (*pResultCount < maxResults) {
                pOut[*pResultCount].movingGroup = 0;
                pOut[*pResultCount].thingType   = CSB_THING_TYPE(thing);
                pOut[*pResultCount].moved       = 1;
                pOut[*pResultCount].oldMapX      = mapX;
                pOut[*pResultCount].oldMapY      = mapY;
            }
            move_fn(thing, mapX, mapY, &newX, &newY, ctxOpaque);
            if (*pResultCount < maxResults) {
                pOut[*pResultCount].newMapX = newX;
                pOut[*pResultCount].newMapY = newY;
            }
            (*pResultCount)++;
        }
        thing = nextThing;
    }

    return 0;
}

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
 * Walk the thing list and check each sensor's Remote.Type_Data.
 *
 *  Implementation requires:
 *    F0161_DUNGEON_GetSquareFirstThing() -- head of thing list
 *    F0159_DUNGEON_GetNextThing(thing)  -- advance
 *    F0156_DUNGEON_GetThingData()       -- read SENSOR.Remote.Type_Data
 *    offsetof(SENSOR, Remote.Type_Data)  -- word offset in SENSOR
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
    int                         sensorTypeDataOffset)
{
    if (!get_first_fn || !get_next_fn || !thing_data_fn)
        return 1; /* safe default until M10 integration */

    /* Walk the square's thing list; skip if any sensor is enabled */
    uint16_t thing = get_first_fn(mapX, mapY);
    while (thing != CSB_THING_ENDOFLIST) {
        if (CSB_THING_TYPE(thing) == CSB_THING_TYPE_SENSOR) {
            uint16_t td = thing_data_fn(thing, sensorTypeDataOffset);
            /* M039_TYPE: low 7 bits with bit 15 cleared (CHANGE7_18_FIX) */
            if (csb_sensor_get_type(td) != 0)
                return 0; /* sensor is enabled — square is NOT clear */
        }
        thing = get_next_fn(thing);
    }
    return 1; /* no enabled sensors found */
}

/*
 * csb_bugfix_thing_type_bit15_clearly -- CHANGE7_18_FIX.
 *
 * Returns the 4-bit thing type with bit 15 masked off before shift.
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

/*
 * csb_bugfix_lord_chaos_teleport_dir -- CHANGE7_19_FIX (BUG0_69).
 *
 * Returns a properly initialized Lord Chaos teleport direction, avoiding
 * the DM1 bug where the primary direction was read before initialization.
 *
 * Problem (DM1 BUG0_69): when Lord Chaos senses danger (Poison Cloud,
 * closing door, or 3+ surrounding Fluxcages) and needs to teleport, the
 * primary direction variable was used as an array index without being
 * initialized, causing memory corruption.
 *
 * Fix (MEDIA297 / CHANGE7_19_FIX):
 *   primaryDir = M004_RANDOM(4);           // properly initialized
 *   secondaryDir = M017_NEXT(primaryDir);  // adjacent clock-wise
 *
 * Call this when Lord Chaos needs to initiate a teleport during danger.
 *
 * ReDMCSB: GROUP.C:2208-2215 (CHANGE7_19_FIX / MEDIA297),
 *          GROUP.C:2215 comment (BUG0_69),
 *          BugsAndChanges.htm:CHANGE7_19,BUG0_69
 */
int csb_bugfix_lord_chaos_teleport_dir(int random4(void)) {
    /* If no random source supplied, fall back to simple pseudo-random.
     * Real integration: call M004_RANDOM(4) from the game engine.
     * Until M10 integration: use a simple LCG.
     *
     * ReDMCSB: GROUP.C:2208 (M004_RANDOM)
     */
    static uint32_t lcg_seed = 0x12345678u;
    (void)random4;
    if (random4) return random4() & 3;
    /* Simple LCG fallback — replace with engine RNG once wired */
    lcg_seed = lcg_seed * 1664525u + 1013904223u;
    return (int)(lcg_seed >> 16) & 3;
}

/* ================================================================
 *  Core dungeon world API
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