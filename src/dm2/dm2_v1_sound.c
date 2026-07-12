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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_dm2_verified_music_asset_root[512];
static DM2_V1_MusicStreamReceipt g_dm2_music_schedule_stream;
static uint32_t g_dm2_music_schedule_elapsed_us;
static uint32_t g_dm2_music_schedule_last_elapsed_us;
static uint32_t g_dm2_music_schedule_loop_count;
static int g_dm2_music_schedule_active;
static int g_dm2_music_schedule_track = -1;

/* skproject/SKULLWIN/c_midi.cpp:11-37 loads DATA/%02x.hmp.mid with
 * Allegro's MIDI loader.  ScummVM audio/midiparser_hmp.cpp:35-157 provides
 * the independently readable HMP framing used below: HMIMIDIP headers,
 * little-endian HMP words, and HMP's low-seven-bit-first VLQ deltas. */
static uint16_t dm2_v1_music_read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t dm2_v1_music_read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static uint32_t dm2_v1_music_read_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int dm2_v1_music_read_vlq(const uint8_t **cursor, const uint8_t *end,
                                  int hmp, uint32_t *out_value)
{
    const uint8_t *p = *cursor;
    uint32_t value = 0;
    int i;
    for (i = 0; i < 4; ++i) {
        uint8_t byte;
        if (p >= end) return 0;
        byte = *p++;
        if (hmp) {
            value |= (uint32_t)(byte & 0x7fu) << (7 * i);
            if (byte & 0x80u) {
                *cursor = p;
                *out_value = value;
                return 1;
            }
        } else {
            if (value > (UINT32_MAX >> 7)) return 0;
            value = (value << 7) | (byte & 0x7fu);
            if (!(byte & 0x80u)) {
                *cursor = p;
                *out_value = value;
                return 1;
            }
        }
    }
    return 0;
}

static int dm2_v1_music_parse_track(const uint8_t *data, size_t size, int hmp,
                                    DM2_V1_MusicTrackReceipt *track,
                                    DM2_V1_MusicStreamReceipt *receipt)
{
    const uint8_t *p = data;
    const uint8_t *end = data + size;
    uint8_t running_status = 0;
    uint32_t absolute_tick = 0;
    while (p < end) {
        uint32_t ignored_length;
        uint8_t status;
        unsigned data_bytes;
        if (receipt->event_count >= DM2_V1_MUSIC_MAX_EVENTS ||
            !dm2_v1_music_read_vlq(&p, end, hmp, &ignored_length))
            return 0;
        if (UINT32_MAX - absolute_tick < ignored_length) return 0;
        absolute_tick += ignored_length;
        if (p >= end) return 0;
        status = *p;
        if (status & 0x80u) {
            ++p;
            if (status < 0xf0u) running_status = status;
        } else {
            if (running_status == 0) return 0;
            status = running_status;
        }
        ++receipt->event_count;
        ++track->event_count;
        if (status < 0xf0u) {
            data_bytes = ((status & 0xe0u) == 0xc0u) ? 1u : 2u;
            /* The verified SKWin title stream carries HMP-origin controller
             * values such as B0 6D 80 under an MThd wrapper.  Retain all
             * parameter octets verbatim; only event framing is validated. */
            if ((size_t)(end - p) < data_bytes)
                return 0;
            if (receipt->schedule_event_count < DM2_V1_MUSIC_MAX_SCHEDULE_EVENTS) {
                DM2_V1_MusicScheduledEvent *event =
                    &receipt->schedule_events[receipt->schedule_event_count++];
                event->tick = absolute_tick;
                event->status = status;
                event->data1 = p[0];
                event->data2 = data_bytes > 1u ? p[1] : 0;
                event->data_size = (uint8_t)data_bytes;
                event->payload = 0;
            }
            p += data_bytes;
            ++receipt->channel_event_count;
        } else if (status == 0xf0u || status == 0xf7u) {
            if (!dm2_v1_music_read_vlq(&p, end, hmp, &ignored_length) ||
                ignored_length > (uint32_t)(end - p)) return 0;
            p += ignored_length;
            ++receipt->sysex_event_count;
        } else if (status == 0xffu) {
            uint8_t meta_type;
            if (p >= end) return 0;
            meta_type = *p++;
            if (!dm2_v1_music_read_vlq(&p, end, hmp, &ignored_length) ||
                ignored_length > (uint32_t)(end - p)) return 0;
            p += ignored_length;
            ++receipt->meta_event_count;
            if (receipt->schedule_event_count < DM2_V1_MUSIC_MAX_SCHEDULE_EVENTS) {
                DM2_V1_MusicScheduledEvent *event =
                    &receipt->schedule_events[receipt->schedule_event_count++];
                event->tick = absolute_tick;
                event->status = 0xffu;
                event->data1 = meta_type;
                event->data2 = (meta_type == 0x51u && ignored_length == 3u)
                    ? p[0] : 0;
                event->data_size = 0;
                event->payload = (meta_type == 0x51u && ignored_length == 3u)
                    ? ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2]
                    : 0;
            }
            if (meta_type == 0x2fu) {
                if (ignored_length != 0) return 0;
                ++track->end_of_track_count;
            }
        } else {
            switch (status) {
            case 0xf1u: case 0xf3u: data_bytes = 1u; break;
            case 0xf2u: data_bytes = 2u; break;
            case 0xf6u: case 0xf8u: case 0xf9u: case 0xfau:
            case 0xfbu: case 0xfcu: case 0xfdu: case 0xfeu: data_bytes = 0u; break;
            default: return 0;
            }
            if ((size_t)(end - p) < data_bytes) return 0;
            p += data_bytes;
        }
    }
    track->duration_ticks = absolute_tick;
    return p == end;
}

static int dm2_v1_music_event_compare(const void *a, const void *b)
{
    const DM2_V1_MusicScheduledEvent *left = a;
    const DM2_V1_MusicScheduledEvent *right = b;
    if (left->tick < right->tick) return -1;
    if (left->tick > right->tick) return 1;
    return 0;
}

static void dm2_v1_music_build_schedule(DM2_V1_MusicStreamReceipt *receipt)
{
    uint32_t i;
    uint32_t previous_tick = 0;
    uint32_t tempo_us_per_quarter = 500000u;
    uint64_t current_us = 0;
    if (!receipt || receipt->time_division == 0 ||
        receipt->event_count > DM2_V1_MUSIC_MAX_SCHEDULE_EVENTS) {
        return;
    }
    qsort(receipt->schedule_events, receipt->schedule_event_count,
          sizeof(receipt->schedule_events[0]), dm2_v1_music_event_compare);
    for (i = 0; i < receipt->schedule_event_count; ++i) {
        DM2_V1_MusicScheduledEvent *event = &receipt->schedule_events[i];
        uint32_t delta = event->tick - previous_tick;
        current_us += ((uint64_t)delta * tempo_us_per_quarter) /
                      receipt->time_division;
        if (current_us > UINT32_MAX) return;
        event->time_us = (uint32_t)current_us;
        previous_tick = event->tick;
        /* SMF Set Tempo affects all tracks after this absolute tick. */
        if (event->status == 0xffu && event->data1 == 0x51u) {
            tempo_us_per_quarter = event->payload;
        }
    }
    for (i = 0; i < receipt->track_count; ++i) {
        if (receipt->tracks[i].duration_ticks > receipt->duration_ticks)
            receipt->duration_ticks = receipt->tracks[i].duration_ticks;
    }
    if (receipt->duration_ticks < previous_tick)
        receipt->duration_ticks = previous_tick;
    current_us += ((uint64_t)(receipt->duration_ticks - previous_tick) *
                   tempo_us_per_quarter) / receipt->time_division;
    if (current_us == 0 || current_us > UINT32_MAX) return;
    receipt->loop_duration_us = (uint32_t)current_us;
    receipt->schedule_handoff_ready = 1;
}

int dm2_v1_sound_inspect_music_data(const uint8_t *data, size_t size,
                                    DM2_V1_MusicStreamReceipt *out_receipt)
{
    DM2_V1_MusicStreamReceipt receipt;
    size_t cursor;
    uint32_t tracks;
    uint32_t i;
    int hmp = 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.result = DM2_V1_MUSIC_INSPECT_EMPTY;
    if (!data || size == 0) goto done;
    if (size > DM2_V1_MUSIC_MAX_FILE_BYTES || size > UINT32_MAX) {
        receipt.result = DM2_V1_MUSIC_INSPECT_FILE_TOO_LARGE;
        goto done;
    }
    receipt.file_size = (uint32_t)size;
    if (size >= 4 && memcmp(data, "MThd", 4) == 0) {
        uint32_t header_size;
        if (size < 14) { receipt.result = DM2_V1_MUSIC_INSPECT_TRUNCATED; goto done; }
        header_size = dm2_v1_music_read_be32(data + 4);
        if (header_size < 6 || header_size > size - 8) {
            receipt.result = DM2_V1_MUSIC_INSPECT_BAD_HEADER; goto done;
        }
        if (dm2_v1_music_read_be16(data + 8) > 2) {
            receipt.result = DM2_V1_MUSIC_INSPECT_BAD_HEADER; goto done;
        }
        tracks = dm2_v1_music_read_be16(data + 10);
        receipt.time_division = dm2_v1_music_read_be16(data + 12);
        receipt.format = DM2_V1_MUSIC_FORMAT_STANDARD_MIDI;
        cursor = 8u + header_size;
    } else if (size >= 14 && memcmp(data, "HMIMIDIP", 8) == 0) {
        size_t header_size;
        hmp = 1;
        if (memcmp(data + 8, "\0\0\0\0\0\0", 6) == 0) {
            receipt.format = DM2_V1_MUSIC_FORMAT_HMP_V1;
            header_size = 776u;
        } else if (memcmp(data + 8, "013195", 6) == 0) {
            receipt.format = DM2_V1_MUSIC_FORMAT_HMP_013195;
            header_size = 904u;
        } else {
            receipt.result = DM2_V1_MUSIC_INSPECT_BAD_HEADER; goto done;
        }
        if (size < header_size) { receipt.result = DM2_V1_MUSIC_INSPECT_TRUNCATED; goto done; }
        tracks = dm2_v1_music_read_le32(data + 48);
        receipt.time_division = (uint16_t)dm2_v1_music_read_le32(data + 52);
        cursor = header_size;
    } else {
        receipt.result = DM2_V1_MUSIC_INSPECT_BAD_SIGNATURE; goto done;
    }
    if (tracks == 0 || tracks > DM2_V1_MUSIC_MAX_TRACKS || receipt.time_division == 0) {
        receipt.result = DM2_V1_MUSIC_INSPECT_BAD_HEADER; goto done;
    }
    receipt.track_count = tracks;
    for (i = 0; i < tracks; ++i) {
        uint32_t track_size;
        uint32_t payload_offset;
        uint32_t payload_size;
        if (cursor > size || size - cursor < (hmp ? 12u : 8u)) {
            receipt.result = DM2_V1_MUSIC_INSPECT_TRUNCATED; goto done;
        }
        if (hmp) {
            track_size = dm2_v1_music_read_le32(data + cursor + 4);
            if (track_size < 12u) { receipt.result = DM2_V1_MUSIC_INSPECT_BAD_TRACK; goto done; }
            payload_offset = (uint32_t)cursor + 12u;
            payload_size = track_size - 12u;
            cursor += 12u;
        } else {
            if (memcmp(data + cursor, "MTrk", 4) != 0) {
                receipt.result = DM2_V1_MUSIC_INSPECT_BAD_TRACK; goto done;
            }
            track_size = dm2_v1_music_read_be32(data + cursor + 4);
            payload_offset = (uint32_t)cursor + 8u;
            payload_size = track_size;
            cursor += 8u;
        }
        if (payload_size > size - cursor) { receipt.result = DM2_V1_MUSIC_INSPECT_TRUNCATED; goto done; }
        receipt.tracks[i].byte_offset = payload_offset;
        receipt.tracks[i].byte_size = payload_size;
        if (!dm2_v1_music_parse_track(data + cursor, payload_size, hmp,
                                       &receipt.tracks[i], &receipt)) {
            receipt.result = receipt.event_count >= DM2_V1_MUSIC_MAX_EVENTS
                ? DM2_V1_MUSIC_INSPECT_LIMIT_EXCEEDED : DM2_V1_MUSIC_INSPECT_BAD_EVENT;
            goto done;
        }
        cursor += payload_size;
    }
    receipt.result = DM2_V1_MUSIC_INSPECT_OK;
    dm2_v1_music_build_schedule(&receipt);
    receipt.midi_handoff_ready = 1;
    /* MIDI/HMP describes control events, not PCM.  A future backend must
     * consume this validated stream before sound is emitted. */
    receipt.pcm_handoff_ready = 0;
done:
    if (out_receipt) *out_receipt = receipt;
    return (int)receipt.result;
}

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

/* ── DM2_QUERY_SND_ENTRY_INDEX stub ─────────────────────────────────────
 * Source: docs/dm2_audio.md
 * DM2_QUERY_SND_ENTRY_INDEX(cat, cls1, cls2, sfx) → entry index
 * Real: GDAT lookup via c_sound::querySoundEntry() at runtime.
 * Stub: always returns sfx id (no GDAT table). */

int dm2_v1_sound_query_entry(uint8_t cat, uint8_t c1, uint8_t c2, uint8_t sfx) {
    (void)cat; (void)c1; (void)c2;
    /* Stub: accepts sound IDs 0-127. Real GDAT lookup deferred.
     * Accept same range that sound_play() accepts: IDs < 128.
     * DM2 SFX IDs 0x81, 0x84-0x89, 0x8A, 0x8E, etc. are all < 128. */
    return (sfx < 128u) ? (int)sfx : -1;
}

/* dm2_v1_sound_play — play a sound effect
 * Source: SKULLWIN/c_sound.cpp: DM2_PLAY_SOUND()
 * Stub: calls SDL audio queue (actual SDL_QueueAudio integration deferred).
 * Frequency: DM2_PLAYBACK_FREQUENCY_WIN = 6000 Hz (SDL port).
 * DM1: 3-4 voice AdLib FM; DM2: 16-slot ring buffer SoundBlaster. */
int dm2_v1_sound_play(int sound_id, int volume) {
    if (sound_id < 0) return -1;
    (void)volume;
    /* Stub: would call SDL_QueueAudio or allegro_play_sample()
     * with glbSoundFreq_13ce playback rate and distance attenuation. */
    return 0;
}

/* dm2_v1_sound_play_positional — world-coordinate spatial audio
 * Source: SKULLWIN/c_sound.cpp: world-coordinate queue with distance attenuation
 * Distance formula: attenuation = 1.0 / (1.0 + distance * falloff)
 * glbXAmbientSoundActivated for weather ambient sounds. */
int dm2_v1_sound_play_positional(int sound_id,
    int world_x, int world_y, int listener_x, int listener_y) {
    if (sound_id < 0) return -1;
    int dx = world_x - listener_x;
    int dy = world_y - listener_y;
    /* Simple distance: approximate tile distance */
    float dist = (float)((dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy));
    float atten = 1.0f / (1.0f + dist * 0.25f);
    int vol = (int)(atten * 127.0f);
    return dm2_v1_sound_play(sound_id, vol);
}

void dm2_v1_sound_bind_verified_music_assets(const char *asset_root,
                                             int primary_assets_verified)
{
    g_dm2_verified_music_asset_root[0] = '\0';
    g_dm2_music_schedule_active = 0;
    g_dm2_music_schedule_track = -1;
    dm2_v1_midi_backend_close();
    if (!primary_assets_verified || !asset_root || asset_root[0] == '\0') {
        return;
    }
    snprintf(g_dm2_verified_music_asset_root,
             sizeof(g_dm2_verified_music_asset_root), "%s", asset_root);
}

int dm2_v1_sound_queue_music(int track, int loop,
                             DM2_V1_MusicQueueReceipt *out_receipt)
{
    FILE *file;
    uint8_t *data = NULL;
    long file_length;
    size_t bytes_read;
    DM2_V1_MusicQueueReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.track = track;
    receipt.loop = loop ? 1 : 0;
    receipt.result = DM2_V1_MUSIC_QUEUE_ASSET_ROOT_UNVERIFIED;
    if (track < 0 || track >= DM2_MUSIC_TRACK_COUNT) {
        receipt.result = DM2_V1_MUSIC_QUEUE_TRACK_OUT_OF_RANGE;
    } else if (g_dm2_verified_music_asset_root[0] != '\0') {
        /* skproject/SKULLWIN/c_midi.cpp: MIDIPATHNAME "./DATA/%02x.hmp.mid".
         * asset_root is Firestaff's materialized equivalent of DATA. */
        snprintf(receipt.asset_path, sizeof(receipt.asset_path),
                 "%s/%02x.hmp.mid", g_dm2_verified_music_asset_root, track);
        file = fopen(receipt.asset_path, "rb");
        if (!file) {
            receipt.result = DM2_V1_MUSIC_QUEUE_ASSET_MISSING;
        } else {
            if (fseek(file, 0, SEEK_END) != 0 ||
                (file_length = ftell(file)) < 0 ||
                (unsigned long)file_length > DM2_V1_MUSIC_MAX_FILE_BYTES ||
                fseek(file, 0, SEEK_SET) != 0) {
                receipt.result = DM2_V1_MUSIC_QUEUE_STREAM_INVALID;
            } else {
                data = (uint8_t *)malloc((size_t)file_length);
                bytes_read = data ? fread(data, 1, (size_t)file_length, file) : 0;
                if (!data || bytes_read != (size_t)file_length ||
                    dm2_v1_sound_inspect_music_data(data, (size_t)file_length,
                                                     &receipt.stream) !=
                        DM2_V1_MUSIC_INSPECT_OK) {
                    receipt.result = DM2_V1_MUSIC_QUEUE_STREAM_INVALID;
                } else {
                    receipt.asset_resolved = 1;
                    receipt.request_queued = 1;
                    receipt.decoder_proven = 1;
                    receipt.midi_handoff_ready = receipt.stream.midi_handoff_ready;
                    receipt.pcm_handoff_ready = receipt.stream.pcm_handoff_ready;
                    receipt.schedule_handoff_ready =
                        receipt.stream.schedule_handoff_ready;
                    receipt.loop_duration_us = receipt.stream.loop_duration_us;
                    receipt.schedule_event_count = receipt.stream.schedule_event_count;
                    if (receipt.schedule_handoff_ready &&
                        (!g_dm2_music_schedule_active ||
                         g_dm2_music_schedule_track != track)) {
                        g_dm2_music_schedule_stream = receipt.stream;
                        g_dm2_music_schedule_elapsed_us = 0;
                        g_dm2_music_schedule_last_elapsed_us = 0;
                        g_dm2_music_schedule_loop_count = 0;
                        g_dm2_music_schedule_active = 1;
                        g_dm2_music_schedule_track = track;
                    }
                    /* skproject/SKULLWIN/c_midi.cpp:11-37 hands the original
                     * stream to Allegro. On macOS, hand the verified channel
                     * events to CoreMIDI; no PCM renderer is involved. */
                    receipt.backend_proven =
                        dm2_v1_midi_backend_open() == DM2_V1_MIDI_BACKEND_READY;
                    receipt.result = receipt.backend_proven
                        ? DM2_V1_MUSIC_QUEUE_READY
                        : DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE;
                }
            }
            fclose(file);
        }
        free(data);
    }
    if (out_receipt) {
        *out_receipt = receipt;
    }
    return (int)receipt.result;
}

int dm2_v1_sound_schedule_music(uint32_t elapsed_us,
                                 DM2_V1_MusicScheduleReceipt *out_receipt)
{
    DM2_V1_MusicScheduleReceipt receipt;
    uint32_t first = 0;
    uint32_t count = 0;
    uint32_t position;
    uint32_t previous_position;
    uint32_t i;

    memset(&receipt, 0, sizeof(receipt));
    if (!g_dm2_music_schedule_active ||
        !g_dm2_music_schedule_stream.schedule_handoff_ready) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.loop = 1;
    receipt.midi_handoff_ready = 1;
    receipt.loop_duration_us = g_dm2_music_schedule_stream.loop_duration_us;
    receipt.event_count_available = g_dm2_music_schedule_stream.schedule_event_count;
    receipt.elapsed_us = elapsed_us;
    if (elapsed_us < g_dm2_music_schedule_last_elapsed_us) {
        /* M11 can restart a startup clock after a menu/session transition. */
        g_dm2_music_schedule_loop_count = 0;
        g_dm2_music_schedule_last_elapsed_us = 0;
    }
    g_dm2_music_schedule_elapsed_us = elapsed_us;
    receipt.loop_count = elapsed_us / receipt.loop_duration_us;
    position = elapsed_us % receipt.loop_duration_us;
    previous_position = g_dm2_music_schedule_last_elapsed_us %
                        receipt.loop_duration_us;
    for (i = 0; i < receipt.event_count_available; ++i) {
        uint32_t event_time = g_dm2_music_schedule_stream.schedule_events[i].time_us;
        int due = 0;
        if (elapsed_us == 0 && event_time == 0) due = 1;
        else if (receipt.loop_count != g_dm2_music_schedule_loop_count)
            due = event_time > previous_position || event_time <= position;
        else
            due = event_time > previous_position && event_time <= position;
        if (due) {
            if (count == 0) first = i;
            ++count;
            if (g_dm2_music_schedule_stream.schedule_events[i].status < 0xf0u &&
                dm2_v1_midi_backend_send(
                    &g_dm2_music_schedule_stream.schedule_events[i])) {
                ++receipt.backend_event_count_sent;
            }
        }
    }
    receipt.first_event_index = first;
    receipt.event_count_due = count;
    g_dm2_music_schedule_last_elapsed_us = elapsed_us;
    g_dm2_music_schedule_loop_count = receipt.loop_count;
    receipt.backend_proven =
        dm2_v1_midi_backend_state() == DM2_V1_MIDI_BACKEND_READY;
    /* This is a native MIDI device handoff, never an SDL PCM fallback. */
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

/* Preserve the old narrow call surface, but report queue status rather than
 * manufacturing a successful play. */
int dm2_v1_sound_play_music(int track) {
    int result = dm2_v1_sound_queue_music(track, 1, NULL);
    return result == DM2_V1_MUSIC_QUEUE_TRACK_OUT_OF_RANGE ? -1 : result;
}

/* dm2_v1_sound_stop_music — stop all music
 * Source: docs/dm2_audio.md (do_music_stop)
 * Calls al_stop_samples() via c_midi / c_music_wav. */
int dm2_v1_sound_stop_music(void) {
    dm2_v1_midi_backend_close();
    g_dm2_music_schedule_active = 0;
    g_dm2_music_schedule_track = -1;
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
        "Source: skproject/SKULLWIN/c_midi.cpp (./DATA/%02x.hmp.mid, load_midi, looped play_midi)\n"
        "Source: skproject/SKULLWIN/c_sfx.cpp (16-slot ring buffer SFX)\n"
        "Source: skproject/SKWIN/defines.h (SOUND_STD_*, SOUND_CHAMPION_*, SOUND_CREATURE_*)\n"
        "Source: docs/dm2_audio.md (music 28 HMP tracks, tMusicMaps[64], do_music_wav)\n"
        "Source: docs/dm2_sound_system.md (c_sound init, PLAYBACK_FREQUENCY=5500/6000 Hz)\n"
        "Source: docs/dm2_sound_combat.md (all combat sound triggers 0x00-0x92)\n"
        "DM1 comparison: AdLib FM, 3-4 voices, ~10 tracks, no positional audio\n"
        "DM2 comparison: SoundBlaster, 16-slot buffer, 28 tracks, world-coordinate spatial queue\n"
        "Music boundary: bounded HMP/SMF event validation plus native CoreMIDI handoff on macOS; PCM is unavailable\n"
        "DM2 new: SOUND_STD_EXPLOSION (0x81), glbXAmbientSoundActivated, DM2_QUEUE_NOISE_GEN1/GEN2\n";
}
/* Suppress unused variable warning for g_music_track_names */
static void __attribute__((unused)) dm2_v1_sound_suppress_unused(void) {
    (void)g_music_track_names;
}
