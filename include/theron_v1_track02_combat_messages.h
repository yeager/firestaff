#ifndef THERON_V1_TRACK02_COMBAT_MESSAGES_H
#define THERON_V1_TRACK02_COMBAT_MESSAGES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Combat and system feedback messages from various ROM locations.
 *
 * Item condition adjectives from UD 0x0865D4.
 * Combat feedback (coin flip, reach, ammo) from UD 0x089A32 region.
 * System messages (WAKE UP, GAME FROZEN) from UD 0x082E13 region.
 * Resurrection message from UD 0x086E70.
 * Theron-specific message from UD 0x08BBBC region.
 * File select menu from UD 0x27519B.
 * Super CD-ROM2 requirement from UD 0x26C39D. */

/* Item condition adjectives (UD 0x0865D4) */
#define THERON_TRACK02_ITEM_CONDITION_COUNT  7u
const char *theron_v1_track02_item_condition(unsigned int index);

/* Combat feedback messages — coin flip, range, ammo (UD 0x089A32 region) */
#define THERON_TRACK02_COMBAT_FEEDBACK_COUNT  5u
const char *theron_v1_track02_combat_feedback(unsigned int index);

/* System/misc messages */
const char *theron_v1_track02_system_wake_up(void);
const char *theron_v1_track02_system_game_frozen(void);
const char *theron_v1_track02_system_resurrected(void);
const char *theron_v1_track02_system_pass(void);
const char *theron_v1_track02_resurrect_theron(void);

/* File select menu (UD 0x27519B) */
const char *theron_v1_track02_file_select_play(void);

/* Super CD-ROM2 requirement (UD 0x26C39D) */
const char *theron_v1_track02_cdrom2_requirement(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_COMBAT_MESSAGES_H */
