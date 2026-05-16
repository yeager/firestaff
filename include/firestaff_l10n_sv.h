
#ifndef FIRESTAFF_L10N_SV_H
#define FIRESTAFF_L10N_SV_H

/* Swedish (sv) in-game text for DM1/CSB/DM2.
 * New Firestaff translation — Swedish was never an official DM1 language. */

const char *fs_sv_base_skill_name(int index);
const char *fs_sv_skill_level(int level);
const char *fs_sv_direction(int dir);
const char *fs_sv_item_attribute(int attr);
const char *fs_sv_viewport_text(int id);
const char *fs_sv_action_name(int id);
const char *fs_sv_entrance(int id);

/* Viewport text IDs */
#define FS_SV_WAKE_UP      0
#define FS_SV_GAME_FROZEN  1
#define FS_SV_REST_IN_PEACE 2
#define FS_SV_CONGRATULATIONS 3
#define FS_SV_BURNT_OUT    4
#define FS_SV_NO_CREATURE  5

#endif
