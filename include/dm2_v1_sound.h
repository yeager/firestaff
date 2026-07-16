#ifndef FIRESTAFF_DM2_V1_SOUND_H
#define FIRESTAFF_DM2_V1_SOUND_H
#include <stdint.h>

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
    uint16_t queued_count;
    uint16_t queue_capacity;
    uint16_t active_sample_count;
    uint16_t pending_positional_count;
    uint16_t pending_immediate_count;
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
    uint16_t returned_index;
    uint16_t queued_count_before;
    uint16_t queued_count_after;
    uint16_t removed_count;
    uint16_t play_count;
    int16_t argument0;
    int16_t argument1;
    int16_t argument2;
    int16_t selected_music_track;
    int16_t volume;
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

/* ── Public API ──────────────────────────────────────────────────────── */

int  dm2_v1_sound_query_entry(uint8_t cat, uint8_t c1, uint8_t c2, uint8_t sfx);
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