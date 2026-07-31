#include "csb_v1_dungeon_world_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include <stdlib.h>
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

int csb_v1_dungeon_get_group_cells_pc34(
    const uint8_t *group_record,
    int record_size,
    uint8_t *out_cells)
{
    /* ReDMCSB DEFS.H GROUP is a 16-byte C04 record: Type at byte 4 and
     * Cells at byte 5. DUNGEON.C F0145 returns that packed byte; it does
     * not normalize the 0xff centered-group sentinel. */
    if (!group_record || !out_cells || record_size < 16 ||
        group_record[4] != CSB_THING_TYPE_GROUP) {
        return 0;
    }
    *out_cells = group_record[5];
    return 1;
}

int csb_v1_dungeon_get_group_directions_pc34(
    const uint8_t *group_record,
    int record_size,
    uint8_t *out_directions)
{
    /* ReDMCSB DEFS.H GROUP is a 16-byte C04 record. Direction is the
     * two-bit field at bits 8..9 of its final little-endian word, so F0147
     * reads bits 0..1 of byte 15. Byte 4 is CreatureType, not a C04 tag. */
    if (!group_record || !out_directions || record_size < 16) {
        return 0;
    }
    *out_directions = (uint8_t)(group_record[15] & 0x03u);
    return 1;
}

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
 *  Dungeon-layer accessors (live M10 integration points)
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
 * Returns the square's first THING handle, or ENDOF if no dungeon is loaded
 * or the square has no things.
 *
 * Real CSB-format dungeons return the imported full THING handle from
 * the square-first-thing table. Legacy 16-bit synthetic fixtures still
 * encode only an untyped index in the square word, so this default runtime
 * accessor leaves those fixtures at ENDOF.
 *
 * ReDMCSB: DUNGEON.C F0160_DUNGEON_GetSquareFirstThingIndex lines 1699-1728,
 *          F0161_DUNGEON_GetSquareFirstThing lines 1730-1746.
 */
uint16_t csb_dungeon_get_first_thing_default(int mapX, int mapY) {
    const CSB_V1_DungeonData *d = csb_v1_dungeon_get_current();
    int thing;
    if (!d || !d->raw_data) return CSB_THING_ENDOFLIST;

    if (d->square_bytes != 1) return CSB_THING_ENDOFLIST;
    thing = csb_v1_dungeon_get_first_thing(
        d, csb_v1_dungeon_get_current_level(), mapX, mapY);
    return (thing >= 0) ? (uint16_t)thing : CSB_THING_ENDOFLIST;
}

/*
 * csb_dungeon_get_next_thing -- F0159 equivalent.
 *
 * Generic.Next is the first little-endian word in every Thing record.
 * ReDMCSB: DUNGEON.C F0159 lines 1664-1676.
 */
uint16_t csb_dungeon_get_next_thing_default(uint16_t thing) {
    const CSB_V1_DungeonData *d = csb_v1_dungeon_get_current();
    const uint8_t *record;
    int size;
    if (!d) return CSB_THING_ENDOFLIST;
    record = csb_v1_dungeon_get_thing_record(d, thing, NULL, NULL, &size);
    if (!record || size < 2) return CSB_THING_ENDOFLIST;
    return (uint16_t)((uint16_t)record[0] | ((uint16_t)record[1] << 8));
}

/*
 * csb_dungeon_thing_data_u16 -- F0156 word read at byte offset.
 * Invalid Thing/offset returns zero, matching the existing narrow u16 helper
 * contract while keeping runtime callers away from fabricated records.
 */
uint16_t csb_dungeon_thing_data_u16_default(uint16_t thing, int offset) {
    const CSB_V1_DungeonData *d = csb_v1_dungeon_get_current();
    const uint8_t *record;
    int size;
    if (!d || offset < 0) return 0;
    record = csb_v1_dungeon_get_thing_record(d, thing, NULL, NULL, &size);
    if (!record || offset > size - 2) return 0;
    return (uint16_t)((uint16_t)record[offset] |
                      ((uint16_t)record[offset + 1] << 8));
}

static uint16_t csb_dungeon_read_u16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void csb_dungeon_write_u16(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)(value >> 8);
}

static int csb_dungeon_current_column_count(const CSB_V1_DungeonData *d) {
    int level;
    int columns = 0;
    if (!d) return -1;
    for (level = 0; level < d->level_count; ++level) {
        columns += d->level_widths[level];
    }
    return columns;
}

static int csb_dungeon_square_list_slot(const CSB_V1_DungeonData *d,
                                        int map_x, int map_y,
                                        int require_present) {
    int level = csb_v1_dungeon_get_current_level();
    int prior_columns = 0;
    int x;
    int y;
    int square_offset;
    int column_count_offset;
    int slot;
    if (!d || !d->raw_data || d->square_bytes != 1 || level < 0 ||
        level >= d->level_count || map_x < 0 || map_y < 0 ||
        map_x >= d->level_widths[level] || map_y >= d->level_heights[level]) {
        return -1;
    }
    square_offset = d->level_offsets[level] +
                    map_x * d->level_heights[level] + map_y;
    if (square_offset < 0 || square_offset >= d->raw_size ||
        (require_present && (d->raw_data[square_offset] & 0x10u) == 0u)) {
        return -1;
    }
    for (x = 0; x < level; ++x) prior_columns += d->level_widths[x];
    column_count_offset = 44 + d->level_count * 16 +
                          (prior_columns + map_x) * 2;
    if (column_count_offset < 0 || column_count_offset + 2 > d->raw_size) {
        return -1;
    }
    slot = (int)csb_dungeon_read_u16(d->raw_data + column_count_offset);
    square_offset = d->level_offsets[level] + map_x * d->level_heights[level];
    for (y = 0; y < map_y; ++y) {
        if (square_offset + y >= d->raw_size) return -1;
        if (d->raw_data[square_offset + y] & 0x10u) ++slot;
    }
    return slot;
}

static int csb_dungeon_adjust_column_counts(CSB_V1_DungeonData *d,
                                            int map_x, int delta) {
    int level = csb_v1_dungeon_get_current_level();
    int prior_columns = 0;
    int column;
    int column_count;
    if (!d || !d->raw_data || level < 0 || level >= d->level_count) return -1;
    for (column = 0; column < level; ++column) prior_columns += d->level_widths[column];
    column_count = csb_dungeon_current_column_count(d);
    if (column_count < 0 || prior_columns + map_x + 1 > column_count) return -1;
    for (column = prior_columns + map_x + 1; column < column_count; ++column) {
        int offset = 44 + d->level_count * 16 + column * 2;
        int value;
        if (offset < 0 || offset + 2 > d->raw_size) return -1;
        value = (int)csb_dungeon_read_u16(d->raw_data + offset) + delta;
        if (value < 0 || value > 0xFFFF) return -1;
        csb_dungeon_write_u16(d->raw_data + offset, (uint16_t)value);
    }
    return 0;
}

static uint8_t *csb_dungeon_mutable_thing_record(CSB_V1_DungeonData *d,
                                                  uint16_t thing,
                                                  int *out_size) {
    const uint8_t *record = csb_v1_dungeon_get_thing_record(
        d, thing, NULL, NULL, out_size);
    return (uint8_t *)record;
}

static int csb_dungeon_unlink_thing(CSB_V1_DungeonData *d, uint16_t thing,
                                    int map_x, int map_y) {
    int slot = csb_dungeon_square_list_slot(d, map_x, map_y, 1);
    uint8_t *first_things;
    uint16_t previous = CSB_THING_ENDOFLIST;
    uint16_t current;
    uint16_t next;
    uint8_t *record;
    int size;
    int guard;
    int square_offset;
    if (slot < 0 || slot >= d->square_first_thing_count) return -1;
    first_things = d->raw_data + d->square_first_thing_base;
    current = csb_dungeon_read_u16(first_things + slot * 2);
    for (guard = 0; guard < d->square_first_thing_count; ++guard) {
        if (current == CSB_THING_ENDOFLIST || current == CSB_THING_PARTY) return -2;
        record = csb_dungeon_mutable_thing_record(d, current, &size);
        if (!record || size < 2) return -1;
        next = csb_dungeon_read_u16(record);
        if ((current & 0x3FFFu) == (thing & 0x3FFFu)) {
            if (previous == CSB_THING_ENDOFLIST) {
                if (next == CSB_THING_ENDOFLIST) {
                    if (slot + 1 < d->square_first_thing_count) {
                        memmove(first_things + slot * 2,
                                first_things + (slot + 1) * 2,
                                (size_t)(d->square_first_thing_count - slot - 1) * 2u);
                    }
                    csb_dungeon_write_u16(first_things +
                                           (d->square_first_thing_count - 1) * 2,
                                           CSB_THING_PARTY);
                    square_offset = d->level_offsets[csb_v1_dungeon_get_current_level()] +
                                    map_x * d->level_heights[csb_v1_dungeon_get_current_level()] + map_y;
                    d->raw_data[square_offset] &= (uint8_t)~0x10u;
                    if (csb_dungeon_adjust_column_counts(d, map_x, -1) != 0) return -1;
                } else {
                    csb_dungeon_write_u16(first_things + slot * 2, next);
                }
            } else {
                uint8_t *previous_record = csb_dungeon_mutable_thing_record(d, previous, &size);
                if (!previous_record || size < 2) return -1;
                csb_dungeon_write_u16(previous_record, next);
            }
            csb_dungeon_write_u16(record, CSB_THING_ENDOFLIST);
            return 0;
        }
        previous = current;
        current = next;
    }
    return -2;
}

static int csb_dungeon_link_thing(CSB_V1_DungeonData *d, uint16_t thing,
                                  int map_x, int map_y) {
    int slot = csb_dungeon_square_list_slot(d, map_x, map_y, 0);
    int present_slot = csb_dungeon_square_list_slot(d, map_x, map_y, 1);
    uint8_t *first_things;
    uint8_t *record;
    uint16_t current;
    uint16_t next;
    int size;
    int guard;
    int square_offset;
    if (slot < 0 || slot > d->square_first_thing_count) return -1;
    record = csb_dungeon_mutable_thing_record(d, thing, &size);
    if (!record || size < 2) return -1;
    first_things = d->raw_data + d->square_first_thing_base;
    csb_dungeon_write_u16(record, CSB_THING_ENDOFLIST);
    if (present_slot < 0) {
        if (slot >= d->square_first_thing_count ||
            csb_dungeon_read_u16(first_things +
                                  (d->square_first_thing_count - 1) * 2) != CSB_THING_PARTY) {
            return -1;
        }
        if (slot < d->square_first_thing_count - 1) {
            memmove(first_things + (slot + 1) * 2, first_things + slot * 2,
                    (size_t)(d->square_first_thing_count - slot - 1) * 2u);
        }
        csb_dungeon_write_u16(first_things + slot * 2, thing);
        square_offset = d->level_offsets[csb_v1_dungeon_get_current_level()] +
                        map_x * d->level_heights[csb_v1_dungeon_get_current_level()] + map_y;
        d->raw_data[square_offset] |= 0x10u;
        return csb_dungeon_adjust_column_counts(d, map_x, 1);
    }
    current = csb_dungeon_read_u16(first_things + present_slot * 2);
    for (guard = 0; guard < d->square_first_thing_count; ++guard) {
        uint8_t *current_record;
        if (current == CSB_THING_ENDOFLIST || current == CSB_THING_PARTY) return -1;
        current_record = csb_dungeon_mutable_thing_record(d, current, &size);
        if (!current_record || size < 2) return -1;
        next = csb_dungeon_read_u16(current_record);
        if (next == CSB_THING_ENDOFLIST) {
            csb_dungeon_write_u16(current_record, thing);
            return 0;
        }
        current = next;
    }
    return -1;
}

int csb_dungeon_move_thing_default(uint16_t thing,
                                   int source_map_x, int source_map_y,
                                   int destination_map_x, int destination_map_y) {
    CSB_V1_DungeonData *d = csb_v1_dungeon_get_current_mutable();
    uint8_t *before = NULL;
    int status = 0;

    if (!d || !d->raw_data || d->square_bytes != 1 ||
        thing == CSB_THING_ENDOFLIST || thing == CSB_THING_PARTY) return -1;
    if (source_map_x == destination_map_x && source_map_y == destination_map_y &&
        source_map_x >= 0) return 0;

    /* F0163/F0164 mutate the same source-owned byte arena: a rejected target
     * must not leave a valid source Thing unlinked.  Snapshot only the loaded
     * original arena, never a decoded substitute; allocation failure closes
     * the operation before F0164 can mutate it. */
    before = (uint8_t *)malloc((size_t)d->raw_size);
    if (!before) return -1;
    memcpy(before, d->raw_data, (size_t)d->raw_size);
    if (source_map_x >= 0) {
        if (csb_dungeon_unlink_thing(d, thing, source_map_x, source_map_y) != 0) {
            status = -2;
            goto rollback;
        }
    }
    if (destination_map_x >= 0) {
        if (csb_dungeon_link_thing(d, thing, destination_map_x, destination_map_y) != 0) {
            status = -1;
            goto rollback;
        }
    }
    free(before);
    return 0;

rollback:
    memcpy(d->raw_data, before, (size_t)d->raw_size);
    free(before);
    return status;
}

int csb_dungeon_move_thing_between_levels_default(
    uint16_t thing,
    int source_level,
    int source_map_x,
    int source_map_y,
    int destination_level,
    int destination_map_x,
    int destination_map_y)
{
    int saved_level;
    int status;

    if (source_level < 0 || destination_level < 0) return -1;
    saved_level = csb_v1_dungeon_get_current_level();
    if (source_level == destination_level) {
        csb_v1_dungeon_set_current_level(source_level);
        status = csb_dungeon_move_thing_default(
            thing, source_map_x, source_map_y,
            destination_map_x, destination_map_y);
        csb_v1_dungeon_set_current_level(saved_level);
        return status;
    }

    csb_v1_dungeon_set_current_level(source_level);
    status = csb_dungeon_move_thing_default(
        thing, source_map_x, source_map_y, -1, -1);
    if (status != 0) {
        csb_v1_dungeon_set_current_level(saved_level);
        return status;
    }
    csb_v1_dungeon_set_current_level(destination_level);
    status = csb_dungeon_move_thing_default(
        thing, -1, -1, destination_map_x, destination_map_y);
    if (status != 0) {
        /* A capacity or destination validation failure must not lose the
         * source Thing; restore it through the same F0163-style primitive. */
        csb_v1_dungeon_set_current_level(source_level);
        (void)csb_dungeon_move_thing_default(
            thing, -1, -1, source_map_x, source_map_y);
    }
    csb_v1_dungeon_set_current_level(saved_level);
    return status;
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
    /* ReDMCSB GROUP.C:2208 (CHANGE7_19_FIX / MEDIA297):
     *   primaryDir = M004_RANDOM(4)
     * Properly initialized before the array-index use that
     * triggers BUG0_69 memory corruption.  The caller MUST
     * supply a function that returns the engine M004_RANDOM
     * result (range 0..3).  When no source is supplied,
     * the function returns 0 (NORTH) which is the source's
     * pre-decrement sentinel value — callers should treat
     * 0 as a "not yet randomised" marker and trigger their
     * own RNG fallback if needed.  v1 deliberately does NOT
     * use a non-deterministic LCG; deterministic-by-default
     * keeps the headless-combat suite reproducible across
     * RNG versions. */
    if (random4) return random4() & 3;
    /* Sentinel: caller should treat 0 as "uninitialised". */
    return 0;
}

/*
 * csb_bugfix_lord_chaos_teleport_dirs -- CHANGE7_19_FIX pair helper.
 *
 * ReDMCSB GROUP.C:2208-2210 assigns both direction variables in one
 * expression:
 *   G0363_i_SecondaryDirectionToOrFromParty =
 *       M017_NEXT(L0454_i_PrimaryDirectionToOrFromParty = M004_RANDOM(4));
 * M017_NEXT is defined in DEFS.H:458/461 as (value + 1) & 3.
 */
int csb_bugfix_lord_chaos_teleport_dirs(int random4(void),
                                        int *primaryDir,
                                        int *secondaryDir) {
    int primary;

    if (!primaryDir || !secondaryDir) {
        return -1;
    }

    primary = csb_bugfix_lord_chaos_teleport_dir(random4);
    *primaryDir = primary;
    *secondaryDir = (primary + 1) & 3;
    return 0;
}
