#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/memory_dungeon_dat_pc34_compat.h"
#include "include/fs_portable_compat.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dungeon.dat>\n", argv[0]);
        return 1;
    }

    const char* path = argv[1];
    struct DungeonDatState_Compat state;
    memset(&state, 0, sizeof(state));

    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "FAIL: Cannot open %s\n", path);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    fprintf(stderr, "FILE SIZE: %ld bytes\n", fileSize);

    unsigned short sig;
    fread(&sig, 2, 1, f);
    fprintf(stderr, "SIG: 0x%04x (compressed = 0x8104)\n", sig);
    if (sig == DUNGEON_COMPRESSED_SIGNATURE) {
        fprintf(stderr, "FAIL: Compressed signature detected, aborting\n");
        fclose(f);
        return 1;
    }

    unsigned short rawMapDataByteCount;
    unsigned char mapCount, unreferenced;
    unsigned short textDataWordCount, initialPartyLocation, squareFirstThingCount;
    
    fread(&rawMapDataByteCount, 2, 1, f);
    fread(&mapCount, 1, 1, f);
    fread(&unreferenced, 1, 1, f);
    fread(&textDataWordCount, 2, 1, f);
    fread(&initialPartyLocation, 2, 1, f);
    fread(&squareFirstThingCount, 2, 1, f);

    fprintf(stderr, "HEADER:\n");
    fprintf(stderr, "  ornamentRandomSeed:     0x%04x (%u)\n", sig, sig);
    fprintf(stderr, "  rawMapDataByteCount:    0x%04x (%u)\n", rawMapDataByteCount, rawMapDataByteCount);
    fprintf(stderr, "  mapCount:               %u\n", mapCount);
    fprintf(stderr, "  unreferenced:           %u\n", unreferenced);
    fprintf(stderr, "  textDataWordCount:      0x%04x (%u)\n", textDataWordCount, textDataWordCount);
    fprintf(stderr, "  initialPartyLocation:   0x%04x (%u)\n", initialPartyLocation, initialPartyLocation);
    fprintf(stderr, "  squareFirstThingCount:  0x%04x (%u)\n", squareFirstThingCount, squareFirstThingCount);

    unsigned short thingCounts[16];
    for (int i = 0; i < 16; i++) fread(&thingCounts[i], 2, 1, f);
    fprintf(stderr, "  thingCounts: ");
    for (int i = 0; i < 16; i++) fprintf(stderr, "[%d]=%u ", i, thingCounts[i]);
    fprintf(stderr, "\n");

    long thingDataTotalBytes = 0;
    for (int i = 0; i < 16; i++) thingDataTotalBytes += thingCounts[i] * s_thingDataByteCount[i];

    struct DungeonMapDesc_Compat maps[32];
    int totalColumns = 0;
    fprintf(stderr, "MAP DESCRIPTORS:\n");
    for (int i = 0; i < mapCount && i < 32; i++) {
        unsigned short rawMapDataByteOffset, aUnref, bUnref, rawBitA, rawB, rawC, rawD;
        unsigned char offsetX, offsetY;
        fread(&rawMapDataByteOffset, 2, 1, f);
        fread(&aUnref, 2, 1, f);
        fread(&bUnref, 2, 1, f);
        fread(&offsetX, 1, 1, f);
        fread(&offsetY, 1, 1, f);
        fread(&rawBitA, 2, 1, f);
        fread(&rawB, 2, 1, f);
        fread(&rawC, 2, 1, f);
        fread(&rawD, 2, 1, f);
        maps[i].rawMapDataByteOffset = rawMapDataByteOffset;
        maps[i].aUnreferenced = aUnref;
        maps[i].bUnreferenced = bUnref;
        maps[i].offsetMapX = offsetX;
        maps[i].offsetMapY = offsetY;
        maps[i].rawBitfieldB = rawB;
        maps[i].rawBitfieldC = rawC;
        maps[i].rawBitfieldD = rawD;
        maps[i].level = rawBitA & 0x3F;
        maps[i].width = ((rawBitA >> 6) & 0x1F) + 1;
        maps[i].height = ((rawBitA >> 11) & 0x1F) + 1;
        totalColumns += maps[i].width;
        fprintf(stderr, "  Map %d: off=0x%04x a=0x%04x b=0x%04x ofs=(%d,%d) bitA=0x%04x level=%d w=%d h=%d\n",
            i, rawMapDataByteOffset, aUnref, bUnref, offsetX, offsetY, rawBitA,
            maps[i].level, maps[i].width, maps[i].height);
    }
    fclose(f);

    long rawMapDataSectionOffset = DUNGEON_HEADER_SIZE +
        (long)mapCount * DUNGEON_MAP_DESC_SIZE +
        (long)totalColumns * 2 +
        (long)squareFirstThingCount * 2 +
        (long)textDataWordCount * 2 +
        thingDataTotalBytes;

    unsigned long expectedFileSize = 44 +
        (unsigned long)mapCount * 16 +
        (unsigned long)totalColumns * 2 +
        (unsigned long)squareFirstThingCount * 2 +
        (unsigned long)textDataWordCount * 2 +
        thingDataTotalBytes +
        rawMapDataByteCount + 2;

    fprintf(stderr, "\nOFFSET CALCULATION:\n");
    fprintf(stderr, "  Header:                    %ld\n", DUNGEON_HEADER_SIZE);
    fprintf(stderr, "  Map descriptors (%u x %d): %ld\n", mapCount, DUNGEON_MAP_DESC_SIZE,
            (long)mapCount * DUNGEON_MAP_DESC_SIZE);
    fprintf(stderr, "  Cumulative col table (%d cols): %ld\n", totalColumns, (long)totalColumns * 2);
    fprintf(stderr, "  SFT (%u x 2):               %ld\n", squareFirstThingCount,
            (long)squareFirstThingCount * 2);
    fprintf(stderr, "  Text (%u x 2):              %ld\n", textDataWordCount,
            (long)textDataWordCount * 2);
    fprintf(stderr, "  Thing data (total):        %ld\n", thingDataTotalBytes);
    fprintf(stderr, "  Raw map data (%u):         %u\n", rawMapDataByteCount, rawMapDataByteCount);
    fprintf(stderr, "  Checksum:                  2\n");
    fprintf(stderr, "  -> rawMapDataSectionOffset: %ld\n", rawMapDataSectionOffset);
    fprintf(stderr, "  -> fileSize:                %ld\n", fileSize);
    fprintf(stderr, "  -> bytes after raw start:   %ld\n",
            fileSize >= rawMapDataSectionOffset ? fileSize - rawMapDataSectionOffset : 0);
    fprintf(stderr, "\nEXPECTED FILE SIZE: %lu\n", expectedFileSize);
    fprintf(stderr, "ACTUAL FILE SIZE:   %ld\n", fileSize);
    fprintf(stderr, "DIFF: %ld (%s)\n",
            (long)expectedFileSize - fileSize,
            expectedFileSize == (unsigned long)fileSize ? "MATCH" : "MISMATCH!");

    fprintf(stderr, "\n=== F0500 ===\n");
    int rc0 = F0500_DUNGEON_LoadDatHeader_Compat(path, &state);
    fprintf(stderr, "F0500: %s\n", rc0 ? "OK" : "FAILED");
    if (!rc0) return 1;

    fprintf(stderr, "\n=== F0502 ===\n");
    int rc2 = F0502_DUNGEON_LoadTileData_Compat(path, &state);
    fprintf(stderr, "F0502: %s\n", rc2 ? "OK" : "FAILED");
    if (!rc2) {
        fprintf(stderr, "TILE DATA LOAD FAILED\n");
        fprintf(stderr, "Expected raw data start: %ld\n", rawMapDataSectionOffset);
        fprintf(stderr, "File has %ld bytes, raw data starts at %ld -> %ld bytes remaining\n",
                fileSize, rawMapDataSectionOffset, fileSize - rawMapDataSectionOffset);
    }

    F0500_DUNGEON_FreeDatHeader_Compat(&state);
    return 0;
}
