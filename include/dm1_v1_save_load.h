#ifndef DM1_V1_SAVE_LOAD_H
#define DM1_V1_SAVE_LOAD_H

/*
 * DM1 V1 Save/Load Game System
 *
 * Source-locked to ReDMCSB:
 *   DEFS.H     lines 469-572  — DM_SAVE_HEADER, GLOBAL_DATA, SAVE_PART structs
 *   SAVEHEAD.C all            — F0429/F0430 header read/write with checksum + obfuscation
 *   READWRIT.C all            — F0415-F0422 byte I/O, obfuscation, checksums
 *   LOADSAVE.C F0433          — F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF
 *   LOADSAVE.C F0435          — F0435_STARTEND_LoadGame
 *
 * This module implements DM1 V1 save/load using the Firestaff-native
 * GameWorld_Compat serialization, wrapped in an authentic-style header
 * that captures the semantically meaningful fields from DM_SAVE_HEADER
 * and GLOBAL_DATA without the copy-protection noise/obfuscation.
 *
 * The QuickSave system (F5/F9) uses a simpler FSM11QS1 format.
 * This module provides the ESC-key / in-game menu save/load path
 * matching the original C140_COMMAND_SAVE_GAME flow.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Save file magic / format constants ────────────────────────── */

#define DM1_SAVE_MAGIC              "FSDM1SV1"   /* 8-byte file magic  */
#define DM1_SAVE_FORMAT_VERSION     1
#define DM1_SAVE_HEADER_SIZE        64
#define DM1_SAVE_MAX_FILE_SIZE      (2 << 20)     /* 2 MiB hard cap     */

/* ── Error codes ──────────────────────────────────────────────── */

enum DM1SaveLoadError {
    DM1_SAVE_OK                    = 0,
    DM1_SAVE_ERROR_NULL_ARG        = 1,
    DM1_SAVE_ERROR_BUFFER_TOO_SMALL = 2,
    DM1_SAVE_ERROR_BAD_MAGIC       = 3,
    DM1_SAVE_ERROR_BAD_VERSION     = 4,
    DM1_SAVE_ERROR_BAD_SIZE        = 5,
    DM1_SAVE_ERROR_BAD_CRC         = 6,
    DM1_SAVE_ERROR_FILE_OPEN       = 7,
    DM1_SAVE_ERROR_FILE_READ       = 8,
    DM1_SAVE_ERROR_FILE_WRITE      = 9,
    DM1_SAVE_ERROR_SERIALIZE       = 10,
    DM1_SAVE_ERROR_DESERIALIZE     = 11,
    DM1_SAVE_ERROR_OUT_OF_MEMORY   = 12,
    DM1_SAVE_ERROR_INTERNAL        = 99
};

/* ── Save file header ─────────────────────────────────────────── */

/*
 * Firestaff DM1 V1 save header (64 bytes, LE).
 *
 * ReDMCSB source ref: DEFS.H lines 469-480 (DM_SAVE_HEADER).
 * The original has 512 bytes with noise/obfuscation for copy protection.
 * We keep only the semantically meaningful fields and add CRC32 integrity.
 *
 * Layout:
 *   [0..7]    magic         "FSDM1SV1"
 *   [8..11]   formatVersion 1
 *   [12..15]  totalFileSize header + world blob
 *   [16..19]  bodyCRC32     CRC32 of bytes [64..EOF)
 *   [20..23]  gameTick      GameWorld.gameTick (= original GameTime)
 *   [24..27]  gameID        random ID for this game instance
 *   [28..29]  partyMapX     party position X
 *   [30..31]  partyMapY     party position Y
 *   [32..33]  partyDirection 0-3 N/E/S/W
 *   [34..35]  partyMapIndex  current dungeon level
 *   [36..37]  championCount  0-4 champions in party
 *   [38..38]  saveAndPlay    0=save-and-quit, 1=save-and-play
 *   [39..39]  formatID       C1_FORMAT_DM_ATARI_ST = 1
 *   [40..40]  musicOn        GLOBAL_DATA.MusicOn / G2024_B_PendingMusicOn
 *   [41..63]  reserved       zero-filled
 */
struct DM1SaveHeader {
    unsigned char magic[8];
    uint32_t formatVersion;
    uint32_t totalFileSize;
    uint32_t bodyCRC32;
    uint32_t gameTick;
    uint32_t gameID;
    uint16_t partyMapX;
    uint16_t partyMapY;
    uint16_t partyDirection;
    uint16_t partyMapIndex;
    uint16_t championCount;
    uint8_t  saveAndPlay;
    uint8_t  formatID;
    uint8_t  musicOn;
    unsigned char reserved[23];
};

/* ── Save menu state ──────────────────────────────────────────── */

/*
 * ReDMCSB source ref: LOADSAVE.C F0433
 * Original menu: SAVE AND PLAY / SAVE AND QUIT / FORMAT FLOPPY / CANCEL
 * Firestaff: SAVE / CANCEL (no floppy concepts)
 */
enum DM1SaveMenuState {
    DM1_SAVE_MENU_CLOSED   = 0,
    DM1_SAVE_MENU_OPEN     = 1,
    DM1_SAVE_MENU_SAVING   = 2,
    DM1_SAVE_MENU_SUCCESS  = 3,
    DM1_SAVE_MENU_FAILED   = 4
};

struct DM1SaveMenuContext {
    int state;
    int selectedChoice;  /* 0=SAVE, 1=CANCEL */
    int resultCode;
    char statusMessage[128];
};

/* ── Core save/load functions ─────────────────────────────────── */

/*
 * Save current game world to file.
 *
 * ReDMCSB source ref: LOADSAVE.C F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF
 *   - Fills GLOBAL_DATA from globals (line ~700-730)
 *   - Sets up 5 SAVE_PART sections (line ~735-755)
 *   - Generates random Keys[16], computes Checksums (line ~760-775)
 *   - Writes obfuscated header via F0430 (SAVEHEAD.C)
 *   - Writes obfuscated save parts via F0420 (READWRIT.C)
 *   - Writes dungeon data with running checksum via F0422 (READWRIT.C)
 *
 * Firestaff implementation: serializes GameWorld_Compat via F0897,
 * wraps in DM1SaveHeader with CRC32 integrity.
 */
struct GameWorld_Compat;
int DM1_SaveGame(const struct GameWorld_Compat* world,
                 const char* path,
                 uint32_t gameID,
                 int saveAndPlay,
                 int musicOn);

/*
 * Load game world from file.
 *
 * ReDMCSB source ref: LOADSAVE.C F0435_STARTEND_LoadGame
 *   - Reads/validates header via F0429 (SAVEHEAD.C)
 *   - Reads/deobfuscates GLOBAL_DATA via F0419 (READWRIT.C)
 *   - Restores game state from GLOBAL_DATA fields (line ~1400-1430)
 *   - Reads party/champions/events/timeline via F0419
 *   - Reads dungeon via F0434_IsLoadDungeonSuccessful_CPSC
 */
int DM1_LoadGame(const char* path,
                 struct GameWorld_Compat* outWorld,
                 struct DM1SaveHeader* outHeader);

/*
 * Validate a save file without loading it.
 * Reads header, checks magic/version/CRC. Does not deserialize world.
 */
int DM1_ValidateSaveFile(const char* path,
                         struct DM1SaveHeader* outHeader);

/* ── CRC32 helper ─────────────────────────────────────────────── */

uint32_t DM1_CRC32(const unsigned char* data, size_t len);

/* ── Save path helper ─────────────────────────────────────────── */

int DM1_GetSavePath(const char* sourceId,
                    char* outPath, int outSize);

/* ── Error string ─────────────────────────────────────────────── */

const char* DM1_SaveLoadErrorString(int code);

/* ── Save menu helpers ────────────────────────────────────────── */

void DM1_SaveMenu_Init(struct DM1SaveMenuContext* ctx);
int  DM1_SaveMenu_IsOpen(const struct DM1SaveMenuContext* ctx);
void DM1_SaveMenu_Open(struct DM1SaveMenuContext* ctx);
void DM1_SaveMenu_Close(struct DM1SaveMenuContext* ctx);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_SAVE_LOAD_H */
