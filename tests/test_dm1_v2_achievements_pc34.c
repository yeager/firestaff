#include "dm1_v2_achievements.h"
#include "dm1_v2_achievements_pc34.h"
#include <stdio.h>
#include <string.h>
int main(void) {
    DM1_V2_Achievements ach; unsigned int fb[16], before[16];
    memset(&ach, 0xA5, sizeof(ach)); memset(fb, 0x3C, sizeof(fb)); memcpy(before, fb, sizeof(fb));
    dm1_v2_achievements_init(&ach); if (dm1_v2_achievement_unlock(&ach, ACH_GAME_COMPLETE) || dm1_v2_achievement_is_unlocked(&ach, ACH_GAME_COMPLETE)) return 1;
    dm1_v2_achievements_render_notification(&ach, fb, 4, 4, 1.0f); if (memcmp(fb,before,sizeof(fb))) return 1;
    v2_achievement_init(); v2_achievement_define(1,"synthetic","synthetic",0); v2_achievement_unlock(1);
    if (v2_achievement_is_unlocked(1) || v2_achievement_get_notification()!=NULL) return 1;
    puts("dm1_v2_achievements_pc34: ok"); return 0;
}
