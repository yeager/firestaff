#include "firestaff/dm1/v1/square_type_to_event_type_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0059_auc_Graphic562_SquareTypeToEventType):
 * - DATA.C:107 - declaration of G0059_auc_Graphic562_SquareTypeToEventType[7]
 * - DATA.C:107/470-477/1157 - declaration + PC 3.4 init + Atari init
 * - DATA.C:470-477 - PC 3.4 init { C06_EVENT_WALL=6, C05_EVENT_CORRIDOR=5,
 *                              C09_EVENT_PIT=9, C00_EVENT_NONE=0,
 *                              C10_EVENT_DOOR=10, C08_EVENT_TELEPORTER=8,
 *                              C07_EVENT_FAKEWALL=7 }
 * - DATA.C:477 - last entry of square-type-to-event-type init block
 * - DATA.C:1157 - post-1.3 Atari init (same values)
 * - MOVESENS.C:1206 - F0268_SENSOR_AddEvent(G0059[SquareType], x, y, cell, ...)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-852 (Graphics.dat init-table gates batches 1-10). This
 * gate is a non-mirror-candidate contract for the G0059
 * square-type-to-event-type mapping.
 */

enum {
    kTableSize  = 7,
    kOutOfRange = 0,

    /* Square-type indices (per the G0059 init block). */
    kWallIdx       = 0,
    kCorridorIdx   = 1,
    kPitIdx        = 2,
    kNoneIdx       = 3,
    kDoorIdx       = 4,
    kTeleporterIdx = 5,
    kFakewallIdx   = 6,

    /* Event-type values (DEFS.H). */
    kEventNone       = 0,
    kEventCorridor   = 5,
    kEventWall       = 6,
    kEventFakewall   = 7,
    kEventTeleporter = 8,
    kEventPit        = 9,
    kEventDoor       = 10
};

static const unsigned char s_g0059[kTableSize] = {
    /* 0 */ 6,  /* C06_EVENT_WALL */
    /* 1 */ 5,  /* C05_EVENT_CORRIDOR */
    /* 2 */ 9,  /* C09_EVENT_PIT */
    /* 3 */ 0,  /* C00_EVENT_NONE */
    /* 4 */ 10, /* C10_EVENT_DOOR */
    /* 5 */ 8,  /* C08_EVENT_TELEPORTER */
    /* 6 */ 7   /* C07_EVENT_FAKEWALL */
};

const unsigned char *
dm1_v1_square_type_to_event_type_table_pc34(void)
{
    return s_g0059;
}

int
dm1_v1_square_type_to_event_type_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_square_type_to_event_type_get_pc34(int square_type)
{
    if (square_type < 0 || square_type >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0059[square_type];
}

int
dm1_v1_square_type_to_event_type_run_pc34(
    DM1_V1_SquareTypeToEventTypeResultPc34 *out)
{
    int table_matches_declaration = 1;
    int square0_wall_event6 = 1;
    int square1_corridor_event5 = 1;
    int square2_pit_event9 = 1;
    int square3_none_event0 = 1;
    int square4_door_event10 = 1;
    int square5_teleporter_event8 = 1;
    int square6_fakewall_event7 = 1;
    int all_events_in_valid_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0059[i];
    }
    out->tableSize = kTableSize;

    if (s_g0059[kWallIdx]       != kEventWall)       square0_wall_event6 = 0;
    if (s_g0059[kCorridorIdx]   != kEventCorridor)   square1_corridor_event5 = 0;
    if (s_g0059[kPitIdx]        != kEventPit)        square2_pit_event9 = 0;
    if (s_g0059[kNoneIdx]       != kEventNone)       square3_none_event0 = 0;
    if (s_g0059[kDoorIdx]       != kEventDoor)       square4_door_event10 = 0;
    if (s_g0059[kTeleporterIdx] != kEventTeleporter) square5_teleporter_event8 = 0;
    if (s_g0059[kFakewallIdx]   != kEventFakewall)   square6_fakewall_event7 = 0;
    out->square0WallEvent6 = square0_wall_event6;
    out->square1CorridorEvent5 = square1_corridor_event5;
    out->square2PitEvent9 = square2_pit_event9;
    out->square3NoneEvent0 = square3_none_event0;
    out->square4DoorEvent10 = square4_door_event10;
    out->square5TeleporterEvent8 = square5_teleporter_event8;
    out->square6FakewallEvent7 = square6_fakewall_event7;

    /* Phase: all events are in [0, 255] (the F0268_SENSOR_AddEvent API). */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0059[i] > 255) {
            all_events_in_valid_range = 0;
        }
    }
    out->allEventsInValidRange = all_events_in_valid_range;

    /* Phase: table matches declared order. */
    {
        static const unsigned char kExpected[kTableSize] = {6, 5, 9, 0, 10, 8, 7};
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0059[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase: lookup function correctness. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_square_type_to_event_type_get_pc34(i) != (int)s_g0059[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase: out-of-range lookup returns 0. */
    if (dm1_v1_square_type_to_event_type_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_square_type_to_event_type_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_square_type_to_event_type_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->square0WallEvent6 &&
        out->square1CorridorEvent5 &&
        out->square2PitEvent9 &&
        out->square3NoneEvent0 &&
        out->square4DoorEvent10 &&
        out->square5TeleporterEvent8 &&
        out->square6FakewallEvent7 &&
        out->allEventsInValidRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 12;
    return out->accepted;
}