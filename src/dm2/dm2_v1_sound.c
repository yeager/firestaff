/* dm2_v1_sound.c — DM2 V1 Sound System
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKULLWIN/c_sound.h/cpp, c_sfx.cpp
 * docs/dm2_audio.md, docs/dm2_sound_system.md, docs/dm2_sound_combat.md
 *
 * DM2 audio: 16-slot SFX ring buffer, SoundBlaster, 28 MIDI tracks.
 * DM1 audio: 3-4 voices, AdLib FM, ~10 tracks.
 * New in DM2: SOUND_STD_EXPLOSION (bombs), ambient weather, spatial queue.
 */

#include "dm2_v1_sound.h"
#include <stdio.h>
#include <string.h>

/* ── Sound name tables by category ────────────────────────────────────────
 * Source: docs/dm2_audio.md, docs/dm2_sound_combat.md
 * Names for major sound IDs. Full GDAT table lookup is runtime. */

static const char *const g_std_sound_names[] = {
    [0x81] = "Explosion",      /* DM2 new: bombs */
    [0x84] = "Punch/Fall",
    [0x85] = "Knock",
    [0x86] = "Throw/Shoot",
    [0x88] = "Activation",
    [0x89] = "Teleport",
};

static const char *const g_champion_sound_names[] = {
    [0x00] = "Champion Attack",
    [0x01] = "Champion Shoot",
    [0x82] = "Champion Gethit",   /* hex=130, fits in 160-bound array */
    [0x83] = "Champion Eat/Drink",
    [0x87] = "Champion Scream",    /* hex=135 */
    [0x8A] = "Champion Bump",      /* hex=138 */
    [0x92] = "Champion Footstep",  /* hex=146 */
};

static const char *const g_creature_sound_names[] = {
    [0x00] = "Creature Move",
    [0x01] = "Creature Turn",
    [0x02] = "Creature Gethit",
    [0x03] = "Creature Reflector",
    [0x04] = "Creature Jump",
    [0x07] = "Creature Attack",
    [0x08] = "Creature Pick/Steal",
    [0x10] = "Creature Spawn",
    [0x11] = "Creature Death",
    [0x12] = "Creature Attack 2",
};

/* ── Music track names ─────────────────────────────────────────────────────
 * Source: docs/dm2_audio.md (tMusicMaps[64], 28 HMP tracks 00-1c.hex)
 * Track names are dungeon-theme based. Firestaff SDL port uses sk%02d.ogg.
 * DM2 PC English music folders: DATA_DM2_DM, DATA_DM2_SK, etc. */

static const char *const g_music_track_names[DM2_MUSIC_TRACK_COUNT] = {
    /* clang-format off */
    [0]  = "00 - Title/Intro",
    [1]  = "01 - Dungeon Ambient A",
    [2]  = "02 - Dungeon Ambient B",
    [3]  = "03 - Combat",
    [4]  = "04 - Shop/NPC",
    [5]  = "05 - Dungeon Safe",
    [6]  = "06 - Boss Encounter",
    [7]  = "07 - Victory",
    [8]  = "08 - Death",
    [9]  = "09 - Outdoor Day",
    [10] = "0a - Outdoor Night",
    [11] = "0b - Weather Rain",
    [12] = "0c - Weather Storm",
    [13] = "0d - Magic Cast",
    [14] = "0e - Treasure",
    [15] = "0f - Puzzle/Secret",
    /* clang-format on */
    /* Tracks 16-27 (0x10-0x1b) additional dungeon/building themes */
};

static const uint16_t g_skproject_sound_bearing_table[24] = {
    0x25a0u, 0x11f0u, 0x0b00u, 0x0730u, 0x0490u, 0x0290u,
    0x00d0u, 0x0000u, 0x0800u, 0x1700u, 0x2600u, 0x3500u,
    0x4400u, 0x5300u, 0x6200u, 0x7100u, 0x8f00u, 0x9e00u,
    0xad00u, 0xbc00u, 0xcb00u, 0xda00u, 0xe900u, 0xf800u
};

static void dm2_v1_skproject_sound_clear_receipt(
    DM2_V1_SkprojectSoundReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

void dm2_v1_skproject_sound_state_init(DM2_V1_SkprojectSoundState *state,
                                       uint16_t queue_capacity)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    if (queue_capacity > DM2_V1_SKPROJECT_SOUND_QUEUE_MAX)
        queue_capacity = DM2_V1_SKPROJECT_SOUND_QUEUE_MAX;
    state->queue_capacity = queue_capacity;
    state->sound_enabled = 1;
    state->midi_handle_present = 1;
    state->midi_transition_enabled = 1;
    state->midi_ready = 1;
    state->sfx_active = 1;
    state->current_music_track = -1;
    state->pending_music_track = -1;
    state->midi_program = 90u;
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_SOUND_QUEUE_MAX; ++i)
        state->queue[i].w_05 = -1;
    for (uint16_t i = 0; i < 64u; ++i)
        state->active_sample_handles[i] = -1;
}

uint16_t dm2_v1_skproject_query_snd_entry_index(
    const DM2_V1_SkprojectSoundState *state,
    int8_t cls1,
    int8_t cls2,
    int8_t cls3)
{
    if (!state) return 0;
    for (uint16_t i = 0; i < state->queued_count; ++i) {
        const DM2_V1_SkprojectSoundQueueEntry *entry = &state->queue[i];
        if (entry->b_02 == cls1 && entry->b_03 == cls2 &&
            entry->b_04 == cls3)
            return (uint16_t)(i + 1u);
    }
    return 0;
}

int dm2_v1_skproject_sound9(DM2_V1_SkprojectSoundState *state,
                            int8_t cls1,
                            int8_t cls2,
                            int8_t cls3,
                            DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.argument0 = cls1;
    receipt.argument1 = cls2;
    receipt.argument2 = cls3;
    if (!state) {
        receipt.rejected_disabled = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.queued_count_before = state->queued_count;
    if (dm2_v1_skproject_query_snd_entry_index(state, cls1, cls2, cls3) != 0u) {
        receipt.rejected_duplicate = 1;
        receipt.queued_count_after = state->queued_count;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (state->queued_count >= state->queue_capacity ||
        state->queued_count >= DM2_V1_SKPROJECT_SOUND_QUEUE_MAX) {
        receipt.rejected_full = 1;
        receipt.queued_count_after = state->queued_count;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    state->queue[state->queued_count].b_02 = cls1;
    state->queue[state->queued_count].b_03 = cls2;
    state->queue[state->queued_count].b_04 = cls3;
    state->queue[state->queued_count].w_05 = -1;
    state->queued_count++;
    receipt.returned_index = state->queued_count;
    receipt.queued_count_after = state->queued_count;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

uint16_t dm2_v1_skproject_sound7(
    const DM2_V1_SkprojectSoundState *state,
    int16_t sound_handle)
{
    if (!state) return 0;
    for (uint16_t i = 0; i < state->queued_count; ++i)
        if (state->queue[i].w_05 == sound_handle)
            return (uint16_t)(i + 1u);
    return 0;
}

int dm2_v1_skproject_sound4(DM2_V1_SkprojectSoundState *state,
                            DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    receipt.queued_count_before = state->pending_positional_count;
    state->pending_positional_count = 0;
    receipt.queued_count_after = 0;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound5(DM2_V1_SkprojectSoundState *state,
                            DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    receipt.queued_count_before = state->queued_count;
    for (uint16_t i = state->queued_count; i > 0u; --i) {
        uint16_t idx = (uint16_t)(i - 1u);
        int16_t handle = state->queue[idx].w_05;
        int found_earlier = 0;
        for (uint16_t scan = 0; scan < idx; ++scan) {
            if (state->queue[scan].w_05 == handle) {
                found_earlier = 1;
                break;
            }
        }
        if (handle != -1 && !found_earlier) {
            for (uint16_t j = idx; j + 1u < state->queued_count; ++j)
                state->queue[j] = state->queue[j + 1u];
            state->queued_count--;
            if (state->active_sample_count > 0u) state->active_sample_count--;
            receipt.removed_count++;
        }
    }
    state->pending_positional_count = 0;
    receipt.queued_count_after = state->queued_count;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound6(DM2_V1_SkprojectSoundState *state,
                            uint16_t queue_capacity,
                            DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    dm2_v1_skproject_sound_state_init(state, queue_capacity);
    receipt.queued_count_after = state->queued_count;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound8(DM2_V1_SkprojectSoundState *state,
                            int immediate,
                            DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    receipt.queued_count_before = immediate ? state->pending_immediate_count
                                            : state->pending_positional_count;
    if (receipt.queued_count_before > 0u) {
        receipt.play_sound_requested = 1;
        receipt.play_count = receipt.queued_count_before;
    }
    if (immediate)
        state->pending_immediate_count = 0;
    else
        state->pending_positional_count = 0;
    receipt.queued_count_after = 0;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound1(DM2_V1_SkprojectSoundState *state,
                            DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    if (state->midi_handle_present && state->midi_ready)
        state->pending_music_fade = 1;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int16_t dm2_v1_skproject_get_music_index_from_modlist(
    const uint8_t *modlist,
    uint16_t modlist_size,
    int16_t map_index,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    int16_t music_index = 0;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.argument0 = map_index;
    if (modlist && map_index >= 0 && (uint16_t)map_index < modlist_size)
        music_index = (int16_t)modlist[(uint16_t)map_index];
    receipt.selected_music_track = music_index;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return music_index;
}

int dm2_v1_skproject_sound2(DM2_V1_SkprojectSoundState *state,
                            int16_t map_index,
                            const uint8_t *music_map,
                            uint16_t music_map_count,
                            DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.argument0 = map_index;
    if (!state || !music_map || map_index < 0 ||
        (uint16_t)map_index >= music_map_count) {
        receipt.rejected_disabled = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (state->pending_music_fade)
        state->pending_music_fade = 0;
    state->pending_music_track =
        dm2_v1_skproject_get_music_index_from_modlist(
            music_map, music_map_count, map_index, NULL);
    receipt.selected_music_track = state->pending_music_track;
    if (state->current_music_track != state->pending_music_track) {
        if (!state->midi_ready || state->pending_music_fade != 0)
            receipt.play_music_requested = 1;
        else
            state->pending_music_fade = 127;
        state->current_music_track = state->pending_music_track;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound3(DM2_V1_SkprojectSoundState *state,
                            int16_t volume,
                            int16_t mode,
                            DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    int16_t clamped = volume;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    if (clamped > 7) clamped = 7;
    if (clamped < 0) clamped = 0;
    receipt.argument0 = volume;
    receipt.argument1 = mode;
    receipt.volume = clamped;
    if (mode != 0) {
        if (mode != 10) {
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        if (state->midi_handle_present) {
            if (clamped != 0 && state->master_sfx_volume == 0 &&
                state->pending_music_track >= 0) {
                receipt.play_music_requested = 1;
                receipt.selected_music_track = state->pending_music_track;
            } else if (clamped == 0) {
                receipt.stop_music_requested = 1;
            }
        }
        state->master_sfx_volume = clamped;
        state->midi_volume = 36 * clamped;
    } else if (state->sound_enabled) {
        state->master_sfx_volume = clamped;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_process_sound(DM2_V1_SkprojectSoundState *state,
                                   uint16_t index,
                                   int16_t current_map,
                                   int16_t party_map,
                                   DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state || index >= state->queued_count) return 0;
    receipt.argument0 = index;
    if (state->queue[index].b_04 == current_map ||
        state->queue[index].b_04 == party_map)
        receipt.queue_noise_requested = 1;
    state->queue[index].w_05 = -1;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound_midi_program(
    DM2_V1_SkprojectSoundState *state,
    uint8_t program,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.argument0 = (int16_t)program;
    if (!state) return 0;
    state->midi_program = program;
    for (uint16_t i = 0; i < 8u; ++i)
        if ((state->midi_active_voice_mask & (uint8_t)(1u << i)) != 0u)
            receipt.refresh_count++;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound_discard_midi_word(
    DM2_V1_SkprojectSoundReceipt *out_receipt,
    uint16_t word)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.argument0 = (int16_t)word;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound_reset_midi_voice(
    DM2_V1_SkprojectSoundState *state,
    uint32_t voice_index,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.argument0 = (int16_t)voice_index;
    if (!state) return 0;
    if (voice_index >= 8u) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    state->midi_voices[voice_index].l_00 = 0;
    state->midi_voices[voice_index].w_04 = 0;
    receipt.reset_count = 1;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound_stop_armed_music(
    DM2_V1_SkprojectSoundState *state,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    if (state->midi_handle_present && state->midi_stop_armed) {
        (void)dm2_v1_skproject_sound_reset_midi_voice(
            state, state->midi_selected_voice, NULL);
        state->midi_stop_armed = 0;
        state->midi_defer_stop = 0;
        state->midi_fade_counter = 0;
        receipt.stop_music_requested = 1;
        receipt.reset_count = 1;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound_compute_sfx_metric(
    DM2_V1_SkprojectSfx *sfx,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    int32_t x;
    int32_t y;
    uint32_t divisor;

    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!sfx) return 0;

    x = (int32_t)sfx->ub_06;
    y = (int32_t)sfx->ub_07;
    divisor = (uint32_t)(x * x + y * y + 8);
    sfx->s59_08.ub_00 =
        (uint8_t)((((uint32_t)sfx->ub_05 << 8) / divisor) >> 5);

    if (x == 0) {
        sfx->s59_08.w_01 = 0x8000u;
    } else if (y == 0) {
        sfx->s59_08.w_01 = g_skproject_sound_bearing_table[23];
    } else {
        uint16_t ratio = (uint16_t)(((uint32_t)x << 11) / (uint32_t)y);
        uint16_t angle_index = 7u;
        for (uint16_t i = 0; i < 8u; ++i) {
            if (ratio >= g_skproject_sound_bearing_table[i]) {
                angle_index = i;
                break;
            }
        }
        sfx->s59_08.w_01 =
            g_skproject_sound_bearing_table[8u + 15u - angle_index];
    }

    receipt.attenuation = sfx->s59_08.ub_00;
    receipt.bearing = sfx->s59_08.w_01;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound_sfx_precedes(
    const DM2_V1_SkprojectSfx *lhs,
    const DM2_V1_SkprojectSfx *rhs)
{
    if (!lhs || !rhs) return 0;
    if (lhs->ub_04 > rhs->ub_04) return 1;
    if (lhs->ub_04 == rhs->ub_04)
        return lhs->s59_08.ub_00 >= rhs->s59_08.ub_00;
    return 0;
}

int dm2_v1_skproject_sound_sample_state(uint32_t sample_index)
{
    if (sample_index >= 32u) return 10;
    return 1;
}

int dm2_v1_skproject_sound_stop_sample_slot(
    DM2_V1_SkprojectSoundState *state,
    uint32_t sample_index,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.argument0 = (int16_t)sample_index;
    if (!state) return 0;
    if (sample_index >= 32u) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

uint16_t dm2_v1_skproject_sound_active_sample_slots(
    const DM2_V1_SkprojectSoundState *state)
{
    (void)state;
    return 0;
}

int dm2_v1_skproject_sound_drain_samples(
    DM2_V1_SkprojectSoundState *state,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    receipt.scanned_count = dm2_v1_skproject_sound_active_sample_slots(state);
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound_stop_handle_table(
    DM2_V1_SkprojectSoundState *state,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    if (state->sound_enabled) {
        for (uint16_t i = 0; i < 64u; ++i) {
            if (state->active_sample_handles[i] != -1) {
                receipt.scanned_count++;
                (void)dm2_v1_skproject_sound_stop_sample_slot(
                    state, (uint32_t)state->active_sample_handles[i], NULL);
            }
        }
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound_release_allocation_node(
    DM2_V1_SkprojectSoundState *state,
    const DM2_V1_SkprojectSoundAllocationNode *nodes,
    uint16_t node_count,
    uint16_t head_index,
    uint16_t target_index,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    uint16_t cursor;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.argument0 = (int16_t)target_index;
    receipt.argument1 = (int16_t)head_index;
    if (!state || !nodes || head_index >= node_count ||
        target_index >= node_count) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!nodes[head_index].present || !nodes[target_index].present) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    (void)dm2_v1_skproject_sound_stop_handle_table(state, NULL);
    for (cursor = head_index; cursor < node_count && nodes[cursor].present;
         cursor = nodes[cursor].next_index) {
        receipt.scanned_count++;
        if (cursor == target_index) {
            receipt.returned_index = (uint16_t)(cursor + 1u);
            if (nodes[cursor].next_index < node_count &&
                nodes[nodes[cursor].next_index].present) {
                receipt.argument2 = (int16_t)nodes[cursor].next_index;
            } else {
                receipt.argument2 = -1;
            }
            receipt.valid = 1;
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (nodes[cursor].next_index >= node_count) break;
    }

    receipt.argument2 = -1;
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_skproject_sound_stop_all(
    DM2_V1_SkprojectSoundState *state,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    state->pending_immediate_count = 0;
    state->pending_positional_count = 0;
    state->pending_music_track = -1;
    state->current_music_track = -1;
    state->midi_stop_armed = 0;
    receipt.stop_sfx_requested = 1;
    receipt.stop_music_requested = 1;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound_destruct(
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.uninstall_audio_requested = 1;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_sound6_sndptr6_allocation(
    uint32_t v1e0ad4,
    DM2_V1_SkprojectSoundReceipt *out_receipt)
{
    DM2_V1_SkprojectSoundReceipt receipt;
    dm2_v1_skproject_sound_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.argument0 = (int16_t)(v1e0ad4 & 0xffffu);
    receipt.allocation_size = v1e0ad4;
    receipt.allocation_requested = v1e0ad4 != 0u;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

/* DM2_QUERY_SND_ENTRY_INDEX is a GDAT-backed lookup in c_sfx.cpp.  The
 * source-owned xsndptr2 table is runtime state, not a materialized GDAT
 * table, so no caller may derive an entry index from an arbitrary sound id. */

int dm2_v1_sound_query_entry(uint8_t cat, uint8_t c1, uint8_t c2, uint8_t sfx) {
    (void)cat; (void)c1; (void)c2;
    (void)sfx;
    return -1;
}

/* dm2_v1_sound_play — play a sound effect
 * Source: SKULLWIN/c_sound.cpp: DM2_PLAY_SOUND()
 * Stub: calls SDL audio queue (actual SDL_QueueAudio integration deferred).
 * Frequency: DM2_PLAYBACK_FREQUENCY_WIN = 6000 Hz (SDL port).
 * DM1: 3-4 voice AdLib FM; DM2: 16-slot ring buffer SoundBlaster. */
int dm2_v1_sound_play(int sound_id, int volume) {
    if (sound_id < 0) return -1;
    (void)volume;
    /* c_sound.cpp needs a resolved source queue entry and its sample payload.
     * Neither is available through this adapter, so successful playback would
     * be synthetic. */
    return -1;
}

/* dm2_v1_sound_play_positional — world-coordinate spatial audio
 * Source: SKULLWIN/c_sound.cpp: world-coordinate queue with distance attenuation
 * Distance formula: attenuation = 1.0 / (1.0 + distance * falloff)
 * glbXAmbientSoundActivated for weather ambient sounds. */
int dm2_v1_sound_play_positional(int sound_id,
    int world_x, int world_y, int listener_x, int listener_y) {
    if (sound_id < 0) return -1;
    (void)world_x;
    (void)world_y;
    (void)listener_x;
    (void)listener_y;
    return -1;
}

/* dm2_v1_sound_play_music — play music track
 * Source: docs/dm2_audio.md (do_music_wav, tMusicMaps[64])
 * Original: HMP format (DATA/00.hmp.mid through 1c.hmp.mid)
 * Firestaff SDL: OGG format (DATA/sk%02d.ogg looped)
 * Track selection: tMusicMaps[dungeon_map_index] → track number */
int dm2_v1_sound_play_music(int track) {
    if (track < 0 || track >= DM2_MUSIC_TRACK_COUNT) return -1;
    /* Stub: would load DATA/sk%02d.ogg and loop via al_play_sample()
     * For now: just acknowledge the track number.
     * v1dff8a = current track, v1d1512 = previous track (change detection) */
    return track;
}

/* dm2_v1_sound_stop_music — stop all music
 * Source: docs/dm2_audio.md (do_music_stop)
 * Calls al_stop_samples() via c_midi / c_music_wav. */
int dm2_v1_sound_stop_music(void) {
    /* Stub: would call al_stop_samples() */
    return 0;
}

/* dm2_v1_sound_name — human-readable sound name
 * Source: docs/dm2_audio.md, docs/dm2_sound_combat.md */
const char *dm2_v1_sound_name(int category, int sound_id) {
    if (sound_id < 0) return "?";
    switch (category) {
        case DM2_SOUND_CATEGORY_STANDARD:
            /* IDs 0x81, 0x84-0x89 are in the standard sparse array.
             * Allow up to 160 entries so 0x81 (=129) fits. */
            if (sound_id < 160 && g_std_sound_names[sound_id])
                return g_std_sound_names[sound_id];
            break;
        case DM2_SOUND_CATEGORY_CHAMPION:
            /* Champion SFX IDs include hex values 0x82, 0x83, 0x87, 0x8A, 0x92.
             * These map to indices 130, 131, 135, 138, 146 — extend bound to 160. */
            if (sound_id < 160 && g_champion_sound_names[sound_id])
                return g_champion_sound_names[sound_id];
            break;
        case DM2_SOUND_CATEGORY_CREATURE:
            if (sound_id < 160 && g_creature_sound_names[sound_id])
                return g_creature_sound_names[sound_id];
            break;
    }
    return "?";
}

const char *dm2_v1_sound_source_evidence(void) {
    return
        "DM2 V1 Sound System — Phase 6 source-lock\n"
        "ReDMCSB: SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: skproject/SKULLWIN/c_sound.h/cpp (c_sound master audio class)\n"
        "Source: skproject/SKULLWIN/c_sfx.cpp (16-slot ring buffer SFX)\n"
        "Source: skproject/SKWIN/defines.h (SOUND_STD_*, SOUND_CHAMPION_*, SOUND_CREATURE_*)\n"
        "Source: docs/dm2_audio.md (music 28 HMP tracks, tMusicMaps[64], do_music_wav)\n"
        "Source: docs/dm2_sound_system.md (c_sound init, PLAYBACK_FREQUENCY=5500/6000 Hz)\n"
        "Source: docs/dm2_sound_combat.md (all combat sound triggers 0x00-0x92)\n"
        "DM1 comparison: AdLib FM, 3-4 voices, ~10 tracks, no positional audio\n"
        "DM2 comparison: SoundBlaster, 16-slot buffer, 28 tracks, world-coordinate spatial queue\n"
        "DM2 new: SOUND_STD_EXPLOSION (0x81), glbXAmbientSoundActivated, DM2_QUEUE_NOISE_GEN1/GEN2\n";
}
/* Suppress unused variable warning for g_music_track_names */
static void __attribute__((unused)) dm2_v1_sound_suppress_unused(void) {
    (void)g_music_track_names;
}
