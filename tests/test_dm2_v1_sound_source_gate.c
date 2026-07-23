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

    assert(dm2_v1_sound_query_entry(0x0E, 0U, DM2_SOUND_STD_EXPLOSION) == -1);
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
    assert(dm2_v1_skproject_get_music_index_from_modlist(
        music_map, 4, 2, &receipt) == 6);
    assert(receipt.valid);
    assert(receipt.argument0 == 2);
    assert(receipt.selected_music_track == 6);
    assert(dm2_v1_skproject_get_music_index_from_modlist(
        music_map, 4, 9, &receipt) == 0);
    assert(receipt.valid);
    assert(receipt.selected_music_track == 0);
    assert(dm2_v1_skproject_get_music_index_from_modlist(
        0, 4, 1, &receipt) == 0);
    assert(receipt.valid);
    assert(dm2_v1_skproject_sound2(&state, 2, music_map, 4, &receipt) == 1);
    assert(receipt.selected_music_track == 6);
    assert(state.current_music_track == 6);
    assert(dm2_v1_skproject_sound2(&state, 9, music_map, 4, &receipt) == 0);

    assert(dm2_v1_skproject_sound9(&state, 7, 8, 9, &receipt) == 1);
    assert(dm2_v1_skproject_process_sound(&state, 0, 9, 1, &receipt) == 1);
    assert(receipt.queue_noise_requested);

    state.midi_active_voice_mask = 0x85u;
    assert(dm2_v1_skproject_sound_midi_program(&state, 64u, &receipt) == 1);
    assert(state.midi_program == 64u);
    assert(receipt.refresh_count == 3u);
    assert(dm2_v1_skproject_sound_discard_midi_word(&receipt, 0x5a5au) == 1);
    assert(receipt.argument0 == 0x5a5a);

    state.midi_voices[3].l_00 = 12345;
    state.midi_voices[3].w_04 = 67;
    assert(dm2_v1_skproject_sound_reset_midi_voice(&state, 3, &receipt) == 1);
    assert(receipt.reset_count == 1u);
    assert(state.midi_voices[3].l_00 == 0);
    assert(state.midi_voices[3].w_04 == 0);
    assert(dm2_v1_skproject_sound_reset_midi_voice(&state, 8, &receipt) == 0);

    state.midi_voices[2].l_00 = 99;
    state.midi_voices[2].w_04 = 11;
    state.midi_selected_voice = 2u;
    state.midi_stop_armed = 1;
    state.midi_defer_stop = 1;
    state.midi_fade_counter = 127;
    assert(dm2_v1_skproject_sound_stop_armed_music(&state, &receipt) == 1);
    assert(receipt.stop_music_requested);
    assert(receipt.reset_count == 1u);
    assert(state.midi_voices[2].l_00 == 0);
    assert(state.midi_voices[2].w_04 == 0);
    assert(state.midi_stop_armed == 0);
    assert(state.midi_defer_stop == 0);
    assert(state.midi_fade_counter == 0);

    DM2_V1_SkprojectSfx front = { 0 };
    front.ub_04 = 3u;
    front.ub_05 = 80u;
    front.ub_06 = 0u;
    front.ub_07 = 4u;
    assert(dm2_v1_skproject_sound_compute_sfx_metric(&front, &receipt) == 1);
    assert(front.s59_08.ub_00 == 26u);
    assert(front.s59_08.w_01 == 0x8000u);
    assert(receipt.attenuation == 26u);
    assert(receipt.bearing == 0x8000u);

    DM2_V1_SkprojectSfx side = { 0 };
    side.ub_04 = 3u;
    side.ub_05 = 80u;
    side.ub_06 = 4u;
    side.ub_07 = 0u;
    assert(dm2_v1_skproject_sound_compute_sfx_metric(&side, &receipt) == 1);
    assert(side.s59_08.ub_00 == 26u);
    assert(side.s59_08.w_01 == 0xf800u);

    DM2_V1_SkprojectSfx diagonal = { 0 };
    diagonal.ub_04 = 2u;
    diagonal.ub_05 = 80u;
    diagonal.ub_06 = 4u;
    diagonal.ub_07 = 4u;
    assert(dm2_v1_skproject_sound_compute_sfx_metric(&diagonal, &receipt) == 1);
    assert(diagonal.s59_08.ub_00 == 16u);
    assert(diagonal.s59_08.w_01 == 0xcb00u);
    assert(dm2_v1_skproject_sound_sfx_precedes(&front, &diagonal) == 1);
    assert(dm2_v1_skproject_sound_sfx_precedes(&diagonal, &front) == 0);
    side.ub_04 = front.ub_04;
    side.s59_08.ub_00 = (uint8_t)(front.s59_08.ub_00 - 1u);
    assert(dm2_v1_skproject_sound_sfx_precedes(&front, &side) == 1);

    assert(dm2_v1_skproject_sound_sample_state(0) == 1);
    assert(dm2_v1_skproject_sound_sample_state(31) == 1);
    assert(dm2_v1_skproject_sound_sample_state(32) == 10);
    assert(dm2_v1_skproject_sound_active_sample_slots(&state) == 0u);
    assert(dm2_v1_skproject_sound_drain_samples(&state, &receipt) == 1);
    assert(receipt.scanned_count == 0u);
    state.active_sample_handles[0] = 3;
    state.active_sample_handles[7] = 31;
    assert(dm2_v1_skproject_sound_stop_handle_table(&state, &receipt) == 1);
    assert(receipt.scanned_count == 2u);
    assert(dm2_v1_skproject_sound_stop_sample_slot(&state, 31, &receipt) == 1);
    assert(dm2_v1_skproject_sound_stop_sample_slot(&state, 32, &receipt) == 0);

    DM2_V1_SkprojectSoundAllocationNode nodes[4] = {
        { 2u, 1 },
        { 0xffffu, 1 },
        { 3u, 1 },
        { 0xffffu, 1 }
    };
    state.active_sample_handles[0] = 5;
    state.active_sample_handles[1] = -1;
    state.active_sample_handles[7] = 12;
    assert(dm2_v1_skproject_sound_release_allocation_node(
        &state, nodes, 4, 0, 2, &receipt) == 1);
    assert(receipt.valid);
    assert(receipt.scanned_count == 2u);
    assert(receipt.returned_index == 3u);
    assert(receipt.argument2 == 3);
    assert(dm2_v1_skproject_sound_release_allocation_node(
        &state, nodes, 4, 0, 1, &receipt) == 0);
    assert(receipt.scanned_count == 3u);
    nodes[2].present = 0;
    assert(dm2_v1_skproject_sound_release_allocation_node(
        &state, nodes, 4, 0, 2, &receipt) == 0);

    state.pending_immediate_count = 4;
    state.pending_positional_count = 5;
    state.pending_music_track = 7;
    state.current_music_track = 7;
    state.midi_stop_armed = 1;
    assert(dm2_v1_skproject_sound_stop_all(&state, &receipt) == 1);
    assert(receipt.stop_sfx_requested);
    assert(receipt.stop_music_requested);
    assert(state.pending_immediate_count == 0);
    assert(state.pending_positional_count == 0);
    assert(state.pending_music_track == -1);
    assert(state.current_music_track == -1);
    assert(state.midi_stop_armed == 0);

    assert(dm2_v1_skproject_sound_destruct(&receipt) == 1);
    assert(receipt.uninstall_audio_requested);
    assert(dm2_v1_skproject_sound6_sndptr6_allocation(4096u, &receipt) == 1);
    assert(receipt.allocation_requested);
    assert(receipt.allocation_size == 4096u);
    assert(dm2_v1_skproject_sound6_sndptr6_allocation(0u, &receipt) == 1);
    assert(!receipt.allocation_requested);
    assert(receipt.allocation_size == 0u);
    return 0;
}
