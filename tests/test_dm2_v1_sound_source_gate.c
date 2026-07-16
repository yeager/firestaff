#include "dm2_v1_sound.h"

#include <assert.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    DM2_V1_SkprojectSoundState state;
    DM2_V1_SkprojectSoundReceipt receipt;
    uint8_t music_map[4] = { 2u, 4u, 6u, 8u };

    assert(dm2_v1_sound_query_entry(DM2_SOUND_CATEGORY_STANDARD, 0U, 0U,
                                    DM2_SOUND_STD_EXPLOSION) == -1);
    assert(dm2_v1_sound_play(DM2_SOUND_STD_EXPLOSION, 127) == -1);
    assert(dm2_v1_sound_play_positional(DM2_SOUND_STD_EXPLOSION,
                                        1, 2, 3, 4) == -1);
    assert(dm2_v1_sound_play(-1, 0) == -1);

    dm2_v1_skproject_sound_state_init(&state, 2);
    assert(state.queue_capacity == 2);
    assert(state.current_music_track == -1);
    assert(dm2_v1_skproject_sound9(&state, 1, 2, 3, &receipt) == 1);
    assert(receipt.valid && receipt.returned_index == 1);
    assert(dm2_v1_skproject_query_snd_entry_index(&state, 1, 2, 3) == 1);
    assert(dm2_v1_skproject_sound9(&state, 1, 2, 3, &receipt) == 0);
    assert(receipt.rejected_duplicate);
    assert(dm2_v1_skproject_sound9(&state, 1, 2, 4, &receipt) == 1);
    assert(dm2_v1_skproject_sound9(&state, 1, 2, 5, &receipt) == 0);
    assert(receipt.rejected_full);

    state.queue[0].w_05 = 44;
    assert(dm2_v1_skproject_sound7(&state, 44) == 1);
    assert(dm2_v1_skproject_sound7(&state, 45) == 0);
    assert(dm2_v1_skproject_sound5(&state, &receipt) == 1);
    assert(receipt.removed_count == 1);
    assert(state.queued_count == 1);

    state.pending_positional_count = 3;
    assert(dm2_v1_skproject_sound8(&state, 0, &receipt) == 1);
    assert(receipt.play_sound_requested && receipt.play_count == 3);
    assert(state.pending_positional_count == 0);
    state.pending_immediate_count = 2;
    assert(dm2_v1_skproject_sound8(&state, 1, &receipt) == 1);
    assert(receipt.play_sound_requested && receipt.play_count == 2);
    assert(state.pending_immediate_count == 0);

    state.pending_positional_count = 5;
    assert(dm2_v1_skproject_sound4(&state, &receipt) == 1);
    assert(state.pending_positional_count == 0);

    assert(dm2_v1_skproject_sound6(&state, 4, &receipt) == 1);
    assert(state.queue_capacity == 4 && state.queued_count == 0);
    assert(dm2_v1_skproject_sound3(&state, 9, 0, &receipt) == 1);
    assert(receipt.volume == 7 && state.master_sfx_volume == 7);
    state.pending_music_track = 6;
    state.master_sfx_volume = 0;
    assert(dm2_v1_skproject_sound3(&state, 3, 10, &receipt) == 1);
    assert(receipt.play_music_requested && receipt.selected_music_track == 6);
    assert(state.midi_volume == 108);
    assert(dm2_v1_skproject_sound3(&state, 0, 10, &receipt) == 1);
    assert(receipt.stop_music_requested);
    assert(dm2_v1_skproject_sound3(&state, 1, 5, &receipt) == 0);

    assert(dm2_v1_skproject_sound1(&state, &receipt) == 1);
    assert(state.pending_music_fade == 1);
    assert(dm2_v1_skproject_sound2(&state, 2, music_map, 4, &receipt) == 1);
    assert(receipt.selected_music_track == 6);
    assert(state.current_music_track == 6);
    assert(dm2_v1_skproject_sound2(&state, 9, music_map, 4, &receipt) == 0);

    assert(dm2_v1_skproject_sound9(&state, 7, 8, 9, &receipt) == 1);
    assert(dm2_v1_skproject_process_sound(&state, 0, 9, 1, &receipt) == 1);
    assert(receipt.queue_noise_requested);
    return 0;
}
