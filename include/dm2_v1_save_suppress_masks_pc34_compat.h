#ifndef DM2_V1_SAVE_SUPPRESS_MASKS_PC34_COMPAT_H
#define DM2_V1_SAVE_SUPPRESS_MASKS_PC34_COMPAT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 save-game SUPPRESS masks for the top-level save orchestrator.
 * Source: dm2data.cpp tables. */

/* Save game buffer mask (table1d631a, 60 bytes). */
const uint8_t *dm2_v1_save_mask_savegame_buffer(void);
#define DM2_SAVE_BUFFER_MASK_SIZE 60

/* Hero struct mask (table1d6356, 263 bytes = sizeof(c_hero)). */
const uint8_t *dm2_v1_save_mask_hero(void);
#define DM2_SAVE_HERO_MASK_SIZE 263

/* Save state mask (table1d645d, 6 bytes). */
const uint8_t *dm2_v1_save_mask_save_state(void);
#define DM2_SAVE_STATE_MASK_SIZE 6

/* Timer mask (vsgame[0..], first bytes of the vsgame array).
 * Alias for the first N bytes of the vsgame raw array. */
const uint8_t *dm2_v1_save_mask_timer(void);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_SUPPRESS_MASKS_PC34_COMPAT_H */
