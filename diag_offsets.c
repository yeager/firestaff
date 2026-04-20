#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_dungeon_dat_pc34_compat.h"

int main(int argc, char* argv[]) {
    struct DungeonDatState_Compat state;
    int i;
    long offset;
    int totalColumns = 0;

    if (argc < 2) { fprintf(stderr, "Usage: %s <DUNGEON.DAT>\n", argv[0]); return 1; }
    if (!F0500_DUNGEON_LoadDatHeader_Compat(argv[1], &state)) { fprintf(stderr, "FAIL\n"); return 1; }

    for (i = 0; i < (int)state.header.mapCount; i++)
        totalColumns += state.maps[i].width;

    offset = DUNGEON_HEADER_SIZE + (long)state.header.mapCount * DUNGEON_MAP_DESC_SIZE;
    printf("After header+maps: offset=%ld\n", offset);
    printf("CumTable: %d entries = %d bytes\n", totalColumns + 1, (totalColumns + 1) * 2);
    offset += (long)(totalColumns + 1) * 2;
    printf("After cumtable: offset=%ld\n", offset);
    printf("SFT: %d entries = %d bytes\n", state.header.squareFirstThingCount, state.header.squareFirstThingCount * 2);
    offset += (long)state.header.squareFirstThingCount * 2;
    printf("After SFT: offset=%ld (= thing data start)\n", offset);

    for (i = 0; i < 16; i++) {
        int bytes = (int)state.header.thingCounts[i] * (int)s_thingDataByteCount[i];
        if (bytes > 0) {
            printf("  Type %2d (%s): %d things * %d bytes = %d bytes, offset=%ld\n",
                   i, s_thingTypeNames[i], state.header.thingCounts[i], s_thingDataByteCount[i], bytes, offset);
        }
        offset += bytes;
    }
    printf("After thing data: offset=%ld\n", offset);
    printf("RawMapData: %d bytes\n", state.header.rawMapDataByteCount);
    offset += state.header.rawMapDataByteCount;
    printf("After rawmapdata: offset=%ld\n", offset);
    printf("TextData: %d words = %d bytes\n", state.header.textDataWordCount, state.header.textDataWordCount * 2);
    offset += state.header.textDataWordCount * 2;
    printf("After textdata: offset=%ld\n", offset);
    printf("File size: %ld\n", state.fileSize);
    printf("Match: %s\n", offset == state.fileSize ? "YES" : "NO");

    /* Now let's verify SFT → Door linkage. Find a Door SFT entry, get its index, read door raw data. */
    FILE* f = fopen(argv[1], "rb");
    long sft_off = DUNGEON_HEADER_SIZE + (long)state.header.mapCount * DUNGEON_MAP_DESC_SIZE + (long)(totalColumns + 1) * 2;
    long door_data_off = sft_off + (long)state.header.squareFirstThingCount * 2;

    /* Read first few SFT entries and find door refs */
    fseek(f, sft_off, SEEK_SET);
    printf("\n--- SFT Door references ---\n");
    for (i = 0; i < state.header.squareFirstThingCount; i++) {
        unsigned char b[2];
        fread(b, 1, 2, f);
        unsigned short t = b[0] | ((unsigned short)b[1] << 8);
        if (t == 0xFFFF || t == 0xFFFE) continue;
        int type = (t >> 10) & 0xF;
        int idx = t & 0x3FF;
        if (type == 0 && idx < 5) { /* Door type, first few */
            printf("  SFT[%d] = 0x%04X → Door[%d]\n", i, t, idx);
            long saved = ftell(f);
            /* Read raw door data at door_data_off + idx*4 */
            fseek(f, door_data_off + idx * 4, SEEK_SET);
            unsigned char doo[4];
            fread(doo, 1, 4, f);
            unsigned short next = doo[0] | ((unsigned short)doo[1] << 8);
            printf("    Raw: [%02X %02X %02X %02X] Next=0x%04X type=%d idx=%d\n",
                   doo[0], doo[1], doo[2], doo[3], next, (next>>10)&0xF, next&0x3FF);
            fseek(f, saved, SEEK_SET);
        }
    }

    fclose(f);
    F0500_DUNGEON_FreeDatHeader_Compat(&state);
    return 0;
}
