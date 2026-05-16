#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_dungeon_dat_pc34_compat.h"

int main(int argc, char* argv[]) {
    struct DungeonDatState_Compat state;
    struct DungeonThings_Compat things;
    int i;

    if (argc < 2) { fprintf(stderr, "Usage: %s <DUNGEON.DAT>\n", argv[0]); return 1; }

    if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &state)) { fprintf(stderr, "FAIL header\n"); return 1; }
    if (!F0502_DUNGEON_LoadTileData_Compat(argv[1], &state)) { fprintf(stderr, "FAIL tiles\n"); return 1; }
    if (!F0504_DUNGEON_LoadThingData_Compat(argv[1], &state, &things)) { fprintf(stderr, "FAIL things\n"); return 1; }

    printf("mapCount=%d textDataWordCount=%d sftCount=%d\n",
           state.header.mapCount, things.textDataWordCount, things.squareFirstThingCount);
    printf("Thing counts: ");
    for (i = 0; i < 16; i++) if (things.thingCounts[i]) printf("[%d]=%d ", i, things.thingCounts[i]);
    printf("\n");

    /* Check TextString offsets */
    printf("\n--- TextString offset check (count=%d, textDataWordCount=%d) ---\n", things.textStringCount, things.textDataWordCount);
    for (i = 0; i < things.textStringCount; i++) {
        struct DungeonTextString_Compat* ts = &things.textStrings[i];
        if (ts->next == 0xFFFFu) continue;
        if (ts->textDataWordOffset >= (unsigned short)things.textDataWordCount) {
            unsigned char* raw = things.rawThingData[2] + i * 4;
            printf("  BAD ts[%d]: next=0x%04X vis=%d offset=%d raw=[%02X %02X %02X %02X]\n",
                   i, ts->next, ts->visible, ts->textDataWordOffset, raw[0], raw[1], raw[2], raw[3]);
        }
    }
    /* Show first 10 used text strings */
    printf("\n--- First 10 used TextStrings ---\n");
    { int shown = 0;
    for (i = 0; i < things.textStringCount && shown < 10; i++) {
        struct DungeonTextString_Compat* ts = &things.textStrings[i];
        if (ts->next == 0xFFFFu) continue;
        unsigned char* raw = things.rawThingData[2] + i * 4;
        printf("  ts[%d]: next=0x%04X vis=%d offset=%d raw=[%02X %02X %02X %02X]\n",
               i, ts->next, ts->visible, ts->textDataWordOffset, raw[0], raw[1], raw[2], raw[3]);
        shown++;
    }}

    /* Check Teleporter targetMapIndex */
    printf("\n--- Teleporter targetMapIndex check (mapCount=%d) ---\n", state.header.mapCount);
    { int shown = 0;
    for (i = 0; i < things.teleporterCount; i++) {
        struct DungeonTeleporter_Compat* tp = &things.teleporters[i];
        if (tp->next == 0xFFFFu) continue;
        if (tp->targetMapIndex >= state.header.mapCount) {
            unsigned char* raw = things.rawThingData[1] + i * 6;
            printf("  BAD tp[%d]: next=0x%04X tgtMap=%d tgtX=%d tgtY=%d raw=[%02X %02X %02X %02X %02X %02X]\n",
                   i, tp->next, tp->targetMapIndex, tp->targetMapX, tp->targetMapY,
                   raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]);
            if (++shown > 5) break;
        }
    }}
    /* Show first 5 used teleporters */
    printf("\n--- First 5 used Teleporters ---\n");
    { int shown = 0;
    for (i = 0; i < things.teleporterCount && shown < 5; i++) {
        struct DungeonTeleporter_Compat* tp = &things.teleporters[i];
        if (tp->next == 0xFFFFu) continue;
        unsigned char* raw = things.rawThingData[1] + i * 6;
        printf("  tp[%d]: next=0x%04X tgtMap=%d tgtX=%d tgtY=%d rot=%d abs=%d scope=%d aud=%d raw=[%02X %02X %02X %02X %02X %02X]\n",
               i, tp->next, tp->targetMapIndex, tp->targetMapX, tp->targetMapY,
               tp->rotation, tp->absoluteRotation, tp->scope, tp->audible,
               raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]);
        shown++;
    }}

    /* Check Door next fields */
    printf("\n--- Door Next field check ---\n");
    { int bad = 0;
    for (i = 0; i < things.doorCount; i++) {
        unsigned short next = things.doors[i].next;
        if (next == 0xFFFFu || next == 0xFFFEu) continue;
        int type = (int)((next & 0x3C00u) >> 10);
        int idx = (int)(next & 0x03FFu);
        if (type < 0 || type >= 16 || idx >= things.thingCounts[type]) {
            unsigned char* raw = things.rawThingData[0] + i * 4;
            printf("  BAD door[%d]: next=0x%04X type=%d idx=%d (max=%d) raw=[%02X %02X %02X %02X]\n",
                   i, next, type, idx, things.thingCounts[type], raw[0], raw[1], raw[2], raw[3]);
            if (++bad > 5) break;
        }
    }}

    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&state);
    return 0;
}
