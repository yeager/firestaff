#ifndef FIRESTAFF_SCREENSHOT_GALLERY_M12_H
#define FIRESTAFF_SCREENSHOT_GALLERY_M12_H

/*
 * Screenshot Gallery — M12 launcher feature.
 *
 * Scans a screenshots directory for BMP/PNG image files, displays
 * them in a navigable thumbnail grid with fullscreen preview, and
 * supports deletion with confirmation.  Entries are sorted by
 * modification time (newest first).
 *
 * Lifecycle:
 *   1. M12_ScreenshotGallery_Scan()        — populate from directory
 *   2. M12_ScreenshotGallery_HandleInput()  — navigate / preview / delete
 *   3. M12_ScreenshotGallery_Draw()         — render current view
 *   4. M12_ScreenshotGallery_GetSelected()  — query highlighted entry
 */

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Limits ──────────────────────────────────────────────────────── */
#define SCREENSHOT_GALLERY_MAX_ENTRIES   256
#define SCREENSHOT_GALLERY_FILENAME_MAX  256
#define SCREENSHOT_GALLERY_COLS          4
#define SCREENSHOT_GALLERY_ROWS          3
#define SCREENSHOT_GALLERY_PAGE_SIZE \
    (SCREENSHOT_GALLERY_COLS * SCREENSHOT_GALLERY_ROWS)

/* ── Gallery entry ───────────────────────────────────────────────── */
typedef struct {
    char  filename[SCREENSHOT_GALLERY_FILENAME_MAX];  /* basename     */
    char  fullPath[SCREENSHOT_GALLERY_FILENAME_MAX];  /* absolute     */
    int   width;                                      /* image width  */
    int   height;                                     /* image height */
    long  fileSize;                                   /* bytes        */
    time_t modTime;                                   /* mtime        */
} M12_ScreenshotGalleryEntry;

/* ── Gallery mode ────────────────────────────────────────────────── */
typedef enum {
    M12_SCREENSHOT_MODE_GRID = 0,   /* thumbnail grid view           */
    M12_SCREENSHOT_MODE_FULLSCREEN  /* single image fullscreen       */
} M12_ScreenshotGalleryMode;

/* ── Gallery state ───────────────────────────────────────────────── */
typedef struct {
    M12_ScreenshotGalleryEntry entries[SCREENSHOT_GALLERY_MAX_ENTRIES];
    int  entryCount;
    int  selectedIndex;
    int  scrollOffset;          /* first visible row * COLS           */
    int  confirmDelete;         /* 1 = awaiting delete confirmation  */
    M12_ScreenshotGalleryMode mode;
} M12_ScreenshotGalleryState;

/* ── API ─────────────────────────────────────────────────────────── */

/* Scan screenshotDir for *.bmp and *.png files.
 * Returns number of entries found (sorted newest-first). */
int M12_ScreenshotGallery_Scan(M12_ScreenshotGalleryState* state,
                               const char* screenshotDir);

/* Handle menu input.  Returns 1 if the user pressed BACK from the
 * grid (caller should pop to previous menu view). */
int M12_ScreenshotGallery_HandleInput(M12_ScreenshotGalleryState* state,
                                      int input);

/* Delete the currently selected screenshot.  Returns 0 on success. */
int M12_ScreenshotGallery_DeleteSelected(M12_ScreenshotGalleryState* state);

/* Get the currently selected entry, or NULL if none. */
const M12_ScreenshotGalleryEntry* M12_ScreenshotGallery_GetSelected(
    const M12_ScreenshotGalleryState* state);

/* Draw the gallery into a framebuffer (text-mode rendering).
 * fb may be NULL for headless / test usage. */
void M12_ScreenshotGallery_Draw(const M12_ScreenshotGalleryState* state,
                                unsigned char* fb, int fbWidth, int fbHeight);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SCREENSHOT_GALLERY_M12_H */
