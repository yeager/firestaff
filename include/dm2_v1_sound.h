#ifndef FIRESTAFF_DM2_V1_SOUND_H
#define FIRESTAFF_DM2_V1_SOUND_H
#include <stddef.h>
#include <stdint.h>

#include "dm2_v1_sound_queue_pc34_compat.h"
#include "dm2_v1_asset_loader.h"

/* DM2 V1 — Sound System
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKULLWIN/c_sound.h/cpp, c_sfx.cpp
 * docs/dm2_audio.md, docs/dm2_sound_system.md, docs/dm2_sound_combat.md
 *
 * DM2 audio:
 *   - 16-slot SFX ring buffer (SKWin) vs 3-4 in DM1
 *   - SoundBlaster-compatible vs AdLib FM in DM1
 *   - 29 HMP/MIDI music tracks (00-1c.hex)
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
 * PC release: 29 HMP records are stored in GRAPHICS.DAT GDAT category 4,
 * type 3 (dtHMP). SONGLIST.DAT maps dungeon maps to track numbers. */

#define DM2_MUSIC_TRACK_COUNT          29  /* 00-1c inclusive */
#define DM2_MUSIC_RUNTIME_MAP_SIZE      64  /* tMusicMaps[64] lookup table */

typedef enum {
    DM2_V1_MUSIC_FORMAT_UNKNOWN = 0,
    DM2_V1_MUSIC_FORMAT_STANDARD_MIDI,
    DM2_V1_MUSIC_FORMAT_HMP_V1
} DM2_V1_MusicFormat;

typedef enum {
    DM2_V1_MUSIC_INSPECT_OK = 0,
    DM2_V1_MUSIC_INSPECT_BAD_EVENT = -1,
    DM2_V1_MUSIC_INSPECT_BAD_HEADER = -2
} DM2_V1_MusicInspectResult;

typedef enum {
    DM2_V1_MUSIC_QUEUE_READY = 1,
    DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE = 0,
    DM2_V1_MUSIC_QUEUE_ASSET_ROOT_UNVERIFIED = -1,
    DM2_V1_MUSIC_QUEUE_TRACK_OUT_OF_RANGE = -2,
    DM2_V1_MUSIC_QUEUE_ASSET_MISSING = -3
} DM2_V1_MusicQueueResult;

typedef struct {
    uint32_t tick;
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
    uint8_t data_size;
} DM2_V1_MusicScheduledEvent;

typedef struct {
    uint16_t end_of_track_count;
} DM2_V1_MusicTrackReceipt;

typedef struct {
    int valid;
    DM2_V1_MusicFormat format;
    uint16_t track_count;
    uint32_t event_count;
    uint32_t channel_event_count;
    uint32_t meta_event_count;
    uint32_t duration_ticks;
    uint32_t loop_duration_us;
    uint16_t schedule_event_count;
    int schedule_handoff_ready;
    int midi_handoff_ready;
    int pcm_handoff_ready;
    /* The inspector walks every source track.  This bounded receipt retains
     * per-track EOT counters for the first 16 only. */
    DM2_V1_MusicTrackReceipt tracks[16];
} DM2_V1_MusicStreamReceipt;

typedef struct {
    int asset_resolved;
    int request_queued;
    int decoder_proven;
    int backend_proven;
    int schedule_handoff_ready;
    uint32_t loop_duration_us;
    uint16_t schedule_event_count;
    char asset_path[256];
} DM2_V1_MusicQueueReceipt;

typedef struct {
    int valid;
    uint16_t event_count_due;
    uint16_t loop_count;
    int backend_proven;
    int pcm_handoff_ready;
} DM2_V1_MusicScheduleReceipt;

/* ── Sound queue ────────────────────────────────────────────────────────
 * Source: skproject/SKULLWIN/c_sound.h/cpp, c_sfx.cpp
 * 16-slot ring buffer with world-coordinate positional audio.
 * DM2_QUEUE_NOISE_GEN1/GEN2 for queued sound effects.
 * glbXAmbientSoundActivated for weather ambient. */

#define DM2_SFX_QUEUE_SIZE             16
#define DM2_V1_SKPROJECT_SOUND_QUEUE_MAX 64u

typedef struct {
    int8_t b_02;
    int8_t b_03;
    int8_t b_04;
    int16_t w_05;
} DM2_V1_SkprojectSoundQueueEntry;

typedef struct {
    int32_t l_00;
    int16_t w_04;
} DM2_V1_SkprojectMidiVoice;

typedef struct {
    uint8_t ub_00;
    uint16_t w_01;
} DM2_V1_SkprojectSoundBearing;

typedef struct {
    uint8_t ub_04;
    uint8_t ub_05;
    uint8_t ub_06;
    uint8_t ub_07;
    DM2_V1_SkprojectSoundBearing s59_08;
    uint8_t ub_0b;
} DM2_V1_SkprojectSfx;

typedef struct {
    uint16_t next_index;
    int present;
} DM2_V1_SkprojectSoundAllocationNode;

typedef struct {
    int sound_enabled;
    int midi_handle_present;
    int midi_transition_enabled;
    int midi_ready;
    int sfx_active;
    int master_sfx_volume;
    int midi_volume;
    int pending_music_track;
    int current_music_track;
    int pending_music_fade;
    int midi_stop_armed;
    int midi_defer_stop;
    int16_t midi_fade_counter;
    uint8_t midi_program;
    uint8_t midi_active_voice_mask;
    uint8_t midi_selected_voice;
    uint16_t queued_count;
    uint16_t queue_capacity;
    uint16_t active_sample_count;
    uint16_t pending_positional_count;
    uint16_t pending_immediate_count;
    int32_t active_sample_handles[64];
    DM2_V1_SkprojectMidiVoice midi_voices[8];
    DM2_V1_SkprojectSoundQueueEntry
        queue[DM2_V1_SKPROJECT_SOUND_QUEUE_MAX];
} DM2_V1_SkprojectSoundState;

typedef struct {
    int valid;
    int rejected_duplicate;
    int rejected_full;
    int rejected_disabled;
    int play_sound_requested;
    int play_music_requested;
    int stop_music_requested;
    int queue_noise_requested;
    int stop_sfx_requested;
    int uninstall_audio_requested;
    int allocation_requested;
    uint16_t returned_index;
    uint16_t queued_count_before;
    uint16_t queued_count_after;
    uint16_t removed_count;
    uint16_t play_count;
    uint16_t reset_count;
    uint16_t refresh_count;
    uint16_t scanned_count;
    uint32_t allocation_size;
    int16_t argument0;
    int16_t argument1;
    int16_t argument2;
    int16_t selected_music_track;
    int16_t volume;
    uint16_t bearing;
    uint8_t attenuation;
} DM2_V1_SkprojectSoundReceipt;

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

/* ── DM2-008 audible playback backend (cycle 16) ─────────────────────────
 * Source: skproject/SKWIN/SkwinSDL.cpp (OpenAudio/sdlAudMix, MAX_SB = 16,
 * PLAYBACK_FREQUENCY = 6000 Hz, sample bytes converted 0x80 + raw_byte at
 * alloc time) and docs/dm2_sound_format.md.  GDAT sound raw entries hold a
 * two-byte format header followed by signed 8-bit mono PCM; the decoded
 * unsigned 8-bit stream is payload_byte ^ 0x80, exactly the conversion the
 * source performs (cf. dm2_v1_gdat_sound_toggle_payload). */

#define DM2_V1_SOUND_VOICE_MAX 16u          /* SKWin MAX_SB */
#define DM2_V1_SOUND_PCM_SAMPLE_RATE_HZ 6000u /* SKWin SDL playback rate */

typedef struct {
    uint8_t accepted;
    uint8_t rejected_no_loader;
    uint8_t rejected_entry_missing;
    uint8_t rejected_capacity;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint16_t raw_index;
    uint32_t sample_count;
    uint32_t sample_rate_hz;
    uint32_t pcm_hash;
} DM2_V1_SoundPcmReceipt;

/* Decoded sample count for a verified GDAT sound entry, 0 when unavailable. */
uint32_t dm2_v1_sound_gdat_pcm_sample_count(uint8_t cls1, uint8_t cls2,
                                            uint8_t cls3);

/* Decode the unsigned 8-bit PCM stream of a verified GDAT sound entry.
 * Returns 1 on success; out_pcm may be NULL to query the receipt only. */
int dm2_v1_sound_decode_gdat_pcm(uint8_t cls1, uint8_t cls2, uint8_t cls3,
                                 uint8_t *out_pcm, size_t out_capacity,
                                 DM2_V1_SoundPcmReceipt *out_receipt);

/* Playback backend vtable.  The sound module stays SDL-free; a concrete
 * backend (SDL3, dummy capture, ...) is bound by the consumer.  start_voice
 * takes ownership of the pcm pointer for the duration of the call only — the
 * caller keeps the buffer alive until the voice reports inactive. */
typedef struct {
    void *ctx;
    int (*open)(void *ctx);
    int (*is_ready)(void *ctx);
    int (*start_voice)(void *ctx, unsigned voice_slot, const uint8_t *pcm,
                       uint32_t sample_count, uint8_t volume);
    int (*voice_active)(void *ctx, unsigned voice_slot);
    void (*stop_all)(void *ctx);
    void (*close)(void *ctx);
} DM2_V1_SoundPlaybackBackend;

void dm2_v1_sound_bind_playback_backend(
    const DM2_V1_SoundPlaybackBackend *backend);

/* True only while a host has bound a real playback backend. This does not
 * open a device or admit sound data; GDAT verification remains mandatory at
 * each playback request. */
int dm2_v1_sound_playback_backend_bound(void);

typedef struct {
    uint8_t valid;
    uint8_t rejected_no_loader;
    uint8_t rejected_no_backend;
    uint8_t rejected_backend_not_ready;
    uint8_t rejected_decode_failed;
    uint8_t rejected_no_free_voice;
    uint8_t playback_started;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t voice_slot;
    uint8_t volume;
    uint8_t attenuation;
    uint16_t raw_index;
    uint32_t sample_count;
} DM2_V1_SoundPlaybackReceipt;

/* Play a verified GDAT sound entry on an allocated voice.  Volume is clamped
 * to the source 0..255 byte range; attenuation is the source-locked R_928
 * metric (c_sound.cpp:256-308) with dx/dy == 0, i.e. full volume. */
int dm2_v1_sound_play_gdat_entry(uint8_t cls1, uint8_t cls2, uint8_t cls3,
                                 int volume,
                                 DM2_V1_SoundPlaybackReceipt *out_receipt);

/* Positional variant: dx/dy are the source rotated deltas (clamped to the
 * int8 s_sfx range); the effective volume is the R_928 attenuation byte. */
int dm2_v1_sound_play_gdat_entry_positional(
    uint8_t cls1, uint8_t cls2, uint8_t cls3, int volume,
    int16_t dx, int16_t dy, DM2_V1_SoundPlaybackReceipt *out_receipt);

void dm2_v1_sound_stop_all_voices(void);
int dm2_v1_sound_voice_active(unsigned voice_slot);

/* ── Public API ──────────────────────────────────────────────────────── */

/* DM2-008 GDAT-backed sound backend. Bind a verified GDAT loader for the
 * later source-owned GDAT resolver and playback routes. DM2_SOUND9 itself
 * does not resolve a sample binding. */
void dm2_v1_sound_bind_gdat_loader(const DM2_V1_AssetLoader *loader,
                                   int verified);

/* Bind the live source-shaped xsndptr2 queue owned by the active DM2
 * runtime.  QUERY_SND_ENTRY_INDEX has no queue parameter in the original
 * call shape, so it must never allocate a private host substitute.  Pass
 * NULL while tearing down or before a verified runtime is active. */
void dm2_v1_sound_bind_runtime_queue(DM2_V1_SoundQueueState *state);

/* DM2_SOUND9: populate dm2sound.xsndptr2 (seven-byte s_ssound entry).
 * The original routine has no sample argument and always leaves w_05 at -1;
 * c_gdatfile.cpp::DM2_482b_0684 later owns the GDAT lookup, raw-index binding
 * and sndptr4 slot allocation. `sample_id` remains only for ABI compatibility
 * and is deliberately ignored. */
int dm2_v1_sound9(DM2_V1_SoundQueueState *state,
                  int8_t cls1,
                  int8_t cls2,
                  int8_t cls3,
                  int16_t sample_id,
                  uint16_t *out_index);

/* DM2_QUERY_SND_ENTRY_INDEX: 1-based index, 0 when absent. */
uint16_t dm2_v1_query_snd_entry_index(const DM2_V1_SoundQueueState *state,
                                      int8_t cls1,
                                      int8_t cls2,
                                      int8_t cls3);

/* DM2_QUERY_SND_ENTRY_INDEX against the bound live runtime queue.  A missing
 * queue is unavailable; this API never creates a private fallback queue.
 * Returns a 1-based index or -1. */
int  dm2_v1_sound_query_entry(uint8_t cls1, uint8_t cls2, uint8_t cls3);
int  dm2_v1_sound_play(int sound_id, int volume);
int  dm2_v1_sound_play_positional(int sound_id, int world_x, int world_y, int listener_x, int listener_y);
void dm2_v1_skproject_sound_state_init(DM2_V1_SkprojectSoundState *state,
                                       uint16_t queue_capacity);
int dm2_v1_skproject_sound1(DM2_V1_SkprojectSoundState *state,
                            DM2_V1_SkprojectSoundReceipt *out_receipt);
int16_t dm2_v1_skproject_get_music_index_from_modlist(
    const uint8_t *modlist,
    uint16_t modlist_size,
    int16_t map_index,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound2(DM2_V1_SkprojectSoundState *state,
                            int16_t map_index,
                            const uint8_t *music_map,
                            uint16_t music_map_count,
                            DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound3(DM2_V1_SkprojectSoundState *state,
                            int16_t volume,
                            int16_t mode,
                            DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound4(DM2_V1_SkprojectSoundState *state,
                            DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound5(DM2_V1_SkprojectSoundState *state,
                            DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound6(DM2_V1_SkprojectSoundState *state,
                            uint16_t queue_capacity,
                            DM2_V1_SkprojectSoundReceipt *out_receipt);
uint16_t dm2_v1_skproject_sound7(
    const DM2_V1_SkprojectSoundState *state,
    int16_t sound_handle);
int dm2_v1_skproject_sound8(DM2_V1_SkprojectSoundState *state,
                            int immediate,
                            DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound9(DM2_V1_SkprojectSoundState *state,
                            int8_t cls1,
                            int8_t cls2,
                            int8_t cls3,
                            DM2_V1_SkprojectSoundReceipt *out_receipt);
uint16_t dm2_v1_skproject_query_snd_entry_index(
    const DM2_V1_SkprojectSoundState *state,
    int8_t cls1,
    int8_t cls2,
    int8_t cls3);
int dm2_v1_skproject_process_sound(DM2_V1_SkprojectSoundState *state,
                                   uint16_t index,
                                   int16_t current_map,
                                   int16_t party_map,
                                   DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound_midi_program(
    DM2_V1_SkprojectSoundState *state,
    uint8_t program,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound_discard_midi_word(
    DM2_V1_SkprojectSoundReceipt *out_receipt,
    uint16_t word);
int dm2_v1_skproject_sound_reset_midi_voice(
    DM2_V1_SkprojectSoundState *state,
    uint32_t voice_index,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound_stop_armed_music(
    DM2_V1_SkprojectSoundState *state,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound_compute_sfx_metric(
    DM2_V1_SkprojectSfx *sfx,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound_sfx_precedes(
    const DM2_V1_SkprojectSfx *lhs,
    const DM2_V1_SkprojectSfx *rhs);
int dm2_v1_skproject_sound_sample_state(uint32_t sample_index);
int dm2_v1_skproject_sound_stop_sample_slot(
    DM2_V1_SkprojectSoundState *state,
    uint32_t sample_index,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
uint16_t dm2_v1_skproject_sound_active_sample_slots(
    const DM2_V1_SkprojectSoundState *state);
int dm2_v1_skproject_sound_drain_samples(
    DM2_V1_SkprojectSoundState *state,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound_stop_handle_table(
    DM2_V1_SkprojectSoundState *state,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound_release_allocation_node(
    DM2_V1_SkprojectSoundState *state,
    const DM2_V1_SkprojectSoundAllocationNode *nodes,
    uint16_t node_count,
    uint16_t head_index,
    uint16_t target_index,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound_stop_all(
    DM2_V1_SkprojectSoundState *state,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound_destruct(
    DM2_V1_SkprojectSoundReceipt *out_receipt);
int dm2_v1_skproject_sound6_sndptr6_allocation(
    uint32_t v1e0ad4,
    DM2_V1_SkprojectSoundReceipt *out_receipt);
/* Compatibility reset for the retired loose-sidecar path. Runtime music is
 * bound exclusively with dm2_v1_sound_bind_gdat_loader(). */
void dm2_v1_sound_bind_verified_music_assets(const char *asset_root,
                                             int primary_assets_verified);
/* SMF inspection can produce a scheduler handoff. Original HMP inspection is
 * diagnostic only and always leaves the handoff fields clear until a direct
 * source-format decoder exists. */
int  dm2_v1_sound_inspect_music_data(const uint8_t *data, size_t size,
                                     DM2_V1_MusicStreamReceipt *out_receipt);
int  dm2_v1_sound_queue_music(int track, int loop,
                              DM2_V1_MusicQueueReceipt *out_receipt);
/* Queue CDDA music: raw 16-bit signed LE stereo 44100Hz PCM.
 * The queue takes ownership (copies the data); caller may free after. */
int  dm2_v1_sound_queue_cdda(const uint8_t *pcm_data, size_t pcm_size,
                              int disc_track, int loop,
                              DM2_V1_MusicQueueReceipt *out_receipt);

typedef struct {
    int valid;
    int disc_track;
    const uint8_t *pcm_data;
    size_t pcm_size;
    int loop;
} DM2_V1_CddaFlushReceipt;

int  dm2_v1_sound_flush_cdda(DM2_V1_CddaFlushReceipt *out_receipt);
void dm2_v1_sound_release_cdda(void);

int  dm2_v1_sound_schedule_music(uint32_t elapsed_us,
                                  DM2_V1_MusicScheduleReceipt *out_receipt);
int  dm2_v1_sound_play_music(int track);
int  dm2_v1_sound_stop_music(void);
const char *dm2_v1_sound_name(int category, int sound_id);
const char *dm2_v1_sound_source_evidence(void);

#endif /* FIRESTAFF_DM2_V1_SOUND_H */
