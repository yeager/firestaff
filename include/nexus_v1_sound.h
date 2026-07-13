#ifndef NEXUS_V1_SOUND_H
#define NEXUS_V1_SOUND_H

#include "nexus_v1_audio_receipt.h"
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

/* Firestaff sound requests. These numeric values are not claimed to be
 * SNDLEV*.MAP event IDs: the original event-dispatch ABI is still unproven. */
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
    int sal_canonical_source_verified;
    int map_canonical_source_verified;
    /* Global SDDRVS.TSK source identity. It does not imply an understood
     * driver ABI, decoder, or playback route. */
    int sound_driver_canonical_source_verified;
    /* Direct byte-to-event routing is deliberately unavailable. MAP records
     * are retained as bounded opaque windows until Saturn dispatch evidence
     * proves their event-ID semantics. */
    int map_event_count;
    int map_mapped_event_count;
    int map_first_sample_index;
    int map_last_sample_index;
    int map_header_checksum16;
    int map_header_nonzero_byte_count;
    int map_header_distinct_byte_count;
    int map_header_transition_count;
    int map_record_table_supported;
    int map_record_count;
    int map_record_terminator_offset;
    int map_first_record_event;
    int map_min_record_event;
    int map_max_record_event;
    int map_record_event_span;
    int map_unique_record_event_count;
    int map_duplicate_record_event_count;
    int map_has_duplicate_record_events;
    int map_first_record_sal_offset;
    int map_first_record_size;
    int map_last_record_sal_offset;
    int map_max_record_end;
    int map_total_record_bytes;
    int map_out_of_bounds_record_count;
    int map_first_window_checksum16;
    int map_first_window_nonzero_byte_count;
    int map_first_window_high_bit_byte_count;
    int map_first_window_first_nonzero_relative_offset;
    int map_first_window_last_nonzero_relative_offset;
    int map_first_window_distinct_byte_count;
    int map_first_window_transition_count;
    int map_last_window_checksum16;
    int map_last_window_nonzero_byte_count;
    int map_last_window_high_bit_byte_count;
    int map_last_window_first_nonzero_relative_offset;
    int map_last_window_last_nonzero_relative_offset;
    int map_last_window_distinct_byte_count;
    int map_last_window_transition_count;
    int last_event;
    int last_sample_index;
    int last_event_record_found;
    int last_event_sal_offset;
    int last_event_sal_size;
    int last_event_window_checksum16;
    int last_event_window_nonzero_byte_count;
    int last_event_window_high_bit_byte_count;
    int last_event_window_first_nonzero_relative_offset;
    int last_event_window_last_nonzero_relative_offset;
    int last_event_window_distinct_byte_count;
    int last_event_window_transition_count;
    int sal_package_profile_supported;
    int sal_container_preamble_supported;
    int sal_payload_offset;
    int sal_opaque_payload_size;
    int sal_word_count;
    int sal_nonzero_byte_count;
    int sal_high_bit_byte_count;
    int sal_zero_run_count;
    int sal_max_zero_run;
    int sal_first_nonzero_offset;
    int sal_last_nonzero_offset;
    int sal_checksum16;
} Nexus_SoundEngine;

typedef enum {
    NEXUS_SFX_RUNTIME_MISSING = 0,
    NEXUS_SFX_RUNTIME_BLOCKED_MISSING_ASSET = 1,
    NEXUS_SFX_RUNTIME_BLOCKED_ASSET_MISMATCH = 2,
    NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE = 3,
    NEXUS_SFX_RUNTIME_READY_DECODED = 4
} Nexus_SfxRuntimeStatus;

typedef struct {
    Nexus_SfxRuntimeStatus status;
    int level_index;
    int cd_track;
    int sal_loaded;
    int map_loaded;
    int sal_canonical_source_verified;
    int map_canonical_source_verified;
    int sound_driver_canonical_source_verified;
    int sal_decode_supported;
    int map_decode_supported;
    int map_event_count;
    int map_mapped_event_count;
    int map_first_sample_index;
    int map_last_sample_index;
    int map_header_checksum16;
    int map_header_nonzero_byte_count;
    int map_header_distinct_byte_count;
    int map_header_transition_count;
    int map_record_table_supported;
    int map_record_count;
    int map_record_terminator_offset;
    int map_first_record_event;
    int map_min_record_event;
    int map_max_record_event;
    int map_record_event_span;
    int map_unique_record_event_count;
    int map_duplicate_record_event_count;
    int map_has_duplicate_record_events;
    int map_first_record_sal_offset;
    int map_first_record_size;
    int map_last_record_sal_offset;
    int map_max_record_end;
    int map_total_record_bytes;
    int map_out_of_bounds_record_count;
    int map_first_window_checksum16;
    int map_first_window_nonzero_byte_count;
    int map_first_window_high_bit_byte_count;
    int map_first_window_first_nonzero_relative_offset;
    int map_first_window_last_nonzero_relative_offset;
    int map_first_window_distinct_byte_count;
    int map_first_window_transition_count;
    int map_last_window_checksum16;
    int map_last_window_nonzero_byte_count;
    int map_last_window_high_bit_byte_count;
    int map_last_window_first_nonzero_relative_offset;
    int map_last_window_last_nonzero_relative_offset;
    int map_last_window_distinct_byte_count;
    int map_last_window_transition_count;
    int last_event;
    int last_sample_index;
    int last_event_record_found;
    int last_event_sal_offset;
    int last_event_sal_size;
    int last_event_window_checksum16;
    int last_event_window_nonzero_byte_count;
    int last_event_window_high_bit_byte_count;
    int last_event_window_first_nonzero_relative_offset;
    int last_event_window_last_nonzero_relative_offset;
    int last_event_window_distinct_byte_count;
    int last_event_window_transition_count;
    int sal_package_profile_supported;
    int sal_container_preamble_supported;
    int sal_payload_offset;
    int sal_opaque_payload_size;
    int sal_word_count;
    int sal_nonzero_byte_count;
    int sal_high_bit_byte_count;
    int sal_zero_run_count;
    int sal_max_zero_run;
    int sal_first_nonzero_offset;
    int sal_last_nonzero_offset;
    int sal_checksum16;
    int playback_enabled;
    int blocks_real_sfx_playback;
    int fallback_visuals_permitted;
    Nexus_V1_AudioReceipt sal_receipt;
    Nexus_V1_AudioReceipt map_receipt;
    /* Expected identity for the globally hash-bound SDDRVS.TSK image.
     * `sound_driver_canonical_source_verified` remains the actual engine
     * ownership gate because the sound module never reads arbitrary files. */
    Nexus_V1_AudioReceipt sound_driver_receipt;
} Nexus_SfxRuntimeReceipt;

/* Init sound system */
int nexus_sound_init(Nexus_SoundEngine *eng);

/* Shutdown */
void nexus_sound_shutdown(Nexus_SoundEngine *eng);

/* Load SFX bank for level (SNDLEV##.SAL + SNDLEV##.MAP).
 * Call on level load. Pass NULL to use default. */
int nexus_sound_load_level(Nexus_SoundEngine *eng, int level_index,
                            const uint8_t *sal_data, int sal_size,
                            const uint8_t *map_data, int map_size);
int nexus_sound_load_canonical_level(Nexus_SoundEngine *eng, int level_index,
                                      const uint8_t *sal_data, int sal_size,
                                      const uint8_t *map_data, int map_size,
                                      int sal_canonical_source_verified,
                                      int map_canonical_source_verified);
void nexus_sound_set_driver_canonical_source_verified(
    Nexus_SoundEngine *eng, int verified);
int nexus_sound_level_runtime_receipt(const Nexus_SoundEngine *eng,
                                      Nexus_SfxRuntimeReceipt *out_receipt);
const char *nexus_sound_sfx_runtime_status_name(
    Nexus_SfxRuntimeStatus status);

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
