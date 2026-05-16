
#include "firestaff_screenshot_gallery.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

int fs_gallery_scan(FS_GalleryState *g, const char *dir) {
    DIR *d;
    struct dirent *ent;
    if (!g || !dir) return -1;
    g->count = 0; g->selected = 0;

    d = opendir(dir);
    if (!d) return -1;

    while ((ent = readdir(d)) != NULL && g->count < FS_GALLERY_MAX) {
        const char *name = ent->d_name;
        int len = (int)strlen(name);
        /* Match .ppm or .png files */
        if (len > 4 && (strcmp(name + len - 4, ".ppm") == 0 ||
                        strcmp(name + len - 4, ".png") == 0)) {
            snprintf(g->entries[g->count].path, 256, "%s/%s", dir, name);
            struct stat st;
            if (stat(g->entries[g->count].path, &st) == 0) {
                g->entries[g->count].timestamp = (uint32_t)st.st_mtime;
            }
            g->count++;
        }
    }
    closedir(d);
    return g->count;
}

const FS_ScreenshotInfo *fs_gallery_selected(const FS_GalleryState *g) {
    if (!g || g->count <= 0) return NULL;
    return &g->entries[g->selected];
}

void fs_gallery_next(FS_GalleryState *g) {
    if (g && g->selected < g->count - 1) g->selected++;
}

void fs_gallery_prev(FS_GalleryState *g) {
    if (g && g->selected > 0) g->selected--;
}

