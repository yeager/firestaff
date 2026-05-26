#ifndef FIRESTAFF_DM1_V1_CUSTOM_DUNGEON_LOADER_H
#define FIRESTAFF_DM1_V1_CUSTOM_DUNGEON_LOADER_H

/*
 * dm1_v1_custom_dungeon_loader — Scan a directory for community
 * custom dungeons.
 *
 * Looks for subdirectories that contain dungeon.dat (case-insensitive,
 * optional graphics.dat).  Validates dungeon.dat by checking the
 * 44-byte header for the expected map-count + signature pattern shared
 * by the canonical DM1 V1 format.
 *
 * Engine-side companion to custom_dungeon_m12 (the launcher version
 * scans dataDir/custom/).  This one scans an arbitrary path, defaulting
 * to ~/.firestaff/dungeons/.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define M11_CUSTOM_DUNGEON_MAX_ENTRIES   64
#define M11_CUSTOM_DUNGEON_NAME_MAX      64
#define M11_CUSTOM_DUNGEON_PATH_MAX     512

typedef struct {
    char name[M11_CUSTOM_DUNGEON_NAME_MAX];
    char path[M11_CUSTOM_DUNGEON_PATH_MAX];           /* directory path */
    char dungeonDatPath[M11_CUSTOM_DUNGEON_PATH_MAX]; /* full path to dungeon.dat */
    char graphicsDatPath[M11_CUSTOM_DUNGEON_PATH_MAX];/* full path to graphics.dat ("" if none) */
    int  valid;                                       /* 1 = header looks good */
    int  mapCount;                                    /* parsed from header (if valid) */
    long fileSize;                                    /* dungeon.dat size in bytes */
} M11_CustomDungeon;

typedef struct {
    M11_CustomDungeon entries[M11_CUSTOM_DUNGEON_MAX_ENTRIES];
    int               count;
    int               selectedIndex;
} M11_CustomDungeonList;

/* Resolve the default custom-dungeon directory, creating it if missing
 * (~/.firestaff/dungeons/).  Returns a static buffer. */
const char* M11_CustomDungeon_DefaultDir(void);

/* Initialize list state to zero (count=0, selectedIndex=0). */
void M11_CustomDungeon_Init(M11_CustomDungeonList* list);

/* Scan `directory` for custom dungeons.  Pass NULL for the default
 * directory.  Returns the number of entries discovered. */
int M11_CustomDungeon_Scan(M11_CustomDungeonList* list, const char* directory);

/* Validate a dungeon.dat path on disk.  Returns 1 if the 44-byte
 * header parses as plausible DM1 V1 data, 0 otherwise.  Populates
 * mapCount with the parsed value when valid. */
int M11_CustomDungeon_Validate(const char* dungeonDatPath, int* mapCount);

/* Return the currently selected entry, or NULL. */
const M11_CustomDungeon* M11_CustomDungeon_GetSelected(
    const M11_CustomDungeonList* list);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CUSTOM_DUNGEON_LOADER_H */
