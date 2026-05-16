
#include "dm1_v2_achievements.h"
#include <string.h>
#include <stdio.h>

static const char *g_achievement_names[ACH_COUNT] = {
    "First Steps", "Full Party", "First Blood", "First Spell",
    "Dungeon Delver", "Deep Explorer", "Boss Slayer", "Firestaff Found",
    "Game Complete", "Deathless", "Speed Run", "Pacifist",
    "Hoarder", "Spell Master", "Explorer"
};

void dm1_v2_achievements_init(DM1_V2_Achievements *ach) {
    if (ach) memset(ach, 0, sizeof(*ach));
}

int dm1_v2_achievement_unlock(DM1_V2_Achievements *ach, int id) {
    if (!ach || id < 0 || id >= ACH_COUNT) return 0;
    if (ach->unlocked[id]) return 0; /* already unlocked */
    ach->unlocked[id] = 1;
    ach->total_unlocked++;
    /* Queue notification */
    if (ach->notification_count < 8)
        ach->notification_queue[ach->notification_count++] = id;
    printf("Achievement unlocked: %s\n", g_achievement_names[id]);
    return 1;
}

int dm1_v2_achievement_is_unlocked(const DM1_V2_Achievements *ach, int id) {
    if (!ach || id < 0 || id >= ACH_COUNT) return 0;
    return ach->unlocked[id];
}

void dm1_v2_achievements_render_notification(DM1_V2_Achievements *ach,
    uint32_t *rgba, int w, int h, float dt)
{
    int x, y2;
    (void)dt;
    if (!ach || !rgba || ach->notification_count == 0) return;

    /* Render gold banner at top center */
    {
        int banner_w = w / 3;
        int banner_h = 24;
        int banner_x = (w - banner_w) / 2;
        int banner_y = 4;

        for (y2 = banner_y; y2 < banner_y + banner_h && y2 < h; y2++)
            for (x = banner_x; x < banner_x + banner_w && x < w; x++) {
                /* Gold background with slight transparency */
                uint32_t bg = rgba[y2*w+x];
                int r = (int)(218 * 0.85f + ((bg>>16)&0xFF) * 0.15f);
                int g = (int)(165 * 0.85f + ((bg>>8)&0xFF) * 0.15f);
                int b = (int)(32 * 0.85f + (bg&0xFF) * 0.15f);
                rgba[y2*w+x] = 0xFF000000|(r<<16)|(g<<8)|b;
            }
    }

    /* Pop oldest notification after display time */
    /* (In real use, track display timer and shift queue) */
}

