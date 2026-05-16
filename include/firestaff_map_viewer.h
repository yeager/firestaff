
#ifndef FIRESTAFF_MAP_VIEWER_H
#define FIRESTAFF_MAP_VIEWER_H
#include <stdint.h>

#define FS_MAP_MAX_W 32
#define FS_MAP_MAX_H 32

typedef struct {
    int width, height;
    uint8_t squares[FS_MAP_MAX_H][FS_MAP_MAX_W]; /* square types */
    uint8_t explored[FS_MAP_MAX_H][FS_MAP_MAX_W]; /* 0=hidden, 1=explored */
    int party_x, party_y, party_dir;
    int level;
    const char *level_name;
} FS_MapViewerState;

void fs_map_viewer_init(FS_MapViewerState *s, int level);
void fs_map_viewer_reveal_all(FS_MapViewerState *s);
uint32_t fs_map_viewer_cell_color(int square_type, int explored);
void fs_map_viewer_render(const FS_MapViewerState *s,
    uint32_t *pixels, int out_w, int out_h, int cell_size);

#endif

