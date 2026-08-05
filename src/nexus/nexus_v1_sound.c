#include "nexus_v1_sound.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

static void nexus_sound_free_decoded(Nexus_SoundEngine *eng);
static void nexus_sound_trigger_tone(Nexus_SoundEngine *eng, int tone_index);
static int nexus_file_exists(const char *path);
static void nexus_cd_ensure_wav_tracks(void);

/* Nexus V1 sound system — source-bound SAL directory implementation.
 * Source: docs/nexus_audio_format.md, docs/nexus_sfx.md,
 * docs/nexus_music.md, nexus_v1_engine.c CD track switching.
 *
 * Per-level SFX: SNDLEV00-15.SAL (sound banks, 290-460 KB each)
 *                SNDLEV00-15.MAP (event index, 66-90 bytes each)
 * CD audio: 8 tracks (2-9) mapped to level pairs.
 * Sound driver: SDDRVS.TSK (26 KB Saturn sound driver task).
 *
 * DMWeb's DataID 0 tone-bank directory and PCM entry metadata are decoded;
 * Saturn event dispatch, SDDRVS ABI submission and host playback remain
 * unproven, so no actual SFX playback is admitted.
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

/* Source-locked event→MAP-selector dispatch table.
 * No Saturn source has yet bound the host NEXUS_SFX_* enum values to MAP
 * record selectors, so every entry defaults to -1 (unmapped / fail-closed).
 * When a real verified corpus supplies a proven mapping, callers can bind
 * selectors with nexus_sound_set_event_selector(); until then, the dispatch
 * path records the lookup attempt and stays blocked. */
static int g_event_selector[EVENT_COUNT];
static int g_event_selector_initialized = 0;

static void ensure_event_selector_init(void) {
    size_t i;
    if (g_event_selector_initialized) return;
    for (i = 0; i < EVENT_COUNT; i++) g_event_selector[i] = -1;
    /* No Saturn event-to-MAP selector binding is authenticated yet. Keep the
     * table fail-closed; only an explicit source/capture-backed setter may
     * establish a runtime association. */
    g_event_selector_initialized = 1;
}

void nexus_sound_set_event_selector(Nexus_SoundEngine *eng,
                                    Nexus_SoundEvent event, int selector) {
    ensure_event_selector_init();
    if (!eng || !eng->initialized) return;
    if (event <= NEXUS_SFX_NONE || event >= EVENT_COUNT) return;
    g_event_selector[event] = selector;
}

/* The 16 hash-verified Japanese Track 1 SAL banks begin with these literal
 * bytes. DM.BIN+0x38ea0..0x39100 contains the adjacent .MAP, .SAL,
 * SNDLEV01..15, and SDDRVS.TSK loader strings. This establishes only an
 * opaque container precondition, not codec or frame semantics. */
static const uint8_t g_sal_container_preamble[NEXUS_SAL_CONTAINER_PREAMBLE_BYTES] = {
    'd', 's', 'p', '0', '1', '.', 'E', 'X'
};

static int read_u16_be(const uint8_t *p) {
    return p ? (int)(((uint16_t)p[0] << 8) | (uint16_t)p[1]) : 0;
}

static int read_u32_be(const uint8_t *p) {
    return p ? (int)(((uint32_t)p[0] << 24) |
                     ((uint32_t)p[1] << 16) |
                     ((uint32_t)p[2] << 8) |
                     (uint32_t)p[3]) : 0;
}

static int read_u24_be(const uint8_t *p) {
    return p ? ((int)p[0] << 16) | ((int)p[1] << 8) | (int)p[2] : 0;
}

static uint64_t nexus_sound_fnv1a64(const uint8_t *data, int size) {
    uint64_t hash = UINT64_C(1469598103934665603);
    int i;

    if (!data || size <= 0) return 0U;
    for (i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
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
    memset(eng->map_records, 0, sizeof(eng->map_records));
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

    if (eng->map_data[0] != 0x20U) goto legacy_map_parser;
    /* DMWeb DMNDataFileDecoder.vbs: MAP is a sequence of eight-byte
     * DataID/ID/start/L/area records from byte zero. Retail maps begin with
     * 20h and terminate with FFh. Keep the older 24-byte fixture grammar
     * isolated for historical unit fixtures that do not carry a retail
     * signature. */
    {
        const int retail_dmweb_map = eng->map_data[0] == 0x20U;
        off = retail_dmweb_map ? 0 : NEXUS_SFX_MAP_HEADER_BYTES;
        while (off < eng->map_size) {
            if (eng->map_data[off] == 0xffU) {
                eng->map_record_terminator_offset = off;
                eng->map_record_table_supported =
                    eng->map_record_count > 0 ? 1 : 0;
                return;
            }
            if (retail_dmweb_map) {
                if (off > eng->map_size - 8)
                    return;
            } else if (off > eng->map_size - 2) {
                return;
            }
            const uint8_t *r = eng->map_data + off;
            int retail_data_id = retail_dmweb_map ? ((r[0] >> 4) & 0x07) : 0;
            int retail_start = retail_dmweb_map ? read_u24_be(r + 1) : read_u32_be(r + 4);
            int retail_size = retail_dmweb_map ? read_u24_be(r + 5) : read_u16_be(r + 2);
            int event_id = r[0];
            int size = retail_size;
            int sal_offset = retail_start;
            int end = (sal_offset >= 0 && size > 0 &&
                       sal_offset <= INT_MAX - size) ?
                sal_offset + size : INT_MAX;
            int checksum16 = 0, nonzero = 0, high = 0, first_nonzero = -1;
            int last_nonzero = -1, distinct = 0, transitions = 0;
            if (eng->map_record_count >= NEXUS_SFX_MAP_MAX_RECORDS) return;
            eng->map_records[eng->map_record_count].selector = event_id;
            eng->map_records[eng->map_record_count].data_id = retail_data_id;
            eng->map_records[eng->map_record_count].id_number = r[0] & 0x0f;
            eng->map_records[eng->map_record_count].attribute = retail_dmweb_map ? (r[0] & 0x0f) : r[1];
            eng->map_records[eng->map_record_count].sal_offset = sal_offset;
            eng->map_records[eng->map_record_count].sal_size = size;
            sal_window_profile(eng->sal_data, eng->sal_size, sal_offset, size,
                               &checksum16, &nonzero, &high, &first_nonzero,
                               &last_nonzero, &distinct, &transitions);
            if (eng->map_record_count == 0) {
                eng->map_first_record_event = event_id;
                eng->map_first_record_sal_offset = sal_offset;
                eng->map_first_record_size = size;
                eng->map_first_window_checksum16 = checksum16;
                eng->map_first_window_nonzero_byte_count = nonzero;
                eng->map_first_window_high_bit_byte_count = high;
                eng->map_first_window_first_nonzero_relative_offset = first_nonzero;
                eng->map_first_window_last_nonzero_relative_offset = last_nonzero;
                eng->map_first_window_distinct_byte_count = distinct;
                eng->map_first_window_transition_count = transitions;
            }
            if (eng->map_min_record_event < 0 || event_id < eng->map_min_record_event) eng->map_min_record_event = event_id;
            if (event_id > eng->map_max_record_event) eng->map_max_record_event = event_id;
            if (record_event_seen[(unsigned char)event_id]) { eng->map_duplicate_record_event_count++; eng->map_has_duplicate_record_events = 1; }
            else { record_event_seen[(unsigned char)event_id] = 1; eng->map_unique_record_event_count++; }
            eng->map_record_event_span = eng->map_max_record_event - eng->map_min_record_event + 1;
            eng->map_last_record_sal_offset = sal_offset;
            eng->map_last_window_checksum16 = checksum16;
            eng->map_last_window_nonzero_byte_count = nonzero;
            eng->map_last_window_high_bit_byte_count = high;
            eng->map_last_window_first_nonzero_relative_offset = first_nonzero;
            eng->map_last_window_last_nonzero_relative_offset = last_nonzero;
            eng->map_last_window_distinct_byte_count = distinct;
            eng->map_last_window_transition_count = transitions;
            if (end > eng->map_max_record_end) eng->map_max_record_end = end;
            eng->map_total_record_bytes += size;
            /* DMWeb's area is a sound-driver memory area, not necessarily a
             * direct SAL-file interval. Do not misclassify it as an invalid
             * file window; DataID remains the required provenance boundary. */
            eng->map_record_count++;
            off += 8;
            continue;
        }
    }
    return;
legacy_map_parser:
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

        if (eng->map_record_count >= NEXUS_SFX_MAP_MAX_RECORDS) {
            return;
        }

        /* The first byte is retained as a raw selector. Its Saturn event
         * meaning is not established; the legacy receipt fields below only
         * describe the observed byte range. */
        event_id = r[0];
        size = read_u16_be(r + 2);
        sal_offset = read_u32_be(r + 4);
        end = (sal_offset >= 0 && size > 0 && sal_offset <= INT_MAX - size)
            ? sal_offset + size : INT_MAX;
        eng->map_records[eng->map_record_count].selector = event_id;
        eng->map_records[eng->map_record_count].attribute = r[1];
        eng->map_records[eng->map_record_count].sal_offset = sal_offset;
        eng->map_records[eng->map_record_count].sal_size = size;
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
    eng->sal_container_preamble_supported = 0;
    eng->sal_payload_offset = -1;
    eng->sal_opaque_payload_size = 0;
    eng->sal_word_count = 0;
    eng->sal_nonzero_byte_count = 0;
    eng->sal_high_bit_byte_count = 0;
    eng->sal_zero_run_count = 0;
    eng->sal_max_zero_run = 0;
    eng->sal_first_nonzero_offset = -1;
    eng->sal_last_nonzero_offset = -1;
    eng->sal_checksum16 = 0;
    eng->sal_tone_bank_directory_supported = 0;
    eng->sal_tone_bank_offset = -1;
    eng->sal_tone_bank_bytes = 0;
    eng->sal_tone_entry_count = 0;
    eng->sal_tone_entry_count_decoded = 0;
    eng->sal_tone_pcm8_count = 0;
    eng->sal_tone_pcm16_count = 0;
    eng->sal_tone_memory_source_count = 0;
    eng->sal_tone_noise_source_count = 0;
    eng->sal_tone_sample_payload_offset = -1;
    eng->sal_tone_sample_payload_bytes = 0;
}

/* DMWeb: DMNDataFileDecoder.vbs DecodeSNDLEVxxMAP, DataID 0.  The MAP
 * entries are consumed in order from the SAL file; after the first part the
 * decoder skips 2752 bytes.  The tone-bank directory then starts with a
 * BE16 offset table, followed by four variable entries and fixed 4+32*n
 * entries.  Keep this as metadata only: no host audio device or Saturn
 * SDDRVS call is implied by parsing it. */
static void parse_sal_tone_bank_directory(Nexus_SoundEngine *eng) {
    int cursor = 0;
    int i;
    int tone_offset = -1;
    int tone_bytes = 0;
    int first_part = 1;
    int directory_offset;
    int entry_count;
    int directory_bytes;
    int entry_table_end;
    int entry_offset;

    if (!eng || !eng->sal_data || !eng->map_data ||
        !eng->map_record_table_supported) return;
    for (i = 0; i < eng->map_record_count; ++i) {
        const Nexus_SoundMapWindow *r = &eng->map_records[i];
        if (r->sal_size <= 0 || cursor > eng->sal_size - r->sal_size)
            return;
        if (r->data_id == 0 && tone_offset < 0) {
            tone_offset = cursor;
            tone_bytes = r->sal_size;
        }
        cursor += r->sal_size;
        if (first_part) {
            cursor = (cursor <= INT_MAX - 2752) ? cursor + 2752 : INT_MAX;
            first_part = 0;
        }
    }
    if (tone_offset < 0 || tone_bytes < 2 ||
        tone_offset > eng->sal_size - tone_bytes) return;

    directory_offset = read_u16_be(eng->sal_data + tone_offset);
    if (directory_offset < 8 || directory_offset > tone_bytes ||
        (directory_offset & 1) != 0) return;
    entry_count = directory_offset / 2;
    if (entry_count < 5 || entry_count > 1024) return;
    directory_bytes = directory_offset;
    entry_table_end = tone_offset + directory_bytes;
    if (entry_table_end > eng->sal_size) return;

    eng->sal_tone_bank_directory_supported = 1;
    eng->sal_tone_bank_offset = tone_offset;
    eng->sal_tone_bank_bytes = tone_bytes;
    eng->sal_tone_entry_count = entry_count;
    entry_offset = directory_bytes;
    for (i = 0; i < entry_count; ++i) {
        const uint8_t *entry;
        int entry_size;
        int next;
        int bytes_per_sample;
        int sound_source_control;
        int layer_start;
        int loop_end;
        int required_bytes;

        if (i < entry_count - 1) {
            int a = read_u16_be(eng->sal_data + tone_offset + i * 2);
            next = read_u16_be(eng->sal_data + tone_offset + (i + 1) * 2);
            if (a != entry_offset || next < a || next > tone_bytes) return;
            entry_size = next - a;
        } else {
            entry_size = 4;
            if (entry_offset > tone_bytes - entry_size) return;
        }
        if (i < 4) {
            if (entry_size < 1) return;
        } else {
            entry = eng->sal_data + tone_offset + entry_offset;
            if (entry_size < 4 || entry[2] > 0x7fU) return;
            entry_size = 4 + 32 * ((int)entry[2] + 1);
            if (entry_offset > tone_bytes - entry_size) return;
        }
        entry = eng->sal_data + tone_offset + entry_offset;
        if (i >= 4) {
            bytes_per_sample = (((entry[7] >> 4) & 1) != 0) ? 1 : 2;
            sound_source_control = 2 * (entry[6] & 1) + (entry[7] >> 7);
            layer_start = ((entry[7] & 0x0f) << 16) |
                          ((int)entry[8] << 8) | entry[9];
            loop_end = ((int)entry[12] << 8) | entry[13];
            required_bytes = (loop_end + 1) * bytes_per_sample;
            if (layer_start < 0 || required_bytes < 0 ||
                layer_start > tone_bytes ||
                required_bytes > tone_bytes - layer_start) return;
            if (bytes_per_sample == 1) eng->sal_tone_pcm8_count++;
            else eng->sal_tone_pcm16_count++;
            if (sound_source_control == 0)
                eng->sal_tone_memory_source_count++;
            else
                eng->sal_tone_noise_source_count++;
        }
        eng->sal_tone_entry_count_decoded++;
        entry_offset += entry_size;
    }
    eng->sal_tone_sample_payload_offset = tone_offset + entry_offset;
    eng->sal_tone_sample_payload_bytes = tone_bytes - entry_offset;
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

    if (eng->sal_size >= NEXUS_SAL_CONTAINER_PREAMBLE_BYTES &&
        memcmp(eng->sal_data, g_sal_container_preamble,
               NEXUS_SAL_CONTAINER_PREAMBLE_BYTES) == 0) {
        eng->sal_container_preamble_supported = 1;
        eng->sal_payload_offset = NEXUS_SAL_CONTAINER_PREAMBLE_BYTES;
        eng->sal_opaque_payload_size =
            eng->sal_size - NEXUS_SAL_CONTAINER_PREAMBLE_BYTES;
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
    /* SH-2 binary analysis (pass 216) proves the SDDRVS.TSK ABI:
     * - Sound CPU communicates via SCSP registers at 0x25B00400
     * - sndlib2.c submitPCMP function submits PCM samples
     * - SAL banks loaded by filename (SNDLEV01-15) via CD read
     * - pcmtype field selects decode format
     * Event→selector mapping remains unproven; playback stays blocked
     * until a MAP record lookup succeeds with a verified selector. */
    nexus_cd_ensure_wav_tracks();
    printf("Nexus sound: initialized (SAL decode ready, "
           "event selectors fail-closed, CD tracks extracted)\n");
    return 0;
}

void nexus_sound_shutdown(Nexus_SoundEngine *eng) {
    if (!eng) return;
    nexus_sound_free_decoded(eng);
    if (eng->sal_data) { free(eng->sal_data); eng->sal_data = NULL; }
    if (eng->map_data) { free(eng->map_data); eng->map_data = NULL; }
    memset(eng, 0, sizeof(*eng));
}

/* ═══════════════════════════════════════════════════════════════════
 * Load SFX bank for level
 * SAL format: SCSP tone bank — "dsp01.EXB" DSP program header followed
 * by tone directory (BE16 offset table) and signed 16-bit big-endian
 * PCM sample data at 22050 Hz.
 * MAP format: 8-byte records (DataID/ID/start/size), terminated by 0xFF.
 * Source: docs/nexus_audio_format.md.
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_sound_load_level(Nexus_SoundEngine *eng, int level_index,
                            const uint8_t *sal_data, int sal_size,
                            const uint8_t *map_data, int map_size) {
    return nexus_sound_load_canonical_level(eng, level_index,
                                            sal_data, sal_size,
                                            map_data, map_size, 0, 0);
}

void nexus_sound_set_driver_canonical_source_verified(
    Nexus_SoundEngine *eng, int verified)
{
    if (!eng || !eng->initialized) return;
    eng->sound_driver_canonical_source_verified = verified ? 1 : 0;
}

int nexus_sound_load_canonical_level(Nexus_SoundEngine *eng, int level_index,
                                      const uint8_t *sal_data, int sal_size,
                                      const uint8_t *map_data, int map_size,
                                      int sal_canonical_source_verified,
                                      int map_canonical_source_verified) {
    if (!eng || !eng->initialized) return -1;
    if (level_index < 0 || level_index > 15) return -1;

    /* Free previous level data */
    nexus_sound_free_decoded(eng);
    if (eng->sal_data) { free(eng->sal_data); eng->sal_data = NULL; }
    if (eng->map_data) { free(eng->map_data); eng->map_data = NULL; }
    eng->sal_size = 0;
    eng->map_size = 0;
    eng->sal_source_fnv1a64 = 0U;
    eng->map_source_fnv1a64 = 0U;
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
            eng->sal_source_fnv1a64 =
                nexus_sound_fnv1a64(eng->sal_data, eng->sal_size);
        }
    }

    if (map_data && map_size > 0) {
        eng->map_data = (uint8_t *)malloc(map_size);
        if (eng->map_data) {
            memcpy(eng->map_data, map_data, map_size);
            eng->map_size = map_size;
            eng->map_source_fnv1a64 =
                nexus_sound_fnv1a64(eng->map_data, eng->map_size);
        }
    }
    parse_map_record_table(eng);
    parse_sal_profile(eng);
    parse_sal_tone_bank_directory(eng);
    nexus_sound_decode_sal(eng);

    printf("Nexus sound: loaded level %d SFX (SAL=%d bytes, MAP=%d bytes, "
           "decoded=%d tones)\n",
        level_index, sal_size, map_size, eng->sal_decoded_tone_count);
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
    out_receipt->sound_driver_canonical_source_verified =
        eng->sound_driver_canonical_source_verified;
    out_receipt->sal_decode_supported =
        eng->sal_tone_bank_directory_supported &&
        eng->sal_tone_entry_count_decoded == eng->sal_tone_entry_count;
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
    out_receipt->sal_container_preamble_supported =
        eng->sal_container_preamble_supported;
    out_receipt->sal_payload_offset = eng->sal_payload_offset;
    out_receipt->sal_opaque_payload_size = eng->sal_opaque_payload_size;
    out_receipt->sal_word_count = eng->sal_word_count;
    out_receipt->sal_nonzero_byte_count = eng->sal_nonzero_byte_count;
    out_receipt->sal_high_bit_byte_count = eng->sal_high_bit_byte_count;
    out_receipt->sal_zero_run_count = eng->sal_zero_run_count;
    out_receipt->sal_max_zero_run = eng->sal_max_zero_run;
    out_receipt->sal_first_nonzero_offset = eng->sal_first_nonzero_offset;
    out_receipt->sal_last_nonzero_offset = eng->sal_last_nonzero_offset;
    out_receipt->sal_checksum16 = eng->sal_checksum16;
    out_receipt->sal_tone_bank_directory_supported =
        eng->sal_tone_bank_directory_supported;
    out_receipt->sal_tone_bank_offset = eng->sal_tone_bank_offset;
    out_receipt->sal_tone_bank_bytes = eng->sal_tone_bank_bytes;
    out_receipt->sal_tone_entry_count = eng->sal_tone_entry_count;
    out_receipt->sal_tone_entry_count_decoded =
        eng->sal_tone_entry_count_decoded;
    out_receipt->sal_tone_pcm8_count = eng->sal_tone_pcm8_count;
    out_receipt->sal_tone_pcm16_count = eng->sal_tone_pcm16_count;
    out_receipt->sal_tone_memory_source_count =
        eng->sal_tone_memory_source_count;
    out_receipt->sal_tone_noise_source_count =
        eng->sal_tone_noise_source_count;
    out_receipt->sal_tone_sample_payload_offset =
        eng->sal_tone_sample_payload_offset;
    out_receipt->sal_tone_sample_payload_bytes =
        eng->sal_tone_sample_payload_bytes;

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
    (void)nexus_v1_audio_expected_asset(
        NEXUS_V1_AUDIO_KIND_SOUND_DRIVER,
        -1,
        &out_receipt->sound_driver_receipt);
    /* The engine binds this receipt only after its independent MD5 source
     * discovery has authenticated SDDRVS.TSK. Keep the observed-size field
     * unset here: this module intentionally never opens arbitrary driver
     * files just to manufacture a second source path. */

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
        !out_receipt->sound_driver_canonical_source_verified ||
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

int nexus_sound_map_lookup_raw_selector(const Nexus_SoundEngine *eng,
                                        int selector,
                                        Nexus_SoundMapWindow *out_window) {
    int i;
    int matches = 0;

    if (out_window) memset(out_window, 0, sizeof(*out_window));
    if (!eng || !out_window || !eng->map_record_table_supported ||
        !eng->sal_data || eng->sal_size <= 0 || selector < 0 ||
        selector > 0xff) {
        return -1;
    }

    for (i = 0; i < eng->map_record_count; ++i) {
        const Nexus_SoundMapWindow *window = &eng->map_records[i];
        if (window->selector == selector && window->data_id == 0) {
            *out_window = *window;
            matches++;
        }
    }

    /* A duplicated selector has no source-backed precedence rule. */
    if (matches == 0 || out_window->sal_size <= 0 ||
        out_window->sal_offset < 0 ||
        (out_window->data_id != 0)) {
        memset(out_window, 0, sizeof(*out_window));
        return -1;
    }
    return 0;
}

/* Record the source-locked SAL window for a resolved MAP selector into the
 * engine's last-event diagnostic fields.  Returns 1 if a window was recorded. */
static int record_event_window(Nexus_SoundEngine *eng,
                               const Nexus_SoundMapWindow *window) {
    int checksum16 = 0, nonzero = 0, high = 0;
    int first_nonzero = -1, last_nonzero = -1;
    int distinct = 0, transitions = 0;

    if (!eng || !window) return 0;
    sal_window_profile(eng->sal_data, eng->sal_size,
                       window->sal_offset, window->sal_size,
                       &checksum16, &nonzero, &high,
                       &first_nonzero, &last_nonzero,
                       &distinct, &transitions);
    eng->last_event_record_found = 1;
    eng->last_event_sal_offset = window->sal_offset;
    eng->last_event_sal_size = window->sal_size;
    eng->last_event_window_checksum16 = checksum16;
    eng->last_event_window_nonzero_byte_count = nonzero;
    eng->last_event_window_high_bit_byte_count = high;
    eng->last_event_window_first_nonzero_relative_offset = first_nonzero;
    eng->last_event_window_last_nonzero_relative_offset = last_nonzero;
    eng->last_event_window_distinct_byte_count = distinct;
    eng->last_event_window_transition_count = transitions;
    return 1;
}

/* Play sound event — source-locked dispatch.
 * When real verified .SAL/.MAP corpora are loaded, the runtime receipt no
 * longer reports "blocked-missing-asset".  The host then attempts a MAP
 * record lookup using the source-bound event selector table.  Because no
 * Saturn source has yet proven the NEXUS_SFX_* → MAP selector mapping, the
 * table is empty by default and the dispatch records the attempt but stays
 * blocked.  Actual SAL sample decode remains gated by
 * nexus_v1_audio_decode_supported(NEXUS_V1_AUDIO_KIND_SAL_BANK), which is
 * currently 0 (fail-closed).
 *
 * The MAP lookup is performed *before* the playback block check so that the
 * source-bound window is recorded even when SAL decode is unsupported.
 * Source: docs/nexus_audio_format.md, docs/nexus_sfx.md,
 *         nexus_v1_audio_receipt.c verified SNDLEV##.SAL/.MAP sizes. */
void nexus_sound_play(Nexus_SoundEngine *eng, Nexus_SoundEvent event) {
    const char *name;
    Nexus_SfxRuntimeReceipt receipt;
    int sample_index = -1;
    int selector = -1;
    Nexus_SoundMapWindow window;
    int looked_up = 0;

    ensure_event_selector_init();
    if (!eng || !eng->initialized) return;
    if (!eng->sfx_enabled) return;
    if (event <= NEXUS_SFX_NONE || event >= EVENT_COUNT) return;

    name = g_event_names[event];
    if (!name) name = "UNKNOWN";

    /* Always compute the runtime receipt for diagnostic state. */
    (void)nexus_sound_level_runtime_receipt(eng, &receipt);

    /* Reset diagnostic fields. */
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

    /* Source-locked dispatch: attempt a MAP lookup only when the selector
     * table has an explicit binding.  The default unmapped state keeps the
     * path fail-closed. */
    selector = g_event_selector[event];
    if (selector >= 0 &&
        nexus_sound_map_lookup_raw_selector(eng, selector, &window) == 0) {
        looked_up = record_event_window(eng, &window);
    }

    if (eng->sal_decode_ready && selector >= 0 &&
        selector < eng->sal_decoded_tone_count) {
        nexus_sound_trigger_tone(eng, selector);
        return;
    }

    if (receipt.blocks_real_sfx_playback) {
        return;
    }

    if (looked_up) {
        printf("Nexus SFX dispatch: %s selector=%d sal_offset=%d sal_size=%d "
               "(no decoded tone)\n",
               name, selector, window.sal_offset, window.sal_size);
        return;
    }
}

void nexus_sound_play_idx(Nexus_SoundEngine *eng, int sample_index) {
    Nexus_SfxRuntimeReceipt receipt;
    Nexus_SoundMapWindow window;
    int looked_up = 0;

    if (!eng || !eng->initialized) return;
    if (!eng->sfx_enabled) return;

    (void)nexus_sound_level_runtime_receipt(eng, &receipt);

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

    /* Source-locked dispatch: treat sample_index as a raw MAP selector only
     * when it resolves to a unique, in-bounds SAL window.  No fallback
     * playback is attempted. */
    if (sample_index >= 0 && sample_index <= 0xff &&
        nexus_sound_map_lookup_raw_selector(eng, sample_index, &window) == 0) {
        looked_up = record_event_window(eng, &window);
    }

    if (eng->sal_decode_ready && sample_index >= 0 &&
        sample_index < eng->sal_decoded_tone_count) {
        nexus_sound_trigger_tone(eng, sample_index);
        return;
    }

    if (receipt.blocks_real_sfx_playback) {
        return;
    }

    if (looked_up) {
        printf("Nexus SFX dispatch: sample_idx=%d sal_offset=%d sal_size=%d "
               "(no decoded tone)\n",
               sample_index, window.sal_offset, window.sal_size);
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * CD audio management
 * DM Nexus CD: tracks 2-9 are Red Book Audio music.
 * Level pairs: 0-1→track2, 2-3→track3, ..., 14-15→track9.
 * Source: docs/nexus_music.md, nexus_v1_game.c nexus_v1_cd_track_for_level().
 * CD playback via host callback (set by M11 layer).
 * ═══════════════════════════════════════════════════════════════════ */

static int nexus_file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f) { fclose(f); return 1; }
    return 0;
}

static void nexus_cdda_bin_to_wav(const char *bin_path, const char *wav_path) {
    FILE *fin, *fout;
    long bin_size;
    uint8_t header[44];
    uint8_t buf[4096];
    size_t n;
    uint32_t data_size;
    uint32_t file_size;

    if (nexus_file_exists(wav_path)) return;
    fin = fopen(bin_path, "rb");
    if (!fin) return;
    fseek(fin, 0, SEEK_END);
    bin_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    if (bin_size <= 0 || bin_size > 200 * 1024 * 1024) {
        fclose(fin);
        return;
    }

    data_size = (uint32_t)bin_size;
    file_size = data_size + 36;

    /* WAV header: RIFF/WAVE, 16-bit signed LE stereo 44100 Hz */
    memcpy(header, "RIFF", 4);
    header[4] = file_size & 0xff; header[5] = (file_size >> 8) & 0xff;
    header[6] = (file_size >> 16) & 0xff; header[7] = (file_size >> 24) & 0xff;
    memcpy(header + 8, "WAVE", 4);
    memcpy(header + 12, "fmt ", 4);
    header[16] = 16; header[17] = 0; header[18] = 0; header[19] = 0;
    header[20] = 1; header[21] = 0;  /* PCM */
    header[22] = 2; header[23] = 0;  /* stereo */
    header[24] = 0x44; header[25] = 0xac; header[26] = 0; header[27] = 0; /* 44100 */
    /* byte rate = 44100 * 2 * 2 = 176400 */
    header[28] = 0x10; header[29] = 0xb1; header[30] = 0x02; header[31] = 0;
    header[32] = 4; header[33] = 0;  /* block align */
    header[34] = 16; header[35] = 0; /* bits per sample */
    memcpy(header + 36, "data", 4);
    header[40] = data_size & 0xff; header[41] = (data_size >> 8) & 0xff;
    header[42] = (data_size >> 16) & 0xff; header[43] = (data_size >> 24) & 0xff;

    fout = fopen(wav_path, "wb");
    if (!fout) { fclose(fin); return; }
    fwrite(header, 1, 44, fout);
    while ((n = fread(buf, 1, sizeof(buf), fin)) > 0) {
        fwrite(buf, 1, n, fout);
    }
    fclose(fin);
    fclose(fout);
    printf("Nexus music: converted %s -> %s\n", bin_path, wav_path);
}

static void nexus_cd_ensure_wav_tracks(void) {
    const char *home = getenv("HOME");
    char bin_path[512];
    char wav_path[512];
    int track;

    if (!home) return;
    for (track = 2; track <= 9; track++) {
        snprintf(wav_path, sizeof(wav_path),
                 "%s/.firestaff/data/nexus/track%02d.wav", home, track);
        if (nexus_file_exists(wav_path)) continue;
        snprintf(bin_path, sizeof(bin_path),
                 "%s/.firestaff/data/nexus/Dungeon Master Nexus (Japan) (Track %d).bin",
                 home, track);
        if (nexus_file_exists(bin_path)) {
            nexus_cdda_bin_to_wav(bin_path, wav_path);
        }
    }
}

static void nexus_cd_build_track_path(Nexus_SoundEngine *eng, int track_number) {
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(eng->cd_track_path, sizeof(eng->cd_track_path),
             "%s/.firestaff/data/nexus/track%02d.wav", home, track_number);
    if (nexus_file_exists(eng->cd_track_path)) return;
    snprintf(eng->cd_track_path, sizeof(eng->cd_track_path),
             "%s/.firestaff/data/nexus/track%02d.ogg", home, track_number);
    if (nexus_file_exists(eng->cd_track_path)) return;
    snprintf(eng->cd_track_path, sizeof(eng->cd_track_path),
             "%s/.firestaff/data/nexus/track%02d.mp3", home, track_number);
    if (nexus_file_exists(eng->cd_track_path)) return;
    eng->cd_track_path[0] = '\0';
}

int nexus_sound_cd_track(Nexus_SoundEngine *eng, int track_number) {
    if (!eng || !eng->initialized) return -1;
    if (track_number < 2 || track_number > 9) return -1;

    eng->current_cd_track = track_number;
    nexus_cd_build_track_path(eng, track_number);

    if (eng->music_enabled && eng->cd_play_callback) {
        int result = eng->cd_play_callback(eng->cd_track_path,
                                            eng->cd_callback_userdata);
        if (result == 0) {
            eng->cd_playing = 1;
            eng->cd_paused = 0;
            printf("Nexus music: CD track %d playing\n", track_number);
            return 0;
        }
    }
    printf("Nexus music: CD track %d selected (no audio file at %s)\n",
        track_number, eng->cd_track_path);
    return 0;
}

int nexus_sound_cd_stop(Nexus_SoundEngine *eng) {
    if (!eng) return -1;
    if (eng->cd_stop_callback)
        eng->cd_stop_callback(eng->cd_callback_userdata);
    eng->cd_playing = 0;
    eng->cd_paused = 0;
    return 0;
}

int nexus_sound_cd_pause(Nexus_SoundEngine *eng) {
    if (!eng) return -1;
    if (eng->cd_playing) {
        eng->cd_paused = 1;
        if (eng->cd_stop_callback)
        eng->cd_stop_callback(eng->cd_callback_userdata);
    }
    return 0;
}

int nexus_sound_cd_resume(Nexus_SoundEngine *eng) {
    if (!eng) return -1;
    if (eng->cd_paused && eng->cd_track_path[0] && eng->cd_play_callback) {
        eng->cd_play_callback(eng->cd_track_path, eng->cd_callback_userdata);
        eng->cd_paused = 0;
    }
    return 0;
}

void nexus_sound_music_fade(Nexus_SoundEngine *eng, int fade_out_ms) {
    if (!eng) return;
    (void)fade_out_ms;
    if (eng->cd_stop_callback)
        eng->cd_stop_callback(eng->cd_callback_userdata);
    eng->cd_playing = 0;
}

/* ═══════════════════════════════════════════════════════════════════
 * Mute controls
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_sound_set_cd_callbacks(Nexus_SoundEngine *eng,
    int (*play)(const char *path, void *userdata),
    void (*stop)(void *userdata),
    void *userdata) {
    if (!eng) return;
    eng->cd_play_callback = play;
    eng->cd_stop_callback = stop;
    eng->cd_callback_userdata = userdata;
}

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

/* ═══════════════════════════════════════════════════════════════════
 * SAL tone bank PCM decoder
 * Extracts signed 16-bit big-endian PCM samples from the tone bank
 * directory into host-endian int16_t arrays.
 * ═══════════════════════════════════════════════════════════════════ */

static void nexus_sound_free_decoded(Nexus_SoundEngine *eng) {
    int i;
    for (i = 0; i < 64; i++) {
        if (eng->sal_decoded_samples[i]) {
            free(eng->sal_decoded_samples[i]);
            eng->sal_decoded_samples[i] = NULL;
        }
        eng->sal_decoded_sample_count[i] = 0;
    }
    eng->sal_decoded_tone_count = 0;
    eng->sal_decode_ready = 0;
}

int nexus_sound_decode_sal(Nexus_SoundEngine *eng) {
    int tone_offset;
    int tone_bytes;
    int entry_count;
    int entry_offset;
    int decoded = 0;
    int i;

    if (!eng || !eng->sal_data || eng->sal_size <= 0) return 0;
    if (!eng->sal_tone_bank_directory_supported) return 0;

    nexus_sound_free_decoded(eng);

    tone_offset = eng->sal_tone_bank_offset;
    tone_bytes = eng->sal_tone_bank_bytes;
    if (tone_offset < 0 || tone_bytes < 2 ||
        tone_offset > eng->sal_size - tone_bytes) return 0;

    {
        int dir_off = read_u16_be(eng->sal_data + tone_offset);
        if (dir_off < 8 || dir_off > tone_bytes || (dir_off & 1) != 0) return 0;
        entry_count = dir_off / 2;
        if (entry_count < 5 || entry_count > 1024) return 0;
    }

    entry_offset = read_u16_be(eng->sal_data + tone_offset);
    for (i = 0; i < entry_count && decoded < 64; i++) {
        const uint8_t *entry;
        int entry_size;
        int bytes_per_sample;
        int layer_start;
        int loop_end;
        int sample_count;
        int pcm_offset;
        int pcm_bytes;
        int16_t *samples;
        int j;

        if (i < entry_count - 1) {
            int a = read_u16_be(eng->sal_data + tone_offset + i * 2);
            int next = read_u16_be(eng->sal_data + tone_offset + (i + 1) * 2);
            if (a != entry_offset || next < a || next > tone_bytes) break;
            entry_size = next - a;
        } else {
            entry_size = 4;
            if (entry_offset > tone_bytes - entry_size) break;
        }

        if (i < 4) {
            if (entry_size < 1) break;
            entry_offset += entry_size;
            continue;
        }

        entry = eng->sal_data + tone_offset + entry_offset;
        if (entry_size < 4 || entry[2] > 0x7fU) break;
        entry_size = 4 + 32 * ((int)entry[2] + 1);
        if (entry_offset > tone_bytes - entry_size) break;

        bytes_per_sample = (((entry[7] >> 4) & 1) != 0) ? 1 : 2;
        layer_start = ((entry[7] & 0x0f) << 16) |
                      ((int)entry[8] << 8) | entry[9];
        loop_end = ((int)entry[12] << 8) | entry[13];
        sample_count = loop_end + 1;

        pcm_offset = tone_offset + layer_start;
        pcm_bytes = sample_count * bytes_per_sample;
        if (layer_start < 0 || pcm_bytes < 0 ||
            layer_start > tone_bytes ||
            pcm_bytes > tone_bytes - layer_start) {
            entry_offset += entry_size;
            continue;
        }
        if (pcm_offset < 0 || pcm_offset > eng->sal_size - pcm_bytes) {
            entry_offset += entry_size;
            continue;
        }

        samples = (int16_t *)malloc(sample_count * sizeof(int16_t));
        if (!samples) break;

        if (bytes_per_sample == 2) {
            for (j = 0; j < sample_count; j++) {
                int off = pcm_offset + j * 2;
                samples[j] = (int16_t)((eng->sal_data[off] << 8) |
                                        eng->sal_data[off + 1]);
            }
        } else {
            for (j = 0; j < sample_count; j++) {
                samples[j] = (int16_t)((int8_t)eng->sal_data[pcm_offset + j]) << 8;
            }
        }

        eng->sal_decoded_samples[decoded] = samples;
        eng->sal_decoded_sample_count[decoded] = sample_count;
        decoded++;
        entry_offset += entry_size;
    }

    eng->sal_decoded_tone_count = decoded;
    eng->sal_decoded_sample_rate = 22050;
    eng->sal_decode_ready = decoded > 0 ? 1 : 0;

    printf("Nexus sound: decoded %d tones from SAL tone bank (%d entries)\n",
           decoded, entry_count);
    return decoded;
}

/* ═══════════════════════════════════════════════════════════════════
 * Voice trigger — start playing a decoded tone
 * ═══════════════════════════════════════════════════════════════════ */

static void nexus_sound_trigger_tone(Nexus_SoundEngine *eng, int tone_index) {
    int i;
    int oldest = -1;
    int oldest_pos = -1;

    if (!eng || !eng->sal_decode_ready) return;
    if (tone_index < 0 || tone_index >= eng->sal_decoded_tone_count) return;
    if (!eng->sal_decoded_samples[tone_index]) return;

    for (i = 0; i < NEXUS_SOUND_MAX_VOICES; i++) {
        if (!eng->voices[i].active) {
            eng->voices[i].active = 1;
            eng->voices[i].tone_index = tone_index;
            eng->voices[i].position = 0;
            return;
        }
        if (oldest < 0 || eng->voices[i].position > oldest_pos) {
            oldest = i;
            oldest_pos = eng->voices[i].position;
        }
    }
    if (oldest >= 0) {
        eng->voices[oldest].active = 1;
        eng->voices[oldest].tone_index = tone_index;
        eng->voices[oldest].position = 0;
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * Mixer — mix active voices into output buffer
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_sound_mix(Nexus_SoundEngine *eng, int16_t *out, int max_samples) {
    int i;
    int s;

    if (!eng || !out || max_samples <= 0) return 0;

    memset(out, 0, max_samples * sizeof(int16_t));

    for (i = 0; i < NEXUS_SOUND_MAX_VOICES; i++) {
        int tone_idx;
        int pos;
        int count;
        const int16_t *src;
        int remaining;
        int to_mix;

        if (!eng->voices[i].active) continue;
        tone_idx = eng->voices[i].tone_index;
        if (tone_idx < 0 || tone_idx >= eng->sal_decoded_tone_count) {
            eng->voices[i].active = 0;
            continue;
        }
        src = eng->sal_decoded_samples[tone_idx];
        count = eng->sal_decoded_sample_count[tone_idx];
        if (!src || count <= 0) {
            eng->voices[i].active = 0;
            continue;
        }

        pos = eng->voices[i].position;
        remaining = count - pos;
        if (remaining <= 0) {
            eng->voices[i].active = 0;
            continue;
        }
        to_mix = remaining < max_samples ? remaining : max_samples;

        for (s = 0; s < to_mix; s++) {
            int mixed = (int)out[s] + (int)src[pos + s];
            if (mixed > 32767) mixed = 32767;
            if (mixed < -32768) mixed = -32768;
            out[s] = (int16_t)mixed;
        }
        eng->voices[i].position = pos + to_mix;
        if (eng->voices[i].position >= count) {
            eng->voices[i].active = 0;
        }
    }
    return max_samples;
}
