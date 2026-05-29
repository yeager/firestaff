#ifndef FIRESTAFF_CSB_V1_UTILITY_IMPORT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_UTILITY_IMPORT_PC34_COMPAT_H

/*
 * CSB V1 Phase 6 — Utility/Import Flow
 *
 * CSB champion import from DM1 saves.
 * CSB uses 256-byte champion blocks (vs DM1's 116-byte records).
 *
 * Import pipeline:
 *   DM1 .SAV file → CSB champion block → CSB party slot
 *
 * The DM1→CSB import reads a DM1 save file (4 champions, 116 bytes each)
 * and converts each champion into a CSB champion block (256 bytes).
 * CSB save format stores champions as 256-byte aligned blocks.
 *
 * Source references:
 *   CSBWin/SaveGame.cpp — DM1 import path (F0100-F0120 equivalent)
 *   ReDMCSB SAVEGAME.C — F0100-F0120 champion import state machine
 *   ReDMCSB CHAMPION.C — champion block layout
 *   ReDMCSB CEDTINC7.C — utility disk prompt strings
 *   ReDMCSB CEDTDATA.C — G3921 PLEASE_INSERT_UTILITY_DISK
 *
 * Champion block size:
 *   CSB V1: 256 bytes per champion block
 *   DM1:    116 bytes per champion record
 *
 * The 256-byte CSB block contains:
 *   - 8 bytes: champion name (fixed-width, space-padded)
 *   - 188 bytes: champion data (stats, skills, vitals)
 *   - 60 bytes: equipment slots (30 × uint16_t)
 *
 * State machine (ReDMCSB F0100-F0120):
 *   State 0: Check DM1 save header magic
 *   State 1: Validate champion count (1-4)
 *   State 2: Read each DM1 champion record
 *   State 3: Convert to CSB 256-byte block
 *   State 4: Verify checksum of converted block
 *   State 5: Store in party Champions[]
 */

#include <stdint.h>

/* CSB_V1_PartyState and CSB_V1_MAX_CHAMPIONS are defined in the
 * character compat header. Include it to resolve the forward reference. */
#include "csb_v1_character_pc34_compat.h"

/* ── CSB champion block size ─────────────────────────────────────────── */
/* CSB V1 uses 256-byte champion blocks (vs DM1's 116-byte records).
 * This is verified against CSBWin/SaveGame.cpp and ReDMCSB SAVEGAME.C. */
#define CSB_V1_CHAMPION_BLOCK_SIZE    256
#define CSB_V1_CHAMPION_BLOCK_COUNT   4       /* max champions per save */
#define CSB_V1_CHAMPION_BLOCK_TOTAL   1024    /* 4 × 256 bytes */

/* ── DM1 save champion record size ───────────────────────────────────── */
#define DM1_CHAMPION_RECORD_SIZE      116
#define DM1_SAVE_HEADER_SIZE           24

/* ── Import state machine states ─────────────────────────────────────── */
typedef enum {
    CSB_V1_IMPORT_STATE_INIT           = 0,
    CSB_V1_IMPORT_STATE_CHECK_HEADER   = 1,
    CSB_V1_IMPORT_STATE_VALIDATE_COUNT = 2,
    CSB_V1_IMPORT_STATE_READ_CHAMPS    = 3,
    CSB_V1_IMPORT_STATE_CONVERT_BLOCKS = 4,
    CSB_V1_IMPORT_STATE_VERIFY_CHECKSUM = 5,
    CSB_V1_IMPORT_STATE_STORE_PARTY    = 6,
    CSB_V1_IMPORT_STATE_DONE           = 7,
    CSB_V1_IMPORT_STATE_ERROR          = 8
} CSB_V1_ImportState;

/* ── Import error codes ─────────────────────────────────────────────── */
#define CSB_V1_IMPORT_OK                0
#define CSB_V1_IMPORT_ERR_NULL          -1   /* null pointer */
#define CSB_V1_IMPORT_ERR_SIZE          -2   /* buffer too small */
#define CSB_V1_IMPORT_ERR_MAGIC         -3   /* not a DM1 save */
#define CSB_V1_IMPORT_ERR_COUNT         -4   /* invalid champion count */
#define CSB_V1_IMPORT_ERR_PARTIAL       -5   /* truncated record */
#define CSB_V1_IMPORT_ERR_BLOCK_ALIGN   -6   /* block size mismatch */
#define CSB_V1_IMPORT_ERR_CHECKSUM      -7   /* block checksum failed */
#define CSB_V1_IMPORT_ERR_NO_DISK       -8   /* disk unreadable */
#define CSB_V1_IMPORT_ERR_WRONG_DISK    -9   /* not the CSB Utility Disk */

/* ── Import result structure ─────────────────────────────────────────── */
typedef struct {
    int              state;          /* current import state */
    int              champion_count; /* number imported (0-4) */
    int              error_code;     /* CSB_V1_IMPORT_ERR_* */
    int              byte_offset;    /* byte offset of error if any */
    uint32_t         dm1_seed;       /* dungeon seed from DM1 save */
    uint32_t         dm1_game_time;  /* game time from DM1 save */
    uint8_t          import_flags;   /* internal flags */
} CSB_V1_ImportResult;

/* ── Champion block (256 bytes) ─────────────────────────────────────── */
/* CSB stores champions as 256-byte aligned blocks.
 * This is different from DM1's 116-byte records.
 * Source: CSBWin/SaveGame.cpp, ReDMCSB SAVEGAME.C F0100-F0120 */
typedef struct __attribute__((packed)) {
    /* ── Bytes 0-7: Champion name (fixed 8-char, space-padded) ── */
    char     Name[8];

    /* ── Bytes 8-195: Champion data (188 bytes) ── */
    /* Vitals */
    int16_t  CurrentHealth;
    int16_t  MaximumHealth;
    int16_t  CurrentStamina;
    int16_t  MaximumStamina;
    int16_t  CurrentMana;
    int16_t  MaximumMana;

    /* ── Statistics (7 stats × 3 values × 2 bytes = 42 bytes) ── */
    uint16_t Statistics[7][3];  /* STR, DEX, WIS, VIT, ANTIMAGIC, ANTIFIRE, LUCK */

    /* ── Skills (16 bytes) ── */
    uint8_t  Skills[16];

    /* ── Orientation (4 bytes) ── */
    uint8_t  Cell;                  /* view cell 0-3 */
    uint8_t  Direction;             /* facing 0-3 */
    uint16_t DirectionMaxDamage;    /* facing max damage */

    /* ── Status (12 bytes) ── */
    uint8_t  ActionIndex;
    int16_t  EnableActionEvent;
    int16_t  HideDamageEvent;
    uint16_t Attributes;            /* DEAD, NEEDS_RENAME, etc. */
    int16_t  Food;
    int16_t  Water;
    uint16_t Load;

    /* ── Bytes 192-255: Equipment slots (64 bytes = 30 × 2 + padding) ── */
    uint16_t Slots[30];              /* 30 equipment slots */
    uint8_t  Reserved[4];           /* padding to 256 bytes */
} CSB_V1_ChampionBlock;

/* ── Import from DM1 save file ───────────────────────────────────────── */
/* csb_v1_import_from_dm1_save_file:
 *   Imports champions from a DM1 .SAV file into a CSB party.
 *   Validates the DM1 save header and converts each 116-byte champion
 *   record into a 256-byte CSB champion block.
 *
 *   The import flow (ReDMCSB SAVEGAME.C F0100-F0120):
 *     1. Read 24-byte DM1 save header
 *     2. Validate magic and champion count
 *     3. Read each 116-byte champion record
 *     4. Convert to 256-byte CSB block
 *     5. Store in party Champions[]
 *
 *   Returns: number of champions imported (0-4), or negative on error.
 *   On error, result->error_code is set.
 */
int csb_v1_import_from_dm1_save_file(CSB_V1_PartyState *party,
                                      const char *dm1_save_path,
                                      CSB_V1_ImportResult *result);

/* ── Import from DM1 save buffer ─────────────────────────────────────── */
/* Same as csb_v1_import_from_dm1_save_file but reads from memory.
 * Useful for testing and for UI-based import (file picker → memory). */
int csb_v1_import_from_dm1_save_buffer(CSB_V1_PartyState *party,
                                       const uint8_t *dm1_buf,
                                       int buf_size,
                                       CSB_V1_ImportResult *result);

/* ── Champion block utilities ─────────────────────────────────────────── */
/* Verify that a 256-byte block is a valid CSB champion block.
 * Checks alignment, magic bytes, and checksum.
 *
 * Returns: 0 = valid, negative = invalid */
int csb_v1_champion_block_verify(const CSB_V1_ChampionBlock *block);

/* Get the champion block size.
 * Returns CSB_V1_CHAMPION_BLOCK_SIZE (256). */
int csb_v1_champion_block_size(void);

/* Copy a champion block into a party slot.
 * Returns: 0 on success, negative on error. */
int csb_v1_champion_block_to_party(CSB_V1_PartyState *party,
                                     int slot_index,
                                     const CSB_V1_ChampionBlock *block);

/* Convert a DM1 champion record (116 bytes) to a CSB champion block (256 bytes).
 * Returns: 0 on success, negative on error. */
int csb_v1_dm1_record_to_csb_block(const uint8_t *dm1_record,
                                    CSB_V1_ChampionBlock *csb_block);

/* ── Source evidence ──────────────────────────────────────────────────── */
const char *csb_v1_utility_import_source_evidence(void);

#endif /* FIRESTAFF_CSB_V1_UTILITY_IMPORT_PC34_COMPAT_H */