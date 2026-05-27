#include "nexus_v1_sound.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Nexus V1 sound system — STUB implementation.
 * Source: docs/nexus_audio_format.md, docs/nexus_sfx.md,
 * docs/nexus_music.md, nexus_v1_engine.c CD track switching.
 *
 * Per-level SFX: SNDLEV00-15.SAL (sound banks, 290-460 KB each)
 *                SNDLEV00-15.MAP (event index, 66-90 bytes each)
 * CD audio: 8 tracks (2-9) mapped to level pairs.
 * Sound driver: SDDRVS.TSK (26 KB Saturn sound driver task).
 *
 * Status: STUB. SAL/MAP format unknown; no actual audio playback.
 * Provides API surface and logs play calls for future SDL_mixer integration.
 * Source: docs/nexus_sfx.md (no SFX implementation found in current source). */

/* Event name table */
static const char *g_event_names[] = {
    "NONE", "FOOTSTEP", "DOOR_OPEN", "DOOR_CLOSE",
    "ATTACK_HIT", "ATTACK_MISS", "CHAMPION_HURT", "CREATURE_DEATH",
    "CREATURE_ATTACK", "SPELL_CAST", "SPELL_IMPACT", "PICKUP_ITEM",
    "DROP_ITEM", "STAIRS", "TELEPORT", "ALARM", "PIT_FALL",
    "MENU_SELECT", "MENU_CONFIRM", "MENU_CANCEL", "GOLD_PICKUP",
    "EXIT_REACHED", "PARTY_HURT", "LEVEL_UP", "MAGIC_SHIELD",
    "MAGIC_HEAL", "MAGIC_DAMAGE"
};
#define EVENT_COUNT (sizeof(g_event_names)/sizeof(g_event_names[0]))

/* ═══════════════════════════════════════════════════════════════════
 * Init
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_sound_init(Nexus_SoundEngine *eng) {
    if (!eng) return -1;
    memset(eng, 0, sizeof(*eng));
    eng->initialized = 1;
    eng->sfx_enabled = 1;
    eng->music_enabled = 1;
    eng->current_cd_track = 2;
    eng->current_level = -1;
    printf("Nexus sound: initialized (stub — no actual audio playback)\n");
    return 0;
}

void nexus_sound_shutdown(Nexus_SoundEngine *eng) {
    if (!eng) return;
    if (eng->sal_data) { free(eng->sal_data); eng->sal_data = NULL; }
    if (eng->map_data) { free(eng->map_data); eng->map_data = NULL; }
    memset(eng, 0, sizeof(*eng));
}

/* ═══════════════════════════════════════════════════════════════════
 * Load SFX bank for level
 * SAL format unknown: 290-460 KB per level suggests compressed samples.
 * MAP format: 66-90 bytes = small index table.
 * TODO: reverse-engineer SAL (compressed PCM? Saturn SAS? ATRAC?)
 *       and MAP (event_id → sample_offset/size?).
 * Source: docs/nexus_audio_format.md.
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_sound_load_level(Nexus_SoundEngine *eng, int level_index,
                            const uint8_t *sal_data, int sal_size,
                            const uint8_t *map_data, int map_size) {
    if (!eng || !eng->initialized) return -1;
    if (level_index < 0 || level_index > 15) return -1;

    /* Free previous level data */
    if (eng->sal_data) { free(eng->sal_data); eng->sal_data = NULL; }
    if (eng->map_data) { free(eng->map_data); eng->map_data = NULL; }

    eng->current_level = level_index;

    if (sal_data && sal_size > 0) {
        eng->sal_data = (uint8_t *)malloc(sal_size);
        if (eng->sal_data) {
            memcpy(eng->sal_data, sal_data, sal_size);
            eng->sal_size = sal_size;
        }
    }

    if (map_data && map_size > 0) {
        eng->map_data = (uint8_t *)malloc(map_size);
        if (eng->map_data) {
            memcpy(eng->map_data, map_data, map_size);
            eng->map_size = map_size;
        }
    }

    printf("Nexus sound: loaded level %d SFX (SAL=%d bytes, MAP=%d bytes)\n",
        level_index, sal_size, map_size);
    return 0;
}

/* Play sound event — STUB logs the request.
 * Real implementation: look up event_id in MAP, get sample offset/size
 * from SAL, decode (if needed), play via SDL_mixer or platform audio.
 * TODO: SDL_mixer integration, SAL decode (unknown format).
 * Source: docs/nexus_audio_format.md (SAL format unknown). */
void nexus_sound_play(Nexus_SoundEngine *eng, Nexus_SoundEvent event) {
    const char *name;

    if (!eng || !eng->initialized) return;
    if (!eng->sfx_enabled) return;
    if (event <= NEXUS_SFX_NONE || event >= EVENT_COUNT) return;

    name = g_event_names[event];
    if (!name) name = "UNKNOWN";

    /* STUB: log only */
    /* TODO: real playback — MAP lookup + SAL decode + SDL_mixer */
    printf("Nexus SFX: %s\n", name);
    (void)eng;
}

void nexus_sound_play_idx(Nexus_SoundEngine *eng, int sample_index) {
    if (!eng || !eng->initialized) return;
    if (!eng->sfx_enabled) return;
    /* STUB: log only */
    printf("Nexus SFX: sample_idx=%d (MAP/SAL format unknown)\n", sample_index);
    (void)eng;
}

/* ═══════════════════════════════════════════════════════════════════
 * CD audio management
 * DM Nexus CD: tracks 2-9 are Red Book Audio music.
 * Level pairs: 0-1→track2, 2-3→track3, ..., 14-15→track9.
 * Source: docs/nexus_music.md, nexus_v1_game.c nexus_v1_cd_track_for_level().
 * TODO: SDL_mixer CD audio playback or platform equivalent.
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_sound_cd_track(Nexus_SoundEngine *eng, int track_number) {
    if (!eng || !eng->initialized) return -1;
    if (track_number < 2 || track_number > 9) return -1;

    eng->current_cd_track = track_number;
    printf("Nexus music: CD track %d (%s)\n",
        track_number,
        eng->music_enabled ? "playing" : "muted");
    return 0;
}

int nexus_sound_cd_stop(Nexus_SoundEngine *eng) {
    if (!eng) return -1;
    printf("Nexus music: stopped (stub)\n");
    return 0;
}

int nexus_sound_cd_pause(Nexus_SoundEngine *eng) {
    if (!eng) return -1;
    printf("Nexus music: paused (stub)\n");
    return 0;
}

int nexus_sound_cd_resume(Nexus_SoundEngine *eng) {
    if (!eng) return -1;
    printf("Nexus music: resumed (stub)\n");
    return 0;
}

void nexus_sound_music_fade(Nexus_SoundEngine *eng, int fade_out_ms) {
    if (!eng) return;
    printf("Nexus music: fade out %d ms → new track (stub)\n", fade_out_ms);
    (void)eng;
}

/* ═══════════════════════════════════════════════════════════════════
 * Mute controls
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_sound_set_sfx(Nexus_SoundEngine *eng, int enabled) {
    if (!eng) return;
    eng->sfx_enabled = enabled ? 1 : 0;
}

void nexus_sound_set_music(Nexus_SoundEngine *eng, int enabled) {
    if (!eng) return;
    eng->music_enabled = enabled ? 1 : 0;
}

const char *nexus_sound_event_name(Nexus_SoundEvent event) {
    if (event <= NEXUS_SFX_NONE || event >= EVENT_COUNT) return "UNKNOWN";
    return g_event_names[event];
}