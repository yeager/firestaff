
#ifndef FIRESTAFF_HUD_H
#define FIRESTAFF_HUD_H
#include <stdint.h>

/* DM1 HUD overlay.
 * - Compass (top-left, shows N/E/S/W)
 * - Movement arrows (bottom of viewport)
 * - Message line (bottom, scrolling text)
 * - Food/water warning icons
 * - Dungeon level indicator */

#define HUD_MSG_MAX_LEN 64
#define HUD_MSG_QUEUE 8

typedef struct {
    char messages[HUD_MSG_QUEUE][HUD_MSG_MAX_LEN];
    int msg_count;
    float msg_timer;        /* current message display time */
    float msg_duration;     /* how long each message shows */
    int party_dir;          /* 0=N 1=E 2=S 3=W for compass */
    int dungeon_level;
    int food_warning;       /* 1 = low food */
    int water_warning;
    int poison_warning;
} FS_HUD;

void fs_hud_init(FS_HUD *hud);
void fs_hud_tick(FS_HUD *hud, float dt);
void fs_hud_message(FS_HUD *hud, const char *text);
void fs_hud_render(const FS_HUD *hud, uint8_t *framebuffer);

#endif

