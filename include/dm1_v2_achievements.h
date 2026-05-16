
#ifndef DM1_V2_ACHIEVEMENTS_H
#define DM1_V2_ACHIEVEMENTS_H

/* Achievement system — milestones for each game.
 * Tracks across sessions via save file. */

#define ACH_MAX 64

typedef enum {
    ACH_FIRST_STEPS = 0,      /* Enter the dungeon */
    ACH_FULL_PARTY,            /* Recruit 4 champions */
    ACH_FIRST_BLOOD,           /* Kill first creature */
    ACH_FIRST_SPELL,           /* Cast first spell */
    ACH_LEVEL_5,               /* Reach level 5 */
    ACH_LEVEL_10,              /* Reach level 10 */
    ACH_BOSS_SLAYER,           /* Kill a boss creature */
    ACH_FIRESTAFF_FOUND,       /* Find the Firestaff */
    ACH_GAME_COMPLETE,         /* Complete the game */
    ACH_NO_DEATHS,             /* Complete without any champion dying */
    ACH_SPEED_RUN,             /* Complete in under 2 hours */
    ACH_PACIFIST,              /* Reach level 5 without killing */
    ACH_HOARDER,               /* Carry 100+ items */
    ACH_SPELL_MASTER,          /* Cast 50 spells */
    ACH_EXPLORER,              /* Explore 90% of the map */
    ACH_COUNT
} DM1_AchievementId;

typedef struct {
    int unlocked[ACH_MAX];
    int unlock_tick[ACH_MAX];
    int total_unlocked;
    int notification_queue[8];
    int notification_count;
} DM1_V2_Achievements;

void dm1_v2_achievements_init(DM1_V2_Achievements *ach);
int dm1_v2_achievement_unlock(DM1_V2_Achievements *ach, int id);
int dm1_v2_achievement_is_unlocked(const DM1_V2_Achievements *ach, int id);
void dm1_v2_achievements_render_notification(DM1_V2_Achievements *ach,
    uint32_t *rgba, int w, int h, float dt);

#endif

