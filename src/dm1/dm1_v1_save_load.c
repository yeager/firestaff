/*
 * DM1 V1 Save/Load Game System — Implementation
 *
 * Source-locked to ReDMCSB (see dm1_v1_save_load.h for full references).
 *
 * Key ReDMCSB functions reimplemented:
 *   F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF (LOADSAVE.C)
 *     → DM1_SaveGame: serialize world, write header + body
 *   F0435_STARTEND_LoadGame (LOADSAVE.C)
 *     → DM1_LoadGame: read header, validate, deserialize world
 *   F0429_STARTEND_IsReadSaveHeaderSuccessful (SAVEHEAD.C)
 *     → header validation via CRC32 (replacing XOR obfuscation)
 *   F0430_STARTEND_IsWriteObfuscatedSaveHeaderSuccessful (SAVEHEAD.C)
 *     → header write with CRC32 (replacing noise/obfuscation)
 *   F0417_SAVEUTIL_GetChecksumAndObfuscate (READWRIT.C)
 *     → replaced by CRC32 over body bytes
 *
 * Original save file layout (ReDMCSB LOADSAVE.C F0433, lines ~735-790):
 *   [0..511]       DM_SAVE_HEADER (noise + obfuscated metadata)
 *   [512..]        Save part 0: GLOBAL_DATA (obfuscated)
 *   [..]           Save part 1: ACTIVE_GROUP array (obfuscated)
 *   [..]           Save part 2: PARTY_INFO + CHAMPION[4] (obfuscated)
 *   [..]           Save part 3: EVENT array (obfuscated)
 *   [..]           Save part 4: TIMELINE array (obfuscated)
 *   [..]           Champion portraits (4 × raw bitmap, PC/FM Towns)
 *   [..]           DUNGEON_HEADER (with running checksum)
 *   [..]           MAP array
 *   [..]           Column cumulative counts
 *   [..]           Square first things
 *   [..]           Text data
 *   [..]           Thing data (16 types)
 *   [..]           Raw map data
 *   [last 2]       Dungeon checksum (int16)
 *
 * Firestaff save file layout:
 *   [0..63]        DM1SaveHeader (magic + metadata + CRC32)
 *   [64..EOF)      GameWorld_Compat blob (F0897 serialization)
 */

#include "dm1_v1_save_load.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── LE read/write helpers ─────────────────────────────────────── */

static void dm1_write_u16_le(unsigned char* p, uint16_t v) {
    p[0] = (unsigned char)(v & 0xFF);
    p[1] = (unsigned char)((v >> 8) & 0xFF);
}

static void dm1_write_u32_le(unsigned char* p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xFF);
    p[1] = (unsigned char)((v >> 8) & 0xFF);
    p[2] = (unsigned char)((v >> 16) & 0xFF);
    p[3] = (unsigned char)((v >> 24) & 0xFF);
}

static uint16_t dm1_read_u16_le(const unsigned char* p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t dm1_read_u32_le(const unsigned char* p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

/* ── CRC32 (IEEE 802.3 reflected) ─────────────────────────────── */
/*
 * Same polynomial as memory_savegame_pc34_compat.c F0770.
 * Replaces the original XOR-key obfuscation from READWRIT.C F0417.
 */

uint32_t DM1_CRC32(const unsigned char* data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;
    int j;

    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320u;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

/* ── Header serialization ─────────────────────────────────────── */

static int dm1_serialize_header(const struct DM1SaveHeader* hdr,
                                unsigned char buf[DM1_SAVE_HEADER_SIZE]) {
    memset(buf, 0, DM1_SAVE_HEADER_SIZE);
    memcpy(buf, hdr->magic, 8);
    dm1_write_u32_le(buf + 8,  hdr->formatVersion);
    dm1_write_u32_le(buf + 12, hdr->totalFileSize);
    dm1_write_u32_le(buf + 16, hdr->bodyCRC32);
    dm1_write_u32_le(buf + 20, hdr->gameTick);
    dm1_write_u32_le(buf + 24, hdr->gameID);
    dm1_write_u16_le(buf + 28, hdr->partyMapX);
    dm1_write_u16_le(buf + 30, hdr->partyMapY);
    dm1_write_u16_le(buf + 32, hdr->partyDirection);
    dm1_write_u16_le(buf + 34, hdr->partyMapIndex);
    dm1_write_u16_le(buf + 36, hdr->championCount);
    buf[38] = hdr->saveAndPlay;
    buf[39] = hdr->formatID;
    buf[40] = (uint8_t)(hdr->musicOn ? 1 : 0);
    dm1_write_u32_le(buf + 41, hdr->bugProfileHash);
    /* bytes 45..63 remain zero (reserved) */
    return 1;
}

static int dm1_deserialize_header(const unsigned char buf[DM1_SAVE_HEADER_SIZE],
                                  struct DM1SaveHeader* hdr) {
    memcpy(hdr->magic, buf, 8);
    hdr->formatVersion = dm1_read_u32_le(buf + 8);
    hdr->totalFileSize = dm1_read_u32_le(buf + 12);
    hdr->bodyCRC32     = dm1_read_u32_le(buf + 16);
    hdr->gameTick      = dm1_read_u32_le(buf + 20);
    hdr->gameID        = dm1_read_u32_le(buf + 24);
    hdr->partyMapX     = dm1_read_u16_le(buf + 28);
    hdr->partyMapY     = dm1_read_u16_le(buf + 30);
    hdr->partyDirection = dm1_read_u16_le(buf + 32);
    hdr->partyMapIndex  = dm1_read_u16_le(buf + 34);
    hdr->championCount  = dm1_read_u16_le(buf + 36);
    hdr->saveAndPlay    = buf[38];
    hdr->formatID       = buf[39];
    hdr->musicOn        = buf[40] ? 1 : 0;
    hdr->bugProfileHash = dm1_read_u32_le(buf + 41);
    memcpy(hdr->reserved, buf + 45, 19);
    return 1;
}

/* ── Save game ────────────────────────────────────────────────── */

/*
 * DM1_SaveGame — Firestaff equivalent of F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF
 *
 * ReDMCSB flow (LOADSAVE.C F0433, DM1 Atari ST branch S10EA):
 *   1. Copy-protection check (skipped — no floppy)
 *   2. Show save dialog (SAVE AND PLAY / SAVE AND QUIT / FORMAT / CANCEL)
 *   3. Check disk type (skipped — no floppy)
 *   4. Delete backup, rename current to backup
 *   5. Create new save file
 *   6. Subtract leader hand object weight (CHANGE1_02_FIX)
 *   7. Fill GLOBAL_DATA from globals
 *   8. Set up 5 SAVE_PART entries (global, active groups, party, events, timeline)
 *   9. Generate random Keys[16], compute Checksums[16]
 *  10. Write obfuscated header (F0430)
 *  11. Write 5 obfuscated save parts (F0420)
 *  12. Write dungeon data with running checksum (F0422)
 *  13. Close file, restore leader hand weight
 *
 * Firestaff simplification:
 *   Steps 1-3 eliminated (no floppy disk).
 *   Steps 4-5: backup + create handled by caller or here.
 *   Steps 6-12: replaced by F0897_WORLD_Serialize_Compat (captures
 *   all game state including dungeon, party, timers, combat).
 *   Header uses CRC32 instead of XOR obfuscation.
 */
int DM1_SaveGame(const struct GameWorld_Compat* world,
                 const char* path,
                 uint32_t gameID,
                 int saveAndPlay,
                 int musicOn) {
    return DM1_SaveGameWithProfile(world, path, gameID, saveAndPlay, musicOn,
                                   DM1_DefaultSaveProfileHash());
}

int DM1_SaveGameWithProfile(const struct GameWorld_Compat* world,
                            const char* path,
                            uint32_t gameID,
                            int saveAndPlay,
                            int musicOn,
                            uint32_t bugProfileHash) {
    struct DM1SaveHeader hdr;
    unsigned char headerBuf[DM1_SAVE_HEADER_SIZE];
    unsigned char* blob = NULL;
    int blobSize, bytesWritten = 0;
    FILE* file = NULL;
    char backupPath[512];
    int rc;

    if (!world || !path) return DM1_SAVE_ERROR_NULL_ARG;

    /* Step 6-7: Compute serialized size */
    blobSize = F0899_WORLD_SerializedSize_Compat(world);
    if (blobSize <= 0) return DM1_SAVE_ERROR_SERIALIZE;

    blob = (unsigned char*)malloc((size_t)blobSize);
    if (!blob) return DM1_SAVE_ERROR_OUT_OF_MEMORY;

    /* Step 8-12: Serialize world (captures all save parts + dungeon) */
    if (!F0897_WORLD_Serialize_Compat(world, blob, blobSize, &bytesWritten) ||
        bytesWritten != blobSize) {
        free(blob);
        return DM1_SAVE_ERROR_SERIALIZE;
    }

    /* Step 4: Backup current save (like original Fdelete/Frename) */
    rc = snprintf(backupPath, sizeof(backupPath), "%s.bak", path);
    if (rc > 0 && rc < (int)sizeof(backupPath)) {
        remove(backupPath);
        rename(path, backupPath);
    }

    /* Build header (replaces F0430 obfuscated header write) */
    memset(&hdr, 0, sizeof(hdr));
    memcpy(hdr.magic, DM1_SAVE_MAGIC, 8);
    hdr.formatVersion = DM1_SAVE_FORMAT_VERSION;
    hdr.totalFileSize = (uint32_t)(DM1_SAVE_HEADER_SIZE + blobSize);
    hdr.bodyCRC32     = DM1_CRC32(blob, (size_t)blobSize);
    hdr.gameTick      = (uint32_t)world->gameTick;
    hdr.gameID        = gameID;
    hdr.partyMapX     = (uint16_t)world->party.mapX;
    hdr.partyMapY     = (uint16_t)world->party.mapY;
    hdr.partyDirection = (uint16_t)world->party.direction;
    hdr.partyMapIndex  = (uint16_t)world->partyMapIndex;
    hdr.championCount  = (uint16_t)world->party.championCount;
    hdr.saveAndPlay    = (uint8_t)(saveAndPlay ? 1 : 0);
    hdr.formatID       = 1; /* C1_FORMAT_DM_ATARI_ST */
    hdr.musicOn        = (uint8_t)(musicOn ? 1 : 0);
    hdr.bugProfileHash = bugProfileHash;

    dm1_serialize_header(&hdr, headerBuf);

    /* Step 5: Create file and write */
    file = fopen(path, "wb");
    if (!file) {
        free(blob);
        /* Restore backup on failure */
        if (rc > 0 && rc < (int)sizeof(backupPath)) {
            rename(backupPath, path);
        }
        return DM1_SAVE_ERROR_FILE_OPEN;
    }

    if (fwrite(headerBuf, 1, DM1_SAVE_HEADER_SIZE, file) != DM1_SAVE_HEADER_SIZE ||
        fwrite(blob, 1, (size_t)blobSize, file) != (size_t)blobSize) {
        fclose(file);
        free(blob);
        remove(path);
        if (rc > 0 && rc < (int)sizeof(backupPath)) {
            rename(backupPath, path);
        }
        return DM1_SAVE_ERROR_FILE_WRITE;
    }

    if (fclose(file) != 0) {
        free(blob);
        return DM1_SAVE_ERROR_FILE_WRITE;
    }

    free(blob);
    return DM1_SAVE_OK;
}

/* ── Load game ────────────────────────────────────────────────── */

/*
 * DM1_LoadGame — Firestaff equivalent of F0435_STARTEND_LoadGame
 *
 * ReDMCSB flow (LOADSAVE.C F0435):
 *   1. Show load dialog (LOAD SAVED GAME / CANCEL)
 *   2. Check disk type, open saved game file (try primary then backup)
 *   3. Read/validate header via F0429 (SAVEHEAD.C)
 *   4. Verify GameID match (for restart)
 *   5. Read/deobfuscate GLOBAL_DATA via F0419
 *   6. Restore game state from GLOBAL_DATA fields
 *   7. Initialize timeline + active groups
 *   8. Read active groups, party, events, timeline via F0419
 *   9. Read champion portraits (PC/FM Towns)
 *  10. Read dungeon via F0434_IsLoadDungeonSuccessful_CPSC
 *  11. Close file, set RestartGameAllowed
 */
int DM1_LoadGame(const char* path,
                 struct GameWorld_Compat* outWorld,
                 struct DM1SaveHeader* outHeader) {
    FILE* file = NULL;
    unsigned char headerBuf[DM1_SAVE_HEADER_SIZE];
    struct DM1SaveHeader hdr;
    long fileSize;
    int blobSize;
    unsigned char* blob = NULL;
    uint32_t computedCRC;

    if (!path || !outWorld) return DM1_SAVE_ERROR_NULL_ARG;

    /* Step 2: Open file */
    file = fopen(path, "rb");
    if (!file) return DM1_SAVE_ERROR_FILE_OPEN;

    /* Get file size */
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return DM1_SAVE_ERROR_FILE_READ;
    }
    fileSize = ftell(file);
    if (fileSize < DM1_SAVE_HEADER_SIZE ||
        fileSize > DM1_SAVE_MAX_FILE_SIZE) {
        fclose(file);
        return DM1_SAVE_ERROR_BAD_SIZE;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return DM1_SAVE_ERROR_FILE_READ;
    }

    /* Step 3: Read header (replaces F0429 read + checksum validation) */
    if (fread(headerBuf, 1, DM1_SAVE_HEADER_SIZE, file) != DM1_SAVE_HEADER_SIZE) {
        fclose(file);
        return DM1_SAVE_ERROR_FILE_READ;
    }

    dm1_deserialize_header(headerBuf, &hdr);

    /* Validate magic */
    if (memcmp(hdr.magic, DM1_SAVE_MAGIC, 8) != 0) {
        fclose(file);
        return DM1_SAVE_ERROR_BAD_MAGIC;
    }

    /* Validate version */
    if (hdr.formatVersion != DM1_SAVE_FORMAT_VERSION) {
        fclose(file);
        return DM1_SAVE_ERROR_BAD_VERSION;
    }

    /* Validate total size */
    if ((long)hdr.totalFileSize != fileSize) {
        fclose(file);
        return DM1_SAVE_ERROR_BAD_SIZE;
    }

    /* Step 5-10: Read body blob */
    blobSize = (int)(fileSize - DM1_SAVE_HEADER_SIZE);
    blob = (unsigned char*)malloc((size_t)blobSize);
    if (!blob) {
        fclose(file);
        return DM1_SAVE_ERROR_OUT_OF_MEMORY;
    }

    if (fread(blob, 1, (size_t)blobSize, file) != (size_t)blobSize) {
        free(blob);
        fclose(file);
        return DM1_SAVE_ERROR_FILE_READ;
    }
    fclose(file);

    /* Validate CRC32 (replaces F0417/F0418/F0419 checksum validation) */
    computedCRC = DM1_CRC32(blob, (size_t)blobSize);
    if (computedCRC != hdr.bodyCRC32) {
        free(blob);
        return DM1_SAVE_ERROR_BAD_CRC;
    }

    /* Deserialize world (replaces F0419 per-section + F0434 dungeon load) */
    memset(outWorld, 0, sizeof(*outWorld));
    if (!F0898_WORLD_Deserialize_Compat(outWorld, blob, blobSize, NULL)) {
        F0883_WORLD_Free_Compat(outWorld);
        free(blob);
        return DM1_SAVE_ERROR_DESERIALIZE;
    }

    free(blob);

    /* Copy header metadata to caller if requested */
    if (outHeader) {
        *outHeader = hdr;
    }

    return DM1_SAVE_OK;
}

/* ── Validate save file ───────────────────────────────────────── */

int DM1_ValidateSaveFile(const char* path,
                         struct DM1SaveHeader* outHeader) {
    FILE* file = NULL;
    unsigned char headerBuf[DM1_SAVE_HEADER_SIZE];
    struct DM1SaveHeader hdr;
    long fileSize;
    int blobSize;
    unsigned char* blob = NULL;
    uint32_t computedCRC;

    if (!path || !outHeader) return DM1_SAVE_ERROR_NULL_ARG;

    file = fopen(path, "rb");
    if (!file) return DM1_SAVE_ERROR_FILE_OPEN;

    if (fseek(file, 0L, SEEK_END) != 0) { fclose(file); return DM1_SAVE_ERROR_FILE_READ; }
    fileSize = ftell(file);
    if (fileSize < DM1_SAVE_HEADER_SIZE || fileSize > DM1_SAVE_MAX_FILE_SIZE) {
        fclose(file);
        return DM1_SAVE_ERROR_BAD_SIZE;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) { fclose(file); return DM1_SAVE_ERROR_FILE_READ; }

    if (fread(headerBuf, 1, DM1_SAVE_HEADER_SIZE, file) != DM1_SAVE_HEADER_SIZE) {
        fclose(file);
        return DM1_SAVE_ERROR_FILE_READ;
    }

    dm1_deserialize_header(headerBuf, &hdr);

    if (memcmp(hdr.magic, DM1_SAVE_MAGIC, 8) != 0) { fclose(file); return DM1_SAVE_ERROR_BAD_MAGIC; }
    if (hdr.formatVersion != DM1_SAVE_FORMAT_VERSION) { fclose(file); return DM1_SAVE_ERROR_BAD_VERSION; }
    if ((long)hdr.totalFileSize != fileSize) { fclose(file); return DM1_SAVE_ERROR_BAD_SIZE; }

    /* Read body for CRC check */
    blobSize = (int)(fileSize - DM1_SAVE_HEADER_SIZE);
    blob = (unsigned char*)malloc((size_t)blobSize);
    if (!blob) { fclose(file); return DM1_SAVE_ERROR_OUT_OF_MEMORY; }

    if (fread(blob, 1, (size_t)blobSize, file) != (size_t)blobSize) {
        free(blob); fclose(file); return DM1_SAVE_ERROR_FILE_READ;
    }
    fclose(file);

    computedCRC = DM1_CRC32(blob, (size_t)blobSize);
    free(blob);

    if (computedCRC != hdr.bodyCRC32) return DM1_SAVE_ERROR_BAD_CRC;

    *outHeader = hdr;
    return DM1_SAVE_OK;
}

uint32_t DM1_SaveProfileHashFromName(const char* profileName) {
    if (!profileName || profileName[0] == '\0') {
        return DM1_SAVE_PROFILE_UNSPECIFIED;
    }
    return DM1_CRC32((const unsigned char*)profileName, strlen(profileName));
}

uint32_t DM1_DefaultSaveProfileHash(void) {
    return DM1_SaveProfileHashFromName(DM1_SAVE_PROFILE_ID_PC34_BASELINE);
}

int DM1_SaveProfileMatches(const struct DM1SaveHeader* header,
                           uint32_t currentBugProfileHash) {
    if (!header) return 0;
    if (header->bugProfileHash == DM1_SAVE_PROFILE_UNSPECIFIED ||
        currentBugProfileHash == DM1_SAVE_PROFILE_UNSPECIFIED) {
        return 1;
    }
    return header->bugProfileHash == currentBugProfileHash;
}

/* ── Save path helper ─────────────────────────────────────────── */

int DM1_GetSavePath(const char* sourceId,
                    char* outPath, int outSize) {
    const char* dataDir;
    int rc;

    if (!sourceId || !outPath || outSize <= 0) return 0;

    dataDir = getenv("FIRESTAFF_DATA_DIR");
    if (!dataDir) dataDir = ".";

    rc = snprintf(outPath, (size_t)outSize,
                  "%s/firestaff-%s-dm1save.sav",
                  dataDir, sourceId);
    return (rc > 0 && rc < outSize) ? 1 : 0;
}

/* ── Error string ─────────────────────────────────────────────── */

const char* DM1_SaveLoadErrorString(int code) {
    switch (code) {
        case DM1_SAVE_OK:                     return "OK";
        case DM1_SAVE_ERROR_NULL_ARG:         return "NULL ARGUMENT";
        case DM1_SAVE_ERROR_BUFFER_TOO_SMALL: return "BUFFER TOO SMALL";
        case DM1_SAVE_ERROR_BAD_MAGIC:        return "BAD FILE MAGIC";
        case DM1_SAVE_ERROR_BAD_VERSION:      return "UNSUPPORTED VERSION";
        case DM1_SAVE_ERROR_BAD_SIZE:         return "INVALID FILE SIZE";
        case DM1_SAVE_ERROR_BAD_CRC:          return "CRC MISMATCH — SAVE DAMAGED";
        case DM1_SAVE_ERROR_FILE_OPEN:        return "CANNOT OPEN FILE";
        case DM1_SAVE_ERROR_FILE_READ:        return "FILE READ FAILED";
        case DM1_SAVE_ERROR_FILE_WRITE:       return "FILE WRITE FAILED";
        case DM1_SAVE_ERROR_SERIALIZE:        return "SERIALISE FAILED";
        case DM1_SAVE_ERROR_DESERIALIZE:      return "DESERIALISE FAILED";
        case DM1_SAVE_ERROR_OUT_OF_MEMORY:    return "OUT OF MEMORY";
        case DM1_SAVE_ERROR_INTERNAL:         return "INTERNAL ERROR";
        default:                              return "UNKNOWN ERROR";
    }
}

/* ── Save menu helpers ────────────────────────────────────────── */

void DM1_SaveMenu_Init(struct DM1SaveMenuContext* ctx) {
    if (!ctx) return;
    ctx->state = DM1_SAVE_MENU_CLOSED;
    ctx->selectedChoice = 0;
    ctx->resultCode = DM1_SAVE_OK;
    ctx->statusMessage[0] = '\0';
}

int DM1_SaveMenu_IsOpen(const struct DM1SaveMenuContext* ctx) {
    return ctx && ctx->state != DM1_SAVE_MENU_CLOSED;
}

void DM1_SaveMenu_Open(struct DM1SaveMenuContext* ctx) {
    if (!ctx) return;
    ctx->state = DM1_SAVE_MENU_OPEN;
    ctx->selectedChoice = 0;
    ctx->resultCode = DM1_SAVE_OK;
    snprintf(ctx->statusMessage, sizeof(ctx->statusMessage), "SAVE GAME?");
}

void DM1_SaveMenu_Close(struct DM1SaveMenuContext* ctx) {
    if (!ctx) return;
    ctx->state = DM1_SAVE_MENU_CLOSED;
}
