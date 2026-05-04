/*
 * screenshot_gallery_m12.c — Screenshot Gallery for the Firestaff launcher.
 *
 * Scans a screenshots directory for BMP/PNG files, sorts by
 * modification time (newest first), and provides grid browsing,
 * fullscreen preview, and delete-with-confirmation.
 */

#include "screenshot_gallery_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                   */
/* ------------------------------------------------------------------ */

/* Case-insensitive suffix check. */
static int has_image_extension(const char* name) {
    size_t len;
    if (!name) return 0;
    len = strlen(name);
    if (len < 5) return 0; /* x.bmp or x.png minimum */

    /* Check last 4 chars case-insensitively */
    if ((name[len-4] == '.') &&
        ((name[len-3] == 'b' || name[len-3] == 'B') &&
         (name[len-2] == 'm' || name[len-2] == 'M') &&
         (name[len-1] == 'p' || name[len-1] == 'P')))
        return 1;

    if ((name[len-4] == '.') &&
        ((name[len-3] == 'p' || name[len-3] == 'P') &&
         (name[len-2] == 'n' || name[len-2] == 'N') &&
         (name[len-1] == 'g' || name[len-1] == 'G')))
        return 1;

    return 0;
}

/* Read BMP dimensions from file header.
 * BMP: bytes 18-21 = width (LE int32), bytes 22-25 = height (LE int32).
 * Returns 1 on success. */
static int read_bmp_dimensions(const char* path, int* w, int* h) {
    FILE* f;
    unsigned char hdr[26];
    int32_t bw, bh;

    f = fopen(path, "rb");
    if (!f) return 0;
    if (fread(hdr, 1, 26, f) != 26) { fclose(f); return 0; }
    fclose(f);

    /* Verify BMP magic */
    if (hdr[0] != 'B' || hdr[1] != 'M') return 0;

    /* Width at offset 18, height at offset 22 (little-endian) */
    bw = (int32_t)(hdr[18] | (hdr[19] << 8) | (hdr[20] << 16) | (hdr[21] << 24));
    bh = (int32_t)(hdr[22] | (hdr[23] << 8) | (hdr[24] << 16) | (hdr[25] << 24));

    /* Height can be negative (top-down BMP) */
    if (bh < 0) bh = -bh;
    if (bw <= 0 || bh <= 0) return 0;

    *w = (int)bw;
    *h = (int)bh;
    return 1;
}

/* Read PNG dimensions from IHDR chunk.
 * PNG: bytes 0-7 = signature, bytes 16-19 = width (BE uint32),
 *      bytes 20-23 = height (BE uint32).
 * Returns 1 on success. */
static int read_png_dimensions(const char* path, int* w, int* h) {
    FILE* f;
    unsigned char hdr[24];
    uint32_t pw, ph;

    f = fopen(path, "rb");
    if (!f) return 0;
    if (fread(hdr, 1, 24, f) != 24) { fclose(f); return 0; }
    fclose(f);

    /* Verify PNG signature */
    if (hdr[0] != 0x89 || hdr[1] != 'P' || hdr[2] != 'N' || hdr[3] != 'G')
        return 0;

    /* Width at offset 16, height at offset 20 (big-endian) */
    pw = ((uint32_t)hdr[16] << 24) | ((uint32_t)hdr[17] << 16) |
         ((uint32_t)hdr[18] << 8)  | (uint32_t)hdr[19];
    ph = ((uint32_t)hdr[20] << 24) | ((uint32_t)hdr[21] << 16) |
         ((uint32_t)hdr[22] << 8)  | (uint32_t)hdr[23];

    if (pw == 0 || ph == 0 || pw > 65535 || ph > 65535) return 0;

    *w = (int)pw;
    *h = (int)ph;
    return 1;
}

/* Read image dimensions (BMP or PNG). */
static int read_image_dimensions(const char* path, int* w, int* h) {
    size_t len = strlen(path);
    *w = 0;
    *h = 0;

    if (len >= 4) {
        const char* ext = path + len - 4;
        if ((ext[1] == 'b' || ext[1] == 'B') &&
            (ext[2] == 'm' || ext[2] == 'M') &&
            (ext[3] == 'p' || ext[3] == 'P'))
            return read_bmp_dimensions(path, w, h);
        if ((ext[1] == 'p' || ext[1] == 'P') &&
            (ext[2] == 'n' || ext[2] == 'N') &&
            (ext[3] == 'g' || ext[3] == 'G'))
            return read_png_dimensions(path, w, h);
    }
    return 0;
}

/* qsort comparator: newest modification time first. */
static int compare_newest_first(const void* a, const void* b) {
    const M12_ScreenshotGalleryEntry* ea = (const M12_ScreenshotGalleryEntry*)a;
    const M12_ScreenshotGalleryEntry* eb = (const M12_ScreenshotGalleryEntry*)b;

    if (ea->modTime > eb->modTime) return -1;
    if (ea->modTime < eb->modTime) return  1;
    return strcmp(ea->filename, eb->filename);
}

/* Clamp selectedIndex to valid range and adjust scroll. */
static void clamp_selection(M12_ScreenshotGalleryState* state) {
    if (state->entryCount == 0) {
        state->selectedIndex = 0;
        state->scrollOffset = 0;
        return;
    }
    if (state->selectedIndex < 0)
        state->selectedIndex = 0;
    if (state->selectedIndex >= state->entryCount)
        state->selectedIndex = state->entryCount - 1;

    /* Ensure selected index is visible in the current page */
    if (state->selectedIndex < state->scrollOffset)
        state->scrollOffset = (state->selectedIndex / SCREENSHOT_GALLERY_COLS)
                              * SCREENSHOT_GALLERY_COLS;
    if (state->selectedIndex >= state->scrollOffset + SCREENSHOT_GALLERY_PAGE_SIZE)
        state->scrollOffset = ((state->selectedIndex / SCREENSHOT_GALLERY_COLS)
                               - (SCREENSHOT_GALLERY_ROWS - 1))
                              * SCREENSHOT_GALLERY_COLS;
    if (state->scrollOffset < 0)
        state->scrollOffset = 0;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

int M12_ScreenshotGallery_Scan(M12_ScreenshotGalleryState* state,
                               const char* screenshotDir) {
    DIR* dir;
    struct dirent* ent;
    struct stat st;
    M12_ScreenshotGalleryEntry* entry;

    if (!state || !screenshotDir) return 0;

    memset(state, 0, sizeof(*state));

    dir = opendir(screenshotDir);
    if (!dir) return 0;

    while ((ent = readdir(dir)) != NULL) {
        if (state->entryCount >= SCREENSHOT_GALLERY_MAX_ENTRIES) break;
        if (!has_image_extension(ent->d_name)) continue;

        entry = &state->entries[state->entryCount];
        memset(entry, 0, sizeof(*entry));

        snprintf(entry->filename, SCREENSHOT_GALLERY_FILENAME_MAX,
                 "%s", ent->d_name);
        snprintf(entry->fullPath, SCREENSHOT_GALLERY_FILENAME_MAX,
                 "%s/%s", screenshotDir, ent->d_name);

        if (stat(entry->fullPath, &st) == 0) {
            entry->fileSize = (long)st.st_size;
            entry->modTime  = st.st_mtime;
        }

        read_image_dimensions(entry->fullPath, &entry->width, &entry->height);

        state->entryCount++;
    }
    closedir(dir);

    /* Sort newest first */
    if (state->entryCount > 1) {
        qsort(state->entries, (size_t)state->entryCount,
              sizeof(M12_ScreenshotGalleryEntry), compare_newest_first);
    }

    state->selectedIndex = 0;
    state->scrollOffset  = 0;
    state->confirmDelete = 0;
    state->mode = M12_SCREENSHOT_MODE_GRID;

    return state->entryCount;
}

int M12_ScreenshotGallery_HandleInput(M12_ScreenshotGalleryState* state,
                                      int input) {
    if (!state) return 0;

    /* ── Delete confirmation mode ──────────────────────────────── */
    if (state->confirmDelete) {
        if (input == M12_MENU_INPUT_ACCEPT) {
            M12_ScreenshotGallery_DeleteSelected(state);
            state->confirmDelete = 0;
        } else if (input == M12_MENU_INPUT_BACK) {
            state->confirmDelete = 0;
        }
        return 0;
    }

    /* ── Fullscreen mode ───────────────────────────────────────── */
    if (state->mode == M12_SCREENSHOT_MODE_FULLSCREEN) {
        switch (input) {
        case M12_MENU_INPUT_BACK:
            state->mode = M12_SCREENSHOT_MODE_GRID;
            return 0;
        case M12_MENU_INPUT_LEFT:
            if (state->selectedIndex > 0)
                state->selectedIndex--;
            return 0;
        case M12_MENU_INPUT_RIGHT:
            if (state->selectedIndex < state->entryCount - 1)
                state->selectedIndex++;
            return 0;
        case M12_MENU_INPUT_ACTION:
            /* ACTION in fullscreen triggers delete confirmation */
            if (state->entryCount > 0)
                state->confirmDelete = 1;
            return 0;
        default:
            return 0;
        }
    }

    /* ── Grid mode ─────────────────────────────────────────────── */
    switch (input) {
    case M12_MENU_INPUT_UP:
        state->selectedIndex -= SCREENSHOT_GALLERY_COLS;
        clamp_selection(state);
        return 0;

    case M12_MENU_INPUT_DOWN:
        state->selectedIndex += SCREENSHOT_GALLERY_COLS;
        clamp_selection(state);
        return 0;

    case M12_MENU_INPUT_LEFT:
        if (state->selectedIndex > 0)
            state->selectedIndex--;
        clamp_selection(state);
        return 0;

    case M12_MENU_INPUT_RIGHT:
        if (state->selectedIndex < state->entryCount - 1)
            state->selectedIndex++;
        clamp_selection(state);
        return 0;

    case M12_MENU_INPUT_ACCEPT:
        /* Enter fullscreen preview */
        if (state->entryCount > 0)
            state->mode = M12_SCREENSHOT_MODE_FULLSCREEN;
        return 0;

    case M12_MENU_INPUT_ACTION:
        /* ACTION in grid triggers delete confirmation */
        if (state->entryCount > 0)
            state->confirmDelete = 1;
        return 0;

    case M12_MENU_INPUT_BACK:
        /* Signal caller to pop back to parent menu */
        return 1;

    default:
        return 0;
    }
}

int M12_ScreenshotGallery_DeleteSelected(M12_ScreenshotGalleryState* state) {
    int i;

    if (!state || state->entryCount == 0) return -1;
    if (state->selectedIndex < 0 || state->selectedIndex >= state->entryCount)
        return -1;

    /* Remove the file */
    if (remove(state->entries[state->selectedIndex].fullPath) != 0)
        return -1;

    /* Shift remaining entries down */
    for (i = state->selectedIndex; i < state->entryCount - 1; i++)
        state->entries[i] = state->entries[i + 1];
    state->entryCount--;

    /* If we deleted the last entry, move selection back */
    if (state->selectedIndex >= state->entryCount && state->entryCount > 0)
        state->selectedIndex = state->entryCount - 1;

    /* If gallery is now empty, reset to grid mode */
    if (state->entryCount == 0) {
        state->selectedIndex = 0;
        state->scrollOffset  = 0;
        state->mode = M12_SCREENSHOT_MODE_GRID;
    }

    clamp_selection(state);
    return 0;
}

const M12_ScreenshotGalleryEntry* M12_ScreenshotGallery_GetSelected(
    const M12_ScreenshotGalleryState* state) {
    if (!state || state->entryCount == 0) return NULL;
    if (state->selectedIndex < 0 || state->selectedIndex >= state->entryCount)
        return NULL;
    return &state->entries[state->selectedIndex];
}

void M12_ScreenshotGallery_Draw(const M12_ScreenshotGalleryState* state,
                                unsigned char* fb, int fbWidth, int fbHeight) {
    /*
     * Text-mode rendering placeholder.  The real SDL renderer in
     * menu_startup_render_modern_m12.c will handle actual thumbnail
     * rendering with SDL_Surface blitting.
     *
     * This function provides the structural draw logic for headless
     * testing (same pattern as save_browser_m12 / bestiary_m12).
     */
    int i, col, row;
    int visibleStart, visibleEnd;

    (void)fb;
    (void)fbWidth;
    (void)fbHeight;

    if (!state) return;

    if (state->entryCount == 0) {
        /* "No screenshots found" would be drawn here */
        return;
    }

    if (state->confirmDelete) {
        /* Draw delete confirmation overlay:
         * "Delete <filename>?  [Accept] = Yes  [Back] = No" */
        return;
    }

    if (state->mode == M12_SCREENSHOT_MODE_FULLSCREEN) {
        /* Draw fullscreen preview of entries[selectedIndex].
         * Display filename + dimensions + file size as overlay text. */
        return;
    }

    /* ── Grid mode ─────────────────────────────────────────────── */
    visibleStart = state->scrollOffset;
    visibleEnd   = visibleStart + SCREENSHOT_GALLERY_PAGE_SIZE;
    if (visibleEnd > state->entryCount)
        visibleEnd = state->entryCount;

    for (i = visibleStart; i < visibleEnd; i++) {
        col = (i - visibleStart) % SCREENSHOT_GALLERY_COLS;
        row = (i - visibleStart) / SCREENSHOT_GALLERY_COLS;
        (void)col;
        (void)row;

        /* Each cell would render:
         *   - Thumbnail (scaled-down image)
         *   - Filename below
         *   - Selection highlight if i == selectedIndex
         */
    }

    /* Draw scroll indicator if more entries exist */
    if (state->scrollOffset > 0) {
        /* "↑ more" indicator */
    }
    if (visibleEnd < state->entryCount) {
        /* "↓ more" indicator */
    }
}
