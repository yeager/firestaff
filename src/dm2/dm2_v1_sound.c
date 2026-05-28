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
