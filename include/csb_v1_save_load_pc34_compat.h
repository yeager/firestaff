#ifndef FIRESTAFF_CSB_V1_SAVE_LOAD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_SAVE_LOAD_PC34_COMPAT_H

#include <stdint.h>

/* CSB V1 Save/Load System
 *
 * CSB saves use a 512-byte header with obfuscation and checksum:
 *   - Bytes 0-255:   raw header data (game info, party, checksum key)
 *   - Bytes 256-511: obfuscated/encrypted data (random pad + checksum)
 *
 * The obfuscation uses a per-save random seed stored in the header.
 * Checksum is computed over the second 256-byte block.
 *
 * DM1 saves use a different header key (DM_SAVE_HEADER_DECRYPTION_KEY_INDEX).
 * CSB saves use CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX.
 *
 * Save disk structure (from LOADSAVE.C):
 *   - Sector 0:  boot sector (unused for save)
 *   - Sector 1:  SAVE_HEADER (512 bytes)
 *   - Sectors 2+: PARTY_DATA, DUNGEON_DATA, EVENT_DATA
 *
 * Source: CSBWin/SaveGame.cpp (2953 lines)
 * Source: ReDMCSB LOADSAVE.C F0435/F0433
 * Source: ReDMCSB SAVEHEAD.C F0429/F0430
 */

/* ── Save header constants ─────────────────────────────────────────────── */
#define CSB_V1_SAVE_HEADER_SIZE        512
#define CSB_V1_SAVE_MAX_PATH            256

/* Header key indices (for deobfuscation) */
#define CSB_V1_DM_SAVE_KEY_INDEX       10   /* C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX */
#define CSB_V1_CSB_SAVE_KEY_INDEX      29   /* C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX */

/* Save format identifiers */
#define CSB_V1_SAVE_MAGIC_DM           0x444D0001u   /* 'DM\0\1' Dungeon Master */
#define CSB_V1_SAVE_MAGIC_CSB          0x43534201u   /* 'CSB\1' Chaos Strikes Back */
#define CSB_V1_SAVE_MAGIC_CEDT         0x43454454u   /* 'CEDT' champion export format */

/* Disk type values (from REQDISK.C / F0452 return codes) */
#define CSB_V1_DISK_TYPE_NONE            0
#define CSB_V1_DISK_TYPE_GAME_DISK       1
#define CSB_V1_DISK_TYPE_SAVE_DISK       2
#define CSB_V1_DISK_TYPE_UTILITY_DISK    3
#define CSB_V1_DISK_TYPE_SAVE_PROTECTED  4   /* write-protected save disk */
#define CSB_V1_DISK_TYPE_UNREADABLE      5
#define CSB_V1_DISK_TYPE_UNKNOWN         6

/* Save game return codes */
#define CSB_V1_SAVE_OK                   0
#define CSB_V1_SAVE_ERR_NO_DISK         -1
#define CSB_V1_SAVE_ERR_WRITE_PROTECTED  -2
#define CSB_V1_SAVE_ERR_UNREADABLE       -3
#define CSB_V1_SAVE_ERR_DIFFERENT_GAME   -4
#define CSB_V1_SAVE_ERR_DAMAGED          -5
#define CSB_V1_SAVE_ERR_CANT_CREATE       -6

/* Load game return codes */
#define CSB_V1_LOAD_OK                   0
#define CSB_V1_LOAD_ERR_NO_DISK         -1
#define CSB_V1_LOAD_ERR_NOT_FOUND       -2
#define CSB_V1_LOAD_ERR_UNREADABLE       -3
#define CSB_V1_LOAD_ERR_DIFFERENT_GAME   -4
#define CSB_V1_LOAD_ERR_DAMAGED         -5
#define CSB_V1_LOAD_ERR_NO_BACKUP       -6

/* ── Save header structure (512 bytes) ─────────────────────────────── */
/* Aligned to ReDMCSB SAVE_HEADER layout.
 * Offset  0-255:  header data (plain after deobfuscation)
 * Offset 256-511: obfuscated data (random + checksum) */
typedef struct __attribute__((packed)) {
    /* ── Offset 0-31: Game identification (plain) ── */
    uint32_t Magic;             /*  0  CSB_V1_SAVE_MAGIC_CSB or CSB_V1_SAVE_MAGIC_DM */
    uint16_t HeaderVersion;     /*  4  save format version */
    uint16_t GameID;           /*  6  game serial number (matches dungeon.dat) */
    uint32_t DungeonSeed;      /*  8  dungeon random seed */
    int16_t  PartyMapX;        /* 12  party X position */
    int16_t  PartyMapY;        /* 14  party Y position */
    int16_t  PartyMapZ;        /* 16  party Z/floor */
    int16_t  PartyDirection;    /* 18  party facing (0-3) */
    uint16_t ChampionCount;     /* 20  number of champions in party */
    uint16_t GameTimeLow;      /* 22  game time (lower 16 bits) */
    uint32_t GameTimeHigh;     /* 24  game time (upper 16 bits) */
    uint32_t PlayTimeMs;       /* 28  total play time in milliseconds */

    /* ── Offset 32-127: Champion summary (28 bytes each = 4×28=112) ── */
    /* Stored as compact champion summary for header display.
     * Full champion data is in the PARTY_DATA section after the header. */
    uint8_t  ChampionSummaries[112];  /* 32  4 × (8 name + 8 stats + 4 misc + 8 spare) */

    /* ── Offset 128-255: Reserved / padding ── */
    uint8_t  Reserved1[112];   /* 144  padding to reach offset 256 */

    /* ── Offset 256-511: Obfuscated data block (256 bytes) ──
     * XOR-obfuscated with DecryptionKey per F0417_SAVEUTIL_GetChecksumAndObfuscate.
     * After deobfuscation this block is:
     *   uint16_t[0-127]: obfuscated checksum words
     * The last word (index 127) is the checksum XOR'd with the key.
     * Layout (plain after deobfuscation):
     *   [256]  uint16_t checksum_word_0
     *   [258]  uint16_t checksum_word_1
     *   ...
     *   [510]  uint16_t checksum_word_127 (last = sum^key)
     * Total: 256 bytes exactly.
     */
    uint16_t ObfuscatedBlock[128];  /* 256  XOR'd with DecryptionKey, 256 bytes */
} CSB_V1_SaveHeader;

/* Check that the header is exactly 512 bytes */
_Static_assert(sizeof(CSB_V1_SaveHeader) == 512,
               "CSB_V1_SaveHeader must be exactly 512 bytes");

/* ── Utility disk strings (from CEDTINC7.C / CEDTDATA.C) ────────────── */
#define CSB_V1_UTIL_MSG_PUT_DISK \
    "PLEASE PUT THE\nCHAOS STRIKES BACK\nUTILITY DISK IN"
#define CSB_V1_UTIL_MSG_WRONG_DISK \
    "THAT'S NOT THE\nCHAOS STRIKES BACK\nUTILITY DISK!"
#define CSB_V1_UTIL_MSG_CORRECT_DISK \
    "THAT'S THE\nCHAOS STRIKES BACK\nUTILITY DISK!"
#define CSB_V1_SAVE_MSG_PUT_DISK \
    "PUT THE GAME SAVE DISK IN"
#define CSB_V1_SAVE_MSG_NO_DISK \
    "THERE IS NO DISK IN"
#define CSB_V1_SAVE_MSG_WRITE_PROTECTED \
    "THAT DISK IS WRITE-PROTECTED!"
#define CSB_V1_SAVE_MSG_CANT_SAVE \
    "UNABLE TO SAVE GAME!"
#define CSB_V1_SAVE_MSG_CANT_LOAD \
    "CAN'T FIND SAVED GAME!"
#define CSB_V1_SAVE_MSG_DAMAGED \
    "THE GAME IS DAMAGED!"
#define CSB_V1_SAVE_MSG_DIFFERENT \
    "THAT'S NOT THE SAME GAME"

/* ── Save path management ─────────────────────────────────────────────── */
/* Returns the default save directory for CSB saves.
 * On Windows: %APPDATA%/Firestaff/csb/saves/
 * On macOS:  ~/Library/Application Support/Firestaff/csb/saves/
 * On Linux:  ~/.local/share/firestaff/csb/saves/ */
const char *csb_v1_save_get_default_save_dir(void);
const char *csb_v1_save_get_default_save_path(int slot);
const char *csb_v1_save_get_backup_path(const char *path);

/* ── Save game ─────────────────────────────────────────────────────── */
int csb_v1_save_game(const char *path,
                      const void *state, int state_size,
                      const CSB_V1_SaveHeader *header);
int csb_v1_save_game_auto(int slot,
                            const void *state, int state_size,
                            const CSB_V1_SaveHeader *header);

/* ── Load game ──────────────────────────────────────────────────────── */
int csb_v1_load_game(const char *path,
                      void *state, int max_size,
                      CSB_V1_SaveHeader *out_header);
int csb_v1_load_game_auto(int slot,
                            void *state, int max_size,
                            CSB_V1_SaveHeader *out_header);

/* ── Header operations ────────────────────────────────────────────────── */
/* Build an obfuscated save header from game state.
 * F0430_STARTEND_IsWriteObfuscatedSaveHeaderSuccessful */
int csb_v1_save_header_build(CSB_V1_SaveHeader *hdr,
                               uint32_t magic,
                               uint16_t game_id,
                               uint32_t dungeon_seed,
                               int party_x, int party_y, int party_z,
                               int party_dir,
                               int champ_count,
                               uint32_t game_time,
                               uint32_t play_time_ms);

/* Read and deobfuscate a save header.
 * F0429_STARTEND_IsReadSaveHeaderSuccessful */
int csb_v1_save_header_read(CSB_V1_SaveHeader *hdr,
                              const uint8_t *raw_512);

/* Compute the expected checksum over a raw 512-byte header */
uint16_t csb_v1_save_header_compute_checksum(const uint8_t *raw_512);

/* Verify the checksum of a save header.
 * Returns 0 if valid, -1 if corrupted. */
int csb_v1_save_header_verify(const CSB_V1_SaveHeader *hdr,
                                const uint8_t *raw_512);

/* Get the decryption key index from a save header magic */
int csb_v1_save_header_get_key_index(uint32_t magic);

/* ── Checksum / obfuscation ──────────────────────────────────────────── */
/* ReDMCSB F0417_SAVEUTIL_GetChecksumAndObfuscate:
 *   XOR-obfuscates 128 uint16_t words using a key derived from key_index.
 *   Then computes a checksum as sum of all uint16_t words. */
void csb_v1_save_obfuscate(uint16_t *data, int word_count,
                             uint16_t key_index);
uint16_t csb_v1_save_checksum(const uint16_t *data, int word_count);

/* ── Backup / safety ──────────────────────────────────────────────────── */
int csb_v1_save_backup(const char *path);
int csb_v1_save_restore_backup(const char *path);

/* ── Disk verification ────────────────────────────────────────────────── */
/* Check what type of disk is in drive_path.
 * Returns CSB_V1_DISK_TYPE_* */
int csb_v1_save_disk_type(const char *drive_path);

/* Check if the save game at path matches the current dungeon.
 * Verifies Magic and GameID match expected values. */
int csb_v1_save_verify_compatible(const char *path,
                                    uint32_t expected_magic,
                                    uint16_t expected_game_id);

/* ── Source evidence ──────────────────────────────────────────────────── */
const char *csb_v1_save_source_evidence(void);

#endif /* FIRESTAFF_CSB_V1_SAVE_LOAD_PC34_COMPAT_H */
