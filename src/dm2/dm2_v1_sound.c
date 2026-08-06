/* dm2_v1_sound.c — DM2 V1 Sound System
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKULLWIN/c_sound.h/cpp, c_sfx.cpp
 * docs/dm2_audio.md, docs/dm2_sound_system.md, docs/dm2_sound_combat.md
 *
 * DM2 audio: 16-slot SFX ring buffer, SoundBlaster, 29 HMP tracks.
 * DM1 audio: 3-4 voices, AdLib FM, ~10 tracks.
 * New in DM2: SOUND_STD_EXPLOSION (bombs), ambient weather, spatial queue.
 */

#include "dm2_v1_sound.h"
#include "dm2_v1_midi_backend.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_sound_queue_pc34_compat.h"
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

static const uint16_t g_skproject_sound_bearing_table[24] = {
    0x25a0u, 0x11f0u, 0x0b00u, 0x0730u, 0x0490u, 0x0290u,
    0x00d0u, 0x0000u, 0x0800u, 0x1700u, 0x2600u, 0x3500u,
    0x4400u, 0x5300u, 0x6200u, 0x7100u, 0x8f00u, 0x9e00u,
    0xad00u, 0xbc00u, 0xcb00u, 0xda00u, 0xe900u, 0xf800u
};

static DM2_V1_MusicScheduledEvent g_dm2_music_events[64];
static uint16_t g_dm2_music_event_count;
static uint32_t g_dm2_music_loop_duration_us;
static int g_dm2_music_loop_enabled;
static int g_dm2_music_backend_proven;
static uint16_t g_dm2_music_ticks_per_quarter = 96u;

/* CDDA state for FM Towns raw PCM playback */
static uint8_t *g_dm2_cdda_pcm_data;
static size_t    g_dm2_cdda_pcm_size;
static int       g_dm2_cdda_disc_track;
static int       g_dm2_cdda_loop;
static int       g_dm2_cdda_queued;

/* ── DM2-008 GDAT-backed sound backend ────────────────────────────────────
 * Source: skproject/SKULLWIN/c_sound.h/cpp, c_sfx.cpp
 * dm2sound.xsndptr2 is the source-owned seven-byte runtime queue populated by
 * DM2_SOUND9.  A verified GDAT loader may be bound so that DM2_SOUND9 can
 * resolve sample bindings; without it the queue accepts explicit sample_id
 * values and every playback path stays fail-closed. */

static const DM2_V1_AssetLoader *g_dm2_sound_gdat_loader;
static int g_dm2_sound_gdat_verified;
/* c_sound.cpp keeps xsndptr2 in the live DM2 sound state.  Retain only a
 * borrow of that owner; constructing a file-scope queue here would create
 * host-only sound history unrelated to the running game. */
static DM2_V1_SoundQueueState *g_dm2_sound_runtime_queue;

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
        /* HMP stores seven-bit delta groups least-significant first and
         * terminates with bit 7 set. This is intentionally distinct from
         * SMF's most-significant-first VLQ above. */
        value |= (uint32_t)(byte & 0x7fu) << (7u * count);
        count++;
        if ((byte & 0x80u) != 0u) {
            *out_value = value;
            return 1;
        }
    } while (1);
}

static uint32_t dm2_music_hmp_ticks_to_us(uint32_t ticks, uint32_t bpm)
{
    if (bpm == 0u) return 0u;
    /* HMP 013195 uses the fixed 60 PPQN cadence consumed by the HMI player.
     * Keep this diagnostic calculation in 64 bits so a long original track
     * cannot wrap into a fabricated short duration. */
    return (uint32_t)(((uint64_t)ticks * 60000000u) / (60u * bpm));
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

static int dm2_inspect_hmp_track(const uint8_t *data,
                                 size_t size,
                                 DM2_V1_MusicStreamReceipt *receipt,
                                 DM2_V1_MusicTrackReceipt *track,
                                 uint32_t *out_duration_ticks)
{
    size_t offset = 0u;
    uint32_t tick = 0;
    uint8_t running_status = 0u;

    if (!data || !receipt || !track || !out_duration_ticks) return 0;
    while (offset < size) {
        uint32_t delta = 0;
        uint8_t status;
        if (!dm2_read_hmp_vlq(data, size, &offset, &delta)) return 0;
        tick += delta;
        if (offset >= size) return 0;
        status = data[offset++];
        receipt->event_count++;
        if (status < 0x80u) {
            if (running_status == 0u) return 0;
            --offset;
            status = running_status;
        } else if (status < 0xf0u) {
            running_status = status;
        }
        if (status == 0xffu) {
            uint8_t meta_type;
            uint32_t meta_length;
            if (offset >= size) return 0;
            meta_type = data[offset++];
            if (!dm2_read_vlq(data, size, &offset, &meta_length) ||
                meta_length > size - offset) return 0;
            ++receipt->meta_event_count;
            if (meta_type == 0x2fu) ++track->end_of_track_count;
            dm2_music_store_event(tick, status, meta_type, 0u, 0u);
            offset += meta_length;
        } else if (status == 0xf0u || status == 0xf7u) {
            uint32_t sysex_length;
            if (!dm2_read_vlq(data, size, &offset, &sysex_length) ||
                sysex_length > size - offset) return 0;
            offset += sysex_length;
        } else if (status < 0xf0u) {
            uint8_t data_size = (uint8_t)(((status & 0xf0u) == 0xc0u ||
                                            (status & 0xf0u) == 0xd0u)
                                           ? 1u : 2u);
            uint8_t data1;
            uint8_t data2 = 0u;
            if (data_size > size - offset) return 0;
            data1 = data[offset++];
            if (data_size == 2u) data2 = data[offset++];
            ++receipt->channel_event_count;
            dm2_music_store_event(tick, status, data1, data2, data_size);
        } else {
            uint8_t data_size = status == 0xf1u || status == 0xf3u ? 1u :
                                status == 0xf2u ? 2u : 0u;
            if (data_size > size - offset) return 0;
            offset += data_size;
        }
    }
    *out_duration_ticks = tick;
    return 1;
}

static int dm2_inspect_hmp(const uint8_t *data,
                           size_t size,
                           DM2_V1_MusicStreamReceipt *receipt)
{
    enum { DM2_HMP_TRACKS_OFFSET = 48, DM2_HMP_BPM_OFFSET = 56,
           DM2_HMP_V1_TRACKS_BASE = 776,
           /* DM2's HMP 013195 payloads have four zero bytes between the
            * 900-byte static header and the first observed track chunk. */
           DM2_HMP_013195_TRACKS_BASE = 904,
           DM2_HMP_TRACK_HEADER_BYTES = 12 };
    uint32_t tracks;
    uint32_t bpm;
    size_t offset;

    /* HMI SOS has two HMP headers that share the 48-byte subtrack count and
     * the tempo at 56. DM2's PC corpus uses V1 (six zero version bytes and a
     * 776-byte header); 013195 adds 128 restore-controller bytes plus the
     * observed four-byte gap, placing chunks at 904. Each chunk is
     * number(4), size(4), track(4), followed by
     * HMP delta/MIDI events. This is cross-checked against the HMI format
     * reader in ScummVM; Firestaff still consumes only its original GDAT
     * payload and does not import a converted MIDI file. */
    if (size < DM2_HMP_V1_TRACKS_BASE ||
        memcmp(data, "HMIMIDIP", 8u) != 0) {
        return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    }
    if (memcmp(data + 8u, "013195", 6u) == 0) {
        offset = DM2_HMP_013195_TRACKS_BASE;
    } else if (data[8] == 0u && data[9] == 0u && data[10] == 0u &&
               data[11] == 0u && data[12] == 0u && data[13] == 0u) {
        offset = DM2_HMP_V1_TRACKS_BASE;
    } else {
        return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    }
    if (size < offset) return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    tracks = dm2_read_le32(data + DM2_HMP_TRACKS_OFFSET);
    bpm = dm2_read_le32(data + DM2_HMP_BPM_OFFSET);
    /* HMP device mappings reserve up to 32 subtracks in the original header;
     * the retail DM2 corpus uses as many as 32. The public receipt retains
     * detailed EOT counters for its first 16 tracks but inspection must walk
     * every source chunk, never silently truncate the remaining music. */
    if (tracks == 0u || tracks > 32u || bpm == 0u) {
        return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
    }
    receipt->format = DM2_V1_MUSIC_FORMAT_HMP_V1;
    receipt->track_count = (uint16_t)tracks;
    g_dm2_music_ticks_per_quarter = 60u;
    for (uint32_t track_index = 0u; track_index < tracks; ++track_index) {
        uint32_t chunk_size;
        uint32_t track_duration = 0u;
        DM2_V1_MusicTrackReceipt overflow_track;
        DM2_V1_MusicTrackReceipt *track_receipt = track_index < 16u
            ? &receipt->tracks[track_index] : &overflow_track;
        memset(&overflow_track, 0, sizeof(overflow_track));
        if (offset > size || size - offset < DM2_HMP_TRACK_HEADER_BYTES) {
            return DM2_V1_MUSIC_INSPECT_BAD_HEADER;
        }
        chunk_size = dm2_read_le32(data + offset + 4u);
        if (chunk_size < DM2_HMP_TRACK_HEADER_BYTES ||
            chunk_size > size - offset ||
            !dm2_inspect_hmp_track(
                data + offset + DM2_HMP_TRACK_HEADER_BYTES,
                chunk_size - DM2_HMP_TRACK_HEADER_BYTES, receipt,
                track_receipt, &track_duration)) {
            return DM2_V1_MUSIC_INSPECT_BAD_EVENT;
        }
        if (track_duration > receipt->duration_ticks) {
            receipt->duration_ticks = track_duration;
        }
        offset += chunk_size;
    }
    receipt->loop_duration_us =
        dm2_music_hmp_ticks_to_us(receipt->duration_ticks, bpm);
    /* Inspection proves the original stream's complete track partition and
     * event bounds, not a source-faithful playback backend. SKProject's MIDI
     * code consumes converted sidecars, so never give these direct-HMP
     * events to the live scheduler until that backend contract is ported. */
    memset(g_dm2_music_events, 0, sizeof(g_dm2_music_events));
    g_dm2_music_event_count = 0;
    receipt->schedule_event_count = 0u;
    receipt->schedule_handoff_ready = 0;
    receipt->midi_handoff_ready = 0;
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

/* ── DM2-008 GDAT binding ─────────────────────────────────────────────────
 * Source: skproject/SKULLWIN/c_sound.h/cpp, c_sfx.cpp
 * A verified GDAT loader may be bound once.  After binding, DM2_SOUND9 can
 * resolve sample bindings and DM2_QUERY_SND_ENTRY_INDEX can fall back to the
 * GDAT table.  Without binding every audio path stays fail-closed. */

void dm2_v1_sound_bind_gdat_loader(const DM2_V1_AssetLoader *loader,
                                   int verified)
{
    g_dm2_sound_gdat_loader = NULL;
    g_dm2_sound_gdat_verified = 0;
    if (loader && verified) {
        g_dm2_sound_gdat_loader = loader;
        g_dm2_sound_gdat_verified = 1;
    }
}

void dm2_v1_sound_bind_runtime_queue(DM2_V1_SoundQueueState *state)
{
    g_dm2_sound_runtime_queue = state;
}

static int dm2_v1_sound_dyn4_has_raw(
    const DM2_V1_GdatDyn4MaterializedSelection *selection,
    uint16_t raw_index)
{
    uint16_t i;

    if (!selection || !selection->valid || !selection->bytes ||
        !selection->raw_indices || !selection->block_offsets) {
        return 0;
    }
    for (i = 0; i < selection->block_count; ++i) {
        uint32_t offset;
        uint16_t raw_length;

        if (selection->raw_indices[i] != raw_index) continue;
        offset = selection->block_offsets[i];
        if (offset > selection->byte_count ||
            selection->byte_count - offset < 4u) {
            return 0;
        }
        raw_length = (uint16_t)selection->bytes[offset] |
            (uint16_t)((uint16_t)selection->bytes[offset + 1u] << 8);
        if ((uint32_t)raw_length > selection->byte_count - offset - 4u)
            return 0;
        return 1;
    }
    return 0;
}

static uint16_t dm2_v1_sound_dyn4_sound7(
    const DM2_V1_SoundQueueState *state,
    uint16_t raw_index)
{
    uint16_t i;

    if (!state) return 0u;
    for (i = 0; i < state->ssound_count; ++i) {
        if (state->ssound[i].w_05 == (int16_t)raw_index)
            return (uint16_t)(i + 1u);
    }
    return 0u;
}

int dm2_v1_sound_resolve_dyn4_samples(
    DM2_V1_SoundQueueState *state,
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatDyn4MaterializedSelection *selection,
    uint16_t sample_capacity,
    DM2_V1_Dyn4SoundResolveReceipt *out_receipt)
{
    DM2_V1_Dyn4SoundResolveReceipt receipt;
    uint16_t i;

    memset(&receipt, 0, sizeof(receipt));
    if (!state || !loader || !dm2_v1_asset_loader_verify(loader) ||
        !selection || !selection->valid ||
        state->sample_binding_count > sample_capacity) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    /* SKProject SKULLWIN/c_gdatfile.cpp::DM2_482b_0684 (932-975): each
     * deferred s_ssound resolves its exact GDAT type-2 entry, then SOUND7
     * either shares an existing sndptr4 slot or allocates the next one. The
     * raw entry must already have been admitted by DYN4. */
    for (i = 0; i < state->ssound_count; ++i) {
        DM2_V1_SoundSsoundEntry *entry = &state->ssound[i];
        DM2_V1_GdatSoundEntryReceipt source;
        uint16_t existing;

        ++receipt.queue_entries_seen;
        if (entry->w_05 != -1) continue;
        if (!dm2_v1_gdat_sound_entry_receipt(
                loader, (uint8_t)entry->b_02, (uint8_t)entry->b_03,
                (uint8_t)entry->b_04, 0, 0, &source) ||
            !source.accepted ||
            !dm2_v1_sound_dyn4_has_raw(selection, source.raw_index)) {
            ++receipt.deferred_missing_material_count;
            continue;
        }
        existing = dm2_v1_sound_dyn4_sound7(state, source.raw_index);
        if (existing != 0u) {
            entry->w_00 = state->ssound[existing - 1u].w_00;
            entry->w_05 = (int16_t)source.raw_index;
            ++receipt.shared_binding_count;
            continue;
        }
        if (state->sample_binding_count >= sample_capacity) {
            receipt.stopped_pool_full = 1u;
            break;
        }
        entry->w_05 = (int16_t)source.raw_index;
        entry->w_00 = (int16_t)state->sample_binding_count;
        ++state->sample_binding_count;
        ++receipt.newly_bound_count;
    }
    receipt.active_sample_count = state->sample_binding_count;
    receipt.valid = 1u;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

/* DM2_SOUND9: populate dm2sound.xsndptr2 (seven-byte s_ssound entry).
 * Source: c_sound.cpp:650-662. The later c_gdatfile.cpp::DM2_482b_0684 owns
 * the GDAT lookup and writes w_00/w_05 after DYN4 has materialised its raw
 * sample. A loader alone must not forge that later state. */
int dm2_v1_sound9(DM2_V1_SoundQueueState *state,
                  int8_t cls1,
                  int8_t cls2,
                  int8_t cls3,
                  int16_t sample_id,
                  uint16_t *out_index)
{
    return dm2_v1_sound_queue_sound9(state, cls1, cls2, cls3, sample_id,
                                     out_index);
}

/* DM2_QUERY_SND_ENTRY_INDEX: 1-based linear scan of xsndptr2.
 * Source: c_sound.cpp:664-673.  Returns 0 when absent. */
uint16_t dm2_v1_query_snd_entry_index(const DM2_V1_SoundQueueState *state,
                                      int8_t cls1,
                                      int8_t cls2,
                                      int8_t cls3)
{
    return dm2_v1_sound_queue_query_entry_index(state, cls1, cls2, cls3);
}

/* DM2_QUERY_SND_ENTRY_INDEX with GDAT resolution.
 * Source: SKProject/SKWINSPX/src/v5/sksound.cpp::DM2_QUERY_SND_ENTRY_INDEX
 * and c_sound.cpp:650-673.  The original query operates on dm2sound.xsndptr2,
 * which is now explicitly bound by the live runtime.  Do not fabricate an
 * independent static queue merely because a GRAPHICS.DAT loader is available. */
int dm2_v1_sound_query_entry(uint8_t cls1, uint8_t cls2, uint8_t cls3)
{
    DM2_V1_SoundQueueState *state = g_dm2_sound_runtime_queue;
    uint16_t index;

    if (!state) return -1;

    index = dm2_v1_sound_queue_query_entry_index(
        state, (int8_t)cls1, (int8_t)cls2, (int8_t)cls3);
    if (index != 0u)
        return (int)index;

    if (!g_dm2_sound_gdat_verified || !g_dm2_sound_gdat_loader)
        return -1;

    if (!dm2_v1_sound9(state, (int8_t)cls1, (int8_t)cls2, (int8_t)cls3,
                       -1, &index))
        return -1;

    return (int)index;
}

static uint32_t dm2_sound_fnv1a(const uint8_t *data, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    if (!data) return 0u;
    for (i = 0; i < size; ++i)
        hash = (hash ^ data[i]) * 16777619u;
    return hash;
}

/* ── DM2-008 PCM decode + voice allocation (cycle 16) ─────────────────────
 * Source: skproject/SKWIN/SkwinSDL.cpp (OpenAudio at PLAYBACK_FREQUENCY
 * 6000 Hz, MAX_SB = 16 simultaneous SndBuf voices, sample bytes converted
 * 0x80 + raw_byte at alloc time) and c_sound.cpp:256-308 (R_928 metric).
 * GDAT sound raw entries hold a two-byte format header (observed 0x77 0x2b
 * on every verified PC English entry) followed by signed 8-bit mono PCM.
 * The decoded unsigned stream is payload ^ 0x80 — the same conversion
 * dm2_v1_gdat_sound_toggle_payload performs. */

typedef struct {
    int allocated;
    uint8_t *pcm;
    uint32_t sample_count;
    uint16_t raw_index;
} DM2_V1_SoundVoiceSlot;

static DM2_V1_SoundVoiceSlot g_dm2_sound_voices[DM2_V1_SOUND_VOICE_MAX];
static const DM2_V1_SoundPlaybackBackend *g_dm2_sound_backend;
static int g_dm2_sound_backend_open_attempted;

uint32_t dm2_v1_sound_gdat_pcm_sample_count(uint8_t cls1, uint8_t cls2,
                                            uint8_t cls3)
{
    DM2_V1_GdatSoundEntryReceipt receipt;
    if (!g_dm2_sound_gdat_verified || !g_dm2_sound_gdat_loader)
        return 0u;
    if (!dm2_v1_gdat_sound_entry_receipt(g_dm2_sound_gdat_loader,
                                         (int)cls1, (int)cls2, (int)cls3,
                                         0, 0, &receipt) ||
        !receipt.accepted) {
        return 0u;
    }
    return receipt.payload_length;
}

int dm2_v1_sound_decode_gdat_pcm(uint8_t cls1, uint8_t cls2, uint8_t cls3,
                                 uint8_t *out_pcm, size_t out_capacity,
                                 DM2_V1_SoundPcmReceipt *out_receipt)
{
    DM2_V1_SoundPcmReceipt receipt;
    DM2_V1_GdatSoundEntryReceipt entry;
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint32_t i;

    memset(&receipt, 0, sizeof(receipt));
    receipt.category = cls1;
    receipt.index = cls2;
    receipt.field = cls3;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));

    if (!g_dm2_sound_gdat_verified || !g_dm2_sound_gdat_loader) {
        receipt.rejected_no_loader = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!dm2_v1_gdat_sound_entry_receipt(g_dm2_sound_gdat_loader,
                                         (int)cls1, (int)cls2, (int)cls3,
                                         0, 0, &entry) ||
        !entry.accepted) {
        receipt.rejected_entry_missing = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.raw_index = entry.raw_index;
    receipt.sample_count = entry.payload_length;
    receipt.sample_rate_hz = DM2_V1_SOUND_PCM_SAMPLE_RATE_HZ;
    if (out_pcm && out_capacity < entry.payload_length) {
        receipt.rejected_capacity = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    raw = dm2_v1_load_gdat_raw_data(g_dm2_sound_gdat_loader, entry.raw_index,
                                    &raw_size);
    if (!raw || raw_size < (size_t)entry.raw_length ||
        entry.header_skip_bytes >= raw_size) {
        receipt.rejected_entry_missing = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (out_pcm) {
        const uint8_t *payload = raw + entry.header_skip_bytes;
        for (i = 0; i < entry.payload_length; ++i)
            out_pcm[i] = (uint8_t)(payload[i] ^ 0x80u);
        receipt.pcm_hash = dm2_sound_fnv1a(out_pcm, entry.payload_length);
    } else {
        /* Query-only decode still hashes the converted payload so receipts
         * are comparable with and without a destination buffer. */
        const uint8_t *payload = raw + entry.header_skip_bytes;
        uint32_t hash = 2166136261u;
        for (i = 0; i < entry.payload_length; ++i)
            hash = (hash ^ (uint8_t)(payload[i] ^ 0x80u)) * 16777619u;
        receipt.pcm_hash = hash;
    }
    receipt.accepted = 1u;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

void dm2_v1_sound_bind_playback_backend(
    const DM2_V1_SoundPlaybackBackend *backend)
{
    unsigned i;
    if (g_dm2_sound_backend && g_dm2_sound_backend != backend &&
        g_dm2_sound_backend->close) {
        g_dm2_sound_backend->close(g_dm2_sound_backend->ctx);
    }
    for (i = 0; i < DM2_V1_SOUND_VOICE_MAX; ++i) {
        free(g_dm2_sound_voices[i].pcm);
        memset(&g_dm2_sound_voices[i], 0, sizeof(g_dm2_sound_voices[i]));
    }
    g_dm2_sound_backend = backend;
    g_dm2_sound_backend_open_attempted = 0;
}

int dm2_v1_sound_playback_backend_bound(void)
{
    return g_dm2_sound_backend != NULL;
}

static int dm2_v1_sound_backend_ready(void)
{
    if (!g_dm2_sound_backend || !g_dm2_sound_backend->is_ready)
        return 0;
    if (g_dm2_sound_backend->is_ready(g_dm2_sound_backend->ctx))
        return 1;
    if (g_dm2_sound_backend_open_attempted || !g_dm2_sound_backend->open)
        return 0;
    g_dm2_sound_backend_open_attempted = 1;
    if (!g_dm2_sound_backend->open(g_dm2_sound_backend->ctx))
        return 0;
    return g_dm2_sound_backend->is_ready(g_dm2_sound_backend->ctx);
}

static int dm2_v1_sound_alloc_voice(void)
{
    unsigned i;
    for (i = 0; i < DM2_V1_SOUND_VOICE_MAX; ++i) {
        if (g_dm2_sound_voices[i].allocated && g_dm2_sound_backend &&
            g_dm2_sound_backend->voice_active &&
            !g_dm2_sound_backend->voice_active(g_dm2_sound_backend->ctx, i)) {
            free(g_dm2_sound_voices[i].pcm);
            memset(&g_dm2_sound_voices[i], 0, sizeof(g_dm2_sound_voices[i]));
        }
    }
    for (i = 0; i < DM2_V1_SOUND_VOICE_MAX; ++i)
        if (!g_dm2_sound_voices[i].allocated)
            return (int)i;
    return -1;
}

static void dm2_v1_sound_playback_clear_receipt(
    DM2_V1_SoundPlaybackReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int dm2_v1_sound_play_gdat_entry_positional(
    uint8_t cls1, uint8_t cls2, uint8_t cls3, int volume,
    int16_t dx, int16_t dy, DM2_V1_SoundPlaybackReceipt *out_receipt)
{
    DM2_V1_SoundPlaybackReceipt receipt;
    DM2_V1_SoundPcmReceipt pcm_receipt;
    DM2_V1_SoundSfx sfx;
    int slot;
    int clamped_volume = volume;

    dm2_v1_sound_playback_clear_receipt(out_receipt);
    memset(&receipt, 0, sizeof(receipt));
    receipt.category = cls1;
    receipt.index = cls2;
    receipt.field = cls3;
    if (clamped_volume < 0) clamped_volume = 0;
    if (clamped_volume > 255) clamped_volume = 255;
    receipt.volume = (uint8_t)clamped_volume;

    if (!g_dm2_sound_gdat_verified || !g_dm2_sound_gdat_loader) {
        receipt.rejected_no_loader = 1u;
        receipt.valid = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!g_dm2_sound_backend) {
        receipt.rejected_no_backend = 1u;
        receipt.valid = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!dm2_v1_sound_backend_ready()) {
        receipt.rejected_backend_not_ready = 1u;
        receipt.valid = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    /* The sample must decode from a verified GDAT sound entry; nothing is
     * synthesized when the entry is absent. */
    if (!dm2_v1_sound_decode_gdat_pcm(cls1, cls2, cls3, NULL, 0u,
                                      &pcm_receipt) ||
        !pcm_receipt.accepted || pcm_receipt.sample_count == 0u) {
        receipt.rejected_decode_failed = 1u;
        receipt.valid = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.raw_index = pcm_receipt.raw_index;
    receipt.sample_count = pcm_receipt.sample_count;

    slot = dm2_v1_sound_alloc_voice();
    if (slot < 0) {
        receipt.rejected_no_free_voice = 1u;
        receipt.valid = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    g_dm2_sound_voices[slot].pcm = (uint8_t *)malloc(pcm_receipt.sample_count);
    if (!g_dm2_sound_voices[slot].pcm ||
        !dm2_v1_sound_decode_gdat_pcm(cls1, cls2, cls3,
                                      g_dm2_sound_voices[slot].pcm,
                                      pcm_receipt.sample_count,
                                      &pcm_receipt)) {
        free(g_dm2_sound_voices[slot].pcm);
        memset(&g_dm2_sound_voices[slot], 0, sizeof(g_dm2_sound_voices[slot]));
        receipt.rejected_decode_failed = 1u;
        receipt.valid = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    /* Source-locked attenuation: R_928 (c_sound.cpp:256-308) over the s_sfx
     * rotated deltas; dx == dy == 0 yields the full queued volume. */
    memset(&sfx, 0, sizeof(sfx));
    sfx.ub_05 = (uint8_t)clamped_volume;
    sfx.ub_06 = (int8_t)(dx > 127 ? 127 : (dx < -128 ? -128 : dx));
    sfx.ub_07 = (int8_t)(dy > 127 ? 127 : (dy < -128 ? -128 : dy));
    dm2_v1_sound_queue_r928_metric(&sfx);
    receipt.attenuation = sfx.metric_attenuation;

    if (!g_dm2_sound_backend->start_voice ||
        !g_dm2_sound_backend->start_voice(g_dm2_sound_backend->ctx,
                                          (unsigned)slot,
                                          g_dm2_sound_voices[slot].pcm,
                                          pcm_receipt.sample_count,
                                          receipt.attenuation)) {
        free(g_dm2_sound_voices[slot].pcm);
        memset(&g_dm2_sound_voices[slot], 0, sizeof(g_dm2_sound_voices[slot]));
        receipt.rejected_backend_not_ready = 1u;
        receipt.valid = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    g_dm2_sound_voices[slot].allocated = 1;
    g_dm2_sound_voices[slot].sample_count = pcm_receipt.sample_count;
    g_dm2_sound_voices[slot].raw_index = pcm_receipt.raw_index;
    receipt.voice_slot = (uint8_t)slot;
    receipt.playback_started = 1u;
    receipt.valid = 1u;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_sound_play_gdat_entry(uint8_t cls1, uint8_t cls2, uint8_t cls3,
                                 int volume,
                                 DM2_V1_SoundPlaybackReceipt *out_receipt)
{
    return dm2_v1_sound_play_gdat_entry_positional(cls1, cls2, cls3, volume,
                                                   0, 0, out_receipt);
}

void dm2_v1_sound_stop_all_voices(void)
{
    unsigned i;
    if (g_dm2_sound_backend && g_dm2_sound_backend->stop_all)
        g_dm2_sound_backend->stop_all(g_dm2_sound_backend->ctx);
    for (i = 0; i < DM2_V1_SOUND_VOICE_MAX; ++i) {
        free(g_dm2_sound_voices[i].pcm);
        memset(&g_dm2_sound_voices[i], 0, sizeof(g_dm2_sound_voices[i]));
    }
}

int dm2_v1_sound_voice_active(unsigned voice_slot)
{
    if (voice_slot >= DM2_V1_SOUND_VOICE_MAX ||
        !g_dm2_sound_voices[voice_slot].allocated ||
        !g_dm2_sound_backend || !g_dm2_sound_backend->voice_active)
        return 0;
    return g_dm2_sound_backend->voice_active(g_dm2_sound_backend->ctx,
                                             voice_slot);
}

/* Resolve a GDAT raw sample binding (xsndptr2 w_00) back to the owning
 * verified SOUND entry so playback always decodes from GDAT evidence. */
static int dm2_v1_sound_find_entry_by_raw_index(uint16_t raw_index,
                                                uint8_t *out_cls1,
                                                uint8_t *out_cls2,
                                                uint8_t *out_cls3)
{
    DM2_V1_GdatEntryIterator iterator;
    DM2_V1_GdatEntryQueryReceipt entry;
    if (!g_dm2_sound_gdat_verified || !g_dm2_sound_gdat_loader)
        return 0;
    memset(&iterator, 0, sizeof(iterator));
    iterator.category_first = 0;
    iterator.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iterator.index_filter = -1;
    iterator.type_filter = DM2_GDAT_ENTRY_TYPE_SOUND;
    iterator.field_filter = -1;
    while (dm2_v1_query_next_gdat_entry(g_dm2_sound_gdat_loader, &iterator,
                                        &entry)) {
        if (entry.loadable_raw && entry.raw_index == raw_index) {
            *out_cls1 = entry.category;
            *out_cls2 = entry.index;
            *out_cls3 = entry.field;
            return 1;
        }
    }
    return 0;
}

/* dm2_v1_sound_play — DM2_PLAY_SOUND()
 * Source: SKULLWIN/c_sound.cpp:342-434
 * sound_id is the GDAT raw sample binding owned by the source queue entry
 * (xsndptr2 w_00).  Playback is audible only when a verified GDAT loader and
 * a proven playback backend are bound and the sample decodes from a verified
 * GDAT sound entry; otherwise the call is explicitly unavailable. */
int dm2_v1_sound_play(int sound_id, int volume) {
    uint8_t cls1;
    uint8_t cls2;
    uint8_t cls3;
    if (sound_id < 0 || sound_id > 0xffff) return -1;
    if (!dm2_v1_sound_find_entry_by_raw_index((uint16_t)sound_id,
                                              &cls1, &cls2, &cls3))
        return -1;
    if (!dm2_v1_sound_play_gdat_entry(cls1, cls2, cls3, volume, NULL))
        return -1;
    return sound_id;
}

/* dm2_v1_sound_play_positional — world-coordinate spatial audio
 * Source: SKULLWIN/c_sound.cpp world-coordinate queue; SKWIN/SkwinSDL.cpp
 * SndPlayLo (dist = abs(dX) + abs(dY)); attenuation is the source R_928
 * metric over the rotated deltas — no attenuation is synthesized. */
int dm2_v1_sound_play_positional(int sound_id,
    int world_x, int world_y, int listener_x, int listener_y) {
    uint8_t cls1;
    uint8_t cls2;
    uint8_t cls3;
    int dx = world_x - listener_x;
    int dy = world_y - listener_y;
    if (sound_id < 0 || sound_id > 0xffff) return -1;
    if (!dm2_v1_sound_find_entry_by_raw_index((uint16_t)sound_id,
                                              &cls1, &cls2, &cls3))
        return -1;
    if (!dm2_v1_sound_play_gdat_entry_positional(
            cls1, cls2, cls3, 255,
            (int16_t)(dx > 127 ? 127 : (dx < -128 ? -128 : dx)),
            (int16_t)(dy > 127 ? 127 : (dy < -128 ? -128 : dy)), NULL))
        return -1;
    return sound_id;
}

void dm2_v1_sound_bind_verified_music_assets(const char *asset_root,
                                             int primary_assets_verified)
{
    /* Compatibility entry point for callers predating the direct GDAT path.
     * Do not retain a host directory: the PC release's music ownership is
     * GRAPHICS.DAT MUSICS/<track>/dtHMP/0, bound through
     * sound_bind_gdat_loader(). */
    (void)asset_root;
    (void)primary_assets_verified;
    g_dm2_music_event_count = 0;
    g_dm2_music_loop_duration_us = 0;
    g_dm2_music_loop_enabled = 0;
    g_dm2_music_backend_proven = 0;
    g_dm2_music_ticks_per_quarter = 96u;
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
    const uint8_t *data;
    size_t size;
    int inspect_result;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (track < 0 || track >= DM2_MUSIC_TRACK_COUNT)
        return DM2_V1_MUSIC_QUEUE_TRACK_OUT_OF_RANGE;
    if (!g_dm2_sound_gdat_verified || !g_dm2_sound_gdat_loader)
        return DM2_V1_MUSIC_QUEUE_ASSET_ROOT_UNVERIFIED;
    /* SKWIN c_sound.cpp::DM2_PLAY_MUSIC probes exactly
     * GDAT_CATEGORY_MUSICS / track / dtHMP / field 0.  The PC release keeps
     * those 29 HMP payloads in the admitted GRAPHICS.DAT; never replace them
     * with a same-named host file beside an otherwise verified install. */
    data = dm2_v1_query_gdat_entry_data_ptr(
        g_dm2_sound_gdat_loader, DM2_GDAT_CATEGORY_MUSICS, track,
        DM2_GDAT_ENTRY_TYPE_HMP, 0, &size);
    if (out_receipt) {
        snprintf(out_receipt->asset_path, sizeof(out_receipt->asset_path),
                 "GRAPHICS.DAT::GDAT(04,%02x,03,00)", track);
    }
    if (!data || size == 0u)
        return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
    if (out_receipt) out_receipt->asset_resolved = 1;
    inspect_result = dm2_v1_sound_inspect_music_data(data, size, &stream);
    if (inspect_result != DM2_V1_MUSIC_INSPECT_OK)
        return DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE;

    /* The verified PC corpus is HMP, not SMF.  A successful structural
     * inspection is deliberately not decoder proof: no original HMP payload
     * may reach the generic MIDI backend until a direct HMP decoder owns its
     * timing and track semantics. */
    if (!stream.midi_handoff_ready) {
        if (out_receipt) {
            out_receipt->decoder_proven = 0;
            out_receipt->schedule_handoff_ready = 0;
            out_receipt->schedule_event_count = 0u;
        }
        return DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE;
    }

    g_dm2_music_loop_enabled = loop ? 1 : 0;
    g_dm2_music_loop_duration_us = stream.loop_duration_us;
    g_dm2_music_backend_proven =
        dm2_v1_midi_backend_open() == DM2_V1_MIDI_BACKEND_READY;
    if (out_receipt) {
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

int dm2_v1_sound_queue_cdda(const uint8_t *pcm_data, size_t pcm_size,
                             int disc_track, int loop,
                             DM2_V1_MusicQueueReceipt *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!pcm_data || pcm_size < 4u)
        return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
    /* 16-bit stereo 44100Hz: 4 bytes per frame */
    if (pcm_size % 4u != 0u)
        return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;

    {
        uint8_t *copy = (uint8_t *)malloc(pcm_size);
        if (!copy) return DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
        memcpy(copy, pcm_data, pcm_size);
        free(g_dm2_cdda_pcm_data);
        g_dm2_cdda_pcm_data = copy;
    }
    g_dm2_cdda_pcm_size = pcm_size;
    g_dm2_cdda_disc_track = disc_track;
    g_dm2_cdda_loop = loop ? 1 : 0;
    g_dm2_cdda_queued = 1;

    if (out_receipt) {
        out_receipt->asset_resolved = 1;
        out_receipt->request_queued = 1;
        out_receipt->decoder_proven = 1;
        out_receipt->backend_proven = 1;
        out_receipt->schedule_handoff_ready = 1;
        out_receipt->loop_duration_us =
            (uint32_t)((pcm_size / 4u) * 1000000ull / 44100ull);
        snprintf(out_receipt->asset_path, sizeof(out_receipt->asset_path),
                 "CDDA::track%02d (FM Towns)", disc_track);
    }
    return DM2_V1_MUSIC_QUEUE_READY;
}

int dm2_v1_sound_flush_cdda(DM2_V1_CddaFlushReceipt *out_receipt)
{
    DM2_V1_CddaFlushReceipt local;
    memset(&local, 0, sizeof(local));
    if (out_receipt == NULL) out_receipt = &local;

    if (!g_dm2_cdda_queued) return 0;
    g_dm2_cdda_queued = 0;

    out_receipt->disc_track = g_dm2_cdda_disc_track;
    out_receipt->pcm_data = g_dm2_cdda_pcm_data;
    out_receipt->pcm_size = g_dm2_cdda_pcm_size;
    out_receipt->loop = g_dm2_cdda_loop;
    out_receipt->valid = 1;
    return 1;
}

void dm2_v1_sound_release_cdda(void)
{
    free(g_dm2_cdda_pcm_data);
    g_dm2_cdda_pcm_data = NULL;
    g_dm2_cdda_pcm_size = 0;
    g_dm2_cdda_queued = 0;
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

/* dm2_v1_sound_play_music — DM2_PLAY_MUSIC
 * Source: docs/dm2_audio.md (do_music_wav, tMusicMaps[64])
 * Original: HMP format in GRAPHICS.DAT GDAT MUSICS/<track>/dtHMP/0.
 * Track selection: SONGLIST.DAT[tMusicMaps[dungeon_map_index]] → track number.
 *
 * Source-locked: successful playback requires both verified music assets and a
 * proven backend.  Without either the call is rejected explicitly. */
int dm2_v1_sound_play_music(int track) {
    DM2_V1_MusicQueueReceipt queue;
    DM2_V1_MusicScheduleReceipt schedule;
    if (dm2_v1_sound_queue_music(track, 1, &queue) !=
        DM2_V1_MUSIC_QUEUE_READY)
        return -1;
    if (!dm2_v1_sound_schedule_music(0u, &schedule) ||
        !schedule.backend_proven || schedule.event_count_due == 0u)
        return -1;
    return track;
}

/* dm2_v1_sound_stop_music — stop all music
 * Source: docs/dm2_audio.md (do_music_stop)
 * Calls al_stop_samples() via c_midi / c_music_wav. */
int dm2_v1_sound_stop_music(void) {
    /* c_music_wav stops the active sequence before a later title/menu cue
     * can schedule it again. Clear the decoded source stream as well as the
     * backend: retaining its events after reporting success would let a
     * stale, previously admitted GDAT track continue to drive playback. */
    dm2_v1_midi_backend_close();
    memset(g_dm2_music_events, 0, sizeof(g_dm2_music_events));
    g_dm2_music_event_count = 0;
    g_dm2_music_loop_duration_us = 0;
    g_dm2_music_loop_enabled = 0;
    g_dm2_music_backend_proven = 0;
    g_dm2_music_ticks_per_quarter = 96u;
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
        "Source: DMWeb music-trigger notes (29 HMP tracks, SONGLIST.DAT, do_music_wav)\n"
        "Source: docs/dm2_sound_system.md (c_sound init, PLAYBACK_FREQUENCY=5500/6000 Hz)\n"
        "Source: docs/dm2_sound_combat.md (all combat sound triggers 0x00-0x92)\n"
        "DM1 comparison: AdLib FM, 3-4 voices, ~10 tracks, no positional audio\n"
        "DM2 comparison: SoundBlaster, 16-slot buffer, 29 tracks, world-coordinate spatial queue\n"
        "DM2 new: SOUND_STD_EXPLOSION (0x81), glbXAmbientSoundActivated, DM2_QUEUE_NOISE_GEN1/GEN2\n";
}
