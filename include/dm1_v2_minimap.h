
#ifndef DM1_V2_MINIMAP_H
#define DM1_V2_MINIMAP_H
#include <stdint.h>

/* Translucent corner minimap showing explored areas.
 * Rendered as overlay on the game viewport. */

#define MINIMAP_SIZE 96       /* pixels */
#define MINIMAP_MAX_MAP 32

typedef struct {
    uint8_t explored[MINIMAP_MAX_MAP][MINIMAP_MAX_MAP];
    int map_width, map_height;
    int party_x, party_y, party_dir;
    int visible;
    float opacity;            /* 0.0 - 1.0 */
} DM1_V2_Minimap;

void dm1_v2_minimap_init(DM1_V2_Minimap *mm, int map_w, int map_h);
void dm1_v2_minimap_update(DM1_V2_Minimap *mm, int px, int py, int dir);
void dm1_v2_minimap_reveal(DM1_V2_Minimap *mm, int x, int y);
void dm1_v2_minimap_render(const DM1_V2_Minimap *mm, uint32_t *rgba, int screen_w, int screen_h);

#endif

