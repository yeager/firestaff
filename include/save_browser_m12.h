#ifndef FIRESTAFF_SAVE_BROWSER_M12_H
#define FIRESTAFF_SAVE_BROWSER_M12_H

/*
 * Save Game Browser — M12 launcher feature.
 *
 * Scans the data directory for firestaff-*.sav files, parses headers
 * to extract metadata (game ID, party level, champion names), and
 * presents a navigable list with load/delete actions.
 */

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAVE_BROWSER_MAX_ENTRIES  64
#define SAVE_BROWSER_FILENAME_MAX 256
#define SAVE_BROWSER_LABEL_MAX    128

#define SAVE_BROWSER_MANIFEST_UNKNOWN      0
#define SAVE_BROWSER_MANIFEST_NOT_PRESENT  1
#define SAVE_BROWSER_MANIFEST_PRESENT      2
#define SAVE_BROWSER_MANIFEST_MATCH        3
#define SAVE_BROWSER_MANIFEST_WRONG_GAME   4
#define SAVE_BROWSER_MANIFEST_UNSUPPORTED  5

typedef struct {
    char filename[SAVE_BROWSER_FILENAME_MAX];   /* basename only          */
    char fullPath[SAVE_BROWSER_FILENAME_MAX];   /* absolute path          */
    char gameId[32];                            /* extracted from filename */
    char label[SAVE_BROWSER_LABEL_MAX];         /* display label          */
    char champions[128];                        /* comma-separated names  */
    uint16_t expectedGameCode;                  /* SAVEGAME_PC34_GAME_CODE_* if known */
    uint16_t manifestGameCode;                  /* LSV-02 code, or 0      */
    int  manifestStatus;                        /* SAVE_BROWSER_MANIFEST_* */
    int  mapLevel;                              /* dungeon level (-1=unknown) */
    int  championCount;                         /* 0-4                    */
    int  valid;                                 /* header parsed OK       */
    time_t fileModTime;                         /* file modification time */
    long   fileSize;                            /* file size in bytes     */
} M12_SaveBrowserEntry;

typedef struct {
    M12_SaveBrowserEntry entries[SAVE_BROWSER_MAX_ENTRIES];
    int entryCount;
    int selectedIndex;
    int scrollOffset;
    int confirmDelete;      /* 1 = awaiting delete confirmation */
} M12_SaveBrowserState;

/* Scan dataDir for firestaff-*.sav files and populate state.
 * Returns number of entries found. */
int M12_SaveBrowser_Scan(M12_SaveBrowserState* state, const char* dataDir);

/* Handle menu input within the save browser. Returns 1 if a load was
 * requested (caller should read entries[selectedIndex].fullPath). */
int M12_SaveBrowser_HandleInput(M12_SaveBrowserState* state, int input);

/* Delete the currently selected save file. Returns 0 on success. */
int M12_SaveBrowser_DeleteSelected(M12_SaveBrowserState* state);

/* Export the selected save file into exportDir using the same basename.
 * Returns 0 on success and optionally writes the destination path. */
int M12_SaveBrowser_ExportSelected(const M12_SaveBrowserState* state,
                                   const char* exportDir,
                                   char* outPath,
                                   int outPathSize);

/* Import a firestaff-*.sav file into dataDir. Existing destination files
 * are preserved. Returns 0 on success and optionally writes the target path. */
int M12_SaveBrowser_ImportFile(const char* dataDir,
                               const char* importPath,
                               char* outPath,
                               int outPathSize);

/* Get the currently selected entry, or NULL if none. */
const M12_SaveBrowserEntry* M12_SaveBrowser_GetSelected(
    const M12_SaveBrowserState* state);

/* Draw the save browser into a framebuffer (text-mode rendering).
 * fb may be NULL for headless / test usage. */
void M12_SaveBrowser_Draw(const M12_SaveBrowserState* state,
                          unsigned char* fb, int fbWidth, int fbHeight);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SAVE_BROWSER_M12_H */
