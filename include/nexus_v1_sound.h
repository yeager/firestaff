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
 * Saturn hardware (pass 216 SH-2 disassembly):
 * - SDDRVS.TSK loaded to MC68EC000 sound RAM
 * - SH-2 communicates via SCSP registers at 0x25B00400
 * - sndlib2.c submitPCMP function submits PCM playback commands
 * - SAL banks loaded by filename via CD sector read
 *
 * Status: SDDRVS ABI and SCSP communication proven from SH-2 code.
 * Event→MAP selector mapping remains unverified; actual SFX playback
 * stays blocked until a verified selector table is established. */

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

/* One raw SNDLEV##.MAP record. `selector` and `attribute` are on-disk bytes;
 * no Saturn event-dispatch or audio-codec meaning is claimed for either. */
typedef struct {
    int data_id;
    int id_number;
    int selector;
    int attribute;
    int sal_offset;
    int sal_size;
} Nexus_SoundMapWindow;

#define NEXUS_SFX_MAP_MAX_RECORDS 16
/* DMWeb retail MAP records start at byte zero.  Keep the 24-byte value
 * exclusively for the historical fixture grammar below. */
#define NEXUS_SFX_MAP_HEADER_BYTES 24
#define NEXUS_SFX_MAP_RETAIL_RECORD_BYTES 8
#define NEXUS_SFX_MAP_RECORD_BYTES 8
#define NEXUS_SAL_CONTAINER_PREAMBLE_BYTES 8
#define NEXUS_SFX_EVENT_SELECTOR_COUNT 32

/* ═══════════════════════════════════════════════════════════════════
 * Sound engine
 * ═══════════════════════════════════════════════════════════════════ */

typedef struct {
    int initialized;
    int sfx_enabled;
    int music_enabled;
    int current_cd_track;
    int current_level;
    /* Configured Nexus data root. CD-DA materialization, when admitted by a
     * host decoder, must resolve relative to the active source root rather
     * than a HOME-relative placeholder. */
    char data_root[512];
    /* SAL/MAP data for current level */
    uint8_t *sal_data;
    int sal_size;
    uint64_t sal_source_fnv1a64;
    uint8_t *map_data;
    int map_size;
    uint64_t map_source_fnv1a64;
    int sal_canonical_source_verified;
    int map_canonical_source_verified;
    /* Global SDDRVS.TSK source identity. It does not imply an understood
     * driver ABI, decoder, or playback route. */
    int sound_driver_canonical_source_verified;
    /* Host diagnostic bindings are owned by this engine instance.  They
     * must not leak across shutdown/init or between simultaneous instances. */
    int event_selector[NEXUS_SFX_EVENT_SELECTOR_COUNT];
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
    Nexus_SoundMapWindow map_records[NEXUS_SFX_MAP_MAX_RECORDS];
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
    /* DMWeb DMNDataFileDecoder.vbs: authenticated DataID 0 tone-bank
     * directory and sample-entry metadata.  These are source facts only;
     * event dispatch and Saturn driver submission remain separate gates. */
    int sal_tone_bank_directory_supported;
    int sal_tone_bank_offset;
    int sal_tone_bank_bytes;
    int sal_tone_entry_count;
    int sal_tone_entry_count_decoded;
    int sal_tone_pcm8_count;
    int sal_tone_pcm16_count;
    int sal_tone_memory_source_count;
    int sal_tone_noise_source_count;
    int sal_tone_sample_payload_offset;
    int sal_tone_sample_payload_bytes;
    uint16_t event_sample_index[32];
    uint32_t event_sal_offset[32];
    uint16_t event_sal_size[32];
    uint16_t event_sal_checksum16[32];
    uint16_t event_sal_nonzero_byte_count[32];
    uint16_t event_sal_high_bit_byte_count[32];
    uint16_t event_sal_first_nonzero_relative_offset[32];
    uint16_t event_sal_last_nonzero_relative_offset[32];
    uint16_t event_sal_distinct_byte_count[32];
    uint16_t event_sal_transition_count[32];
    /* Decoded PCM sample cache (extracted from tone bank) */
    int sal_decode_ready;
    int sal_decoded_tone_count;
    int16_t *sal_decoded_samples[64];
    int sal_decoded_sample_count[64];
    int sal_decoded_sample_rate;
    /* Active playback voices */
#define NEXUS_SOUND_MAX_VOICES 8
    struct {
        int active;
        int tone_index;
        int position;
    } voices[NEXUS_SOUND_MAX_VOICES];
    int voice_count;
    /* SDDRVS identity does not prove the Saturn event dispatcher. This
     * remains zero until an original event-to-MAP trace is admitted. */
    int event_dispatch_source_verified;
    /* CD audio state */
    int cd_playing;
    int cd_paused;
    char cd_track_path[512];
    /* CD audio callbacks (set by host layer, e.g. M11) */
    int (*cd_play_callback)(const char *path, void *userdata);
    void (*cd_stop_callback)(void *userdata);
    void *cd_callback_userdata;
} Nexus_SoundEngine;

typedef enum {
    NEXUS_SFX_RUNTIME_MISSING = 0,
    NEXUS_SFX_RUNTIME_BLOCKED_MISSING_ASSET = 1,
    NEXUS_SFX_RUNTIME_BLOCKED_ASSET_MISMATCH = 2,
    NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE = 3,
    NEXUS_SFX_RUNTIME_BLOCKED_EVENT_DISPATCH = 4,
    NEXUS_SFX_RUNTIME_READY_DECODED = 5
} Nexus_SfxRuntimeStatus;

typedef struct {
    Nexus_SfxRuntimeStatus status;
    int level_index;
    int cd_track;
    int sal_loaded;
    int map_loaded;
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
    int sal_tone_bank_directory_supported;
    int sal_tone_bank_offset;
    int sal_tone_bank_bytes;
    int sal_tone_entry_count;
    int sal_tone_entry_count_decoded;
    int sal_tone_pcm8_count;
    int sal_tone_pcm16_count;
    int sal_tone_memory_source_count;
    int sal_tone_noise_source_count;
    int sal_tone_sample_payload_offset;
    int sal_tone_sample_payload_bytes;
    int sal_canonical_source_verified;
    int map_canonical_source_verified;
    int sound_driver_canonical_source_verified;
    int event_dispatch_source_verified;
    int playback_enabled;
    int blocks_real_sfx_playback;
    int fallback_visuals_permitted;
    Nexus_V1_AudioReceipt sal_receipt;
    Nexus_V1_AudioReceipt map_receipt;
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
int nexus_sound_load_canonical_level(Nexus_SoundEngine *eng,
                                      int level_index,
                                      const uint8_t *sal_data,
                                      int sal_size,
                                      const uint8_t *map_data,
                                      int map_size,
                                      int sal_canonical_source_verified,
                                      int map_canonical_source_verified);
void nexus_sound_set_driver_canonical_source_verified(
    Nexus_SoundEngine *eng, int verified);
int nexus_sound_level_runtime_receipt(const Nexus_SoundEngine *eng,
                                      Nexus_SfxRuntimeReceipt *out_receipt);
const char *nexus_sound_sfx_runtime_status_name(
    Nexus_SfxRuntimeStatus status);

/* Resolve one unique raw MAP selector to its bounded SAL byte window.
 * This is a container route only: callers must not interpret or play the
 * returned bytes without separate Saturn driver and SAL codec evidence. */
int nexus_sound_map_lookup_raw_selector(const Nexus_SoundEngine *eng,
                                        int selector,
                                        Nexus_SoundMapWindow *out_window);

/* Bind a host sound event to a raw MAP selector.  Default is unmapped (-1).
 * Only explicit source-bound selectors enable dispatch; this keeps the path
 * fail-closed until original Saturn event→selector evidence is available. */
void nexus_sound_set_event_selector(Nexus_SoundEngine *eng,
                                    Nexus_SoundEvent event, int selector);

/* Play a sound event by ID (from SNDLEV*.MAP mapping) */
void nexus_sound_play(Nexus_SoundEngine *eng, Nexus_SoundEvent event);

/* Play a sound by raw sample index (for direct SAL access) */
void nexus_sound_play_idx(Nexus_SoundEngine *eng, int sample_index);

/* CD audio track management */
int nexus_sound_cd_track(Nexus_SoundEngine *eng, int track_number);
void nexus_sound_set_data_root(Nexus_SoundEngine *eng, const char *data_root);
int nexus_sound_cd_stop(Nexus_SoundEngine *eng);
int nexus_sound_cd_pause(Nexus_SoundEngine *eng);
int nexus_sound_cd_resume(Nexus_SoundEngine *eng);

/* Music fade (level transition) */
void nexus_sound_music_fade(Nexus_SoundEngine *eng, int fade_out_ms);

/* CD audio host callbacks */
void nexus_sound_set_cd_callbacks(Nexus_SoundEngine *eng,
    int (*play)(const char *path, void *userdata),
    void (*stop)(void *userdata),
    void *userdata);

/* Mute/unmute */
void nexus_sound_set_sfx(Nexus_SoundEngine *eng, int enabled);
void nexus_sound_set_music(Nexus_SoundEngine *eng, int enabled);

/* Decode SAL tone bank into PCM sample cache.
 * Called after loading a level's SAL data. Returns number of tones decoded. */
int nexus_sound_decode_sal(Nexus_SoundEngine *eng);

/* Mix active voices into output buffer (signed 16-bit mono, host byte order).
 * Returns number of samples written. Call from audio tick at 22050 Hz. */
int nexus_sound_mix(Nexus_SoundEngine *eng, int16_t *out, int max_samples);

/* Event name for debug */
const char *nexus_sound_event_name(Nexus_SoundEvent event);

#endif /* NEXUS_V1_SOUND_H */
