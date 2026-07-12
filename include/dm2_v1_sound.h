#ifndef FIRESTAFF_DM2_V1_SOUND_H
#define FIRESTAFF_DM2_V1_SOUND_H
#include <stdint.h>
#include <stddef.h>

/* DM2 V1 — Sound System
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKULLWIN/c_sound.h/cpp, c_sfx.cpp
 * docs/dm2_audio.md, docs/dm2_sound_system.md, docs/dm2_sound_combat.md
 *
 * DM2 audio:
 *   - 16-slot SFX ring buffer (SKWin) vs 3-4 in DM1
 *   - SoundBlaster-compatible vs AdLib FM in DM1
 *   - 28 HMP/MIDI music tracks (00-1c.hex)
 *   - World-coordinate spatial queue with distance attenuation
 *   - SOUND_STD_EXPLOSION (0x81) for bombs — DM1 had no bombs
 */

/* ── Sound categories ───────────────────────────────────────────────────
 * Source: skproject/SKWIN/defines.h, docs/dm2_audio.md
 * GDAT sound entries resolved via DM2_QUERY_SND_ENTRY_INDEX(cls1,cls2,cls3) */

#define DM2_SOUND_CATEGORY_STANDARD   0   /* general SFX */
#define DM2_SOUND_CATEGORY_CHAMPION   1   /* champion actions */
#define DM2_SOUND_CATEGORY_CREATURE   2   /* creature SFX */
#define DM2_SOUND_CATEGORY_SPECIAL    3   /* special (0x60-0x92) */
#define DM2_SOUND_CATEGORY_MUSIC      4   /* music tracks */

/* ── Standard SFX constants ───────────────────────────────────────────
 * Source: docs/dm2_audio.md, docs/dm2_sound_combat.md
 * Hex values from defines.h (GDAT2 V5, V=0-4) */

#define DM2_SOUND_STD_EXPLOSION        0x81  /* bombs — DM2 new */
#define DM2_SOUND_STD_DEFAULT          0x84  /* punch, fall, test wall, gethit */
#define DM2_SOUND_STD_KNOCK            0x85  /* falling item, punch knock */
#define DM2_SOUND_STD_THROW            0x86  /* throw/shoot item */
#define DM2_SOUND_STD_ACTIVATION       0x88  /* GDAT2 V5 activation */
#define DM2_SOUND_STD_TELEPORT         0x89  /* GDAT2 V5 teleport */
#define DM2_SOUND_STD_ACTIVATION_MSG   0x00  /* message tick (PC9821 only) */
#define DM2_SOUND_STD_SPELL_MESSAGE    0x01  /* spell message (PC9821 only) */
#define DM2_SOUND_STD_TELEPORT_MSG     0x02  /* teleporter message (PC9821 only) */

/* ── Champion SFX constants ────────────────────────────────────────── */

#define DM2_SOUND_CHAMPION_ATTACK      0x00  /* attack swing */
#define DM2_SOUND_CHAMPION_SHOOT       0x01  /* ranged attack */
#define DM2_SOUND_CHAMPION_GETHIT      0x82  /* takes damage */
#define DM2_SOUND_CHAMPION_EAT_DRINK   0x83  /* consume food/water */
#define DM2_SOUND_CHAMPION_SCREAM      0x87  /* death scream */
#define DM2_SOUND_CHAMPION_BUMP        0x8A  /* collision/bump */
#define DM2_SOUND_CHAMPION_FOOTSTEP    0x92  /* SPX custom, not in retail */

/* ── Creature SFX constants ─────────────────────────────────────────── */

#define DM2_SOUND_CREATURE_MOVE        0x00  /* movement */
#define DM2_SOUND_CREATURE_TURN        0x01  /* turning (Minion) */
#define DM2_SOUND_CREATURE_GET_HIT     0x02  /* hit reaction */
#define DM2_SOUND_CREATURE_REFLECTOR   0x03  /* reflecting (Dragoth) */
#define DM2_SOUND_CREATURE_JUMP        0x04  /* jump (Rocky) */
#define DM2_SOUND_CREATURE_ATTACK_1    0x07  /* melee attack */
#define DM2_SOUND_CREATURE_PICK_STEAL  0x08  /* pick/steal (Thief) */
#define DM2_SOUND_CREATURE_SPAWN       0x10  /* spawn/appear */
#define DM2_SOUND_CREATURE_DEATH       0x11  /* death */
#define DM2_SOUND_CREATURE_ATTACK_2    0x12  /* secondary attack (Thorn Demon) */

/* ── Special SFX constants ─────────────────────────────────────────── */

#define DM2_SOUND_ITEM_TAKE            0x60  /* SPX custom */
#define DM2_SOUND_ITEM_PUT_DOWN        0x61  /* SPX custom */
#define DM2_SOUND_DOOR_STEP            0x8E  /* step on door */

/* ── Playback frequency ───────────────────────────────────────────────
 * Source: docs/dm2_audio.md
 * DOS original: 5500 Hz. Windows SDL port: 6000 Hz. */

#define DM2_PLAYBACK_FREQUENCY_DOS     5500
#define DM2_PLAYBACK_FREQUENCY_WIN     6000

/* ── Music track count ─────────────────────────────────────────────────
 * Source: docs/dm2_audio.md
 * 28 HMP tracks (DATA/00.hmp.mid through DATA/1c.hmp.mid)
 * tMusicMaps[64] maps dungeon map to track number */

#define DM2_MUSIC_TRACK_COUNT          28  /* 00-1c = 28 tracks */
#define DM2_MUSIC_MAP_COUNT             64  /* tMusicMaps[64] lookup table */
#define DM2_V1_MUSIC_MAX_FILE_BYTES     (4u * 1024u * 1024u)
#define DM2_V1_MUSIC_MAX_TRACKS         32u
#define DM2_V1_MUSIC_MAX_EVENTS         262144u
/* The title stream has 564 events.  Keep the M11 handoff bounded even when
 * inspecting a hostile but otherwise valid MIDI file. */
#define DM2_V1_MUSIC_MAX_SCHEDULE_EVENTS 4096u

typedef struct DM2_V1_MusicScheduledEvent {
    uint32_t tick;
    uint32_t time_us;
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
    uint8_t data_size;
    uint32_t payload;
} DM2_V1_MusicScheduledEvent;

/* `00.hmp.mid` is loaded by SKWin's Allegro MIDI path.  The shipped
 * SKWin title fixture is an SMF (`MThd`) under that original filename, while
 * original HMI SOS HMP streams start `HMIMIDIP`.  Preserve both forms as
 * MIDI event streams; neither form contains PCM samples. */
typedef enum DM2_V1_MusicFormat {
    DM2_V1_MUSIC_FORMAT_NONE = 0,
    DM2_V1_MUSIC_FORMAT_STANDARD_MIDI,
    DM2_V1_MUSIC_FORMAT_HMP_V1,
    DM2_V1_MUSIC_FORMAT_HMP_013195
} DM2_V1_MusicFormat;

typedef enum DM2_V1_MusicInspectResult {
    DM2_V1_MUSIC_INSPECT_OK = 0,
    DM2_V1_MUSIC_INSPECT_EMPTY,
    DM2_V1_MUSIC_INSPECT_FILE_TOO_LARGE,
    DM2_V1_MUSIC_INSPECT_BAD_SIGNATURE,
    DM2_V1_MUSIC_INSPECT_TRUNCATED,
    DM2_V1_MUSIC_INSPECT_BAD_HEADER,
    DM2_V1_MUSIC_INSPECT_BAD_TRACK,
    DM2_V1_MUSIC_INSPECT_BAD_EVENT,
    DM2_V1_MUSIC_INSPECT_LIMIT_EXCEEDED
} DM2_V1_MusicInspectResult;

typedef struct DM2_V1_MusicTrackReceipt {
    uint32_t byte_offset;
    uint32_t byte_size;
    uint32_t event_count;
    uint32_t end_of_track_count;
    uint32_t duration_ticks;
} DM2_V1_MusicTrackReceipt;

typedef struct DM2_V1_MusicStreamReceipt {
    DM2_V1_MusicInspectResult result;
    DM2_V1_MusicFormat format;
    uint32_t file_size;
    uint32_t track_count;
    uint32_t event_count;
    uint32_t channel_event_count;
    uint32_t meta_event_count;
    uint32_t sysex_event_count;
    uint16_t time_division;
    uint32_t duration_ticks;
    uint32_t loop_duration_us;
    uint32_t schedule_event_count;
    int schedule_handoff_ready;
    int midi_handoff_ready;
    int pcm_handoff_ready;
    DM2_V1_MusicTrackReceipt tracks[DM2_V1_MUSIC_MAX_TRACKS];
    DM2_V1_MusicScheduledEvent
        schedule_events[DM2_V1_MUSIC_MAX_SCHEDULE_EVENTS];
} DM2_V1_MusicStreamReceipt;

/* M11 advances this receipt from its monotonic audio-timing handoff. Due
 * events may be handed to a capability-proven native MIDI device; no PCM
 * queue is implied. */
typedef struct DM2_V1_MusicScheduleReceipt {
    int valid;
    int loop;
    int backend_proven;
    int midi_handoff_ready;
    int pcm_handoff_ready;
    uint32_t elapsed_us;
    uint32_t loop_duration_us;
    uint32_t loop_count;
    uint32_t event_count_due;
    uint32_t backend_event_count_sent;
    uint32_t first_event_index;
    uint32_t event_count_available;
} DM2_V1_MusicScheduleReceipt;

/* skproject/SKULLWIN/c_sound.cpp DM2_PLAY_MUSIC() calls c_midi::do_music(),
 * which loads ./DATA/%02x.hmp.mid and asks the original backend to loop it. */
typedef enum DM2_V1_MusicQueueResult {
    DM2_V1_MUSIC_QUEUE_READY = 0,
    DM2_V1_MUSIC_QUEUE_ASSET_ROOT_UNVERIFIED,
    DM2_V1_MUSIC_QUEUE_TRACK_OUT_OF_RANGE,
    DM2_V1_MUSIC_QUEUE_ASSET_MISSING,
    DM2_V1_MUSIC_QUEUE_STREAM_INVALID,
    DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE
} DM2_V1_MusicQueueResult;

typedef struct DM2_V1_MusicQueueReceipt {
    DM2_V1_MusicQueueResult result;
    int track;
    int loop;
    int asset_resolved;
    int request_queued;
    int decoder_proven;
    int backend_proven;
    int midi_handoff_ready;
    int pcm_handoff_ready;
    int schedule_handoff_ready;
    uint32_t loop_duration_us;
    uint32_t schedule_event_count;
    DM2_V1_MusicStreamReceipt stream;
    char asset_path[512];
} DM2_V1_MusicQueueReceipt;

/* ── Sound queue ────────────────────────────────────────────────────────
 * Source: skproject/SKULLWIN/c_sound.h/cpp, c_sfx.cpp
 * 16-slot ring buffer with world-coordinate positional audio.
 * DM2_QUEUE_NOISE_GEN1/GEN2 for queued sound effects.
 * glbXAmbientSoundActivated for weather ambient. */

#define DM2_SFX_QUEUE_SIZE             16

/* ── Sound definition entry (per-category lookup) ────────────────────────
 * Source: docs/dm2_audio.md (SoundEntryInfo, glbSoundList[64])
 * DM2_QUERY_SND_ENTRY_INDEX(cls1, cls2, cls3) → entry index → DM2_PLAY_SOUND()
 *
 * Sound entry contains: category, class1, class2, sound ID, volume, pitch.
 * World-coordinate queue with distance attenuation. */

typedef struct {
    uint8_t category;
    uint8_t class1;
    uint8_t class2;
    uint8_t sound_id;
    int8_t  volume;     /* -100 to +100 relative */
    int8_t  pitch_shift;
} DM2_SoundEntry;

/* ── Public API ──────────────────────────────────────────────────────── */

int  dm2_v1_sound_query_entry(uint8_t cat, uint8_t c1, uint8_t c2, uint8_t sfx);
int  dm2_v1_sound_play(int sound_id, int volume);
int  dm2_v1_sound_play_positional(int sound_id, int world_x, int world_y, int listener_x, int listener_y);
void dm2_v1_sound_bind_verified_music_assets(const char *asset_root,
                                             int primary_assets_verified);
int  dm2_v1_sound_inspect_music_data(const uint8_t *data, size_t size,
                                     DM2_V1_MusicStreamReceipt *out_receipt);
int  dm2_v1_sound_queue_music(int track, int loop,
                              DM2_V1_MusicQueueReceipt *out_receipt);
int  dm2_v1_sound_schedule_music(uint32_t elapsed_us,
                                  DM2_V1_MusicScheduleReceipt *out_receipt);
int  dm2_v1_sound_play_music(int track);
int  dm2_v1_sound_stop_music(void);
const char *dm2_v1_sound_name(int category, int sound_id);
const char *dm2_v1_sound_source_evidence(void);

#endif /* FIRESTAFF_DM2_V1_SOUND_H */
