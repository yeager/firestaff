#ifndef FIRESTAFF_DM2_V1_CHANGE_PLAYER_POS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CHANGE_PLAYER_POS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_CHANGE_PLAYER_POS (c_hero.cpp:4037) ----
 * Reorder party formation: the low byte packs a source slot (0..4), the high
 * bit suppresses the dragged hero's absdir update. With no drag pending,
 * the source starts an icon drag and records the source slot; it does not
 * change formation. With a drag pending, the next slot click commits the
 * position exchange (or moves into an empty slot) and clears the pending
 * drag. Cursor animation remains a caller-owned UI effect. */
typedef struct {
    int drag_in_progress;      /* ddat.v1e00b8 != 0 */
    int drag_source_position;  /* ddat.v1e00b8 - 1, source slot 0..4 */
} DM2_V1_ChangePlayerPosState;

typedef struct {
    int (*get_player_at_position)(void *ctx, int position);
    void (*begin_drag)(void *ctx, int source_position);
    void (*clear_drag)(void *ctx);
    void (*set_position)(void *ctx, int hero_idx, int8_t position);
    void (*set_direction)(void *ctx, int hero_idx, int8_t direction);
    int8_t (*get_party_facing)(void *ctx);
} DM2_V1_ChangePlayerPosCallbacks;

void dm2_v1_change_player_pos(
    int16_t packed_pos, const DM2_V1_ChangePlayerPosState *state,
    const DM2_V1_ChangePlayerPosCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
