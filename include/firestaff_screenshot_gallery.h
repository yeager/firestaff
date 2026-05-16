
#ifndef FIRESTAFF_SCREENSHOT_GALLERY_H
#define FIRESTAFF_SCREENSHOT_GALLERY_H
#include <stdint.h>

#define FS_GALLERY_MAX 100

typedef struct {
    char path[256];
    uint32_t timestamp;
    int width, height;
    int game_id;
} FS_ScreenshotInfo;

typedef struct {
    FS_ScreenshotInfo entries[FS_GALLERY_MAX];
    int count;
    int selected;
} FS_GalleryState;

int fs_gallery_scan(FS_GalleryState *g, const char *screenshots_dir);
const FS_ScreenshotInfo *fs_gallery_selected(const FS_GalleryState *g);
void fs_gallery_next(FS_GalleryState *g);
void fs_gallery_prev(FS_GalleryState *g);

#endif

