#ifndef NEXUS_V1_SOUND_H
#define NEXUS_V1_SOUND_H

#include <stdint.h>

/* Nexus V1 sound system — SFX + CD audio.
 * Source: docs/nexus_audio_format.md, docs/nexus_sfx.md,
 * docs/nexus_music.md, nexus_v1_engine.c CD track switching.
 *
 * Per-level SFX: SNDLEV00-15.SAL (sound banks) + SNDLEV00-15.MAP (event map).
 * CD audio: 8 tracks (2-9) mapped to level pairs, Red Book Audio.
 * Sound driver: SDDRVS.TSK (26 KB Saturn sound driver task).
 *
 * Status: STUB. SAL/MAP format unknown, no actual SFX playback.
 * Stub provides API surface and future hook points. */

/* Sound event types (matching SNDLEV*.MAP event IDs) */
typedef enum {
    NEXUS_SFX_NONE = 0,
    NEXUS_SFX_FOOTSTEP       = 1,   /* party footstep */
    NEXUS_SFX_DOOR_OPEN      = 2,
    NEXUS_SFX_DOOR_CLOSE     = 3,
    NEXUS_SFX_ATTACK_HIT     = 4,
    NEXUS_SFX_ATTACK_MISS    = 5,
    NEXUS_SFX_CHAMPION_HURT  = 6,
    NEXUS_SFX_CREATURE_DEATH = 7,
    NEXUS_SFX_CREATURE_ATTACK= 8,
    NEXUS_SFX_SPELL_CAST     = 9,
    NEXUS_SFX_SPELL_IMPACT   = 10,
    NEXUS_SFX_PICKUP_ITEM    = 11,
    NEXUS_SFX_DROP_ITEM      = 12,
    NEXUS_SFX_STAIRS         = 13,
    NEXUS_SFX_TELEPORT       = 14,
    NEXUS_SFX_ALARM          = 15,
    NEXUS_SFX_PIT_FALL       = 16,
    NEXUS_SFX_MENU_SELECT    = 17,
    NEXUS_SFX_MENU_CONFIRM   = 18,
    NEXUS_SFX_MENU_CANCEL    = 19,
    NEXUS_SFX_GOLD_PICKUP    = 20,
    NEXUS_SFX_EXIT_REACHED   = 21,  /* dungeon exit / game complete */
    NEXUS_SFX_PARTY_HURT    = 22,  /* party member damaged */
    NEXUS_SFX_LEVEL_UP      = 23,  /* champion leveled up */
    NEXUS_SFX_MAGIC_SHIELD  = 24,  /* spell effect: shield */
    NEXUS_SFX_MAGIC_HEAL    = 25,  /* spell effect: heal */
    NEXUS_SFX_MAGIC_DAMAGE  = 26   /* spell effect: damage */
} Nexus_SoundEvent;

/* ═══════════════════════════════════════════════════════════════════
 * Sound engine
 * ═══════════════════════════════════════════════════════════════════ */

typedef struct {
    int initialized;
    int sfx_enabled;
    int music_enabled;
    int current_cd_track;
    int current_level;
    /* SAL/MAP data for current level */
    uint8_t *sal_data;
    int sal_size;
    uint8_t *map_data;
    int map_size;
} Nexus_SoundEngine;

/* Init sound system */
int nexus_sound_init(Nexus_SoundEngine *eng);

/* Shutdown */
void nexus_sound_shutdown(Nexus_SoundEngine *eng);

/* Load SFX bank for level (SNDLEV##.SAL + SNDLEV##.MAP).
 * Call on level load. Pass NULL to use default. */
int nexus_sound_load_level(Nexus_SoundEngine *eng, int level_index,
                            const uint8_t *sal_data, int sal_size,
                            const uint8_t *map_data, int map_size);

/* Play a sound event by ID (from SNDLEV*.MAP mapping) */
void nexus_sound_play(Nexus_SoundEngine *eng, Nexus_SoundEvent event);

/* Play a sound by raw sample index (for direct SAL access) */
void nexus_sound_play_idx(Nexus_SoundEngine *eng, int sample_index);

/* CD audio track management */
int nexus_sound_cd_track(Nexus_SoundEngine *eng, int track_number);
int nexus_sound_cd_stop(Nexus_SoundEngine *eng);
int nexus_sound_cd_pause(Nexus_SoundEngine *eng);
int nexus_sound_cd_resume(Nexus_SoundEngine *eng);

/* Music fade (level transition) */
void nexus_sound_music_fade(Nexus_SoundEngine *eng, int fade_out_ms);

/* Mute/unmute */
void nexus_sound_set_sfx(Nexus_SoundEngine *eng, int enabled);
void nexus_sound_set_music(Nexus_SoundEngine *eng, int enabled);

/* Event name for debug */
const char *nexus_sound_event_name(Nexus_SoundEvent event);

#endif /* NEXUS_V1_SOUND_H */