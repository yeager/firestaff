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

#define DM1_V1_CUSTOM_DUNGEON_MAX_ENTRIES   64
#define DM1_V1_CUSTOM_DUNGEON_NAME_MAX      64
#define DM1_V1_CUSTOM_DUNGEON_PATH_MAX     512

typedef struct {
    char name[DM1_V1_CUSTOM_DUNGEON_NAME_MAX];
    char path[DM1_V1_CUSTOM_DUNGEON_PATH_MAX];           /* directory path */
    char dungeonDatPath[DM1_V1_CUSTOM_DUNGEON_PATH_MAX]; /* full path to dungeon.dat */
    char graphicsDatPath[DM1_V1_CUSTOM_DUNGEON_PATH_MAX];/* full path to graphics.dat ("" if none) */
    int  valid;                                       /* 1 = header looks good */
    int  mapCount;                                    /* parsed from header (if valid) */
    long fileSize;                                    /* dungeon.dat size in bytes */
} DM1_V1_CustomDungeonPc34;

typedef struct {
    DM1_V1_CustomDungeonPc34 entries[DM1_V1_CUSTOM_DUNGEON_MAX_ENTRIES];
    int               count;
    int               selectedIndex;
} DM1_V1_CustomDungeonListPc34;

/* Resolve the default custom-dungeon directory, creating it if missing
 * (~/.firestaff/dungeons/).  Returns a static buffer. */
const char* DM1_V1_CustomDungeon_DefaultDirPc34Compat(void);

/* Initialize list state to zero (count=0, selectedIndex=0). */
void DM1_V1_CustomDungeon_InitPc34Compat(DM1_V1_CustomDungeonListPc34* list);

/* Scan `directory` for custom dungeons.  Pass NULL for the default
 * directory.  Returns the number of entries discovered. */
int DM1_V1_CustomDungeon_ScanPc34Compat(DM1_V1_CustomDungeonListPc34* list, const char* directory);

/* Validate a dungeon.dat path on disk.  Returns 1 if the 44-byte
 * header parses as plausible DM1 V1 data, 0 otherwise.  Populates
 * mapCount with the parsed value when valid. */
int DM1_V1_CustomDungeon_ValidatePc34Compat(const char* dungeonDatPath, int* mapCount);

/* Return the currently selected entry, or NULL. */
const DM1_V1_CustomDungeonPc34* DM1_V1_CustomDungeon_GetSelectedPc34Compat(
    const DM1_V1_CustomDungeonListPc34* list);

/* Compatibility aliases for older M11 call sites. */
#define M11_CUSTOM_DUNGEON_MAX_ENTRIES DM1_V1_CUSTOM_DUNGEON_MAX_ENTRIES
#define M11_CUSTOM_DUNGEON_NAME_MAX DM1_V1_CUSTOM_DUNGEON_NAME_MAX
#define M11_CUSTOM_DUNGEON_PATH_MAX DM1_V1_CUSTOM_DUNGEON_PATH_MAX
typedef DM1_V1_CustomDungeonPc34 M11_CustomDungeon;
typedef DM1_V1_CustomDungeonListPc34 M11_CustomDungeonList;
#define M11_CustomDungeon_DefaultDir DM1_V1_CustomDungeon_DefaultDirPc34Compat
#define M11_CustomDungeon_Init DM1_V1_CustomDungeon_InitPc34Compat
#define M11_CustomDungeon_Scan DM1_V1_CustomDungeon_ScanPc34Compat
#define M11_CustomDungeon_Validate DM1_V1_CustomDungeon_ValidatePc34Compat
#define M11_CustomDungeon_GetSelected DM1_V1_CustomDungeon_GetSelectedPc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CUSTOM_DUNGEON_LOADER_H */
