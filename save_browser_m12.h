#ifndef FIRESTAFF_SAVE_BROWSER_M12_H
#define FIRESTAFF_SAVE_BROWSER_M12_H

/*
 * Save Game Browser — M12 launcher feature.
 *
 * Scans the data directory for firestaff-*.sav files, parses headers
 * to extract metadata (game ID, party level, champion names), and
 * presents a navigable list with load/delete actions.
 */

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAVE_BROWSER_MAX_ENTRIES  64
#define SAVE_BROWSER_FILENAME_MAX 256
#define SAVE_BROWSER_LABEL_MAX    128

typedef struct {
    char filename[SAVE_BROWSER_FILENAME_MAX];   /* basename only          */
    char fullPath[SAVE_BROWSER_FILENAME_MAX];   /* absolute path          */
    char gameId[32];                            /* extracted from filename */
    char label[SAVE_BROWSER_LABEL_MAX];         /* display label          */
    char champions[128];                        /* comma-separated names  */
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
