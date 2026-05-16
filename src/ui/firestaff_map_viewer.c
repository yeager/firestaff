
#include "firestaff_map_viewer.h"
#include <string.h>

static const char *g_dm1_level_names[] = {
    "Hall of Champions", "Dungeon Level 1", "Dungeon Level 2",
    "Dungeon Level 3", "Dungeon Level 4", "Dungeon Level 5",
    "Dungeon Level 6", "Dungeon Level 7", "Dungeon Level 8",
    "Dungeon Level 9", "Dungeon Level 10", "Dungeon Level 11",
    "Dungeon Level 12", "Dungeon Level 13"
};

void fs_map_viewer_init(FS_MapViewerState *s, int level) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->level = level;
    s->width = 32; s->height = 32;
    s->level_name = (level >= 0 && level < 14) ? g_dm1_level_names[level] : "Unknown";
}

void fs_map_viewer_reveal_all(FS_MapViewerState *s) {
    if (!s) return;
    memset(s->explored, 1, sizeof(s->explored));
}

uint32_t fs_map_viewer_cell_color(int sq, int explored) {
    if (!explored) return 0xFF101010;
    switch (sq & 0x1F) {
        case 0:  return 0xFF303030; /* wall */
        case 1:  return 0xFF909090; /* corridor */
        case 2:  return 0xFF4444FF; /* stairs up */
        case 3:  return 0xFF2222CC; /* stairs down */
        case 4:  return 0xFFCC2222; /* pit */
        case 5:  return 0xFF22CCCC; /* teleporter */
        case 16: return 0xFF886633; /* door front */
        case 17: return 0xFF886633; /* door side */
        default: return 0xFF606060;
    }
}

void fs_map_viewer_render(const FS_MapViewerState *s,
    uint32_t *pixels, int out_w, int out_h, int cell_size)
{
    if (!s || !pixels) return;
    memset(pixels, 0, out_w * out_h * 4);
    for (int y = 0; y < s->height && y * cell_size < out_h; y++) {
        for (int x = 0; x < s->width && x * cell_size < out_w; x++) {
            uint32_t color = fs_map_viewer_cell_color(
                s->squares[y][x], s->explored[y][x]);
            /* Party marker */
            if (x == s->party_x && y == s->party_y)
                color = 0xFFFFFFFF;
            /* Fill cell */
            for (int py = 0; py < cell_size && y*cell_size+py < out_h; py++)
                for (int px = 0; px < cell_size && x*cell_size+px < out_w; px++)
                    pixels[(y*cell_size+py)*out_w + x*cell_size+px] = color;
        }
    }
}

