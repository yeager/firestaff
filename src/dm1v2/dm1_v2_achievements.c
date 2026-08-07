#include "dm1_v2_achievements.h"

/* PC34 has source-owned messages and an endgame sequence, not a persistent
 * achievement ledger or a modern banner. */
void dm1_v2_achievements_init(DM1_V2_Achievements *ach) { (void)ach; }
int dm1_v2_achievement_unlock(DM1_V2_Achievements *ach, int id) { (void)ach; (void)id; return 0; }
int dm1_v2_achievement_is_unlocked(const DM1_V2_Achievements *ach, int id) { (void)ach; (void)id; return 0; }
void dm1_v2_achievements_render_notification(DM1_V2_Achievements *ach, uint32_t *rgba, int w, int h, float dt) { (void)ach; (void)rgba; (void)w; (void)h; (void)dt; }
