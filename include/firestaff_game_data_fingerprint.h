#ifndef FIRESTAFF_GAME_DATA_FINGERPRINT_H
#define FIRESTAFF_GAME_DATA_FINGERPRINT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FIRESTAFF_GAME_UNKNOWN = 0,
    FIRESTAFF_GAME_DM1     = 1,
    FIRESTAFF_GAME_CSB     = 2,
    FIRESTAFF_GAME_DM2     = 3
} FirestaffGame;

typedef enum {
    FIRESTAFF_PLATFORM_UNKNOWN   = 0,
    FIRESTAFF_PLATFORM_AMIGA     = 1,
    FIRESTAFF_PLATFORM_ATARI_ST  = 2,
    FIRESTAFF_PLATFORM_PC        = 3,
    FIRESTAFF_PLATFORM_FM_TOWNS  = 4,
    FIRESTAFF_PLATFORM_PC98      = 5,
    FIRESTAFF_PLATFORM_X68000    = 6,
    FIRESTAFF_PLATFORM_APPLE_IIGS = 7,
    FIRESTAFF_PLATFORM_MACINTOSH = 8,
    FIRESTAFF_PLATFORM_SEGA_CD   = 9,
    FIRESTAFF_PLATFORM_IBM_PSV   = 10,
    FIRESTAFF_PLATFORM_PC9821    = 11
} FirestaffPlatform;

typedef enum {
    FIRESTAFF_LANG_UNKNOWN       = 0,
    FIRESTAFF_LANG_ENGLISH       = 1,
    FIRESTAFF_LANG_FRENCH        = 2,
    FIRESTAFF_LANG_GERMAN        = 3,
    FIRESTAFF_LANG_JAPANESE      = 4,
    FIRESTAFF_LANG_MULTILANGUAGE = 5
} FirestaffLanguage;

typedef enum {
    FIRESTAFF_FILE_UNKNOWN       = 0,
    FIRESTAFF_FILE_GRAPHICS_DAT  = 1,
    FIRESTAFF_FILE_SONG_DAT      = 2,
    FIRESTAFF_FILE_HCSB_DAT      = 3,
    FIRESTAFF_FILE_CEDTLS_DAT    = 4,
    FIRESTAFF_FILE_ANIMATE_DAT   = 5,
    FIRESTAFF_FILE_ENTER_SNG     = 6,
    FIRESTAFF_FILE_UTILITY_AMG   = 7,
    FIRESTAFF_FILE_DMCOORD_DAT   = 8,
    FIRESTAFF_FILE_DEMOIIGS_DAT  = 9,
    FIRESTAFF_FILE_GRAPHICS_GAME = 10,
    FIRESTAFF_FILE_DUNGEON_DAT   = 11,
    FIRESTAFF_FILE_TITL_DAT      = 12,
    FIRESTAFF_FILE_ENDA_DAT      = 13,
    FIRESTAFF_FILE_SWSH_FTL      = 14,
    FIRESTAFF_FILE_HCSB_HTC      = 15,
    FIRESTAFF_FILE_ANIMATE_SCR   = 16,
    FIRESTAFF_FILE_HINT_FTL      = 17,
    FIRESTAFF_FILE_SWITCH_DAT    = 18,
    FIRESTAFF_FILE_MINI_DAT      = 19,
    /* Atari ST loadable FTL modules.  These are executable source media,
     * not generic host-program placeholders. */
    FIRESTAFF_FILE_CSB_FTL_MODULE = 20
} FirestaffFileType;

typedef struct {
    uint8_t          md5[16];
    FirestaffGame    game;
    FirestaffPlatform platform;
    FirestaffLanguage language;
    FirestaffFileType file_type;
    const char      *version;
    const char      *description;
} FirestaffGameDataFingerprint;

typedef struct {
    int                              valid;
    const FirestaffGameDataFingerprint *entry;
} FirestaffGameDataClassifyResult;

#define FIRESTAFF_FINGERPRINT_COUNT 86

extern const FirestaffGameDataFingerprint firestaff_fingerprint_table[FIRESTAFF_FINGERPRINT_COUNT];

/* Classify game data by its MD5 hash (16-byte binary).
 * Returns a result with valid=1 and entry pointing into the table on match. */
FirestaffGameDataClassifyResult firestaff_game_data_classify_md5(
    const uint8_t md5[16]);

/* Classify game data by its MD5 hex string (32 chars, case-insensitive).
 * Returns a result with valid=1 and entry pointing into the table on match. */
FirestaffGameDataClassifyResult firestaff_game_data_classify_hex(
    const char *hex32);

/* Return the human-readable name for a game enum. */
const char *firestaff_game_name(FirestaffGame game);

/* Return the human-readable name for a platform enum. */
const char *firestaff_platform_name(FirestaffPlatform platform);

/* Return the human-readable name for a language enum. */
const char *firestaff_language_name(FirestaffLanguage language);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_GAME_DATA_FINGERPRINT_H */
