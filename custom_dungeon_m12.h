#ifndef FIRESTAFF_CUSTOM_DUNGEON_M12_H
#define FIRESTAFF_CUSTOM_DUNGEON_M12_H

/*
 * Custom Dungeon Importer — M12 launcher feature.
 *
 * Scans a "custom/" subdirectory under the data dir for DUNGEON.DAT
 * files.  Each immediate subdirectory containing a DUNGEON.DAT is
 * treated as a custom dungeon entry.  The header is read to extract
 * map count, dungeon ID, and basic validation (uncompressed, sane
 * map count).
 *
 * Lifecycle:
 *   1. M12_CustomDungeon_Scan()   — populate the entry list
 *   2. M12_CustomDungeon_Select() — pick an entry by index
 *   3. Read selectedEntry for the path to pass to the engine
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CUSTOM_DUNGEON_MAX_ENTRIES   32
#define CUSTOM_DUNGEON_NAME_MAX     128
#define CUSTOM_DUNGEON_PATH_MAX     512

typedef enum {
    CUSTOM_DUNGEON_STATUS_UNKNOWN = 0,
    CUSTOM_DUNGEON_STATUS_VALID,
    CUSTOM_DUNGEON_STATUS_INVALID_HEADER,
    CUSTOM_DUNGEON_STATUS_TOO_SMALL,
    CUSTOM_DUNGEON_STATUS_COMPRESSED,
    CUSTOM_DUNGEON_STATUS_READ_ERROR
} M12_CustomDungeonStatus;

typedef struct {
    char name[CUSTOM_DUNGEON_NAME_MAX];           /* subdirectory name      */
    char path[CUSTOM_DUNGEON_PATH_MAX];           /* full path to DUNGEON.DAT */
    char dirPath[CUSTOM_DUNGEON_PATH_MAX];        /* full path to the subdir  */
    long fileSize;                                /* DUNGEON.DAT size (bytes) */
    M12_CustomDungeonStatus status;
    int  mapCount;                                /* from header (if valid)   */
    int  dungeonId;                               /* ornamentRandomSeed proxy */
} M12_CustomDungeonEntry;

typedef struct {
    M12_CustomDungeonEntry entries[CUSTOM_DUNGEON_MAX_ENTRIES];
    int entryCount;
    int selectedIndex;
    int scrollOffset;
    int scanned;
} M12_CustomDungeonState;

/**
 * Initialize state to zero.
 */
void M12_CustomDungeon_Init(M12_CustomDungeonState* state);

/**
 * Scan dataDir/custom/ for subdirectories containing DUNGEON.DAT.
 * Each found file is header-validated.  Returns number of entries found.
 */
int M12_CustomDungeon_Scan(M12_CustomDungeonState* state,
                           const char* dataDir);

/**
 * Handle menu navigation input within the custom dungeon browser.
 * Returns 1 if the user confirmed a selection (caller reads
 * entries[selectedIndex]).
 */
int M12_CustomDungeon_HandleInput(M12_CustomDungeonState* state, int input);

/**
 * Select entry by index.  Returns 1 if valid, 0 if out of range or
 * the entry failed validation.
 */
int M12_CustomDungeon_Select(M12_CustomDungeonState* state, int index);

/**
 * Return the currently selected entry, or NULL if none selected or
 * list is empty.
 */
const M12_CustomDungeonEntry* M12_CustomDungeon_GetSelected(
    const M12_CustomDungeonState* state);

/**
 * Return a human-readable status label for a validation status code.
 */
const char* M12_CustomDungeon_StatusLabel(M12_CustomDungeonStatus status);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CUSTOM_DUNGEON_M12_H */
