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
#define NEXUS_SFX_MAP_HEADER_BYTES 24
#define NEXUS_SFX_MAP_RECORD_BYTES 8

static int read_u16_be(const uint8_t *p) {
    return p ? (int)(((uint16_t)p[0] << 8) | (uint16_t)p[1]) : 0;
}

static int read_u32_be(const uint8_t *p) {
    return p ? (int)(((uint32_t)p[0] << 24) |
                     ((uint32_t)p[1] << 16) |
                     ((uint32_t)p[2] << 8) |
                     (uint32_t)p[3]) : 0;
}

static int optional_u16_to_int(uint16_t value) {
    return value == 0xffffU ? -1 : (int)value;
}

static void sal_window_profile(const uint8_t *data,
                               int data_size,
                               int offset,
                               int size,
                               int *checksum16,
                               int *nonzero_count,
                               int *high_bit_count,
                               int *first_nonzero_relative_offset,
                               int *last_nonzero_relative_offset,
                               int *distinct_byte_count,
                               int *transition_count) {
    int i;
    int checksum = 0;
    int nonzero = 0;
    int high = 0;
    int first_nonzero = -1;
    int last_nonzero = -1;
    int distinct = 0;
    int transitions = 0;
    unsigned char seen[256];

    if (checksum16) *checksum16 = 0;
    if (nonzero_count) *nonzero_count = 0;
    if (high_bit_count) *high_bit_count = 0;
    if (first_nonzero_relative_offset) *first_nonzero_relative_offset = -1;
    if (last_nonzero_relative_offset) *last_nonzero_relative_offset = -1;
    if (distinct_byte_count) *distinct_byte_count = 0;
    if (transition_count) *transition_count = 0;
    if (!data || offset < 0 || size <= 0 ||
        offset > data_size || size > data_size - offset) {
        return;
    }
    memset(seen, 0, sizeof(seen));
    for (i = 0; i < size; ++i) {
        int b = data[offset + i];
        checksum = (checksum + b) & 0xffff;
        if (!seen[b]) {
            seen[b] = 1;
            distinct++;
        }
        if (i > 0 && b != data[offset + i - 1]) {
            transitions++;
        }
        if (b != 0) {
            if (first_nonzero < 0) first_nonzero = i;
            last_nonzero = i;
            nonzero++;
            if ((b & 0x80) != 0) high++;
        }
    }
    if (checksum16) *checksum16 = checksum;
    if (nonzero_count) *nonzero_count = nonzero;
    if (high_bit_count) *high_bit_count = high;
    if (first_nonzero_relative_offset) {
        *first_nonzero_relative_offset = first_nonzero;
    }
    if (last_nonzero_relative_offset) {
        *last_nonzero_relative_offset = last_nonzero;
    }
    if (distinct_byte_count) *distinct_byte_count = distinct;
    if (transition_count) *transition_count = transitions;
}

static void clear_map_route(Nexus_SoundEngine *eng) {
    if (!eng) return;
    eng->map_event_count = 0;
    eng->map_mapped_event_count = 0;
    eng->map_first_sample_index = -1;
    eng->map_last_sample_index = -1;
    eng->map_header_checksum16 = 0;
    eng->map_header_nonzero_byte_count = 0;
    eng->map_header_distinct_byte_count = 0;
    eng->map_header_transition_count = 0;
    eng->map_record_table_supported = 0;
    eng->map_record_count = 0;
    eng->map_record_terminator_offset = -1;
    eng->map_first_record_event = -1;
    eng->map_min_record_event = -1;
    eng->map_max_record_event = -1;
    eng->map_record_event_span = 0;
    eng->map_unique_record_event_count = 0;
    eng->map_duplicate_record_event_count = 0;
    eng->map_has_duplicate_record_events = 0;
    eng->map_first_record_sal_offset = -1;
    eng->map_first_record_size = 0;
    eng->map_last_record_sal_offset = -1;
    eng->map_max_record_end = 0;
    eng->map_total_record_bytes = 0;
    eng->map_out_of_bounds_record_count = 0;
    eng->map_first_window_checksum16 = 0;
    eng->map_first_window_nonzero_byte_count = 0;
    eng->map_first_window_high_bit_byte_count = 0;
    eng->map_first_window_first_nonzero_relative_offset = -1;
    eng->map_first_window_last_nonzero_relative_offset = -1;
    eng->map_first_window_distinct_byte_count = 0;
    eng->map_first_window_transition_count = 0;
    eng->map_last_window_checksum16 = 0;
    eng->map_last_window_nonzero_byte_count = 0;
    eng->map_last_window_high_bit_byte_count = 0;
    eng->map_last_window_first_nonzero_relative_offset = -1;
    eng->map_last_window_last_nonzero_relative_offset = -1;
    eng->map_last_window_distinct_byte_count = 0;
    eng->map_last_window_transition_count = 0;
    eng->last_event = 0;
    eng->last_sample_index = -1;
    eng->last_event_record_found = 0;
    eng->last_event_sal_offset = -1;
    eng->last_event_sal_size = 0;
    eng->last_event_window_checksum16 = 0;
    eng->last_event_window_nonzero_byte_count = 0;
    eng->last_event_window_high_bit_byte_count = 0;
    eng->last_event_window_first_nonzero_relative_offset = -1;
    eng->last_event_window_last_nonzero_relative_offset = -1;
    eng->last_event_window_distinct_byte_count = 0;
    eng->last_event_window_transition_count = 0;
    memset(eng->event_sample_index, 0, sizeof(eng->event_sample_index));
    memset(eng->event_sal_offset, 0, sizeof(eng->event_sal_offset));
    memset(eng->event_sal_size, 0, sizeof(eng->event_sal_size));
    memset(eng->event_sal_checksum16, 0, sizeof(eng->event_sal_checksum16));
    memset(eng->event_sal_nonzero_byte_count, 0,
           sizeof(eng->event_sal_nonzero_byte_count));
    memset(eng->event_sal_high_bit_byte_count, 0,
           sizeof(eng->event_sal_high_bit_byte_count));
    memset(eng->event_sal_first_nonzero_relative_offset, 0,
           sizeof(eng->event_sal_first_nonzero_relative_offset));
    memset(eng->event_sal_last_nonzero_relative_offset, 0,
           sizeof(eng->event_sal_last_nonzero_relative_offset));
    memset(eng->event_sal_distinct_byte_count, 0,
           sizeof(eng->event_sal_distinct_byte_count));
    memset(eng->event_sal_transition_count, 0,
           sizeof(eng->event_sal_transition_count));
}

static void parse_map_record_table(Nexus_SoundEngine *eng) {
    int off;
    int header_high = 0;
    int header_first = -1;
    int header_last = -1;
    unsigned char record_event_seen[256];

    if (!eng || !eng->map_data || eng->map_size < NEXUS_SFX_MAP_HEADER_BYTES + 2)
        return;

    sal_window_profile(eng->map_data,
                       eng->map_size,
                       0,
                       NEXUS_SFX_MAP_HEADER_BYTES,
                       &eng->map_header_checksum16,
                       &eng->map_header_nonzero_byte_count,
                       &header_high,
                       &header_first,
                       &header_last,
                       &eng->map_header_distinct_byte_count,
                       &eng->map_header_transition_count);
    (void)header_high;
    (void)header_first;
    (void)header_last;
    memset(record_event_seen, 0, sizeof(record_event_seen));

    off = NEXUS_SFX_MAP_HEADER_BYTES;
    while (off + 2 <= eng->map_size) {
        const uint8_t *r = eng->map_data + off;
        int event_id;
        int size;
        int sal_offset;
        int end;
        int checksum16 = 0;
        int nonzero = 0;
        int high = 0;
        int first_nonzero = -1;
        int last_nonzero = -1;
        int distinct = 0;
        int transitions = 0;

        if (r[0] == 0xffU && r[1] == 0xffU) {
            eng->map_record_terminator_offset = off;
            eng->map_record_table_supported = eng->map_record_count > 0 ? 1 : 0;
            return;
        }
        if (off + NEXUS_SFX_MAP_RECORD_BYTES > eng->map_size) {
            return;
        }

        event_id = r[0];
        size = read_u16_be(r + 2);
        sal_offset = read_u32_be(r + 4);
        end = sal_offset + size;
        sal_window_profile(eng->sal_data,
                           eng->sal_size,
                           sal_offset,
                           size,
                           &checksum16,
                           &nonzero,
                           &high,
                           &first_nonzero,
                           &last_nonzero,
                           &distinct,
                           &transitions);
        if (eng->map_record_count == 0) {
            eng->map_first_record_event = event_id;
            eng->map_first_record_sal_offset = sal_offset;
            eng->map_first_record_size = size;
            eng->map_first_window_checksum16 = checksum16;
            eng->map_first_window_nonzero_byte_count = nonzero;
            eng->map_first_window_high_bit_byte_count = high;
            eng->map_first_window_first_nonzero_relative_offset =
                first_nonzero;
            eng->map_first_window_last_nonzero_relative_offset =
                last_nonzero;
            eng->map_first_window_distinct_byte_count = distinct;
            eng->map_first_window_transition_count = transitions;
        }
        if (eng->map_min_record_event < 0 ||
            event_id < eng->map_min_record_event) {
            eng->map_min_record_event = event_id;
        }
        if (event_id > eng->map_max_record_event) {
            eng->map_max_record_event = event_id;
        }
        if (record_event_seen[(unsigned char)event_id]) {
            eng->map_duplicate_record_event_count++;
            eng->map_has_duplicate_record_events = 1;
        } else {
            record_event_seen[(unsigned char)event_id] = 1;
            eng->map_unique_record_event_count++;
        }
        if (eng->map_min_record_event >= 0 &&
            eng->map_max_record_event >= eng->map_min_record_event) {
            eng->map_record_event_span =
                eng->map_max_record_event - eng->map_min_record_event + 1;
        }
        eng->map_last_record_sal_offset = sal_offset;
        eng->map_last_window_checksum16 = checksum16;
        eng->map_last_window_nonzero_byte_count = nonzero;
        eng->map_last_window_high_bit_byte_count = high;
        eng->map_last_window_first_nonzero_relative_offset = first_nonzero;
        eng->map_last_window_last_nonzero_relative_offset = last_nonzero;
        eng->map_last_window_distinct_byte_count = distinct;
        eng->map_last_window_transition_count = transitions;
        if (event_id >= 0 &&
            event_id < (int)(sizeof(eng->event_sal_size) /
                             sizeof(eng->event_sal_size[0])) &&
            size > 0 &&
            sal_offset >= 0 &&
            end >= sal_offset &&
            eng->sal_data &&
            end <= eng->sal_size) {
            eng->event_sal_offset[event_id] = (uint32_t)sal_offset;
            eng->event_sal_size[event_id] = (uint16_t)size;
            eng->event_sal_checksum16[event_id] = (uint16_t)checksum16;
            eng->event_sal_nonzero_byte_count[event_id] = (uint16_t)nonzero;
            eng->event_sal_high_bit_byte_count[event_id] = (uint16_t)high;
            eng->event_sal_first_nonzero_relative_offset[event_id] =
                (uint16_t)first_nonzero;
            eng->event_sal_last_nonzero_relative_offset[event_id] =
                (uint16_t)last_nonzero;
            eng->event_sal_distinct_byte_count[event_id] =
                (uint16_t)distinct;
            eng->event_sal_transition_count[event_id] =
                (uint16_t)transitions;
        }
        if (end > eng->map_max_record_end) {
            eng->map_max_record_end = end;
        }
        eng->map_total_record_bytes += size;
        if (size <= 0 || sal_offset < 0 || end < sal_offset ||
            !eng->sal_data || end > eng->sal_size) {
            eng->map_out_of_bounds_record_count++;
        }
        eng->map_record_count++;
        off += NEXUS_SFX_MAP_RECORD_BYTES;
    }
}

static void clear_sal_profile(Nexus_SoundEngine *eng) {
    if (!eng) return;
    eng->sal_package_profile_supported = 0;
    eng->sal_word_count = 0;
    eng->sal_nonzero_byte_count = 0;
    eng->sal_high_bit_byte_count = 0;
    eng->sal_zero_run_count = 0;
    eng->sal_max_zero_run = 0;
    eng->sal_first_nonzero_offset = -1;
    eng->sal_last_nonzero_offset = -1;
    eng->sal_checksum16 = 0;
}

static void parse_sal_profile(Nexus_SoundEngine *eng) {
    int i;
    int zero_run = 0;
    int checksum = 0;

    if (!eng) return;
    clear_sal_profile(eng);
    if (!eng->sal_data || eng->sal_size <= 0 || (eng->sal_size & 1) != 0) {
        return;
    }

    for (i = 0; i < eng->sal_size; ++i) {
        int b = eng->sal_data[i];
        if (b != 0) {
            if (eng->sal_first_nonzero_offset < 0) {
                eng->sal_first_nonzero_offset = i;
            }
            eng->sal_last_nonzero_offset = i;
            eng->sal_nonzero_byte_count++;
            if ((b & 0x80) != 0) {
                eng->sal_high_bit_byte_count++;
            }
            if (zero_run > 0) {
                eng->sal_zero_run_count++;
                if (zero_run > eng->sal_max_zero_run) {
                    eng->sal_max_zero_run = zero_run;
                }
                zero_run = 0;
            }
        } else {
            zero_run++;
        }
    }
    if (zero_run > 0) {
        eng->sal_zero_run_count++;
        if (zero_run > eng->sal_max_zero_run) {
            eng->sal_max_zero_run = zero_run;
        }
    }
    for (i = 0; i + 1 < eng->sal_size; i += 2) {
        int word = ((int)eng->sal_data[i] << 8) | (int)eng->sal_data[i + 1];
        checksum = (checksum + word) & 0xffff;
    }

    if (eng->sal_nonzero_byte_count > 0) {
        eng->sal_package_profile_supported = 1;
        eng->sal_word_count = eng->sal_size / 2;
        eng->sal_checksum16 = checksum;
    }
}

static void parse_map_route(Nexus_SoundEngine *eng) {
    int i;
    int limit;

    if (!eng) return;
    clear_map_route(eng);
    if (!eng->map_data || eng->map_size <= 0) return;

    limit = eng->map_size;
    if (limit > (int)EVENT_COUNT) limit = (int)EVENT_COUNT;
    if (limit > (int)(sizeof(eng->event_sample_index) /
                      sizeof(eng->event_sample_index[0]))) {
        limit = (int)(sizeof(eng->event_sample_index) /
                      sizeof(eng->event_sample_index[0]));
    }
    eng->map_event_count = limit;

    /* The verified MAP assets are compact per-level event tables. Until the
     * SAL sample payload is decoded, Firestaff consumes the bounded event
     * index route only: byte N maps event N to a sample index, zero means no
     * level-local sample route. */
    for (i = 1; i < limit; ++i) {
        int sample = eng->map_data[i];
        eng->event_sample_index[i] = (uint16_t)sample;
        if (sample > 0) {
            if (eng->map_mapped_event_count == 0 ||
                sample < eng->map_first_sample_index) {
                eng->map_first_sample_index = sample;
            }
            if (sample > eng->map_last_sample_index) {
                eng->map_last_sample_index = sample;
            }
            ++eng->map_mapped_event_count;
        }
    }
}

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
    clear_map_route(eng);
    clear_sal_profile(eng);
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
    return nexus_sound_load_canonical_level(eng, level_index,
                                            sal_data, sal_size,
                                            map_data, map_size, 0, 0);
}

int nexus_sound_load_canonical_level(Nexus_SoundEngine *eng, int level_index,
                                      const uint8_t *sal_data, int sal_size,
                                      const uint8_t *map_data, int map_size,
                                      int sal_canonical_source_verified,
                                      int map_canonical_source_verified) {
    if (!eng || !eng->initialized) return -1;
    if (level_index < 0 || level_index > 15) return -1;

    /* Free previous level data */
    if (eng->sal_data) { free(eng->sal_data); eng->sal_data = NULL; }
    if (eng->map_data) { free(eng->map_data); eng->map_data = NULL; }
    eng->sal_size = 0;
    eng->map_size = 0;
    clear_map_route(eng);
    clear_sal_profile(eng);

    eng->current_level = level_index;
    eng->sal_canonical_source_verified =
        sal_canonical_source_verified ? 1 : 0;
    eng->map_canonical_source_verified =
        map_canonical_source_verified ? 1 : 0;

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
    parse_map_route(eng);
    parse_map_record_table(eng);
    parse_sal_profile(eng);

    printf("Nexus sound: loaded level %d SFX (SAL=%d bytes, MAP=%d bytes)\n",
        level_index, sal_size, map_size);
    return 0;
}

static int receipt_class_is_asset_ready(Nexus_V1_AudioReceiptClass cls) {
    return cls == NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH ||
           cls == NEXUS_V1_AUDIO_RECEIPT_VERIFIED_HASH;
}

int nexus_sound_level_runtime_receipt(const Nexus_SoundEngine *eng,
                                      Nexus_SfxRuntimeReceipt *out_receipt) {
    char sal_name[16];
    char map_name[16];
    int sal_ready;
    int map_ready;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_SFX_RUNTIME_MISSING;
    out_receipt->level_index = -1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!eng || !eng->initialized) return 0;

    out_receipt->level_index = eng->current_level;
    out_receipt->cd_track = eng->current_level >= 0
        ? nexus_v1_audio_cd_track_for_level_receipt(eng->current_level)
        : -1;
    out_receipt->sal_loaded = eng->sal_data && eng->sal_size > 0;
    out_receipt->map_loaded = eng->map_data && eng->map_size > 0;
    out_receipt->sal_canonical_source_verified =
        eng->sal_canonical_source_verified;
    out_receipt->map_canonical_source_verified =
        eng->map_canonical_source_verified;
    out_receipt->sal_decode_supported =
        nexus_v1_audio_decode_supported(NEXUS_V1_AUDIO_KIND_SAL_BANK);
    out_receipt->map_decode_supported =
        nexus_v1_audio_decode_supported(NEXUS_V1_AUDIO_KIND_MAP_TABLE);
    out_receipt->map_event_count = eng->map_event_count;
    out_receipt->map_mapped_event_count = eng->map_mapped_event_count;
    out_receipt->map_first_sample_index = eng->map_first_sample_index;
    out_receipt->map_last_sample_index = eng->map_last_sample_index;
    out_receipt->map_header_checksum16 = eng->map_header_checksum16;
    out_receipt->map_header_nonzero_byte_count =
        eng->map_header_nonzero_byte_count;
    out_receipt->map_header_distinct_byte_count =
        eng->map_header_distinct_byte_count;
    out_receipt->map_header_transition_count =
        eng->map_header_transition_count;
    out_receipt->map_record_table_supported =
        eng->map_record_table_supported;
    out_receipt->map_record_count = eng->map_record_count;
    out_receipt->map_record_terminator_offset =
        eng->map_record_terminator_offset;
    out_receipt->map_first_record_event = eng->map_first_record_event;
    out_receipt->map_min_record_event = eng->map_min_record_event;
    out_receipt->map_max_record_event = eng->map_max_record_event;
    out_receipt->map_record_event_span = eng->map_record_event_span;
    out_receipt->map_unique_record_event_count =
        eng->map_unique_record_event_count;
    out_receipt->map_duplicate_record_event_count =
        eng->map_duplicate_record_event_count;
    out_receipt->map_has_duplicate_record_events =
        eng->map_has_duplicate_record_events;
    out_receipt->map_first_record_sal_offset =
        eng->map_first_record_sal_offset;
    out_receipt->map_first_record_size = eng->map_first_record_size;
    out_receipt->map_last_record_sal_offset =
        eng->map_last_record_sal_offset;
    out_receipt->map_max_record_end = eng->map_max_record_end;
    out_receipt->map_total_record_bytes = eng->map_total_record_bytes;
    out_receipt->map_out_of_bounds_record_count =
        eng->map_out_of_bounds_record_count;
    out_receipt->map_first_window_checksum16 =
        eng->map_first_window_checksum16;
    out_receipt->map_first_window_nonzero_byte_count =
        eng->map_first_window_nonzero_byte_count;
    out_receipt->map_first_window_high_bit_byte_count =
        eng->map_first_window_high_bit_byte_count;
    out_receipt->map_first_window_first_nonzero_relative_offset =
        eng->map_first_window_first_nonzero_relative_offset;
    out_receipt->map_first_window_last_nonzero_relative_offset =
        eng->map_first_window_last_nonzero_relative_offset;
    out_receipt->map_first_window_distinct_byte_count =
        eng->map_first_window_distinct_byte_count;
    out_receipt->map_first_window_transition_count =
        eng->map_first_window_transition_count;
    out_receipt->map_last_window_checksum16 =
        eng->map_last_window_checksum16;
    out_receipt->map_last_window_nonzero_byte_count =
        eng->map_last_window_nonzero_byte_count;
    out_receipt->map_last_window_high_bit_byte_count =
        eng->map_last_window_high_bit_byte_count;
    out_receipt->map_last_window_first_nonzero_relative_offset =
        eng->map_last_window_first_nonzero_relative_offset;
    out_receipt->map_last_window_last_nonzero_relative_offset =
        eng->map_last_window_last_nonzero_relative_offset;
    out_receipt->map_last_window_distinct_byte_count =
        eng->map_last_window_distinct_byte_count;
    out_receipt->map_last_window_transition_count =
        eng->map_last_window_transition_count;
    out_receipt->last_event = eng->last_event;
    out_receipt->last_sample_index = eng->last_sample_index;
    out_receipt->last_event_record_found = eng->last_event_record_found;
    out_receipt->last_event_sal_offset = eng->last_event_sal_offset;
    out_receipt->last_event_sal_size = eng->last_event_sal_size;
    out_receipt->last_event_window_checksum16 =
        eng->last_event_window_checksum16;
    out_receipt->last_event_window_nonzero_byte_count =
        eng->last_event_window_nonzero_byte_count;
    out_receipt->last_event_window_high_bit_byte_count =
        eng->last_event_window_high_bit_byte_count;
    out_receipt->last_event_window_first_nonzero_relative_offset =
        eng->last_event_window_first_nonzero_relative_offset;
    out_receipt->last_event_window_last_nonzero_relative_offset =
        eng->last_event_window_last_nonzero_relative_offset;
    out_receipt->last_event_window_distinct_byte_count =
        eng->last_event_window_distinct_byte_count;
    out_receipt->last_event_window_transition_count =
        eng->last_event_window_transition_count;
    out_receipt->sal_package_profile_supported =
        eng->sal_package_profile_supported;
    out_receipt->sal_word_count = eng->sal_word_count;
    out_receipt->sal_nonzero_byte_count = eng->sal_nonzero_byte_count;
    out_receipt->sal_high_bit_byte_count = eng->sal_high_bit_byte_count;
    out_receipt->sal_zero_run_count = eng->sal_zero_run_count;
    out_receipt->sal_max_zero_run = eng->sal_max_zero_run;
    out_receipt->sal_first_nonzero_offset = eng->sal_first_nonzero_offset;
    out_receipt->sal_last_nonzero_offset = eng->sal_last_nonzero_offset;
    out_receipt->sal_checksum16 = eng->sal_checksum16;

    if (eng->current_level < 0 ||
        eng->current_level >= NEXUS_V1_AUDIO_LEVEL_COUNT) {
        out_receipt->blocks_real_sfx_playback = 1;
        return 0;
    }

    snprintf(sal_name, sizeof(sal_name), "SNDLEV%02d.SAL",
             eng->current_level);
    snprintf(map_name, sizeof(map_name), "SNDLEV%02d.MAP",
             eng->current_level);
    (void)nexus_v1_audio_classify_file(
        sal_name,
        (uint32_t)(eng->sal_size > 0 ? eng->sal_size : 0),
        NULL,
        &out_receipt->sal_receipt);
    (void)nexus_v1_audio_classify_file(
        map_name,
        (uint32_t)(eng->map_size > 0 ? eng->map_size : 0),
        NULL,
        &out_receipt->map_receipt);

    if (!out_receipt->sal_loaded || !out_receipt->map_loaded) {
        out_receipt->status = NEXUS_SFX_RUNTIME_BLOCKED_MISSING_ASSET;
        out_receipt->blocks_real_sfx_playback = 1;
        return 0;
    }

    sal_ready = receipt_class_is_asset_ready(
        out_receipt->sal_receipt.receipt_class);
    map_ready = receipt_class_is_asset_ready(
        out_receipt->map_receipt.receipt_class);
    if (!sal_ready || !map_ready) {
        out_receipt->status = NEXUS_SFX_RUNTIME_BLOCKED_ASSET_MISMATCH;
        out_receipt->blocks_real_sfx_playback = 1;
        return 0;
    }

    if (!out_receipt->sal_canonical_source_verified ||
        !out_receipt->map_canonical_source_verified ||
        !out_receipt->sal_decode_supported ||
        !out_receipt->map_decode_supported) {
        out_receipt->status = NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE;
        out_receipt->blocks_real_sfx_playback = 1;
        return 0;
    }

    out_receipt->status = NEXUS_SFX_RUNTIME_READY_DECODED;
    out_receipt->playback_enabled = eng->sfx_enabled ? 1 : 0;
    return 0;
}

const char *nexus_sound_sfx_runtime_status_name(
    Nexus_SfxRuntimeStatus status) {
    switch (status) {
    case NEXUS_SFX_RUNTIME_MISSING: return "missing";
    case NEXUS_SFX_RUNTIME_BLOCKED_MISSING_ASSET:
        return "blocked-missing-asset";
    case NEXUS_SFX_RUNTIME_BLOCKED_ASSET_MISMATCH:
        return "blocked-asset-mismatch";
    case NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE:
        return "blocked-unsupported-decode";
    case NEXUS_SFX_RUNTIME_READY_DECODED: return "ready-decoded";
    default: return "unknown";
    }
}

/* Play sound event — STUB logs the request.
 * Real implementation: look up event_id in MAP, get sample offset/size
 * from SAL, decode (if needed), play via SDL_mixer or platform audio.
 * TODO: SDL_mixer integration, SAL decode (unknown format).
 * Source: docs/nexus_audio_format.md (SAL format unknown). */
void nexus_sound_play(Nexus_SoundEngine *eng, Nexus_SoundEvent event) {
    const char *name;
    Nexus_SfxRuntimeReceipt receipt;
    int sample_index = -1;

    if (!eng || !eng->initialized) return;
    if (!eng->sfx_enabled) return;
    if (event <= NEXUS_SFX_NONE || event >= EVENT_COUNT) return;
    if ((int)event < eng->map_event_count) {
        sample_index = eng->event_sample_index[event];
        if (sample_index == 0) sample_index = -1;
    }
    eng->last_event = event;
    eng->last_sample_index = sample_index;
    eng->last_event_record_found = 0;
    eng->last_event_sal_offset = -1;
    eng->last_event_sal_size = 0;
    eng->last_event_window_checksum16 = 0;
    eng->last_event_window_nonzero_byte_count = 0;
    eng->last_event_window_high_bit_byte_count = 0;
    eng->last_event_window_first_nonzero_relative_offset = -1;
    eng->last_event_window_last_nonzero_relative_offset = -1;
    eng->last_event_window_distinct_byte_count = 0;
    eng->last_event_window_transition_count = 0;
    if ((int)event >= 0 &&
        (int)event < (int)(sizeof(eng->event_sal_size) /
                           sizeof(eng->event_sal_size[0])) &&
        eng->event_sal_size[event] > 0) {
        eng->last_event_record_found = 1;
        eng->last_event_sal_offset = (int)eng->event_sal_offset[event];
        eng->last_event_sal_size = (int)eng->event_sal_size[event];
        eng->last_event_window_checksum16 =
            (int)eng->event_sal_checksum16[event];
        eng->last_event_window_nonzero_byte_count =
            (int)eng->event_sal_nonzero_byte_count[event];
        eng->last_event_window_high_bit_byte_count =
            (int)eng->event_sal_high_bit_byte_count[event];
        eng->last_event_window_first_nonzero_relative_offset =
            optional_u16_to_int(
                eng->event_sal_first_nonzero_relative_offset[event]);
        eng->last_event_window_last_nonzero_relative_offset =
            optional_u16_to_int(
                eng->event_sal_last_nonzero_relative_offset[event]);
        eng->last_event_window_distinct_byte_count =
            (int)eng->event_sal_distinct_byte_count[event];
        eng->last_event_window_transition_count =
            (int)eng->event_sal_transition_count[event];
    }
    if (nexus_sound_level_runtime_receipt(eng, &receipt) == 0 &&
        receipt.blocks_real_sfx_playback) {
        printf("Nexus SFX blocked: %s\n",
               nexus_sound_sfx_runtime_status_name(receipt.status));
        return;
    }

    name = g_event_names[event];
    if (!name) name = "UNKNOWN";

    /* STUB: log only */
    /* TODO: real playback — MAP lookup + SAL decode + SDL_mixer */
    printf("Nexus SFX: %s sample_idx=%d\n", name, sample_index);
    (void)eng;
}

void nexus_sound_play_idx(Nexus_SoundEngine *eng, int sample_index) {
    Nexus_SfxRuntimeReceipt receipt;
    if (!eng || !eng->initialized) return;
    if (!eng->sfx_enabled) return;
    eng->last_event = 0;
    eng->last_sample_index = sample_index;
    eng->last_event_record_found = 0;
    eng->last_event_sal_offset = -1;
    eng->last_event_sal_size = 0;
    eng->last_event_window_checksum16 = 0;
    eng->last_event_window_nonzero_byte_count = 0;
    eng->last_event_window_high_bit_byte_count = 0;
    eng->last_event_window_first_nonzero_relative_offset = -1;
    eng->last_event_window_last_nonzero_relative_offset = -1;
    eng->last_event_window_distinct_byte_count = 0;
    eng->last_event_window_transition_count = 0;
    if (nexus_sound_level_runtime_receipt(eng, &receipt) == 0 &&
        receipt.blocks_real_sfx_playback) {
        printf("Nexus SFX blocked: %s\n",
               nexus_sound_sfx_runtime_status_name(receipt.status));
        return;
    }
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
