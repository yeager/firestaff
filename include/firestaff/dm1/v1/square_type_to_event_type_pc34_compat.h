#ifndef FIRESTAFF_DM1_V1_SQUARETYPETOEVENTTYPE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SQUARETYPETOEVENTTYPE_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0059_auc_Graphic562_SquareTypeToEventType[7].
 *
 * G0059 is the 7-entry map from square-type index to event-type
 * index. PC 3.4 init = {C06_EVENT_WALL=6, C05_EVENT_CORRIDOR=5,
 * C09_EVENT_PIT=9, C00_EVENT_NONE=0, C10_EVENT_DOOR=10,
 * C08_EVENT_TELEPORTER=8, C07_EVENT_FAKEWALL=7}. Read sites:
 * MOVESENS.C:1206 (F0268_SENSOR_AddEvent using G0059[SquareType]).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-852.
 */

#define DM1_V1_SQUARE_TYPE_TO_EVENT_TYPE_PC34_COMPAT_SIZE 7

typedef struct DM1_V1_SquareTypeToEventTypeResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_SQUARE_TYPE_TO_EVENT_TYPE_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int square0WallEvent6;
    int square1CorridorEvent5;
    int square2PitEvent9;
    int square3NoneEvent0;
    int square4DoorEvent10;
    int square5TeleporterEvent8;
    int square6FakewallEvent7;
    int allEventsInValidRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_SquareTypeToEventTypeResultPc34;

const unsigned char *
dm1_v1_square_type_to_event_type_table_pc34(void);

int
dm1_v1_square_type_to_event_type_size_pc34(void);

int
dm1_v1_square_type_to_event_type_get_pc34(int square_type);

int
dm1_v1_square_type_to_event_type_run_pc34(
    DM1_V1_SquareTypeToEventTypeResultPc34 *out);

#endif