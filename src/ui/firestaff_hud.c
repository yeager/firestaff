
#include "firestaff_hud.h"
#include <string.h>
#include <stdio.h>

static const char *g_compass_labels[] = {"N", "E", "S", "W"};
static const uint8_t g_compass_colors[] = {15, 14, 12, 9};

void fs_hud_init(FS_HUD *hud) {
    if (!hud) return;
    memset(hud, 0, sizeof(*hud));
    hud->msg_duration = 3.0f;
}

void fs_hud_tick(FS_HUD *hud, float dt) {
    if (!hud) return;
    if (hud->msg_count > 0) {
        hud->msg_timer += dt;
        if (hud->msg_timer >= hud->msg_duration) {
            /* Pop oldest message */
            memmove(&hud->messages[0], &hud->messages[1],
                (HUD_MSG_QUEUE - 1) * HUD_MSG_MAX_LEN);
            hud->msg_count--;
            hud->msg_timer = 0;
        }
    }
}

void fs_hud_message(FS_HUD *hud, const char *text) {
    if (!hud || !text || hud->msg_count >= HUD_MSG_QUEUE) return;
    strncpy(hud->messages[hud->msg_count], text, HUD_MSG_MAX_LEN - 1);
    hud->msg_count++;
}

static void draw_filled_rect(uint8_t *fb, int x, int y, int w, int h, uint8_t c) {
    int px, py;
    for (py = y; py < y + h && py < 200; py++)
        for (px = x; px < x + w && px < 320; px++)
            if (px >= 0 && py >= 0)
                fb[py * 320 + px] = c;
}

void fs_hud_render(const FS_HUD *hud, uint8_t *fb) {
    int dir, px, py;
    if (!hud || !fb) return;

    /* Compass: top-left 24×24 */
    draw_filled_rect(fb, 2, 2, 24, 24, 0);  /* black bg */
    dir = hud->party_dir & 3;
    {
        /* Direction indicator: colored dot in compass direction */
        int cx = 14, cy = 14;
        int ddx[4] = {0, 8, 0, -8};
        int ddy[4] = {-8, 0, 8, 0};
        /* Compass rose: 4 dots */
        int d;
        for (d = 0; d < 4; d++) {
            int dx = cx + ddx[d], dy = cy + ddy[d];
            uint8_t color = (d == dir) ? 15 : 7;
            if (dx >= 0 && dx < 320 && dy >= 0 && dy < 200)
                fb[dy * 320 + dx] = color;
            /* Larger dot for current direction */
            if (d == dir) {
                int ax, ay;
                for (ay = -1; ay <= 1; ay++)
                    for (ax = -1; ax <= 1; ax++)
                        if (dx+ax >= 0 && dx+ax < 320 && dy+ay >= 0 && dy+ay < 200)
                            fb[(dy+ay) * 320 + dx+ax] = g_compass_colors[d];
            }
        }
    }

    /* Movement arrows: bottom of viewport area */
    /* Left arrow */
    draw_filled_rect(fb, 40, 138, 12, 10, 7);
    /* Forward arrow */
    draw_filled_rect(fb, 100, 138, 40, 10, 7);
    /* Right arrow */
    draw_filled_rect(fb, 190, 138, 12, 10, 7);
    /* Turn arrows */
    draw_filled_rect(fb, 60, 150, 20, 8, 7);   /* turn left */
    draw_filled_rect(fb, 160, 150, 20, 8, 7);  /* turn right */
    /* Back */
    draw_filled_rect(fb, 100, 150, 40, 8, 7);

    /* Message line at very bottom */
    if (hud->msg_count > 0) {
        /* Message background */
        draw_filled_rect(fb, 0, 190, 320, 10, 0);
        /* Message indicator (colored dot by urgency) */
        fb[194 * 320 + 4] = 15;
        fb[194 * 320 + 5] = 15;
    }

    /* Warning icons: top-right */
    if (hud->food_warning) {
        draw_filled_rect(fb, 295, 2, 6, 6, 6); /* brown = food */
    }
    if (hud->water_warning) {
        draw_filled_rect(fb, 305, 2, 6, 6, 13); /* blue = water */
    }
    if (hud->poison_warning) {
        draw_filled_rect(fb, 295, 10, 6, 6, 2); /* green = poison */
    }

    /* Level indicator */
    {
        int lx = 4, ly = 28;
        uint8_t lc = 14; /* yellow */
        draw_filled_rect(fb, lx, ly, 12, 6, lc);
    }
}

