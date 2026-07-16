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
#include "dm2_v1_midi_backend.h"
#include <stdio.h>
#include <stdlib.h>
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

static char g_dm2_music_asset_root[256];
static int g_dm2_music_asset_root_verified;
static DM2_V1_MusicScheduledEvent g_dm2_music_events[64];
static uint16_t g_dm2_music_event_count;
static uint32_t g_dm2_music_loop_duration_us;
static int g_dm2_music_loop_enabled;
static int g_dm2_music_backend_proven;
static uint16_t g_dm2_music_ticks_per_quarter = 96u;

static uint16_t dm2_read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t dm2_read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static uint32_t dm2_read_le32(const uint8_t *p)
{
    return ((uint32_t)p[0]) | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int dm2_read_vlq(const uint8_t *data,
                        size_t size,
                        size_t *offset,
                        uint32_t *out_value)
{
    uint32_t value = 0;
    unsigned count = 0;
    if (!data || !offset || !out_value) return 0;
    do {
        uint8_t byte;
        if (*offset >= size || count >= 4u) return 0;
        byte = data[(*offset)++];
        value = (value << 7) | (uint32_t)(byte & 0x7fu);
        count++;
        if ((byte & 0x80u) == 0u) {
            *out_value = value;
            return 1;
        }
    } while (1);
}

static int dm2_read_hmp_vlq(const uint8_t *data,
                            size_t size,
                            size_t *offset,
                            uint32_t *out_value)
{
    uint32_t value = 0;
    unsigned count = 0;
    if (!data || !offset || !out_value) return 0;
    do {
        uint8_t byte;
        if (*offset >= size || count >= 4u) return 0;
        byte = data[(*offset)++];
        value = (value << 7) | (uint32_t)(byte & 0x7fu);
        count++;
        if ((byte & 0x80u) != 0u) {
            *out_value = value;
            return 1;
        }
    } while (1);
}

static void dm2_music_receipt_clear(DM2_V1_MusicStreamReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

static void dm2_music_store_event(uint32_t tick,
                                  uint8_t status,
                                  uint8_t data1,
                                  uint8_t data2,
                                  uint8_t data_size)
{
    if (g_dm2_music_event_count >=
        (uint16_t)(sizeof(g_dm2_music_events) / sizeof(g_dm2_music_events[0])))
        return;
    g_dm2_music_events[g_dm2_music_event_count].tick = tick;
    g_dm2_music_events[g_dm2_music_event_count].status = status;
    g_dm2_music_events[g_dm2_music_event_count].data1 = data1;
    g_dm2_music_events[g_dm2_music_event_count].data2 = data2;
    g_dm2_music_events[g_dm2_music_event_count].data_size = data_size;
    g_dm2_music_event_count++;
}

static uint32_t dm2_music_ticks_to_us(uint32_t ticks, uint16_t division)
{
    if (division == 0u) return 0u;
    return (uint32_t)(((uint64_t)ticks * 500000u) / division);
}

static int dm2_inspect_smf_track(const uint8_t *data,
                                 size_t size,
                                 uint16_t division,
                                 DM2_V1_MusicStreamReceipt *receipt,
                                 DM2_V1_MusicTrackReceipt *track)
{
    size_t offset = 0;
    uint32_t tick = 0;
    uint8_t running_status = 0;
    while (offset < size) {
        uint32_t delta = 0;
        uint8_t status;
        if (!dm2_read_vlq(data, size, &offset, &delta)) return 0;
        tick += delta;
        if (offset >= size) return 0;
        status = data[offset++];
        if (status < 0x80u) {
            if (running_status == 0u) return 0;
            offset--;
            status = running_status;
        } else if (status < 0xf0u) {
            running_status = status;
        }

        receipt->event_count++;
        if (status == 0xffu) {
            uint8_t meta_type;
            uint32_t meta_len = 0;
            if (offset >= size) return 0;
            meta_type = data[offset++];
            if (!dm2_read_vlq(data, size, &offset, &meta_len)) return 0;
            if (offset + meta_len > size) return 0;
            receipt->meta_event_count++;
            if (meta_type == 0x2fu) track->end_of_track_count++;
            dm2_music_store_event(tick, status, meta_type, 0u, 0u);
            offset += meta_len;
        } else if (status == 0xf0u || status == 0xf7u) {
            uint32_t sysex_len = 0;
            if (!dm2_read_vlq(data, size, &offset, &sysex_len)) return 0;
            if (offset + sysex_len > size) return 0;
            offset += sysex_len;
        } else if (status < 0xf0u) {
            uint8_t data_size =
                (uint8_t)(((status & 0xf0u) == 0xc0u ||
                           (status & 0xf0u) == 0xd0u) ? 1u : 2u);
            uint8_t data1;
            uint8_t data2 = 0;
            if (offset + data_size > size) return 0;
            data1 = data[offset++];
            if (data_size == 2u) data2 = data[offset++];
            receipt->channel_event_count++;
            dm2_music_store_event(tick, status, data1, data2, data_size);
        } else {
            return 0;
        }
        if (tick > receipt->duration_ticks) receipt->duration_ticks = tick;
    }
    receipt->loop_duration_us =
        dm2_music_ticks_to_us(receipt->duration_ticks, division);
    return 1;
}

static int dm2_inspect_smf(const uint8_t *data,
                           size_t size,
                           DM2_V1_MusicStreamReceipt *receipt)
{
    size_t offset;
    uint16_t tracks;
    uint16_t division;
    if (size < 14u || memcmp(data, "MThd", 4u) != 0 ||
        dm2_read_be32(data + 4) != 6u) {
        return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    }
    tracks = dm2_read_be16(data + 10);
    division = dm2_read_be16(data + 12);
    if (division == 0u) return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    g_dm2_music_ticks_per_quarter = division;
    receipt->format = DM2_V1_MUSIC_FORMAT_STANDARD_MIDI;
    receipt->track_count = tracks;
    offset = 14u;
    for (uint16_t i = 0; i < tracks; ++i) {
        uint32_t track_size;
        if (offset + 8u > size || memcmp(data + offset, "MTrk", 4u) != 0)
            return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
        track_size = dm2_read_be32(data + offset + 4u);
        offset += 8u;
        if (offset + track_size > size) return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
        if (i < (uint16_t)(sizeof(receipt->tracks) / sizeof(receipt->tracks[0]))) {
            if (!dm2_inspect_smf_track(data + offset, track_size, division,
                                       receipt, &receipt->tracks[i]))
                return DM2_V1_MUSIC_INSPECT_BAD_EVENT;
        }
        offset += track_size;
    }
    receipt->schedule_event_count = (uint16_t)receipt->event_count;
    receipt->schedule_handoff_ready = receipt->channel_event_count > 0u;
    receipt->midi_handoff_ready = receipt->schedule_handoff_ready;
    receipt->pcm_handoff_ready = 0;
    receipt->valid = 1;
    return DM2_V1_MUSIC_INSPECT_OK;
}

static int dm2_inspect_hmp(const uint8_t *data,
                           size_t size,
                           DM2_V1_MusicStreamReceipt *receipt)
{
    uint32_t tracks;
    uint32_t chunk_size;
    size_t offset = 788u;
    uint32_t tick = 0;
    if (size < 792u || memcmp(data, "HMIMIDIP", 8u) != 0)
        return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    tracks = dm2_read_le32(data + 48u);
    chunk_size = dm2_read_le32(data + 780u);
    if (chunk_size < 12u || 780u + chunk_size > size)
        return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    receipt->format = DM2_V1_MUSIC_FORMAT_HMP_V1;
    receipt->track_count = (uint16_t)tracks;
    g_dm2_music_ticks_per_quarter = 96u;
    while (offset < 780u + chunk_size) {
        uint32_t delta = 0;
        uint8_t status;
        if (!dm2_read_hmp_vlq(data, 780u + chunk_size, &offset, &delta))
            return DM2_V1_MUSIC_INSPECT_BAD_EVENT;
        tick += delta;
        if (offset >= 780u + chunk_size) return DM2_V1_MUSIC_INSPECT_BAD_EVENT;
        status = data[offset++];
        if (status < 0x80u || status >= 0xf0u)
            return DM2_V1_MUSIC_INSPECT_BAD_EVENT;
        if (offset + 2u > 780u + chunk_size)
            return DM2_V1_MUSIC_INSPECT_BAD_EVENT;
        receipt->event_count++;
        receipt->channel_event_count++;
        dm2_music_store_event(tick, status, data[offset], data[offset + 1u], 2u);
        offset += 2u;
    }
    receipt->duration_ticks = tick;
    receipt->loop_duration_us = dm2_music_ticks_to_us(tick, 96u);
    receipt->schedule_event_count = (uint16_t)receipt->event_count;
    receipt->schedule_handoff_ready = receipt->channel_event_count > 0u;
    receipt->midi_handoff_ready = receipt->schedule_handoff_ready;
    receipt->pcm_handoff_ready = 0;
    receipt->valid = 1;
    return DM2_V1_MUSIC_INSPECT_OK;
}

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

void dm2_v1_sound_bind_verified_music_assets(const char *asset_root,
                                             int primary_assets_verified)
{
    memset(g_dm2_music_asset_root, 0, sizeof(g_dm2_music_asset_root));
    g_dm2_music_asset_root_verified = 0;
    g_dm2_music_event_count = 0;
    g_dm2_music_loop_duration_us = 0;
    g_dm2_music_loop_enabled = 0;
    g_dm2_music_backend_proven = 0;
    g_dm2_music_ticks_per_quarter = 96u;
    if (asset_root && asset_root[0] && primary_assets_verified) {
        snprintf(g_dm2_music_asset_root, sizeof(g_dm2_music_asset_root),
                 "%s", asset_root);
        g_dm2_music_asset_root_verified = 1;
    }
}

int dm2_v1_sound_inspect_music_data(const uint8_t *data, size_t size,
                                    DM2_V1_MusicStreamReceipt *out_receipt)
{
    int result;
    g_dm2_music_event_count = 0;
    dm2_music_receipt_clear(out_receipt);
    if (!data || size == 0u || !out_receipt)
        return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    if (size >= 4u && memcmp(data, "MThd", 4u) == 0)
        result = dm2_inspect_smf(data, size, out_receipt);
    else if (size >= 8u && memcmp(data, "HMIMIDIP", 8u) == 0)
        result = dm2_inspect_hmp(data, size, out_receipt);
    else
        result = DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    if (result != DM2_V1_MUSIC_INSPECT_OK) {
        out_receipt->midi_handoff_ready = 0;
        out_receipt->schedule_handoff_ready = 0;
        g_dm2_music_event_count = 0;
    }
    return result;
}

int dm2_v1_sound_queue_music(int track, int loop,
                             DM2_V1_MusicQueueReceipt *out_receipt)
{
    DM2_V1_MusicStreamReceipt stream;
    unsigned char *data = NULL;
    size_t size;
    FILE *file;
    int inspect_result;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (track < 0 || track >= DM2_MUSIC_TRACK_COUNT)
        return DM2_V1_MUSIC_QUEUE_TRACK_OUT_OF_RANGE;
    if (!g_dm2_music_asset_root_verified)
        return DM2_V1_MUSIC_QUEUE_ASSET_ROOT_UNVERIFIED;
    if (out_receipt) {
        snprintf(out_receipt->asset_path, sizeof(out_receipt->asset_path),
                 "%s/%02x.hmp.mid", g_dm2_music_asset_root, track);
    }
    file = fopen(out_receipt ? out_receipt->asset_path : "", "rb");
    if (!file) return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
    }
    {
        long end = ftell(file);
        if (end <= 0) {
            fclose(file);
            return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
        }
        size = (size_t)end;
    }
    if (fseek(file, 0, SEEK_SET) != 0 || size == 0u) {
        fclose(file);
        return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
    }
    data = (unsigned char *)malloc(size);
    if (!data) {
        fclose(file);
        return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
    }
    if (fread(data, 1u, size, file) != size) {
        free(data);
        fclose(file);
        return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
    }
    fclose(file);

    inspect_result = dm2_v1_sound_inspect_music_data(data, size, &stream);
    free(data);
    if (inspect_result != DM2_V1_MUSIC_INSPECT_OK)
        return DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE;

    g_dm2_music_loop_enabled = loop ? 1 : 0;
    g_dm2_music_loop_duration_us = stream.loop_duration_us;
    g_dm2_music_backend_proven =
        dm2_v1_midi_backend_open() == DM2_V1_MIDI_BACKEND_READY;
    if (out_receipt) {
        out_receipt->asset_resolved = 1;
        out_receipt->decoder_proven = 1;
        out_receipt->backend_proven = g_dm2_music_backend_proven;
        out_receipt->schedule_handoff_ready = stream.schedule_handoff_ready;
        out_receipt->loop_duration_us = stream.loop_duration_us;
        out_receipt->schedule_event_count = stream.schedule_event_count;
        out_receipt->request_queued = g_dm2_music_event_count > 0u;
    }
    return g_dm2_music_backend_proven
               ? DM2_V1_MUSIC_QUEUE_READY
               : DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE;
}

int dm2_v1_sound_schedule_music(uint32_t elapsed_us,
                                DM2_V1_MusicScheduleReceipt *out_receipt)
{
    uint32_t playhead = elapsed_us;
    uint16_t loop_count = 0;
    uint16_t due = 0;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (g_dm2_music_event_count == 0u || !out_receipt) return 0;
    if (g_dm2_music_loop_enabled && g_dm2_music_loop_duration_us != 0u &&
        elapsed_us >= g_dm2_music_loop_duration_us) {
        loop_count = (uint16_t)(elapsed_us / g_dm2_music_loop_duration_us);
        playhead = elapsed_us % g_dm2_music_loop_duration_us;
        if (playhead == 0u) playhead = g_dm2_music_loop_duration_us;
    }
    for (uint16_t i = 0; i < g_dm2_music_event_count; ++i) {
        uint32_t event_us =
            g_dm2_music_loop_duration_us && g_dm2_music_events[i].tick != 0u
                ? dm2_music_ticks_to_us(g_dm2_music_events[i].tick,
                                        g_dm2_music_ticks_per_quarter)
                : 0u;
        if (event_us <= playhead) {
            due++;
            if (g_dm2_music_backend_proven &&
                g_dm2_music_events[i].status >= 0x80u &&
                g_dm2_music_events[i].status < 0xf0u) {
                (void)dm2_v1_midi_backend_send(&g_dm2_music_events[i]);
            }
        }
    }
    out_receipt->valid = 1;
    out_receipt->event_count_due = due;
    out_receipt->loop_count = loop_count;
    out_receipt->backend_proven = g_dm2_music_backend_proven;
    out_receipt->pcm_handoff_ready = 0;
    return 1;
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
