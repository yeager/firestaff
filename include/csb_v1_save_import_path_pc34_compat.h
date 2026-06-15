/*
 * csb_v1_save_import_path_pc34_compat.h
 *
 * CSB V1 Champion Transfer/Import (Champions GAP 3, HoC
 * delta).  Source-locked per ReDMCSB CHARACTER.C / CHAMPION.C
 * ReadingChampion()/WritingChampion(), DEFS.H:1289
 * (CSBGAME.DAT magic), CEDT006.C:101-118 (save-section
 * dispatch), and Character.cpp:14 (reincarnation globals).
 *
 * v2 (2026-06-16, Champions GAP 3): real CSB v2.0 + v2.1
 * save importer.  The v1 detector is retained; the loader
 * now parses the CSB roster format and maps each champion
 * into Firestaff's CSB_V1_PartyState / CSB_V1_Champion
 * structures, applies the CSB-only reincarnation stat-cap
 * penalty (CHANGE7_24) to reincarnated champions, and
 * stamps the party as "imported from CSB save" so a
 * re-edit will not re-import.
 *
 * The CSB save container Firestaff reads is:
 *   Header (256 bytes):
 *     [0..7]   magic "CSBGAME\0"
 *     [8..11]  version uint32 LE (0x200 = v2.0, 0x201 = v2.1)
 *     [12]     champion count (1..4)
 *     [13..15] reserved
 *     [16..19] game id uint32 LE
 *     [20..255] reserved
 *   Then `count` champion records (CSB_SAVE_CHAMP_SIZE each).
 */
#ifndef REDMCSB_CSB_V1_SAVE_IMPORT_PATH_PC34_COMPAT_H
#define REDMCSB_CSB_V1_SAVE_IMPORT_PATH_PC34_COMPAT_H

#include "csb_v1_character_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Save-file variant detection.  The first 8 bytes of the
 * DM1 / CSB save file are the magic field.  DM1 PC 3.4 saves
 * use "RDMCSB15"; CSB v2.x saves use "CSBGAME\0" followed by
 * a 32-bit version word. */
typedef enum {
    CSB_V1_SAVE_VARIANT_DM1_PC34 = 0,
    CSB_V1_SAVE_VARIANT_CSB_V20 = 1,
    CSB_V1_SAVE_VARIANT_CSB_V21 = 2,
    CSB_V1_SAVE_VARIANT_UNKNOWN = -1
} CSB_V1_SaveVariant;

/* Detect the save-file variant from the first bytes of the
 * file.  Returns CSB_V1_SAVE_VARIANT_UNKNOWN for unrecognized
 * magic / too-short header. */
CSB_V1_SaveVariant csb_v1_detect_save_variant(
    const unsigned char* header, int headerLen);

/* Champions GAP 3 is now implemented for the CSB v2.0/v2.1
 * roster format.  Returns 1. */
int  csb_v1_save_import_path_implemented(void);

/* ── CSB save container layout ────────────────────────────── */

#define CSB_SAVE_MAGIC_LEN        8
#define CSB_SAVE_HEADER_SIZE    256
#define CSB_SAVE_VERSION_V20  0x00000200u
#define CSB_SAVE_VERSION_V21  0x00000201u

#define CSB_SAVE_HDR_OFF_MAGIC        0   /* "CSBGAME\0" */
#define CSB_SAVE_HDR_OFF_VERSION      8   /* uint32 LE */
#define CSB_SAVE_HDR_OFF_CHAMP_COUNT 12   /* uint8 1..4 */
#define CSB_SAVE_HDR_OFF_GAME_ID     16   /* uint32 LE */

/* CSB champion record (160 bytes).  Source-faithful field
 * order per CHARACTER.C ReadingChampion(): identity, the
 * CSB reincarnation flag (HoC delta), vitals, the 7-stat
 * current/max rows, skills, and the 30 equipped slots. */
#define CSB_SAVE_CHAMP_SIZE         160
#define CSB_SAVE_CH_OFF_NAME          0   /* 16 bytes, NUL-padded */
#define CSB_SAVE_CH_OFF_REINCARNATED 16   /* uint8: 1 = reincarnated */
#define CSB_SAVE_CH_OFF_DEAD         17   /* uint8: 1 = dead */
#define CSB_SAVE_CH_OFF_CUR_HP       20   /* int16 LE */
#define CSB_SAVE_CH_OFF_MAX_HP       22
#define CSB_SAVE_CH_OFF_CUR_STA      24
#define CSB_SAVE_CH_OFF_MAX_STA      26
#define CSB_SAVE_CH_OFF_CUR_MANA     28
#define CSB_SAVE_CH_OFF_MAX_MANA     30
#define CSB_SAVE_CH_OFF_STAT_CUR     32   /* 7 × int16 LE (STR..LUCK) */
#define CSB_SAVE_CH_OFF_STAT_MAX     46   /* 7 × int16 LE */
#define CSB_SAVE_CH_OFF_SKILLS       60   /* 16 × uint8 */
#define CSB_SAVE_CH_OFF_SLOTS        76   /* 30 × uint16 LE = 60 bytes */
/* [136..159] reserved */

/* Import-source stamp value used by csb_v1_import_csb_save_*
 * (party->ImportSource).  Distinct from the DM1 values
 * (0 none / 1 utility disk / 2 dm1 save). */
#define CSB_SAVE_IMPORT_SOURCE       3

/* Result/error codes.  On success the import functions return
 * the champion count (>0).  Failures return a negative code. */
typedef enum {
    CSB_SAVE_IMPORT_ERR_NULL        = -1, /* NULL argument */
    CSB_SAVE_IMPORT_ERR_IO          = -2, /* file open/read error */
    CSB_SAVE_IMPORT_ERR_BAD_MAGIC   = -3, /* not a CSB save */
    CSB_SAVE_IMPORT_ERR_VERSION     = -4, /* unsupported/mismatched version */
    CSB_SAVE_IMPORT_ERR_NO_CHAMPIONS = -5, /* count 0 or > 4 */
    CSB_SAVE_IMPORT_ERR_TRUNCATED   = -6  /* buffer too short for records */
} CSB_SaveImportResult;

/* Import a CSB v2.0/v2.1 save from an in-memory buffer into
 * `party`.  Maps the CSB roster into CSB_V1_Champion slots,
 * applies the CHANGE7_24 reincarnation stat-cap penalty to
 * any champion flagged reincarnated, and stamps the party
 * (ImportSource = CSB_SAVE_IMPORT_SOURCE, variant recorded
 * in party->Reserved[0]).  Returns champion count on success
 * or a negative CSB_SaveImportResult. */
int csb_v1_import_csb_save_buffer(CSB_V1_PartyState* party,
                                  const unsigned char* buf,
                                  long len);

/* File wrapper around csb_v1_import_csb_save_buffer. */
int csb_v1_import_csb_save_file(CSB_V1_PartyState* party,
                                const char* path);

/* Build a CSB save buffer from a party (inverse of the
 * importer; used for round-trip tests and re-export).  Writes
 * the 256-byte header + one record per present champion.
 * `version` must be CSB_SAVE_VERSION_V20 or _V21.  Returns
 * total bytes written or a negative error. */
long csb_v1_build_csb_save_buffer(const CSB_V1_PartyState* party,
                                  unsigned int version,
                                  unsigned char* out,
                                  long outCapacity);

/* Legacy entry point (retained for the OMFATTANDE-stub gate).
 * Now performs a real path-based import into a throwaway
 * party just to validate the file; returns the champion count
 * on success, or a negative CSB_SaveImportResult on failure
 * (e.g. nonexistent path -> CSB_SAVE_IMPORT_ERR_IO). */
int  csb_v1_import_csb_save(const char* path);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_SAVE_IMPORT_PATH_PC34_COMPAT_H */
