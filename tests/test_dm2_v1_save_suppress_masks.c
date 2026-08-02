/* Test DM2 save-game SUPPRESS masks. */
#include "dm2_v1_save_suppress_masks_pc34_compat.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Save game buffer mask. */
    const uint8_t *buf_mask = dm2_v1_save_mask_savegame_buffer();
    assert(buf_mask != NULL);
    assert(buf_mask[0] == 0xff);
    assert(buf_mask[3] == 0x00);
    assert(buf_mask[8] == 0x07);
    assert(buf_mask[59] == 0x00);

    /* Hero mask. */
    const uint8_t *hero_mask = dm2_v1_save_mask_hero();
    assert(hero_mask != NULL);
    assert(hero_mask[0] == 0x7f);
    assert(hero_mask[7] == 0x00);
    assert(hero_mask[262] == 0x00);

    /* Save state mask. */
    const uint8_t *state_mask = dm2_v1_save_mask_save_state();
    assert(state_mask != NULL);
    assert(state_mask[0] == 0x00);
    assert(state_mask[3] == 0xff);
    assert(state_mask[4] == 0xff);
    assert(state_mask[5] == 0x00);

    /* Timer mask (alias for vsgame[0..]). */
    const uint8_t *timer_mask = dm2_v1_save_mask_timer();
    assert(timer_mask != NULL);

    printf("PASS: dm2_v1_save_suppress_masks\n");
    return 0;
}
