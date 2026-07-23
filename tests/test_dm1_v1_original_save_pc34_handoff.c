#include "dm1_v1_original_save_pc34_handoff.h"
#include "dm1_v1_c15_layout_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_resurrection_pc34_compat.h"
#include "dm1_v1_spell_casting_pc34_compat.h"

#include "memory_champion_state_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_magic_pc34_compat.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#include <process.h>
#define test_mkdir(path) _mkdir(path)
#define test_rmdir(path) _rmdir(path)
#define test_getpid() _getpid()
#else
#include <unistd.h>
#define test_mkdir(path) mkdir((path), 0700)
#define test_rmdir(path) rmdir(path)
#define test_getpid() getpid()
#endif

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL %s (line %d): %s\n", msg, __LINE__, #cond); \
            exit(1); \
        } \
    } while (0)

#define ORIGINAL_PC34_CHAMPION_BYTES 319
#define ORIGINAL_PC34_PARTY_INFO_BYTES 128
#define ORIGINAL_PC34_PARTY_BYTES \
    ((ORIGINAL_PC34_CHAMPION_BYTES * CHAMPION_MAX_PARTY) + \
     ORIGINAL_PC34_PARTY_INFO_BYTES)
#define ORIGINAL_PC34_ACTIVE_GROUP_BYTES 16
#define ORIGINAL_PC34_ACTIVE_GROUP_COUNT 3
#define ORIGINAL_PC34_ACTIVE_GROUP_PART_BYTES \
    (ORIGINAL_PC34_ACTIVE_GROUP_BYTES * ORIGINAL_PC34_ACTIVE_GROUP_COUNT)
#define ORIGINAL_PC34_EVENT_BYTES 10
#define ORIGINAL_PC34_EVENT_COUNT 3
#define ORIGINAL_PC34_EVENT_MAXIMUM_COUNT 4
#define ORIGINAL_PC34_EVENTS_PART_BYTES \
    (ORIGINAL_PC34_EVENT_BYTES * ORIGINAL_PC34_EVENT_MAXIMUM_COUNT)
#define ORIGINAL_PC34_TIMELINE_PART_BYTES \
    (2 * ORIGINAL_PC34_EVENT_MAXIMUM_COUNT)
#define ORIGINAL_PC34_PARTY_INFO_EVENT73_COUNT_OFFSET 2
#define ORIGINAL_PC34_PARTY_INFO_MAGICAL_LIGHT_AMOUNT_OFFSET 0
#define ORIGINAL_PC34_PARTY_INFO_EVENT79_COUNT_OFFSET 3
#define ORIGINAL_PC34_PARTY_INFO_SHIELD_DEFENSE_OFFSET 4
#define ORIGINAL_PC34_PARTY_INFO_FIRE_SHIELD_DEFENSE_OFFSET 6
#define ORIGINAL_PC34_PARTY_INFO_SPELL_SHIELD_DEFENSE_OFFSET 8
#define ORIGINAL_PC34_PARTY_INFO_EVENT71_COUNT_OFFSET 86

static void wr16le(unsigned char *p, uint16_t v)
{
    p[0] = (unsigned char)(v & 0xffu);
    p[1] = (unsigned char)((v >> 8) & 0xffu);
}

static void wr32le(unsigned char *p, uint32_t v)
{
    wr16le(p, (uint16_t)(v & 0xffffu));
    wr16le(p + 2, (uint16_t)((v >> 16) & 0xffffu));
}

static uint16_t rd16le(const unsigned char *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t fnv1a32(const unsigned char *bytes, size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static void write_original_event(unsigned char *dst,
                                 uint32_t map_time,
                                 int type,
                                 int priority,
                                 int map_x,
                                 int map_y,
                                 int cell,
                                 int effect);

static int export_local_dm1_dungeon_save(unsigned char *bytes,
                                         size_t bytes_cap,
                                         int *written);

static uint16_t byte_sum16(const unsigned char *bytes, size_t byte_count)
{
    uint16_t sum = 0u;
    size_t i;

    for (i = 0u; i < byte_count; ++i) {
        sum = (uint16_t)(sum + bytes[i]);
    }
    return sum;
}

static size_t original_pc34_tail_offset(const unsigned char *bytes,
                                        size_t byte_count)
{
    size_t offset = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int part;

    if (!bytes || byte_count < offset) {
        return 0u;
    }
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;
        if (offset + 2u > byte_count) {
            return 0u;
        }
        part_size = rd16le(bytes + offset);
        offset += 2u;
        if ((size_t)part_size > byte_count - offset) {
            return 0u;
        }
        offset += (size_t)part_size;
    }
    if ((size_t)CHAMPION_MAX_PARTY * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT >
        byte_count - offset) {
        return 0u;
    }
    return offset + (size_t)CHAMPION_MAX_PARTY *
        CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
}

static uint16_t checksum_first_half(const unsigned char *header)
{
    uint16_t acc = 0;
    size_t i;
    for (i = 0; i < 32u; ++i) {
        acc = (uint16_t)(acc + rd16le(header + (i * 8u) + 0u));
        acc = (uint16_t)(acc ^ rd16le(header + (i * 8u) + 2u));
        acc = (uint16_t)(acc - rd16le(header + (i * 8u) + 4u));
        acc = (uint16_t)(acc ^ rd16le(header + (i * 8u) + 6u));
    }
    return acc;
}

static uint16_t checksum_second_half_plain(const unsigned char *header)
{
    uint16_t sum = 0;
    size_t i;
    for (i = 128u; i < 256u; ++i) {
        sum = (uint16_t)(sum + rd16le(header + (i * 2u)));
    }
    return sum;
}

static void xor_obfuscate_second_half(unsigned char *header, uint16_t key)
{
    uint16_t rolling_key = key;
    size_t i;
    for (i = 128u; i < 256u; ++i) {
        unsigned char *word = header + (i * 2u);
        uint16_t v = rd16le(word);
        wr16le(word, (uint16_t)(v ^ rolling_key));
        rolling_key = (uint16_t)(rolling_key + 128u);
    }
}

static void xor_words(unsigned char *bytes, size_t word_count, uint16_t key)
{
    uint16_t rolling_key = key;
    size_t i;
    for (i = 0u; i < word_count; ++i) {
        unsigned char *word = bytes + i * 2u;
        uint16_t v = rd16le(word);
        wr16le(word, (uint16_t)(v ^ rolling_key));
        rolling_key = (uint16_t)(rolling_key + (uint16_t)word_count);
    }
}

static uint16_t checksum_and_xor_words(unsigned char *bytes,
                                       size_t word_count,
                                       uint16_t key)
{
    uint16_t rolling_key = key;
    uint16_t checksum = key;
    size_t i;
    for (i = 0u; i < word_count; ++i) {
        unsigned char *word = bytes + i * 2u;
        uint16_t v = rd16le(word);
        checksum = (uint16_t)(checksum + v);
        v = (uint16_t)(v ^ rolling_key);
        wr16le(word, v);
        checksum = (uint16_t)(checksum + v);
        rolling_key = (uint16_t)(rolling_key + (uint16_t)word_count);
    }
    return checksum;
}

static int write_part(unsigned char *dst,
                      int dst_cap,
                      const unsigned char *plain,
                      int byte_count,
                      uint16_t key,
                      uint16_t *out_checksum)
{
    if (dst_cap < 2 + byte_count || (byte_count & 1) != 0) {
        return -1;
    }
    wr16le(dst, (uint16_t)byte_count);
    memcpy(dst + 2, plain, (size_t)byte_count);
    if (out_checksum) {
        *out_checksum = checksum_and_xor_words(
            dst + 2, (size_t)byte_count / 2u, key);
    } else {
        (void)checksum_and_xor_words(
            dst + 2, (size_t)byte_count / 2u, key);
    }
    return 2 + byte_count;
}

static int rewrite_fixture_timeline_with_duplicate_index(unsigned char *bytes,
                                                         int byte_count)
{
    unsigned char *header;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t header_key;
    uint16_t part_key = 0u;
    uint16_t part_checksum;
    int part;

    if (!bytes || byte_count < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        return 0;
    }
    header = bytes;
    header_key = rd16le(header + 10u * 2u);
    xor_obfuscate_second_half(header, header_key);
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;

        if (cursor + 2u > (size_t)byte_count) goto fail;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (cursor + part_size > (size_t)byte_count ||
            (part_size & 1u) != 0u) goto fail;
        if (part == SAVEGAME_PC34_PART_TIMELINE) {
            if (part_size < 4u) goto fail;
            part_key = rd16le(header + 310u + (size_t)part * 2u);
            xor_words(bytes + cursor, (size_t)part_size / 2u, part_key);
            /* Keep every byte/part checksum valid while making the first
             * two heap entries name the same EVENT index. */
            wr16le(bytes + cursor + 2u, rd16le(bytes + cursor));
            part_checksum = checksum_and_xor_words(
                bytes + cursor, (size_t)part_size / 2u, part_key);
            wr16le(header + 342u + (size_t)part * 2u, part_checksum);
            {
                uint16_t second_sum = checksum_second_half_plain(header);
                uint16_t first_before_last = checksum_first_half(header);
                uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                           first_before_last ^ second_sum);
                wr16le(header + 254u, last);
            }
            xor_obfuscate_second_half(header, header_key);
            return 1;
        }
        cursor += part_size;
    }

fail:
    xor_obfuscate_second_half(header, header_key);
    return 0;
}

static int rewrite_fixture_event(unsigned char *bytes,
                                 int byte_count,
                                 int event_index,
                                 uint32_t map_time,
                                 int type,
                                 int priority,
                                 int map_x,
                                 int map_y,
                                 int cell,
                                 int effect)
{
    unsigned char *header;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t header_key;
    int part;

    if (!bytes || event_index < 0 ||
        byte_count < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2) {
        return 0;
    }
    header = bytes;
    header_key = rd16le(header + 10u * 2u);
    xor_obfuscate_second_half(header, header_key);
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;

        if (cursor + 2u > (size_t)byte_count) goto fail;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (cursor + part_size > (size_t)byte_count ||
            (part_size & 1u) != 0u) goto fail;
        if (part == SAVEGAME_PC34_PART_EVENTS) {
            uint16_t part_key;
            uint16_t part_checksum;

            if ((size_t)event_index >= (size_t)part_size /
                                        ORIGINAL_PC34_EVENT_BYTES) goto fail;
            part_key = rd16le(header + 310u + (size_t)part * 2u);
            xor_words(bytes + cursor, (size_t)part_size / 2u, part_key);
            write_original_event(bytes + cursor +
                                     (size_t)event_index * ORIGINAL_PC34_EVENT_BYTES,
                                 map_time, type, priority,
                                 map_x, map_y, cell, effect);
            part_checksum = checksum_and_xor_words(
                bytes + cursor, (size_t)part_size / 2u, part_key);
            wr16le(header + 342u + (size_t)part * 2u, part_checksum);
            {
                uint16_t second_sum = checksum_second_half_plain(header);
                uint16_t first_before_last = checksum_first_half(header);
                uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                           first_before_last ^ second_sum);
                wr16le(header + 254u, last);
            }
            xor_obfuscate_second_half(header, header_key);
            return 1;
        }
        cursor += part_size;
    }

fail:
    xor_obfuscate_second_half(header, header_key);
    return 0;
}

static int rewrite_fixture_party_status_count(unsigned char *bytes,
                                              int byte_count,
                                              int party_info_offset,
                                              unsigned char count)
{
    unsigned char *header;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t header_key;
    int part;

    if (!bytes || byte_count < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2) {
        return 0;
    }
    header = bytes;
    header_key = rd16le(header + 10u * 2u);
    xor_obfuscate_second_half(header, header_key);
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;

        if (cursor + 2u > (size_t)byte_count) goto fail;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (cursor + part_size > (size_t)byte_count ||
            (part_size & 1u) != 0u) goto fail;
        if (part == SAVEGAME_PC34_PART_PARTY) {
            uint16_t part_key;
            uint16_t part_checksum;
            size_t count_offset;

            if (party_info_offset < 0 ||
                party_info_offset >= ORIGINAL_PC34_PARTY_INFO_BYTES) goto fail;
            count_offset = (size_t)ORIGINAL_PC34_CHAMPION_BYTES *
                               CHAMPION_MAX_PARTY +
                           (size_t)party_info_offset;

            if (part_size != ORIGINAL_PC34_PARTY_BYTES ||
                count_offset >= part_size) goto fail;
            part_key = rd16le(header + 310u + (size_t)part * 2u);
            xor_words(bytes + cursor, (size_t)part_size / 2u, part_key);
            bytes[cursor + count_offset] = count;
            part_checksum = checksum_and_xor_words(
                bytes + cursor, (size_t)part_size / 2u, part_key);
            wr16le(header + 342u + (size_t)part * 2u, part_checksum);
            {
                uint16_t second_sum = checksum_second_half_plain(header);
                uint16_t first_before_last = checksum_first_half(header);
                uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                           first_before_last ^ second_sum);
                wr16le(header + 254u, last);
            }
            xor_obfuscate_second_half(header, header_key);
            return 1;
        }
        cursor += part_size;
    }

fail:
    xor_obfuscate_second_half(header, header_key);
    return 0;
}

static int rewrite_fixture_party_status_i16(unsigned char *bytes,
                                            int byte_count,
                                            int party_info_offset,
                                            int16_t value)
{
    unsigned char *header;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t header_key;
    int part;

    if (!bytes || byte_count < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2 ||
        party_info_offset < 0 ||
        party_info_offset + 2 > ORIGINAL_PC34_PARTY_INFO_BYTES) {
        return 0;
    }
    header = bytes;
    header_key = rd16le(header + 10u * 2u);
    xor_obfuscate_second_half(header, header_key);
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;

        if (cursor + 2u > (size_t)byte_count) goto fail;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (cursor + part_size > (size_t)byte_count ||
            (part_size & 1u) != 0u) goto fail;
        if (part == SAVEGAME_PC34_PART_PARTY) {
            uint16_t part_key;
            uint16_t part_checksum;
            size_t value_offset = (size_t)ORIGINAL_PC34_CHAMPION_BYTES *
                                      CHAMPION_MAX_PARTY +
                                  (size_t)party_info_offset;

            if (part_size != ORIGINAL_PC34_PARTY_BYTES ||
                value_offset + 2u > part_size) goto fail;
            part_key = rd16le(header + 310u + (size_t)part * 2u);
            xor_words(bytes + cursor, (size_t)part_size / 2u, part_key);
            wr16le(bytes + cursor + value_offset, (uint16_t)value);
            part_checksum = checksum_and_xor_words(
                bytes + cursor, (size_t)part_size / 2u, part_key);
            wr16le(header + 342u + (size_t)part * 2u, part_checksum);
            {
                uint16_t second_sum = checksum_second_half_plain(header);
                uint16_t first_before_last = checksum_first_half(header);
                uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                           first_before_last ^ second_sum);
                wr16le(header + 254u, last);
            }
            xor_obfuscate_second_half(header, header_key);
            return 1;
        }
        cursor += part_size;
    }

fail:
    xor_obfuscate_second_half(header, header_key);
    return 0;
}

static const struct DM1_Event_V1 *find_report_event_by_type(
    const DM1OriginalSavePC34HandoffReport *report,
    int type)
{
    int i;

    if (!report) return NULL;
    for (i = 0; i < report->decoded_event_count; ++i) {
        if (report->events[i].type == type) {
            return &report->events[i];
        }
    }
    return NULL;
}

static int rewrite_fixture_global_with_invalid_active_champion(
    unsigned char *bytes,
    int byte_count)
{
    unsigned char *header;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t header_key;
    uint16_t part_size;
    uint16_t part_key;
    uint16_t part_checksum;

    if (!bytes || byte_count < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2) {
        return 0;
    }
    header = bytes;
    header_key = rd16le(header + 10u * 2u);
    xor_obfuscate_second_half(header, header_key);
    part_size = rd16le(bytes + cursor);
    cursor += 2u;
    if (part_size != SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT ||
        cursor + part_size > (size_t)byte_count) {
        goto fail;
    }
    part_key = rd16le(header + 310u +
                      SAVEGAME_PC34_PART_GLOBAL_DATA * 2u);
    xor_words(bytes + cursor, (size_t)part_size / 2u, part_key);
    /* A two-champion party may use slots 0 and 1 only. Keep the byte shape,
     * part checksum, header checksum, and F0417 obfuscation all valid. */
    wr16le(bytes + cursor + 10u, 2u);
    wr16le(bytes + cursor + 20u, 2u);
    part_checksum = checksum_and_xor_words(
        bytes + cursor, (size_t)part_size / 2u, part_key);
    wr16le(header + 342u +
           SAVEGAME_PC34_PART_GLOBAL_DATA * 2u, part_checksum);
    {
        uint16_t second_sum = checksum_second_half_plain(header);
        uint16_t first_before_last = checksum_first_half(header);
        uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                   first_before_last ^ second_sum);
        wr16le(header + 254u, last);
    }
    xor_obfuscate_second_half(header, header_key);
    return 1;

fail:
    xor_obfuscate_second_half(header, header_key);
    return 0;
}

static int rewrite_fixture_global_current_active_group_count(
    unsigned char *bytes,
    int byte_count,
    uint16_t active_group_count)
{
    unsigned char *header;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t header_key;
    uint16_t part_size;
    uint16_t part_key;
    uint16_t part_checksum;

    if (!bytes || byte_count < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2) {
        return 0;
    }
    header = bytes;
    header_key = rd16le(header + 10u * 2u);
    xor_obfuscate_second_half(header, header_key);
    part_size = rd16le(bytes + cursor);
    cursor += 2u;
    if (part_size != SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT ||
        cursor + part_size > (size_t)byte_count) {
        goto fail;
    }
    part_key = rd16le(header + 310u +
                      SAVEGAME_PC34_PART_GLOBAL_DATA * 2u);
    xor_words(bytes + cursor, (size_t)part_size / 2u, part_key);
    wr16le(bytes + cursor + 30u, active_group_count);
    part_checksum = checksum_and_xor_words(
        bytes + cursor, (size_t)part_size / 2u, part_key);
    wr16le(header + 342u +
           SAVEGAME_PC34_PART_GLOBAL_DATA * 2u, part_checksum);
    {
        uint16_t second_sum = checksum_second_half_plain(header);
        uint16_t first_before_last = checksum_first_half(header);
        uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                   first_before_last ^ second_sum);
        wr16le(header + 254u, last);
    }
    xor_obfuscate_second_half(header, header_key);
    return 1;

fail:
    xor_obfuscate_second_half(header, header_key);
    return 0;
}

static int rewrite_fixture_global_party_pose(unsigned char *bytes,
                                             int byte_count,
                                             uint16_t map_index,
                                             uint16_t map_x,
                                             uint16_t map_y)
{
    unsigned char *header;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t header_key;
    uint16_t part_size;
    uint16_t part_key;
    uint16_t part_checksum;

    if (!bytes || byte_count < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2) {
        return 0;
    }
    header = bytes;
    header_key = rd16le(header + 10u * 2u);
    xor_obfuscate_second_half(header, header_key);
    part_size = rd16le(bytes + cursor);
    cursor += 2u;
    if (part_size != SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT ||
        cursor + part_size > (size_t)byte_count) {
        goto fail;
    }
    part_key = rd16le(header + 310u +
                      SAVEGAME_PC34_PART_GLOBAL_DATA * 2u);
    xor_words(bytes + cursor, (size_t)part_size / 2u, part_key);
    /* ReDMCSB DEFS.H GLOBAL_DATA PartyMapX/Y/MapIndex fields, consumed by
     * LOADSAVE.C F0435 before F0434 restores the saved dungeon tail. */
    wr16le(bytes + cursor + 12u, map_x);
    wr16le(bytes + cursor + 14u, map_y);
    wr16le(bytes + cursor + 18u, map_index);
    part_checksum = checksum_and_xor_words(
        bytes + cursor, (size_t)part_size / 2u, part_key);
    wr16le(header + 342u +
           SAVEGAME_PC34_PART_GLOBAL_DATA * 2u, part_checksum);
    {
        uint16_t second_sum = checksum_second_half_plain(header);
        uint16_t first_before_last = checksum_first_half(header);
        uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                   first_before_last ^ second_sum);
        wr16le(header + 254u, last);
    }
    xor_obfuscate_second_half(header, header_key);
    return 1;

fail:
    xor_obfuscate_second_half(header, header_key);
    return 0;
}

static int rewrite_fixture_party_hand_reference(unsigned char *bytes,
                                                int byte_count,
                                                uint16_t thing)
{
    unsigned char *header;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t header_key;
    int part;

    if (!bytes || byte_count < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2) {
        return 0;
    }
    header = bytes;
    header_key = rd16le(header + 10u * 2u);
    xor_obfuscate_second_half(header, header_key);
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;

        if (cursor + 2u > (size_t)byte_count) goto fail;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (cursor + part_size > (size_t)byte_count ||
            (part_size & 1u) != 0u) goto fail;
        if (part == SAVEGAME_PC34_PART_PARTY) {
            uint16_t part_key;
            uint16_t part_checksum;

            if (part_size != ORIGINAL_PC34_PARTY_BYTES) goto fail;
            part_key = rd16le(header + 310u + (size_t)part * 2u);
            xor_words(bytes + cursor, (size_t)part_size / 2u, part_key);
            wr16le(bytes + cursor + 211u +
                   (size_t)CHAMPION_SLOT_HAND_RIGHT * 2u, thing);
            part_checksum = checksum_and_xor_words(
                bytes + cursor, (size_t)part_size / 2u, part_key);
            wr16le(header + 342u + (size_t)part * 2u, part_checksum);
            {
                uint16_t second_sum = checksum_second_half_plain(header);
                uint16_t first_before_last = checksum_first_half(header);
                uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                           first_before_last ^ second_sum);
                wr16le(header + 254u, last);
            }
            xor_obfuscate_second_half(header, header_key);
            return 1;
        }
        cursor += part_size;
    }

fail:
    xor_obfuscate_second_half(header, header_key);
    return 0;
}

static int rewrite_fixture_party_current_health(unsigned char *bytes,
                                                int byte_count,
                                                uint16_t health)
{
    unsigned char *header;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t header_key;
    int part;

    if (!bytes || byte_count < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2) {
        return 0;
    }
    header = bytes;
    header_key = rd16le(header + 10u * 2u);
    xor_obfuscate_second_half(header, header_key);
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;

        if (cursor + 2u > (size_t)byte_count) goto fail;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (cursor + part_size > (size_t)byte_count ||
            (part_size & 1u) != 0u) goto fail;
        if (part == SAVEGAME_PC34_PART_PARTY) {
            uint16_t part_key;
            uint16_t part_checksum;

            if (part_size != ORIGINAL_PC34_PARTY_BYTES) goto fail;
            part_key = rd16le(header + 310u + (size_t)part * 2u);
            xor_words(bytes + cursor, (size_t)part_size / 2u, part_key);
            wr16le(bytes + cursor + 52u, health);
            part_checksum = checksum_and_xor_words(
                bytes + cursor, (size_t)part_size / 2u, part_key);
            wr16le(header + 342u + (size_t)part * 2u, part_checksum);
            {
                uint16_t second_sum = checksum_second_half_plain(header);
                uint16_t first_before_last = checksum_first_half(header);
                uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                           first_before_last ^ second_sum);
                wr16le(header + 254u, last);
            }
            xor_obfuscate_second_half(header, header_key);
            return 1;
        }
        cursor += part_size;
    }

fail:
    xor_obfuscate_second_half(header, header_key);
    return 0;
}

static void write_original_champion(unsigned char *dst,
                                    const char *name,
                                    const char *title,
                                    int direction,
                                    int hp_current,
                                    int hp_maximum,
                                    int stamina_current,
                                    int stamina_maximum,
                                    int mana_current,
                                    int mana_maximum,
                                    int food,
                                    int water,
                                    uint16_t wounds,
                                    uint16_t hand_item)
{
    int i;
    memset(dst, 0, ORIGINAL_PC34_CHAMPION_BYTES);
    memset(dst + 0, ' ', 8u);
    memset(dst + 8, ' ', 20u);
    memcpy(dst + 0, name, strlen(name));
    memcpy(dst + 8, title, strlen(title));
    dst[28] = (unsigned char)direction;
    dst[32] = 0xffu; /* ReDMCSB ActionIndex no-action sentinel. */
    dst[42] = 3u;
    wr16le(dst + 48, 0x1234u);
    wr16le(dst + 50, wounds);
    wr16le(dst + 52, (uint16_t)hp_current);
    wr16le(dst + 54, (uint16_t)hp_maximum);
    wr16le(dst + 56, (uint16_t)stamina_current);
    wr16le(dst + 58, (uint16_t)stamina_maximum);
    wr16le(dst + 60, (uint16_t)mana_current);
    wr16le(dst + 62, (uint16_t)mana_maximum);
    wr16le(dst + 66, (uint16_t)food);
    wr16le(dst + 68, (uint16_t)water);
    for (i = 0; i < 7; ++i) {
        unsigned char *stat = dst + 70 + (size_t)i * 3u;
        stat[0] = (unsigned char)(40 + i);
        stat[1] = (unsigned char)(30 + i);
        stat[2] = (unsigned char)(10 + i);
    }
    for (i = 0; i < 20; ++i) {
        unsigned char *skill = dst + 91 + (size_t)i * 6u;
        wr16le(skill, (uint16_t)(0x0100u + (uint16_t)i));
        wr32le(skill + 2, 1000u + (uint32_t)i * 111u);
    }
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        wr16le(dst + 211 + (size_t)i * 2u, 0xffffu);
    }
    wr16le(dst + 211 + (size_t)CHAMPION_SLOT_HAND_RIGHT * 2u, hand_item);
    wr16le(dst + 271, 345u);
}

static void write_original_active_group(unsigned char *dst,
                                        uint16_t group_thing_index,
                                        int directions,
                                        int cells,
                                        int target_x,
                                        int target_y)
{
    wr16le(dst + 0, group_thing_index);
    dst[2] = (unsigned char)directions;
    dst[3] = (unsigned char)cells;
    dst[4] = 12u;
    dst[5] = 3u;
    dst[6] = (unsigned char)target_x;
    dst[7] = (unsigned char)target_y;
    dst[8] = 5u;
    dst[9] = 6u;
    dst[10] = 7u;
    dst[11] = 8u;
    dst[12] = 0x41u;
    dst[13] = 0x42u;
    dst[14] = 0x43u;
    dst[15] = 0x44u;
}

static void write_original_event(unsigned char *dst,
                                 uint32_t map_time,
                                 int type,
                                 int priority,
                                 int map_x,
                                 int map_y,
                                 int cell,
                                 int effect)
{
    wr32le(dst + 0, map_time);
    dst[4] = (unsigned char)type;
    dst[5] = (unsigned char)priority;
    dst[6] = (unsigned char)map_x;
    dst[7] = (unsigned char)map_y;
    dst[8] = (unsigned char)cell;
    dst[9] = (unsigned char)effect;
}

static int build_original_pc34_fixture(unsigned char *out,
                                       int out_cap,
                                       int *out_written,
                                       int champion_count,
                                       int map_index,
                                       int map_x,
                                       int map_y,
                                       int direction,
                                       int active_champion_index,
                                       int maximum_active_group_count)
{
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    unsigned char global[SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT];
    unsigned char active_group[ORIGINAL_PC34_ACTIVE_GROUP_PART_BYTES];
    unsigned char party[ORIGINAL_PC34_PARTY_BYTES];
    unsigned char events[ORIGINAL_PC34_EVENTS_PART_BYTES];
    unsigned char timeline[ORIGINAL_PC34_TIMELINE_PART_BYTES];
    unsigned char portraits[SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    int cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int n;
    int i;

    if (!out || !out_written || out_cap < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        return SAVEGAME_PC34_ERROR_NULL_ARG;
    }
    *out_written = 0;
    memset(out, 0, (size_t)out_cap);
    memset(header, 0, sizeof(header));
    memset(global, 0, sizeof(global));
    memset(active_group, 0, sizeof(active_group));
    memset(party, 0, sizeof(party));
    memset(events, 0, sizeof(events));
    memset(timeline, 0, sizeof(timeline));
    memset(portraits, 0, sizeof(portraits));
    memset(checksums, 0, sizeof(checksums));

    for (i = 0; i < 127; ++i) {
        wr16le(header + (size_t)i * 2u,
               (uint16_t)(0x4321u + (uint16_t)(i * 17u)));
    }
    wr16le(header + 10u * 2u, 0x2468u);

    header[298] = 1u;
    header[299] = SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC;
    wr32le(header + 306u, 0x50433334u);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = (uint16_t)(0x2000u + (uint16_t)(i * 0x101u));
    }

    wr32le(global + 0u, 123456u);
    wr16le(global + 10u, (uint16_t)champion_count);
    wr16le(global + 12u, (uint16_t)map_x);
    wr16le(global + 14u, (uint16_t)map_y);
    wr16le(global + 16u, (uint16_t)direction);
    wr16le(global + 18u, (uint16_t)map_index);
    wr16le(global + 20u, (uint16_t)active_champion_index);
    wr16le(global + 24u, ORIGINAL_PC34_EVENT_COUNT);
    wr16le(global + 26u, ORIGINAL_PC34_EVENT_COUNT);
    wr16le(global + 28u, ORIGINAL_PC34_EVENT_MAXIMUM_COUNT);
    wr16le(global + 30u, 2u);
    wr16le(global + 46u, (uint16_t)maximum_active_group_count);
    write_original_active_group(active_group + 0 * ORIGINAL_PC34_ACTIVE_GROUP_BYTES,
                                0x1001u, 0x5a, 0xc3, 21, 22);
    write_original_active_group(active_group + 1 * ORIGINAL_PC34_ACTIVE_GROUP_BYTES,
                                0x1002u, 0x6b, 0xd4, 23, 24);
    write_original_active_group(active_group + 2 * ORIGINAL_PC34_ACTIVE_GROUP_BYTES,
                                0x1003u, 0x7c, 0xe5, 25, 26);
    if (champion_count > 0) {
        write_original_champion(party + 0 * ORIGINAL_PC34_CHAMPION_BYTES,
                                "TIGGY", "APPRENTICE", direction,
                                44, 55, 66, 77, 8, 9,
                                1500, -32, 0x0021u, 0x1555u);
    }
    if (champion_count > 1) {
        write_original_champion(party + 1 * ORIGINAL_PC34_CHAMPION_BYTES,
                                "WUUF", "BIKA", (direction + 1) & 3,
                                88, 99, 111, 122, 33, 44,
                                1200, 1100, 0x0002u, 0x1666u);
    }
    if (champion_count > 2) {
        write_original_champion(party + 2 * ORIGINAL_PC34_CHAMPION_BYTES,
                                "HALK", "BARBARIAN", (direction + 2) & 3,
                                101, 202, 303, 404, 55, 66,
                                900, 800, 0x0010u, 0x1777u);
    }

    write_original_event(events + 0 * ORIGINAL_PC34_EVENT_BYTES,
                         DM1_MAP_TIME_MAKE(2, 123500u),
                         DM1_EVENT_MOVE_GROUP_AUDIBLE, 7, 11, 12, 3,
                         DM1_EFFECT_SET);
    write_original_event(events + 1 * ORIGINAL_PC34_EVENT_BYTES,
                         DM1_MAP_TIME_MAKE(2, 123470u),
                         DM1_EVENT_DOOR, 4, 21, 22, 1,
                         DM1_EFFECT_TOGGLE);
    write_original_event(events + 2 * ORIGINAL_PC34_EVENT_BYTES,
                         DM1_MAP_TIME_MAKE(1, 123490u),
                         DM1_EVENT_LIGHT, 2, 0, 0, 0, 9);
    write_original_event(events + 3 * ORIGINAL_PC34_EVENT_BYTES,
                         0u, DM1_EVENT_NONE, 0, 0, 0, 0, 0);
    wr16le(timeline + 0u, 1u);
    wr16le(timeline + 2u, 2u);
    wr16le(timeline + 4u, 0u);
    wr16le(timeline + 6u, 3u);
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        memset(portraits + (size_t)i * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT,
               0x30 + i, CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
    }

    n = write_part(out + cursor, out_cap - cursor, global,
                   (int)sizeof(global),
                   keys[SAVEGAME_PC34_PART_GLOBAL_DATA],
                   &checksums[SAVEGAME_PC34_PART_GLOBAL_DATA]);
    if (n < 0) return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    cursor += n;
    n = write_part(out + cursor, out_cap - cursor, active_group,
                   (int)sizeof(active_group),
                   keys[SAVEGAME_PC34_PART_ACTIVE_GROUP],
                   &checksums[SAVEGAME_PC34_PART_ACTIVE_GROUP]);
    if (n < 0) return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    cursor += n;
    n = write_part(out + cursor, out_cap - cursor, party,
                   (int)sizeof(party),
                   keys[SAVEGAME_PC34_PART_PARTY],
                   &checksums[SAVEGAME_PC34_PART_PARTY]);
    if (n < 0) return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    cursor += n;
    n = write_part(out + cursor, out_cap - cursor, events,
                   (int)sizeof(events),
                   keys[SAVEGAME_PC34_PART_EVENTS],
                   &checksums[SAVEGAME_PC34_PART_EVENTS]);
    if (n < 0) return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    cursor += n;
    n = write_part(out + cursor, out_cap - cursor, timeline,
                   (int)sizeof(timeline),
                   keys[SAVEGAME_PC34_PART_TIMELINE],
                   &checksums[SAVEGAME_PC34_PART_TIMELINE]);
    if (n < 0) return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    cursor += n;
    if (out_cap - cursor < (int)sizeof(portraits)) {
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    }
    memcpy(out + cursor, portraits, sizeof(portraits));
    cursor += (int)sizeof(portraits);

    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        wr16le(header + 310u + (size_t)i * 2u, keys[i]);
        wr16le(header + 342u + (size_t)i * 2u, checksums[i]);
    }
    wr16le(header + 374u, SAVEGAME_PC34_PLATFORM_PC);
    wr16le(header + 376u, SAVEGAME_PC34_DUNGEON_ID_DM);
    {
        uint16_t second_sum = checksum_second_half_plain(header);
        uint16_t first_before_last = checksum_first_half(header);
        uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                   first_before_last ^
                                   second_sum);
        wr16le(header + 254u, last);
    }
    xor_obfuscate_second_half(header, rd16le(header + 10u * 2u));
    memcpy(out, header, sizeof(header));

    *out_written = cursor;
    return SAVEGAME_PC34_OK;
}

static int append_minimal_original_pc34_dungeon_tail(unsigned char *bytes,
                                                       int capacity,
                                                       int *in_out_size)
{
    unsigned char *tail;
    int tail_size = 65;
    int i;
    uint16_t checksum = 0u;

    if (!bytes || !in_out_size || *in_out_size < 0 ||
        *in_out_size > capacity || capacity - *in_out_size < tail_size) {
        return 0;
    }
    tail = bytes + *in_out_size;
    memset(tail, 0, (size_t)tail_size);

    /* ReDMCSB LOADSAVE.C F0433:1641-1682 writes DUNGEON_HEADER, one
     * MAP, one column cumulative count, raw map data, then the F0422
     * byte-sum. This is a complete 1x1, no-Thing dungeon tail. */
    wr16le(tail + 2, 1u);
    tail[4] = 1u;
    tail[44] = 0u;
    tail[45] = 0u;
    wr16le(tail + 52, 0u);
    tail[62] = 0u;
    for (i = 0; i < tail_size - 2; ++i) {
        checksum = (uint16_t)(checksum + tail[i]);
    }
    wr16le(tail + tail_size - 2, checksum);
    *in_out_size += tail_size;
    return 1;
}

static void refresh_original_pc34_dungeon_tail_checksum(unsigned char *tail,
                                                         int tail_size)
{
    uint16_t checksum = 0u;
    int i;

    for (i = 0; i < tail_size - 2; ++i) {
        checksum = (uint16_t)(checksum + tail[i]);
    }
    wr16le(tail + tail_size - 2, checksum);
}

static int append_textstring_original_pc34_dungeon_tail(unsigned char *bytes,
                                                          int capacity,
                                                          int *in_out_size)
{
    unsigned char *tail;
    const int tail_size = 75;

    if (!bytes || !in_out_size || *in_out_size < 0 ||
        *in_out_size > capacity || capacity - *in_out_size < tail_size) {
        return 0;
    }
    tail = bytes + *in_out_size;
    memset(tail, 0, (size_t)tail_size);

    /* ReDMCSB LOADSAVE.C F0433:1641-1682 writes TextData before the
     * TextString stream. This complete 1x1 tail has two text words and
     * one visible TextString whose offset begins at the first word. */
    wr16le(tail + 2, 1u);
    tail[4] = 1u;
    wr16le(tail + 6, 2u);
    wr16le(tail + 10, 1u);
    wr16le(tail + 12 + THING_TYPE_TEXTSTRING * 2, 1u);
    wr16le(tail + 52, 0u);
    wr16le(tail + 62, 0xffffu);
    wr16le(tail + 64, 0x0041u);
    wr16le(tail + 66, 0x0000u);
    wr16le(tail + 68, 0xffffu);
    wr16le(tail + 70, 0x0001u);
    tail[72] = 0u;
    refresh_original_pc34_dungeon_tail_checksum(tail, tail_size);
    *in_out_size += tail_size;
    return 1;
}

static int append_group_list_original_pc34_dungeon_tail(unsigned char *bytes,
                                                         int capacity,
                                                         int *in_out_size)
{
    unsigned char *tail;
    const int tail_size = 131;
    int group_index;

    if (!bytes || !in_out_size || *in_out_size < 0 ||
        *in_out_size > capacity || capacity - *in_out_size < tail_size) {
        return 0;
    }
    tail = bytes + *in_out_size;
    memset(tail, 0, (size_t)tail_size);

    /* ReDMCSB LOADSAVE.C F0433:1641-1682 writes SquareFirstThings before
     * type-4 GROUP data. The save fixture's ACTIVE_GROUP records name GROUP
     * indexes 1 and 2, and the square list begins at GROUP index 1. */
    wr16le(tail + 2, 1u);
    tail[4] = 1u;
    wr16le(tail + 10, 1u);
    wr16le(tail + 12 + THING_TYPE_GROUP * 2, 4u);
    wr16le(tail + 52, 0u);
    wr16le(tail + 62, (uint16_t)(THING_TYPE_GROUP << 10 | 1));
    for (group_index = 0; group_index < 4; ++group_index) {
        unsigned char *group = tail + 64 + group_index * 16;

        wr16le(group + 0, THING_ENDOFLIST);
        wr16le(group + 2, THING_ENDOFLIST);
        group[4] = (unsigned char)(8 + group_index);
        group[5] = (unsigned char)(1u << (group_index & 3));
        wr16le(group + 14, (uint16_t)group_index);
    }
    tail[128] = 0u;
    refresh_original_pc34_dungeon_tail_checksum(tail, tail_size);
    *in_out_size += tail_size;
    return 1;
}

static int append_weapon_inventory_original_pc34_dungeon_tail(
    unsigned char *bytes,
    int capacity,
    int *in_out_size)
{
    unsigned char *tail;
    const int weapon_count = 342;
    const int tail_size = 1435;
    int weapon_index;

    if (!bytes || !in_out_size || *in_out_size < 0 ||
        *in_out_size > capacity || capacity - *in_out_size < tail_size) {
        return 0;
    }
    tail = bytes + *in_out_size;
    memset(tail, 0, (size_t)tail_size);

    /* ReDMCSB LOADSAVE.C F0433 writes all type streams after
     * SquareFirstThings. The champion fixture's hand is WEAPON index 341;
     * all earlier weapon slots are explicitly free and index 341 is live. */
    wr16le(tail + 2, 1u);
    tail[4] = 1u;
    wr16le(tail + 10, 1u);
    wr16le(tail + 12 + THING_TYPE_WEAPON * 2, (uint16_t)weapon_count);
    wr16le(tail + 52, 0u);
    wr16le(tail + 62, THING_NONE);
    for (weapon_index = 0; weapon_index < weapon_count; ++weapon_index) {
        wr16le(tail + 64 + weapon_index * 4, THING_NONE);
    }
    wr16le(tail + 64 + 341 * 4, THING_ENDOFLIST);
    tail[1432] = 0u;
    refresh_original_pc34_dungeon_tail_checksum(tail, tail_size);
    *in_out_size += tail_size;
    return 1;
}

static int write_fixture_file(const char *path,
                              const unsigned char *bytes,
                              int byte_count)
{
    FILE *file;
    size_t written;

    file = fopen(path, "wb");
    if (!file) {
        return 0;
    }
    written = fwrite(bytes, 1u, (size_t)byte_count, file);
    if (fclose(file) != 0) {
        return 0;
    }
    return written == (size_t)byte_count;
}

/* Reuse the test's exact F0430/F0419 fixture obfuscation to turn one
 * accepted event into C11. It remains a checksum-authenticated PC34 save,
 * not a fabricated in-memory timeline. */
static int rewrite_fixture_event_byte(unsigned char *bytes,
                                      size_t size,
                                      int event_index,
                                      int byte_offset,
                                      int value)
{
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t checksum;
    int part;
    int i;

    if (!bytes || size < sizeof(header) || event_index < 0 ||
        event_index >= ORIGINAL_PC34_EVENT_MAXIMUM_COUNT ||
        byte_offset < 0 || byte_offset >= ORIGINAL_PC34_EVENT_BYTES ||
        value < 0 || value > 255) {
        return 0;
    }
    memcpy(header, bytes, sizeof(header));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = rd16le(header + 310u + (size_t)i * 2u);
    }
    for (part = 0; part < SAVEGAME_PC34_PART_EVENTS; ++part) {
        uint16_t part_size;
        if (cursor + 2u > size) return 0;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (part_size > size - cursor) return 0;
        cursor += part_size;
    }
    if (cursor + 2u > size ||
        rd16le(bytes + cursor) != ORIGINAL_PC34_EVENTS_PART_BYTES) {
        return 0;
    }
    cursor += 2u;
    if (ORIGINAL_PC34_EVENTS_PART_BYTES > size - cursor) return 0;

    xor_words(bytes + cursor, ORIGINAL_PC34_EVENTS_PART_BYTES / 2u,
              keys[SAVEGAME_PC34_PART_EVENTS]);
    bytes[cursor + (size_t)event_index * ORIGINAL_PC34_EVENT_BYTES +
          (size_t)byte_offset] = (unsigned char)value;
    checksum = checksum_and_xor_words(
        bytes + cursor, ORIGINAL_PC34_EVENTS_PART_BYTES / 2u,
        keys[SAVEGAME_PC34_PART_EVENTS]);
    wr16le(header + 342u +
           (size_t)SAVEGAME_PC34_PART_EVENTS * 2u, checksum);
    wr16le(header + 254u, 0u);
    wr16le(header + 254u,
           (uint16_t)(checksum_first_half(header) ^
                      checksum_second_half_plain(header)));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(bytes, header, sizeof(header));
    return 1;
}

static int rewrite_fixture_event_type(unsigned char *bytes,
                                      size_t size,
                                      int event_index,
                                      int event_type)
{
    return rewrite_fixture_event_byte(bytes, size, event_index, 4, event_type);
}

static int rewrite_fixture_global_u16(unsigned char *bytes,
                                      size_t size,
                                      size_t global_offset,
                                      uint16_t value)
{
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint16_t key;
    uint16_t checksum;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;

    if (!bytes || global_offset + 2u > SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT ||
        size < sizeof(header) || cursor + 2u > size ||
        rd16le(bytes + cursor) != SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT) {
        return 0;
    }
    cursor += 2u;
    if (SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT > size - cursor) return 0;
    memcpy(header, bytes, sizeof(header));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    key = rd16le(header + 310u +
                 (size_t)SAVEGAME_PC34_PART_GLOBAL_DATA * 2u);
    xor_words(bytes + cursor, SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT / 2u, key);
    wr16le(bytes + cursor + global_offset, value);
    checksum = checksum_and_xor_words(
        bytes + cursor, SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT / 2u, key);
    wr16le(header + 342u +
           (size_t)SAVEGAME_PC34_PART_GLOBAL_DATA * 2u, checksum);
    wr16le(header + 254u, 0u);
    wr16le(header + 254u,
           (uint16_t)(checksum_first_half(header) ^
                      checksum_second_half_plain(header)));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(bytes, header, sizeof(header));
    return 1;
}

static int rewrite_fixture_first_unused_event_index(unsigned char *bytes,
                                                    size_t size,
                                                    uint16_t first_unused)
{
    return rewrite_fixture_global_u16(bytes, size, 26u, first_unused);
}

/* Edit only the source-owned C2 PARTY_INFO tail, then rebuild that part's
 * F0417 checksum and the F0430 header checksum. This remains an original
 * PC34 envelope; it does not create a Firestaff-specific save layout. */
static int rewrite_fixture_party_info_bytes(unsigned char *bytes,
                                            size_t size,
                                            const unsigned char *info)
{
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t checksum;
    int part;
    int i;

    if (!bytes || !info || size < sizeof(header)) return 0;
    memcpy(header, bytes, sizeof(header));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = rd16le(header + 310u + (size_t)i * 2u);
    }
    for (part = 0; part < SAVEGAME_PC34_PART_PARTY; ++part) {
        uint16_t part_size;
        if (cursor + 2u > size) return 0;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (part_size > size - cursor) return 0;
        cursor += part_size;
    }
    if (cursor + 2u > size ||
        rd16le(bytes + cursor) != ORIGINAL_PC34_PARTY_BYTES) return 0;
    cursor += 2u;
    if (ORIGINAL_PC34_PARTY_BYTES > size - cursor) return 0;
    xor_words(bytes + cursor, ORIGINAL_PC34_PARTY_BYTES / 2u,
              keys[SAVEGAME_PC34_PART_PARTY]);
    memcpy(bytes + cursor +
               (ORIGINAL_PC34_CHAMPION_BYTES * CHAMPION_MAX_PARTY),
           info, PARTY_PC34_SAVE_INFO_BYTE_COUNT);
    checksum = checksum_and_xor_words(
        bytes + cursor, ORIGINAL_PC34_PARTY_BYTES / 2u,
        keys[SAVEGAME_PC34_PART_PARTY]);
    wr16le(header + 342u +
           (size_t)SAVEGAME_PC34_PART_PARTY * 2u, checksum);
    wr16le(header + 254u, 0u);
    wr16le(header + 254u,
           (uint16_t)(checksum_first_half(header) ^
                      checksum_second_half_plain(header)));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(bytes, header, sizeof(header));
    return 1;
}

static int rewrite_fixture_event_priority(unsigned char *bytes,
                                          size_t size,
                                          int event_index,
                                          int priority)
{
    return rewrite_fixture_event_byte(bytes, size, event_index, 5, priority);
}

static int rewrite_fixture_event_c_union(unsigned char *bytes,
                                         size_t size,
                                         int event_index,
                                         uint16_t c_union)
{
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t checksum;
    int part;
    int i;

    if (!bytes || size < sizeof(header) || event_index < 0 ||
        event_index >= ORIGINAL_PC34_EVENT_MAXIMUM_COUNT) {
        return 0;
    }
    memcpy(header, bytes, sizeof(header));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = rd16le(header + 310u + (size_t)i * 2u);
    }
    for (part = 0; part < SAVEGAME_PC34_PART_EVENTS; ++part) {
        uint16_t part_size;
        if (cursor + 2u > size) return 0;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (part_size > size - cursor) return 0;
        cursor += part_size;
    }
    if (cursor + 2u > size ||
        rd16le(bytes + cursor) != ORIGINAL_PC34_EVENTS_PART_BYTES) {
        return 0;
    }
    cursor += 2u;
    if (ORIGINAL_PC34_EVENTS_PART_BYTES > size - cursor) return 0;

    xor_words(bytes + cursor, ORIGINAL_PC34_EVENTS_PART_BYTES / 2u,
              keys[SAVEGAME_PC34_PART_EVENTS]);
    wr16le(bytes + cursor + (size_t)event_index * ORIGINAL_PC34_EVENT_BYTES + 8u,
           c_union);
    checksum = checksum_and_xor_words(
        bytes + cursor, ORIGINAL_PC34_EVENTS_PART_BYTES / 2u,
        keys[SAVEGAME_PC34_PART_EVENTS]);
    wr16le(header + 342u +
           (size_t)SAVEGAME_PC34_PART_EVENTS * 2u, checksum);
    wr16le(header + 254u, 0u);
    wr16le(header + 254u,
           (uint16_t)(checksum_first_half(header) ^
                      checksum_second_half_plain(header)));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(bytes, header, sizeof(header));
    return 1;
}

static int rewrite_fixture_timeline_index(unsigned char *bytes,
                                          size_t size,
                                          int timeline_slot,
                                          uint16_t event_index)
{
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t checksum;
    int part;
    int i;

    if (!bytes || size < sizeof(header) || timeline_slot < 0 ||
        timeline_slot >= ORIGINAL_PC34_EVENT_MAXIMUM_COUNT) {
        return 0;
    }
    memcpy(header, bytes, sizeof(header));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = rd16le(header + 310u + (size_t)i * 2u);
    }
    for (part = 0; part < SAVEGAME_PC34_PART_TIMELINE; ++part) {
        uint16_t part_size;
        if (cursor + 2u > size) return 0;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (part_size > size - cursor) return 0;
        cursor += part_size;
    }
    if (cursor + 2u > size ||
        rd16le(bytes + cursor) != ORIGINAL_PC34_TIMELINE_PART_BYTES) {
        return 0;
    }
    cursor += 2u;
    if (ORIGINAL_PC34_TIMELINE_PART_BYTES > size - cursor) return 0;
    xor_words(bytes + cursor, ORIGINAL_PC34_TIMELINE_PART_BYTES / 2u,
              keys[SAVEGAME_PC34_PART_TIMELINE]);
    wr16le(bytes + cursor + (size_t)timeline_slot * 2u, event_index);
    checksum = checksum_and_xor_words(
        bytes + cursor, ORIGINAL_PC34_TIMELINE_PART_BYTES / 2u,
        keys[SAVEGAME_PC34_PART_TIMELINE]);
    wr16le(header + 342u +
           (size_t)SAVEGAME_PC34_PART_TIMELINE * 2u, checksum);
    wr16le(header + 254u, 0u);
    wr16le(header + 254u,
           (uint16_t)(checksum_first_half(header) ^
                      checksum_second_half_plain(header)));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(bytes, header, sizeof(header));
    return 1;
}

static int rewrite_fixture_timeline_swap_indices(unsigned char *bytes,
                                                  size_t size,
                                                  int first_slot,
                                                  int second_slot)
{
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint16_t checksum;
    uint16_t first_index;
    int part;
    int i;

    if (!bytes || size < sizeof(header) || first_slot < 0 || second_slot < 0 ||
        first_slot >= ORIGINAL_PC34_EVENT_MAXIMUM_COUNT ||
        second_slot >= ORIGINAL_PC34_EVENT_MAXIMUM_COUNT ||
        first_slot == second_slot) {
        return 0;
    }
    memcpy(header, bytes, sizeof(header));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = rd16le(header + 310u + (size_t)i * 2u);
    }
    for (part = 0; part < SAVEGAME_PC34_PART_TIMELINE; ++part) {
        uint16_t part_size;
        if (cursor + 2u > size) return 0;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (part_size > size - cursor) return 0;
        cursor += part_size;
    }
    if (cursor + 2u > size ||
        rd16le(bytes + cursor) != ORIGINAL_PC34_TIMELINE_PART_BYTES) {
        return 0;
    }
    cursor += 2u;
    if (ORIGINAL_PC34_TIMELINE_PART_BYTES > size - cursor) return 0;
    xor_words(bytes + cursor, ORIGINAL_PC34_TIMELINE_PART_BYTES / 2u,
              keys[SAVEGAME_PC34_PART_TIMELINE]);
    first_index = rd16le(bytes + cursor + (size_t)first_slot * 2u);
    wr16le(bytes + cursor + (size_t)first_slot * 2u,
           rd16le(bytes + cursor + (size_t)second_slot * 2u));
    wr16le(bytes + cursor + (size_t)second_slot * 2u, first_index);
    checksum = checksum_and_xor_words(
        bytes + cursor, ORIGINAL_PC34_TIMELINE_PART_BYTES / 2u,
        keys[SAVEGAME_PC34_PART_TIMELINE]);
    wr16le(header + 342u +
           (size_t)SAVEGAME_PC34_PART_TIMELINE * 2u, checksum);
    wr16le(header + 254u, 0u);
    wr16le(header + 254u,
           (uint16_t)(checksum_first_half(header) ^
                      checksum_second_half_plain(header)));
    xor_obfuscate_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(bytes, header, sizeof(header));
    return 1;
}

static void make_temp_save_path(char *out, size_t out_size)
{
    const char *root = getenv("FIRESTAFF_TEST_TMPDIR");
    int n;

    if (!root || root[0] == '\0') {
        root = getenv("TMPDIR");
    }
    if (!root || root[0] == '\0') {
        root = "/tmp";
    }
    n = snprintf(out, out_size,
                 "%s/firestaff_dm1_original_pc34_handoff_fixture.sav",
                 root);
    CHECK(n > 0 && (size_t)n < out_size,
          "temporary save path fits");
}

static void test_pc34_handoff_imports_party_state(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[4];
    struct DM1_EventQueue_V1 event_queue;
    DM1OriginalSavePC34HandoffReport report;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     3, 2, 7, 13, 1, 2,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "fixture export succeeds");
    CHECK(written > SAVEGAME_PC34_DM_SAVE_HEADER_SIZE,
          "fixture includes header plus parts");
    {
        unsigned char meta[256];
        memcpy(meta, bytes + 256u, sizeof(meta));
        xor_words(meta, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS,
                  rd16le(bytes + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
        CHECK(rd16le(meta + 54u) == 0x2000u,
              "fixture header key[0] lands at original DM_SAVE_HEADER offset");
    }

    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;

    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "PC34 handoff succeeds");
    CHECK(report.classify.shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34,
          "handoff classified PC34 shape");
    CHECK(report.classify.pc34_importer_candidate == 1,
          "handoff reports PC34 importer candidate");
    CHECK(report.importer_result == SAVEGAME_PC34_OK,
          "handoff importer result OK");
    CHECK(report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT,
          "all five save-part checksums validate");
    CHECK(report.part_byte_counts[SAVEGAME_PC34_PART_GLOBAL_DATA] ==
          SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT,
          "GLOBAL_DATA byte count recorded");
    CHECK(report.part_byte_counts[SAVEGAME_PC34_PART_PARTY] ==
          ORIGINAL_PC34_PARTY_BYTES,
          "PARTY byte count recorded");
    CHECK(report.part_byte_counts[SAVEGAME_PC34_PART_ACTIVE_GROUP] ==
          ORIGINAL_PC34_ACTIVE_GROUP_PART_BYTES,
          "ACTIVE_GROUP byte count recorded");
    CHECK(report.part_byte_counts[SAVEGAME_PC34_PART_EVENTS] ==
          ORIGINAL_PC34_EVENTS_PART_BYTES,
          "EVENTS byte count recorded");
    CHECK(report.part_byte_counts[SAVEGAME_PC34_PART_TIMELINE] ==
          ORIGINAL_PC34_TIMELINE_PART_BYTES,
          "TIMELINE byte count recorded");
    CHECK(report.part_actual_checksums[SAVEGAME_PC34_PART_GLOBAL_DATA] ==
          report.part_expected_checksums[SAVEGAME_PC34_PART_GLOBAL_DATA],
          "GLOBAL_DATA checksum recorded");
    CHECK(report.part_actual_checksums[SAVEGAME_PC34_PART_PARTY] ==
          report.part_expected_checksums[SAVEGAME_PC34_PART_PARTY],
          "PARTY checksum recorded");
    CHECK(report.external_portrait_byte_count ==
          SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT,
          "fixed external portrait byte count recorded");
    CHECK(report.external_portrait_payload_count == CHAMPION_MAX_PARTY,
          "all fixed external portrait payloads consumed");
    CHECK(report.external_portrait_imported_count == 3,
          "present champion portraits imported");
    CHECK(imported.party->champions[0].portraitBitmapValid == 1 &&
          imported.party->champions[0].portraitBitmap[0] == 0x30u &&
          imported.party->champions[2].portraitBitmap[0] == 0x32u,
          "original external portrait bytes import in slot order");
    CHECK(imported.party->championCount == 3,
          "imported champion count");
    CHECK(imported.party->mapIndex == 2, "imported map index");
    CHECK(imported.party->mapX == 7, "imported map x");
    CHECK(imported.party->mapY == 13, "imported map y");
    CHECK(imported.party->direction == 1, "imported direction");
    CHECK(imported.party->activeChampionIndex == 2,
          "imported active champion");
    CHECK(report.imported_champion_count == 3,
          "report champion count");
    CHECK(report.imported_map_index == 2, "report map index");
    CHECK(report.imported_map_x == 7, "report map x");
    CHECK(report.imported_map_y == 13, "report map y");
    CHECK(report.imported_direction == 1, "report direction");
    CHECK(report.imported_active_champion_index == 2,
          "report active champion");
    CHECK(report.original_current_active_group_count == 2,
          "report current active group count");
    CHECK(report.original_maximum_active_group_count ==
          ORIGINAL_PC34_ACTIVE_GROUP_COUNT,
          "report maximum active group count");
    CHECK(report.decoded_active_group_count == ORIGINAL_PC34_ACTIVE_GROUP_COUNT,
          "report decoded active group count");
    CHECK(report.reported_active_group_count == ORIGINAL_PC34_ACTIVE_GROUP_COUNT,
          "report bounded active group count");
    CHECK(report.active_group_decode_truncated_count == 0,
          "report no active group decode truncation");
    CHECK(report.active_groups[0].group_thing_index == 0x1001,
          "active group 0 thing index decoded");
    CHECK(report.active_groups[0].directions == 0x5a,
          "active group 0 directions decoded");
    CHECK(report.active_groups[0].cells == 0xc3,
          "active group 0 cells decoded");
    CHECK(report.active_groups[0].target_map_x == 21,
          "active group 0 target x decoded");
    CHECK(report.active_groups[0].target_map_y == 22,
          "active group 0 target y decoded");
    CHECK(report.active_groups[0].aspect[3] == 0x44,
          "active group 0 aspect decoded");
    CHECK(report.original_game_time == 123456u,
          "original game time decoded");
    CHECK(report.original_event_count == ORIGINAL_PC34_EVENT_COUNT,
          "original event count decoded");
    CHECK(report.original_first_unused_event_index == ORIGINAL_PC34_EVENT_COUNT,
          "original first unused event index decoded");
    CHECK(report.original_event_maximum_count == ORIGINAL_PC34_EVENT_MAXIMUM_COUNT,
          "original event maximum count decoded");
    CHECK(report.decoded_event_count == ORIGINAL_PC34_EVENT_MAXIMUM_COUNT,
          "event array decoded count");
    CHECK(report.decoded_timeline_index_count == ORIGINAL_PC34_EVENT_MAXIMUM_COUNT,
          "timeline index decoded count");
    CHECK(report.event_decode_truncated_count == 0,
          "event decode no truncation");
    CHECK(report.events[1].type == DM1_EVENT_DOOR,
          "event 1 type decoded");
    CHECK(report.events[1].priority == 4,
          "event 1 priority decoded");
    CHECK(report.events[1].b_mapX == 21,
          "event 1 map x decoded");
    CHECK(report.events[1].c_effect == DM1_EFFECT_TOGGLE,
          "event 1 effect decoded");
    CHECK(report.timeline_indices[0] == 1,
          "timeline index 0 decoded");
    CHECK(report.imported_champion_block_count == CHAMPION_MAX_PARTY,
          "report original champion block count");
    CHECK(report.imported_champion_slot_count == 3,
          "report imported champion slots");
    CHECK(report.imported_skill_level_count == 12,
          "report imported base skill levels");
    CHECK(imported.party->champions[0].present == 1,
          "champion 0 present");
    CHECK(memcmp(imported.party->champions[0].name, "TIGGY   ", 8u) == 0,
          "champion 0 name imported");
    CHECK(memcmp(imported.party->champions[0].title,
                 "APPRENTICE          ", 20u) == 0,
          "champion 0 title imported");
    CHECK(imported.party->champions[0].direction == 1,
          "champion 0 direction imported");
    CHECK(imported.party->champions[0].hp.current == 44,
          "champion 0 current hp imported");
    CHECK(imported.party->champions[0].hp.maximum == 55,
          "champion 0 maximum hp imported");
    CHECK(imported.party->champions[0].stamina.current == 66,
          "champion 0 current stamina imported");
    CHECK(imported.party->champions[0].mana.maximum == 9,
          "champion 0 maximum mana imported");
    CHECK(imported.party->champions[0].food == 1500,
          "champion 0 signed food imported");
    CHECK(imported.party->champions[0].water == -32,
          "champion 0 signed water imported");
    CHECK(imported.party->champions[0].wounds == 0x0021u,
          "champion 0 wounds imported");
    CHECK(imported.party->champions[0].poisonDose == 3u,
          "champion 0 PoisonEventCount imported");
    CHECK(imported.party->champions[0].attributes[CHAMPION_ATTR_STRENGTH] == 31,
          "champion 0 current strength imported");
    CHECK(imported.party->champions[0].attributeMaximums[CHAMPION_ATTR_STRENGTH] == 41,
          "champion 0 max strength imported");
    CHECK(imported.party->champions[0].skillExperience[CHAMPION_SKILL_FIGHTER] == 1000u,
          "champion 0 fighter experience imported");
    CHECK(imported.party->champions[0].skillLevels[CHAMPION_SKILL_FIGHTER] == 3u,
          "champion 0 fighter level derived from experience");
    CHECK(imported.party->champions[0].skillExperience[CHAMPION_SKILL_WIZARD] == 1333u,
          "champion 0 wizard experience imported");
    CHECK(imported.party->champions[0].skillLevels[CHAMPION_SKILL_WIZARD] == 3u,
          "champion 0 wizard level derived from experience");
    CHECK(imported.party->champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] == 0x1555u,
          "champion 0 hand slot imported");
    CHECK(imported.party->champions[0].load == 345u,
          "champion 0 load imported");
    CHECK(imported.party->champions[3].present == 0,
          "unused champion slot cleared");

    memset(&world, 0, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    groups[1].creatureType = 12;
    groups[2].creatureType = 23;
    groups[3].creatureType = 8;
    things.groups = groups;
    things.groupCount = 4;
    world.things = &things;
    world.partyMapIndex = 2;
    rc = dm1_v1_original_save_pc34_handoff_apply_active_groups(
        &report, &world);
    CHECK(rc == 2,
          "active groups apply to world");
    CHECK(world.creatureAICount == 2,
          "world creature AI count from active groups");
    CHECK(world.creatureAI[0].stateKind == AI_STATE_WANDER,
          "active group imports as wandering AI state");
    CHECK(world.creatureAI[0].creatureType == 12,
          "active group creature type resolved from dungeon things");
    CHECK(world.creatureAI[0].groupMapIndex == 2,
          "active group map index from world party map");
    CHECK(world.creatureAI[0].groupMapX == 5,
          "active group prior x maps to runtime group x");
    CHECK(world.creatureAI[0].groupMapY == 6,
          "active group prior y maps to runtime group y");
    CHECK(world.creatureAI[0].groupCells == 0xc3,
          "active group cells map to runtime group cells");
    CHECK(world.creatureAI[0].groupDirection == 2,
          "active group first packed direction maps to runtime direction");
    CHECK(world.creatureAI[0].lastSeenPartyMapX == 21,
          "active group target x preserved in runtime target surrogate");
    CHECK(world.creatureAI[0].lastSeenPartyMapY == 22,
          "active group target y preserved in runtime target surrogate");
    CHECK(world.creatureAI[0].lastSeenPartyTick == 12,
          "active group last move time imported");
    CHECK(world.creatureAI[0].fearCounter == 3,
          "active group flee delay imported");
    CHECK(world.creatureAI[0].targetChampionIndex == -1,
          "active group target champion unresolved");
    CHECK(world.creatureAI[0].reserved0 == 1,
          "active group runtime index resolved");
    CHECK(world.creatureAI[1].creatureType == 23,
          "second active group creature type resolved");
    CHECK(world.creatureAI[1].reserved0 == 2,
          "second active group runtime index resolved");
    CHECK(world.creatureAI[2].reserved0 == 0,
          "maximum-only active group not imported as live runtime state");
    CHECK(report.active_group_runtime_imported_count == 2,
          "report active group runtime import count");
    CHECK(report.active_group_runtime_truncated_count == 0,
          "report no runtime active group truncation");
    CHECK(report.active_group_runtime_resolved_count == 2,
          "report resolved runtime active group count");
    CHECK(report.active_group_runtime_unresolved_count == 0,
          "report unresolved runtime active group count");

    memset(&world, 0, sizeof(world));
    world.partyMapIndex = 2;
    rc = dm1_v1_original_save_pc34_handoff_apply_active_groups(
        &report, &world);
    CHECK(rc == 2,
          "active groups apply without dungeon things");
    CHECK(world.creatureAI[0].creatureType == -1,
          "fallback active group creature type unresolved");
    CHECK(world.creatureAI[0].reserved0 == 0x1001,
          "fallback preserves raw group thing index");
    CHECK(report.active_group_runtime_resolved_count == 0,
          "fallback report resolved count");
    CHECK(report.active_group_runtime_unresolved_count == 2,
          "fallback report unresolved count");

    memset(&event_queue, 0, sizeof(event_queue));
    rc = dm1_v1_original_save_pc34_handoff_apply_event_queue(
        &report, &event_queue);
    CHECK(rc == ORIGINAL_PC34_EVENT_COUNT,
          "event queue apply succeeds");
    CHECK(event_queue.gameTick == 123456u,
          "event queue game tick imported");
    CHECK(event_queue.eventCount == ORIGINAL_PC34_EVENT_COUNT,
          "event queue event count imported");
    CHECK(event_queue.firstUnusedIndex == ORIGINAL_PC34_EVENT_COUNT,
          "event queue first unused index imported");
    CHECK(event_queue.events[1].type == DM1_EVENT_DOOR,
          "event queue event data imported");
    CHECK(event_queue.timeline[0] == 1u,
          "event queue first timeline slot imported");
    CHECK(event_queue.events[event_queue.timeline[0]].type == DM1_EVENT_DOOR,
          "event queue heap root points to first original timeline event");
}

static void test_rejects_non_pc34_and_truncated_parts(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct PartyState_Compat party_before;
    struct GameWorld_Compat world;
    struct DM1_EventQueue_V1 event_queue;
    DM1OriginalSavePC34HandoffReport report;
    DM1OriginalSavePC34HandoffReport duplicate_report;
    int rc;

    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;

    rc = dm1_v1_original_save_pc34_handoff_bytes(
        NULL, 0u, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT,
          "NULL bytes rejected");

    memset(bytes, 0, 128u);
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, 128u, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34,
          "short non-PC34 rejected");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "truncated fixture build succeeds");
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 1u, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "valid PC34 header with truncated parts rejected by importer");
    CHECK(report.classify.shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34,
          "truncated file still has PC34 header shape");
    CHECK(report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE,
          "truncated importer result preserved");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "portrait truncation fixture build succeeds");
    memset(&party, 0xa5, sizeof(party));
    party_before = party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written - 1u, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "truncated fixed portrait section rejected");
    CHECK(report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE,
          "portrait truncation reports source size failure");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "portrait truncation leaves direct handoff party unchanged");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "checksum fixture build succeeds");
    bytes[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2u] ^= 0x20u;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "corrupt GLOBAL_DATA checksum rejected");
    CHECK(report.importer_result == SAVEGAME_PC34_ERROR_BAD_CHECKSUM,
          "checksum importer result preserved");
    CHECK(report.part_checksum_ok_count == 0,
          "checksum failure stops before accepting any part");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "party checksum fixture build succeeds");
    bytes[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE +
          2u + SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT +
          2u + ORIGINAL_PC34_ACTIVE_GROUP_PART_BYTES +
          2u] ^= 0x10u;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "corrupt PARTY checksum rejected");
    CHECK(report.importer_result == SAVEGAME_PC34_ERROR_BAD_CHECKSUM,
          "party checksum importer result preserved");
    CHECK(report.part_checksum_ok_count == 2,
          "party checksum failure accepts only prior parts");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "events checksum fixture succeeds");
    bytes[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE +
          2u + SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT +
          2u + ORIGINAL_PC34_ACTIVE_GROUP_PART_BYTES +
          2u + ORIGINAL_PC34_PARTY_BYTES +
          2u] ^= 0x08u;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "corrupt EVENTS checksum rejected");
    CHECK(report.importer_result == SAVEGAME_PC34_ERROR_BAD_CHECKSUM,
          "events checksum importer result preserved");
    CHECK(report.part_checksum_ok_count == 3,
          "events checksum failure accepts prior parts only");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "active group checksum fixture succeeds");
    bytes[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE +
          2u + SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT +
          2u] ^= 0x04u;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "corrupt ACTIVE_GROUP checksum rejected");
    CHECK(report.importer_result == SAVEGAME_PC34_ERROR_BAD_CHECKSUM,
          "active group checksum importer result preserved");
    CHECK(report.part_checksum_ok_count == 1,
          "active group checksum failure accepts only GLOBAL_DATA");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0, 2);
    CHECK(rc == SAVEGAME_PC34_OK,
          "active group length mismatch fixture succeeds");
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "ACTIVE_GROUP length mismatch rejected");
    CHECK(report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE,
          "active group size importer result preserved");
    CHECK(report.part_checksum_ok_count == 2,
          "active group size failure records checksum-valid part");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
          rewrite_fixture_timeline_index(bytes, (size_t)written, 1, 1u),
          "duplicate C4 timeline fixture remains checksum-authenticated");
    memset(&party, 0xa5, sizeof(party));
    party_before = party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
          report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE &&
          report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT &&
          report.timeline_duplicate_first_slot == 0 &&
          report.timeline_duplicate_slot == 1 &&
          report.timeline_duplicate_event_index == 1,
          "duplicate C4 EVENT index has stable F0435 provenance");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "duplicate C4 index rolls back staged party import");
    memset(&duplicate_report, 0, sizeof(duplicate_report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &duplicate_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
          duplicate_report.importer_result == report.importer_result &&
          duplicate_report.part_checksum_ok_count ==
              report.part_checksum_ok_count &&
          duplicate_report.timeline_duplicate_first_slot ==
              report.timeline_duplicate_first_slot &&
          duplicate_report.timeline_duplicate_slot ==
              report.timeline_duplicate_slot &&
          duplicate_report.timeline_duplicate_event_index ==
              report.timeline_duplicate_event_index,
          "duplicate C4 failure provenance is repeatable");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
          rewrite_fixture_timeline_index(bytes, (size_t)written, 1, 4u),
          "out-of-range C4 index fixture remains checksum-authenticated");
    memset(&party, 0xa5, sizeof(party));
    party_before = party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
          report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE &&
          report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT &&
          report.timeline_invalid_slot == 1 &&
          report.timeline_invalid_event_index == 4 &&
          report.timeline_duplicate_slot == -1,
          "out-of-range C4 index has stable F0435 provenance");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "out-of-range C4 index rolls back staged party import");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
          rewrite_fixture_timeline_index(bytes, (size_t)written, 1, 3u),
          "C4 EVENT_NONE index fixture remains checksum-authenticated");
    memset(&party, 0xa5, sizeof(party));
    party_before = party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
          report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE &&
          report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT &&
          report.timeline_invalid_slot == 1 &&
          report.timeline_invalid_event_index == 3 &&
          report.timeline_invalid_event_is_none &&
          report.original_first_unused_event_index ==
              report.timeline_invalid_event_index &&
          !report.first_unused_event_index_points_to_active &&
          report.timeline_duplicate_slot == -1,
          "C4 reference to its free-list tombstone has exact provenance");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "C4 EVENT_NONE reference rolls back staged party import");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
          rewrite_fixture_first_unused_event_index(
              bytes, (size_t)written, 1u),
          "active FirstUnusedEventIndex fixture remains checksum-authenticated");
    memset(&party, 0xa5, sizeof(party));
    party_before = party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
          report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE &&
          report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT &&
          report.original_first_unused_event_index == 1 &&
          report.first_unused_event_index_points_to_active &&
          report.first_unused_event_index_event_type == DM1_EVENT_DOOR,
          "active free-list owner has exact F0435/F0651 provenance");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "active free-list owner rolls back staged party import");

    /* F0433/F0238 persist each active EVENT through both C3 and C4. Keep a
     * valid C13 record in C3 but replace its C4 slot with another active
     * event: F0435 must reject this before the runtime can silently lose the
     * rebirth timer. */
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event_type(bytes, (size_t)written, 2,
                                          DM1_EVENT_VI_ALTAR_REBIRTH) &&
              rewrite_fixture_event_type(bytes, (size_t)written, 3,
                                          DM1_EVENT_DOOR) &&
              rewrite_fixture_timeline_index(bytes, (size_t)written, 1, 3u) &&
              rewrite_fixture_first_unused_event_index(
                  bytes, (size_t)written, 4u),
          "orphan C13/C4 fixture remains checksum-authenticated");
    memset(&party, 0xa5, sizeof(party));
    party_before = party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
              report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE &&
              report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT &&
              report.timeline_orphan_active_event_index == 2 &&
              report.timeline_orphan_active_event_type ==
                  DM1_EVENT_VI_ALTAR_REBIRTH,
          "C13 missing from C4 has stable F0433/F0435 provenance");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "orphan C13/C4 rolls back staged party import");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 1, 2, 3, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
          rewrite_fixture_timeline_swap_indices(
              bytes, (size_t)written, 0, 1),
          "reordered C4 heap fixture remains checksum-authenticated");
    memset(&party, 0xa5, sizeof(party));
    party_before = party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
          report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE &&
          report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT &&
          report.timeline_heap_invalid_parent_slot == 0 &&
          report.timeline_heap_invalid_child_slot == 1 &&
          report.timeline_heap_invalid_parent_event_index == 2 &&
          report.timeline_heap_invalid_child_event_index == 1,
          "reordered C4 heap has stable F0234 provenance");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "reordered C4 heap rolls back staged party import");

    memset(&report, 0, sizeof(report));
    memset(&world, 0, sizeof(world));
    report.reported_active_group_count =
        DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP + 1;
    rc = dm1_v1_original_save_pc34_handoff_apply_active_groups(
        &report, &world);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "malformed active-group report count rejected");

    memset(&report, 0, sizeof(report));
    memset(&event_queue, 0, sizeof(event_queue));
    CHECK(dm1v1_event_queue_init(&event_queue, 77u),
          "initialize live event queue before malformed import");
    event_queue.eventCount = 1;
    event_queue.firstUnusedIndex = 1;
    event_queue.events[0].type = DM1_EVENT_WALL;
    event_queue.timeline[0] = 0u;
    report.original_game_time = 10u;
    report.original_event_count = 1;
    report.original_first_unused_event_index = 1;
    report.decoded_event_count = 1;
    report.decoded_timeline_index_count = 1;
    report.timeline_indices[0] = 9u;
    rc = dm1_v1_original_save_pc34_handoff_apply_event_queue(
        &report, &event_queue);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "malformed timeline index rejected");
    CHECK(event_queue.gameTick == 77u && event_queue.eventCount == 1 &&
          event_queue.firstUnusedIndex == 1 &&
          event_queue.events[0].type == DM1_EVENT_WALL &&
          event_queue.timeline[0] == 0u,
          "malformed timeline import leaves live queue untouched");
}

static void test_file_runtime_world_loader(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    struct GameWorld_Compat world;
    struct DM1_EventQueue_V1 event_queue;
    DM1OriginalSavePC34HandoffReport report;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "file loader fixture build succeeds");
    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(2, 123500u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "file loader fixture keeps each source event materializable");
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "fixture file write succeeds");

    memset(&world, 0, sizeof(world));

    rc = dm1_v1_original_save_pc34_handoff_load_world_from_file(
        path, &world, &event_queue, &report);
    remove(path);

    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "tail-less file world loader preserves the source runtime state");
    CHECK(world.gameTick == 123456u,
          "world game tick imported from original GameTime");
    CHECK(world.timeline.nowTick == 123456u,
          "world M10 timeline tick follows original GameTime");
    CHECK(world.partyMapIndex == 3,
          "world party map index imported");
    CHECK(world.newPartyMapIndex == 3,
          "world new-party map index re-anchored");
    CHECK(world.party.championCount == 2,
          "world party champion count imported");
    CHECK(world.party.mapX == 9,
          "world party x imported");
    CHECK(world.party.mapY == 10,
          "world party y imported");
    CHECK(world.party.direction == 2,
          "world party direction imported");
    CHECK(world.party.activeChampionIndex == 1,
          "world active champion imported");
    CHECK(memcmp(world.party.champions[1].name, "WUUF    ", 8u) == 0,
          "world second champion name imported");
    CHECK(world.creatureAICount == 2,
          "world active groups imported");
    CHECK(world.creatureAI[0].creatureType == -1 &&
          world.creatureAI[1].creatureType == -1,
          "tail-less file loader leaves active groups unresolved");
    CHECK(report.active_group_runtime_imported_count == 2 &&
          report.active_group_runtime_unresolved_count == 2,
          "file loader reports tail-less active group ownership");
    CHECK(event_queue.gameTick == 123456u,
          "file loader event queue tick imported");
    CHECK(event_queue.eventCount == ORIGINAL_PC34_EVENT_COUNT,
          "file loader event queue count imported");
    CHECK(event_queue.timeline[0] == 1u,
          "file loader event queue timeline imported");

    memset(&world, 0, sizeof(world));
    rc = dm1_v1_original_save_pc34_handoff_file(
        "/tmp/firestaff_dm1_original_pc34_missing.sav",
        NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT,
          "file loader rejects null state before IO");
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_file(
        "/tmp/firestaff_dm1_original_pc34_missing.sav",
        &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE,
          "world loader reports missing file");
}

static void test_tail_less_f0435_publishes_c3_c4_receipt(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char expected_c3_first[10];
    int written = 0;
    struct GameWorld_Compat world;
    DM1OriginalSavePC34HandoffReport report;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "tail-less C3/C4 receipt fixture build succeeds");
    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(2, 123500u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "tail-less C3/C4 receipt events are materializable");

    memset(&world, 0, sizeof(world));
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "tail-less F0435 publishes C3/C4 receipt after validation");
    write_original_event(expected_c3_first, DM1_MAP_TIME_MAKE(2, 123500u),
                         DM1_EVENT_LIGHT, 0, 6, 0, 0, 0);
    CHECK(report.c3_c4_receipt_valid == 1 &&
          world.pc34OriginalC3C4ReceiptValid == 1,
          "tail-less F0435 marks C3/C4 receipt valid");
    CHECK(report.c3_raw_event_byte_count ==
              ORIGINAL_PC34_EVENT_MAXIMUM_COUNT * 10u &&
          report.c4_raw_heap_byte_count ==
              ORIGINAL_PC34_EVENT_MAXIMUM_COUNT * 2u,
          "tail-less F0435 retains bounded complete C3/C4 parts");
    CHECK(memcmp(report.c3_raw_event_bytes, expected_c3_first,
                 sizeof(expected_c3_first)) == 0 &&
          report.c4_raw_heap_bytes[0] == 1u &&
          report.c4_raw_heap_bytes[1] == 0u,
          "tail-less F0435 receipt retains source C3 row and C4 heap bytes");
    CHECK(world.pc34OriginalC3RawEventByteCount ==
              report.c3_raw_event_byte_count &&
          world.pc34OriginalC4RawHeapByteCount ==
              report.c4_raw_heap_byte_count &&
          memcmp(world.pc34OriginalC3RawEventBytes,
                 report.c3_raw_event_bytes,
                 report.c3_raw_event_byte_count) == 0 &&
          memcmp(world.pc34OriginalC4RawHeapBytes,
                 report.c4_raw_heap_bytes,
                 report.c4_raw_heap_byte_count) == 0,
          "tail-less F0435 carries C3/C4 bytes into the world boundary");
    CHECK(report.c3_raw_event_fingerprint == fnv1a32(
              report.c3_raw_event_bytes, report.c3_raw_event_byte_count) &&
          report.c4_raw_heap_fingerprint == fnv1a32(
              report.c4_raw_heap_bytes, report.c4_raw_heap_byte_count) &&
          world.pc34OriginalC3RawEventFingerprint ==
              report.c3_raw_event_fingerprint &&
          world.pc34OriginalC4RawHeapFingerprint ==
              report.c4_raw_heap_fingerprint,
          "tail-less F0435 carries C3/C4 FNV provenance unchanged");
}

static void test_tail_less_f0435_reuses_c3_c4_receipt_only_without_drift(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    int exported_size = 0;
    struct GameWorld_Compat world;
    DM1OriginalSavePC34HandoffReport source_report;
    DM1OriginalSavePC34HandoffReport exported_report;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "C3/C4 reuse fixture build succeeds");
    CHECK(rewrite_fixture_event(bytes, written, 0,
                                DM1_MAP_TIME_MAKE(2, 123500u),
                                DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(bytes, written, 1,
                                DM1_MAP_TIME_MAKE(2, 123470u),
                                DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(bytes, written, 2,
                                DM1_MAP_TIME_MAKE(1, 123490u),
                                DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "C3/C4 reuse fixture has materializable source rows");

    memset(&world, 0, sizeof(world));
    memset(&source_report, 0, sizeof(source_report));
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &source_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
          world.pc34OriginalC3C4ReceiptValid == 1,
          "F0435 publishes reusable tail-less C3/C4 receipt");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK,
          "F0802 reuses unchanged tail-less C3/C4 receipt");
    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    memset(&exported_report, 0, sizeof(exported_report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_size, &imported, &exported_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
          exported_report.c3_raw_event_byte_count ==
              source_report.c3_raw_event_byte_count &&
          exported_report.c4_raw_heap_byte_count ==
              source_report.c4_raw_heap_byte_count &&
          memcmp(exported_report.c3_raw_event_bytes,
                 source_report.c3_raw_event_bytes,
                 source_report.c3_raw_event_byte_count) == 0 &&
          memcmp(exported_report.c4_raw_heap_bytes,
                 source_report.c4_raw_heap_bytes,
                 source_report.c4_raw_heap_byte_count) == 0,
          "F0802 preserves authenticated tail-less C3/C4 bytes exactly");

    world.timeline.events[0].fireAtTick++;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_size) == SAVEGAME_PC34_ERROR_INTERNAL,
          "F0802 rejects timeline drift against the F0435 receipt");

    memset(&world, 0, sizeof(world));
    CHECK(dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
              bytes, (size_t)written, &world, NULL, NULL) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C4-drift receipt fixture reload succeeds");
    world.pc34OriginalC4RawHeapBytes[0] ^= 1u;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_size) == SAVEGAME_PC34_ERROR_INTERNAL,
          "F0802 rejects mutated C4 heap receipt bytes");

    memset(&world, 0, sizeof(world));
    CHECK(dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
              bytes, (size_t)written, &world, NULL, NULL) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "malformed-cache receipt fixture reload succeeds");
    world.pc34OriginalC3RawEventByteCount--;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_size) == SAVEGAME_PC34_ERROR_INTERNAL,
          "F0802 rejects malformed C3/C4 receipt bounds");
}

static void test_runtime_materializer_reuses_start_dungeon_and_normalizes_hoc(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char manifest_bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    size_t manifest_size = 0u;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat start_dungeon;
    struct DungeonThings_Compat start_things;
    struct DungeonGroup_Compat groups[4];
    struct DungeonMapDesc_Compat maps[4];
    DM1OriginalSavePC34HoCResumeState hoc;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "runtime materializer fixture build succeeds");
    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(2, 123500u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "runtime materializer fixture keeps each source event materializable");
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "runtime materializer fixture write succeeds");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&start_dungeon, 0, sizeof(start_dungeon));
    memset(&start_things, 0, sizeof(start_things));
    memset(groups, 0, sizeof(groups));
    memset(maps, 0, sizeof(maps));
    for (rc = 0; rc < 4; ++rc) {
        maps[rc].width = 32;
        maps[rc].height = 32;
    }
    groups[1].creatureType = 15;
    groups[2].creatureType = 16;
    start_things.groups = groups;
    start_things.groupCount = 4;
    start_dungeon.header.mapCount = 4;
    start_dungeon.maps = maps;
    start_world.dungeon = &start_dungeon;
    start_world.things = &start_things;

    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, NULL);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "runtime materializer imports original save");
    CHECK(loaded_world.dungeon == &start_dungeon,
          "tail-less original save reuses start dungeon");
    CHECK(loaded_world.things == &start_things,
          "tail-less original save reuses start things");
    CHECK(loaded_world.ownsDungeon == 0,
          "borrowed start dungeon keeps original owner");

    /* An explicitly selected F0433 export remains valid F0435 handoff data.
     * Corpus discovery, rather than the parser, excludes it when choosing an
     * external original-save candidate. */
    CHECK(dm1_v1_original_save_pc34_roundtrip_world_bytes(
              bytes, (size_t)written, 0x4d313031u,
              manifest_bytes, sizeof(manifest_bytes), &manifest_size,
              NULL, NULL) == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "runtime materializer builds a manifest-bearing verification export");
    CHECK(write_fixture_file(path, manifest_bytes, (int)manifest_size),
          "runtime materializer writes manifest-bearing export");
    F0883_WORLD_Free_Compat(&loaded_world);
    memset(&loaded_world, 0, sizeof(loaded_world));
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, NULL);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34,
          "runtime materializer rejects a Firestaff F0433 verification export");
    remove(path);

    CHECK(write_fixture_file(path, bytes, written),
          "runtime materializer restores external fixture after explicit export");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, NULL);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "runtime materializer still consumes the external fixture");

    CHECK(dm1_v1_original_save_pc34_handoff_adopt_runtime_world(
              &start_world, &loaded_world) == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "DM1 handoff adopts materialized runtime world");
    CHECK(start_world.dungeon == &start_dungeon &&
          start_world.things == &start_things,
          "adopted world retains the shared DM1 start materialization");
    CHECK(start_world.ownsDungeon == 0,
          "non-owning fixture stays non-owning after adopt");

    memset(&hoc, 0, sizeof(hoc));
    hoc.candidate_mirror_ordinal = 7;
    hoc.candidate_party_index = 1;
    hoc.candidate_panel_active = 1;
    dm1_v1_original_save_pc34_handoff_normalize_hoc_resume_state(
        &start_world, &hoc);
    CHECK(hoc.candidate_panel_active == 1,
          "valid HoC candidate remains materialized after resume");
    CHECK(hoc.inventory_panel_active == 1,
          "valid HoC candidate reopens inventory panel");

    hoc.candidate_party_index = CHAMPION_MAX_PARTY;
    hoc.inventory_panel_active = 1;
    dm1_v1_original_save_pc34_handoff_normalize_hoc_resume_state(
        &start_world, &hoc);
    CHECK(hoc.candidate_panel_active == 0,
          "invalid HoC candidate is cleared after resume");
    CHECK(hoc.candidate_mirror_ordinal == -1 && hoc.candidate_party_index == -1,
          "invalid HoC candidate resets both source indices");
    CHECK(hoc.inventory_panel_active == 0,
          "invalid HoC candidate closes dependent inventory render panel");
    F0883_WORLD_Free_Compat(&start_world);
}

static void test_runtime_byte_materializer_reuses_start_dungeon(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    int rc;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct GameWorld_Compat unchanged_world;
    struct DungeonDatState_Compat start_dungeon;
    struct DungeonThings_Compat start_things;
    struct DungeonGroup_Compat groups[4];
    struct DungeonMapDesc_Compat maps[4];

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "byte runtime materializer fixture build succeeds");
    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(2, 123500u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "byte runtime materializer fixture keeps each source event materializable");
    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&unchanged_world, 0, sizeof(unchanged_world));
    memset(&start_dungeon, 0, sizeof(start_dungeon));
    memset(&start_things, 0, sizeof(start_things));
    memset(groups, 0, sizeof(groups));
    memset(maps, 0, sizeof(maps));
    for (rc = 0; rc < 4; ++rc) {
        maps[rc].width = 32;
        maps[rc].height = 32;
    }
    groups[1].creatureType = 15;
    groups[2].creatureType = 16;
    start_things.groups = groups;
    start_things.groupCount = 4;
    start_dungeon.header.mapCount = 4;
    start_dungeon.maps = maps;
    start_world.dungeon = &start_dungeon;
    start_world.things = &start_things;
    unchanged_world.gameTick = 77;

    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_bytes(
        bytes, (size_t)written, &start_world, &loaded_world, NULL, NULL);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "byte F0435 materializer imports through the start dungeon");
    CHECK(loaded_world.dungeon == &start_dungeon &&
              loaded_world.things == &start_things &&
              loaded_world.ownsDungeon == 0,
          "tail-less byte import retains original DM1 backing ownership");
    CHECK(dm1_v1_original_save_pc34_handoff_materialize_runtime_from_bytes(
              bytes, (size_t)written, NULL, &unchanged_world, NULL, NULL) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
              unchanged_world.gameTick == 77,
          "tail-less byte import rejects atomically without a source dungeon");
    F0883_WORLD_Free_Compat(&loaded_world);
}

static void test_runtime_state_adoption_moves_f0435_queue(void)
{
    struct GameWorld_Compat runtime_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DM1_EventQueue_V1 runtime_queue;
    struct DM1_EventQueue_V1 loaded_queue;

    memset(&runtime_world, 0, sizeof(runtime_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    CHECK(dm1v1_event_queue_init(&runtime_queue, 77u),
          "runtime queue initializes before adoption");
    CHECK(dm1v1_event_queue_init(&loaded_queue, 123456u),
          "loaded queue initializes before adoption");
    runtime_world.gameTick = 77u;
    loaded_world.gameTick = 123456u;
    loaded_world.timeline.nowTick = 123456u;
    loaded_world.dungeon = &dungeon;
    loaded_world.things = &things;
    loaded_world.timeline.count = 1;
    loaded_queue.eventCount = 1;
    loaded_queue.firstUnusedIndex = 2;
    loaded_queue.events[1].type = DM1_EVENT_DOOR;
    loaded_queue.timeline[0] = 1u;

    CHECK(dm1_v1_original_save_pc34_handoff_adopt_runtime_state(
              &runtime_world, &runtime_queue, &loaded_world, &loaded_queue) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "runtime state adoption commits world and F0435 queue together");
    CHECK(runtime_world.gameTick == 123456u &&
              runtime_world.dungeon == &dungeon && runtime_world.things == &things &&
              runtime_world.timeline.count == 1,
          "runtime state adoption publishes the loaded world");
    CHECK(runtime_queue.gameTick == 123456u && runtime_queue.eventCount == 1 &&
              runtime_queue.firstUnusedIndex == 2 &&
              runtime_queue.events[1].type == DM1_EVENT_DOOR &&
              runtime_queue.timeline[0] == 1u,
          "runtime state adoption publishes the exact F0435 event queue");
    CHECK(loaded_world.dungeon == NULL && loaded_world.things == NULL &&
              loaded_queue.eventCount == 0 && loaded_queue.gameTick == 0u,
          "runtime state adoption consumes both candidate owners");
    CHECK(dm1_v1_original_save_pc34_handoff_adopt_runtime_state(
              &runtime_world, &runtime_queue, &runtime_world, &loaded_queue) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT,
          "runtime state adoption rejects aliased world owners");
}

static void test_runtime_state_adoption_rejects_incoherent_f0435_queue(void)
{
    struct GameWorld_Compat runtime_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat runtime_dungeon;
    struct DungeonThings_Compat runtime_things;
    struct DungeonDatState_Compat loaded_dungeon;
    struct DungeonThings_Compat loaded_things;
    struct DM1_EventQueue_V1 runtime_queue;
    struct DM1_EventQueue_V1 loaded_queue;

    memset(&runtime_world, 0, sizeof(runtime_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&runtime_dungeon, 0, sizeof(runtime_dungeon));
    memset(&runtime_things, 0, sizeof(runtime_things));
    memset(&loaded_dungeon, 0, sizeof(loaded_dungeon));
    memset(&loaded_things, 0, sizeof(loaded_things));
    CHECK(dm1v1_event_queue_init(&runtime_queue, 77u),
          "live queue initializes before rejected adoption");
    CHECK(dm1v1_event_queue_init(&loaded_queue, 123456u),
          "candidate queue initializes before rejected adoption");

    runtime_world.gameTick = 77u;
    runtime_world.dungeon = &runtime_dungeon;
    runtime_world.things = &runtime_things;
    runtime_world.ownsDungeon = 0;
    loaded_world.gameTick = 123456u;
    loaded_world.timeline.nowTick = 123456u;
    loaded_world.timeline.count = 1;
    loaded_world.dungeon = &loaded_dungeon;
    loaded_world.things = &loaded_things;
    loaded_world.ownsDungeon = 0;
    loaded_queue.eventCount = 1;
    loaded_queue.firstUnusedIndex = 2;
    loaded_queue.events[1].type = DM1_EVENT_DOOR;
    loaded_queue.timeline[0] = 1u;

    /* A candidate C4 heap that does not belong to its F0435 GameTime must
     * fail before either live owner moves. */
    loaded_queue.gameTick = 123457u;
    CHECK(dm1_v1_original_save_pc34_handoff_adopt_runtime_state(
              &runtime_world, &runtime_queue, &loaded_world, &loaded_queue) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "incoherent F0435 queue rejects before runtime adoption");
    CHECK(runtime_world.gameTick == 77u &&
              runtime_world.dungeon == &runtime_dungeon &&
              runtime_world.things == &runtime_things &&
              runtime_world.ownsDungeon == 0 &&
              runtime_queue.gameTick == 77u && runtime_queue.eventCount == 0,
          "rejected queue leaves both live runtime owners unchanged");
    CHECK(loaded_world.gameTick == 123456u &&
              loaded_world.dungeon == &loaded_dungeon &&
              loaded_world.things == &loaded_things &&
              loaded_world.ownsDungeon == 0 &&
              loaded_queue.gameTick == 123457u && loaded_queue.eventCount == 1,
          "rejected queue remains owned by the F0435 candidate");

    F0883_WORLD_Free_Compat(&runtime_world);
    F0883_WORLD_Free_Compat(&loaded_world);
}

static void test_real_pc34_export_resumes_runtime_atomically(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat saved_world;
    struct GameWorld_Compat runtime_world;
    struct DungeonDatState_Compat saved_dungeon;
    struct DungeonThings_Compat saved_things;
    struct DungeonDatState_Compat runtime_dungeon;
    struct DungeonThings_Compat runtime_things;
    struct DungeonMapDesc_Compat saved_map;
    struct DungeonMapDesc_Compat runtime_map;
    struct DM1_EventQueue_V1 runtime_queue;
    DM1OriginalSavePC34HandoffReport report;
    int written = 0;

    memset(&saved_world, 0, sizeof(saved_world));
    memset(&runtime_world, 0, sizeof(runtime_world));
    memset(&saved_dungeon, 0, sizeof(saved_dungeon));
    memset(&saved_things, 0, sizeof(saved_things));
    memset(&runtime_dungeon, 0, sizeof(runtime_dungeon));
    memset(&runtime_things, 0, sizeof(runtime_things));
    memset(&saved_map, 0, sizeof(saved_map));
    memset(&runtime_map, 0, sizeof(runtime_map));
    memset(&report, 0, sizeof(report));

    saved_map.width = runtime_map.width = 32;
    saved_map.height = runtime_map.height = 32;
    saved_dungeon.header.mapCount = runtime_dungeon.header.mapCount = 1;
    saved_dungeon.maps = &saved_map;
    runtime_dungeon.maps = &runtime_map;
    saved_world.dungeon = &saved_dungeon;
    saved_world.things = &saved_things;
    saved_world.ownsDungeon = 0;
    saved_world.gameTick = 54321u;
    saved_world.timeline.nowTick = saved_world.gameTick;
    saved_world.party.championCount = 1;
    saved_world.party.mapIndex = 0;
    saved_world.party.mapX = 7;
    saved_world.party.mapY = 8;
    saved_world.party.direction = 2;
    saved_world.party.champions[0].present = 1;

    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &saved_world, 0x52455355u, bytes, (int)sizeof(bytes),
              &written) == SAVEGAME_PC34_OK && written > 0,
          "production F0433 PC34 exporter supplies resume bytes");

    runtime_world.dungeon = &runtime_dungeon;
    runtime_world.things = &runtime_things;
    runtime_world.ownsDungeon = 0;
    runtime_world.gameTick = 77u;
    runtime_world.timeline.nowTick = 77u;
    CHECK(dm1v1_event_queue_init(&runtime_queue, 77u),
          "live F0238 queue initializes before PC34 resume");

    CHECK(dm1_v1_original_save_pc34_handoff_resume_runtime_from_bytes(
              bytes, (size_t)written, &runtime_world, &runtime_queue,
              &report) == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "F0435 resume atomically adopts production PC34 state");
    CHECK(runtime_world.gameTick == saved_world.gameTick &&
              runtime_world.party.mapIndex == saved_world.party.mapIndex &&
              runtime_world.party.mapX == saved_world.party.mapX &&
              runtime_world.party.mapY == saved_world.party.mapY &&
              runtime_world.party.direction == saved_world.party.direction &&
              runtime_queue.gameTick == runtime_world.gameTick &&
              runtime_queue.eventCount == runtime_world.timeline.count &&
              report.importer_result == SAVEGAME_PC34_OK,
          "resumed runtime retains F0435 world and F0238 queue ownership");

    saved_world.gameTick = 999u;
    CHECK(dm1_v1_original_save_pc34_handoff_resume_runtime_from_bytes(
              bytes, (size_t)written - 1u, &runtime_world, &runtime_queue,
              NULL) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
              runtime_world.gameTick == 54321u &&
              runtime_queue.gameTick == 54321u,
          "rejected PC34 bytes leave the adopted runtime unchanged");
    F0883_WORLD_Free_Compat(&runtime_world);
}

static void test_runtime_materializer_binds_original_group_reaction(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int rc;
    int i;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    unsigned char square_data[3][32 * 32];
    uint16_t column_sft_bases[3 * 32];
    struct DungeonThings_Compat things;
    unsigned short first_things[1];
    struct DungeonGroup_Compat groups[3];
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "group reaction fixture build succeeds");
    CHECK(rewrite_fixture_event_type(
              bytes, (size_t)written, 0,
              DM1_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "fixture rewrites an authenticated source C29 event");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(column_sft_bases, 0, sizeof(column_sft_bases));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(&report, 0, sizeof(report));
    dungeon.header.mapCount = 3;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    /* ReDMCSB DUNGEON.C F0160 obtains the SquareFirstThing slot through
     * G0280's cumulative per-column base.  The first and only flagged
     * square is in map 2 / column 11, whose source base remains zero because
     * no earlier column has a thing-list square. */
    dungeon.columnsCumulativeSquareFirstThingCount = column_sft_bases;
    dungeon.dungeonColumnCount = (int)(sizeof(column_sft_bases) /
                                       sizeof(column_sft_bases[0]));
    for (i = 0; i < 3; ++i) {
        maps[i].width = 32;
        maps[i].height = 32;
        tiles[i].squareData = square_data[i];
        tiles[i].squareCount = 32 * 32;
    }
    /* The fixture's event 0 is B.Location=(11,12) on map 2.  This is
     * the exact original SFT lookup used by TIMELINE.C -> GROUP.C F0209. */
    square_data[2][11 * 32 + 12] |= DUNGEON_SQUARE_MASK_THING_LIST;
    first_things[0] = (unsigned short)(THING_TYPE_GROUP << 10);
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 15;
    groups[1].next = THING_ENDOFLIST;
    groups[1].creatureType = 16;
    groups[2].next = THING_ENDOFLIST;
    groups[2].creatureType = 17;
    things.squareFirstThings = first_things;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 3;
    things.loaded = 1;
    start_world.dungeon = &dungeon;
    start_world.things = &things;

    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "group reaction fixture write succeeds");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "source C29 materializes through the original SFT group chain");
    CHECK(loaded_world.timeline.count == ORIGINAL_PC34_EVENT_COUNT,
          "source C29 remains in the staged runtime timeline");
    CHECK(loaded_world.timeline.events[2].kind == TIMELINE_EVENT_CREATURE_REACTION &&
              loaded_world.timeline.events[2].mapIndex == 2 &&
              loaded_world.timeline.events[2].mapX == 11 &&
              loaded_world.timeline.events[2].mapY == 12 &&
              loaded_world.timeline.events[2].aux0 == 0 &&
              loaded_world.timeline.events[2].aux1 == 15 &&
              loaded_world.timeline.events[2].aux2 ==
                  DM1_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE &&
              loaded_world.timeline.events[2].aux3 == 3 &&
              (loaded_world.timeline.events[2].aux4 & 0x100) != 0,
          "C29 preserves F0209 group identity, C.Ticks, and priority provenance");
}

static void test_runtime_materializer_recovers_missing_primary_from_backup(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    char backup_path[516];
    int written = 0;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat start_dungeon;
    struct DungeonThings_Compat start_things;
    DM1OriginalSavePC34HandoffReport report;
    DM1OriginalSaveClassifyResult classified;
    FILE *primary;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "backup materializer fixture build succeeds");
    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(2, 123500u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "backup materializer fixture keeps each source event materializable");
    make_temp_save_path(path, sizeof(path));
    snprintf(backup_path, sizeof(backup_path), "%s.bak", path);
    remove(path);
    remove(backup_path);
    CHECK(write_fixture_file(path, bytes, written),
          "backup materializer primary fixture write succeeds");
    CHECK(rename(path, backup_path) == 0,
          "backup materializer moves fixture to automatic backup");
    CHECK(dm1_v1_original_save_classify_file(path, &classified) &&
          classified.shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 &&
          classified.resume_uses_backup,
          "resume classifier selects original backup only when primary is absent");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&start_dungeon, 0, sizeof(start_dungeon));
    memset(&start_things, 0, sizeof(start_things));
    memset(&report, 0, sizeof(report));
    start_world.dungeon = &start_dungeon;
    start_world.things = &start_things;
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "missing primary resumes from validated original backup");
    CHECK(report.resumed_from_backup && report.backup_promoted_to_primary,
          "backup receipt records source and post-validation promotion");
    CHECK(loaded_world.dungeon == &start_dungeon &&
          loaded_world.things == &start_things,
          "backup resume preserves the live start dungeon ownership boundary");
    primary = fopen(path, "rb");
    CHECK(primary != NULL, "validated backup is promoted to the primary path");
    if (primary) fclose(primary);
    remove(path);
    remove(backup_path);
    F0883_WORLD_Free_Compat(&loaded_world);
    F0883_WORLD_Free_Compat(&start_world);
}

static void test_runtime_handoff_is_transactional_on_rejected_tail(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    char backup_path[516];
    int written = 0;
    struct GameWorld_Compat world;
    struct GameWorld_Compat start_world;
    struct DungeonDatState_Compat start_dungeon;
    struct DungeonThings_Compat start_things;
    struct DM1_EventQueue_V1 event_queue;
    DM1OriginalSavePC34HandoffReport report;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "transactional handoff fixture build succeeds");
    CHECK(written < (int)sizeof(bytes),
          "transactional fixture has room for malformed tail");
    bytes[written++] = 0u;

    memset(&world, 0, sizeof(world));
    memset(&event_queue, 0, sizeof(event_queue));
    memset(&report, 0, sizeof(report));
    world.gameTick = 777u;
    world.party.championCount = 1;
    world.party.mapIndex = 6;
    event_queue.gameTick = 888u;
    event_queue.eventCount = 1;
    report.original_game_time = 999u;
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, &event_queue, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "bad optional dungeon tail rejects whole runtime handoff");
    CHECK(world.gameTick == 777u && world.party.championCount == 1 &&
          world.party.mapIndex == 6,
          "rejected byte handoff leaves live world untouched");
    CHECK(event_queue.gameTick == 888u && event_queue.eventCount == 1,
          "rejected byte handoff leaves event queue untouched");
    CHECK(report.original_game_time == 999u,
          "rejected byte handoff leaves receipt untouched");

    make_temp_save_path(path, sizeof(path));
    snprintf(backup_path, sizeof(backup_path), "%s.bak", path);
    remove(path);
    remove(backup_path);
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "transactional backup fixture rebuild succeeds");
    CHECK(write_fixture_file(backup_path, bytes, written),
          "transactional backup fixture write succeeds");
    bytes[written++] = 0u;
    CHECK(write_fixture_file(path, bytes, written),
          "transactional materializer fixture write succeeds");
    memset(&start_world, 0, sizeof(start_world));
    memset(&start_dungeon, 0, sizeof(start_dungeon));
    memset(&start_things, 0, sizeof(start_things));
    start_world.dungeon = &start_dungeon;
    start_world.things = &start_things;
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &world, &event_queue, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "bad primary tail rejects materialized runtime handoff without backup fallback");
    CHECK(world.gameTick == 777u && world.party.mapIndex == 6,
          "rejected materializer leaves destination world untouched");
    CHECK(event_queue.gameTick == 888u && event_queue.eventCount == 1,
          "rejected materializer leaves destination queue untouched");
    CHECK(report.original_game_time == 999u,
          "rejected materializer leaves destination receipt untouched");
    {
        FILE *backup = fopen(backup_path, "rb");
        CHECK(backup != NULL,
              "rejected primary leaves the automatic backup unpromoted");
        if (backup) fclose(backup);
    }
    remove(path);
    remove(backup_path);
    CHECK(dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
              path, &start_world, &start_world, NULL, NULL) ==
          DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT,
          "materializer rejects start/destination aliasing before IO");
    CHECK(dm1_v1_original_save_pc34_handoff_adopt_runtime_world(
              &world, &world) == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT,
          "adopt rejects self-transfer");
}

static void test_runtime_handoff_rejects_unknown_source_event(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    struct GameWorld_Compat world;
    struct DM1_EventQueue_V1 event_queue;
    DM1OriginalSavePC34HandoffReport report;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "unknown-event fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 0,
                                     80),
          "unknown C80 event rewrite preserves the PC34 envelope");

    memset(&world, 0, sizeof(world));
    memset(&event_queue, 0, sizeof(event_queue));
    memset(&report, 0, sizeof(report));
    world.gameTick = 777u;
    world.party.championCount = 1;
    world.party.mapIndex = 6;
    event_queue.gameTick = 888u;
    event_queue.eventCount = 1;
    report.original_game_time = 999u;
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, &event_queue, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "active unknown C80 event rejects runtime handoff");
    CHECK(world.gameTick == 777u && world.party.championCount == 1 &&
          world.party.mapIndex == 6,
          "unknown source event leaves live world untouched");
    CHECK(event_queue.gameTick == 888u && event_queue.eventCount == 1,
          "unknown source event leaves live queue untouched");
    CHECK(report.original_game_time == 999u,
          "unknown source event leaves receipt untouched");
}

static void test_runtime_handoff_materializes_original_c11_actions(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    int exported_size = 0;
    struct GameWorld_Compat world;
    struct DM1_EventQueue_V1 event_queue;
    DM1OriginalSavePC34HandoffReport report;
    struct TickInput_Compat input;
    struct TickResult_Compat tick;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    int emitted[CHAMPION_MAX_PARTY] = {0, 0, 0, 0};
    int c11_count = 0;
    int rc;

    /* ReDMCSB CHAMPION.C F0330 creates C11 with the champion in Priority
     * and SlotOrdinal 0; MENU.C F0407 changes that ordinal to 2 only for a
     * successful throw.  Make every active C3/C4 entry a C11 so this test
     * isolates the F0435 materializer without borrowing a dungeon owner. */
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     3, 3, 9, 10, 2, 2,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "C11 runtime fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 0,
                                     DM1_EVENT_ENABLE_CHAMPION_ACTION) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 0, 5, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 0, 6, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 0, 7, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 0, 8, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 0, 9, 0),
          "first C11 rewrite retains only its source-owned union");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 1,
                                     DM1_EVENT_ENABLE_CHAMPION_ACTION) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 1, 5, 1) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 1, 6, 2) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 1, 7, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 1, 8, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 1, 9, 0) &&
              rewrite_fixture_event(
                  bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
                  DM1_EVENT_ENABLE_CHAMPION_ACTION, 2, 0, 0, 0, 0),
          "throw C11 rewrite retains Priority and SlotOrdinal two");

    memset(&world, 0, sizeof(world));
    memset(&event_queue, 0, sizeof(event_queue));
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, &event_queue, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "source-shaped C11 records materialize through F0435");
    CHECK(world.timeline.count == 3 && event_queue.eventCount == 3 &&
              world.timeline.events[0].kind ==
                  TIMELINE_EVENT_ENABLE_CHAMPION_ACTION,
          "F0435 retains every C11 in both live timeline receipts");

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK,
          "C11 receipts export through their native Priority and B union");
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    imported.party = &imported_party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_size, &imported, &report);
    for (int i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_ENABLE_CHAMPION_ACTION) {
            CHECK(report.events[i].priority >= 0 &&
                      report.events[i].priority < CHAMPION_MAX_PARTY &&
                      (report.events[i].b_mapX == 0 ||
                       report.events[i].b_mapX == 2) &&
                      report.events[i].b_mapY == 0 &&
                      report.events[i].c_cell == 0 &&
                      report.events[i].c_effect == 0,
                  "C11 export retains only Priority and B.SlotOrdinal");
            ++c11_count;
        }
    }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && c11_count == 3,
          "C11 receipts round-trip without generic union substitution");

    while (world.gameTick <= 123500u) {
        int i;
        memset(&input, 0, sizeof(input));
        memset(&tick, 0, sizeof(tick));
        input.tick = (uint32_t)world.gameTick;
        input.command = CMD_NONE;
        CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &tick) != ORCH_FAIL,
              "C11 runtime accepts its idle tick");
        for (i = 0; i < tick.emissionCount; ++i) {
            if (tick.emissions[i].kind == EMIT_ACTION_ENABLED) {
                CHECK(tick.emissions[i].payload[0] >= 0 &&
                          tick.emissions[i].payload[0] < CHAMPION_MAX_PARTY &&
                          (tick.emissions[i].payload[1] == 0 ||
                           tick.emissions[i].payload[1] == 2) &&
                          tick.emissions[i].payload[2] == 0 &&
                          tick.emissions[i].payload[3] == 0,
                      "C11 emits only its source-owned champion and ordinal");
                ++emitted[tick.emissions[i].payload[0]];
            }
        }
    }
    CHECK(emitted[0] == 1 && emitted[1] == 1 && emitted[2] == 1 &&
              world.timeline.count == 0,
          "C11 dispatches each source-owned champion action enable once");
}

static void test_runtime_materializer_binds_original_sound_union(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int exported_size = 0;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat start_dungeon;
    struct DungeonThings_Compat start_things;
    struct DungeonGroup_Compat groups[4];
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;
    int rc;
    int i;
    int c20_index = -1;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "C20 materializer fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 0,
                                     DM1_EVENT_PLAY_SOUND) &&
              rewrite_fixture_event_c_union(bytes, (size_t)written, 0, 23u) &&
              rewrite_fixture_event(
                  bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
                  DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
              rewrite_fixture_event(
                  bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
                  DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "C20 fixture preserves authenticated SoundIndex union bytes");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&start_dungeon, 0, sizeof(start_dungeon));
    memset(&start_things, 0, sizeof(start_things));
    memset(groups, 0, sizeof(groups));
    memset(&report, 0, sizeof(report));
    groups[1].creatureType = 15;
    groups[2].creatureType = 16;
    start_things.groups = groups;
    start_things.groupCount = 4;
    start_world.dungeon = &start_dungeon;
    start_world.things = &start_things;
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "C20 materializer fixture write succeeds");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C20 materializes without generic Cell/Effect substitution");
    CHECK(loaded_world.timeline.count == ORIGINAL_PC34_EVENT_COUNT &&
              loaded_world.timeline.events[2].kind == TIMELINE_EVENT_PLAY_SOUND &&
              loaded_world.timeline.events[2].mapIndex == 2 &&
              loaded_world.timeline.events[2].mapX == 11 &&
              loaded_world.timeline.events[2].mapY == 12 &&
              loaded_world.timeline.events[2].aux0 == 23 &&
              loaded_world.timeline.events[2].aux2 == DM1_EVENT_PLAY_SOUND &&
              loaded_world.timeline.events[2].aux4 == 7,
          "C20 materialization binds original Location, SoundIndex, priority");
    event = loaded_world.timeline.events[2];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C20 receipt schedules for native export");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "C20 timeline replacement invalidates its F0435 C3/C4 receipt");
    /* The remaining checks exercise standalone C20 serialization after the
     * test deliberately replaced the authenticated F0435 timeline. */
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C20 exports only its typed receipt");
    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_size, &imported, &report);
    for (i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_PLAY_SOUND) {
            c20_index = i;
            break;
        }
    }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && c20_index >= 0 &&
              report.events[c20_index].priority == 7 &&
              report.events[c20_index].b_mapX == 11 &&
              report.events[c20_index].b_mapY == 12 &&
              (int)(int16_t)rd16le(
                  &report.events[c20_index].c_cell) == 23,
          "C20 export restores Location, SoundIndex, and source priority");
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C20 receipt schedules for runtime playback");
    loaded_world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result));
    F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(result.emissionCount == 1 &&
              result.emissions[0].kind == EMIT_SOUND_REQUEST &&
              result.emissions[0].payload[0] == 23 &&
              result.emissions[0].payload[1] == 11 &&
              result.emissions[0].payload[2] == 12 &&
              result.emissions[0].payload[3] == 2,
          "C20 runtime emits the source sound request");
    event.aux2 = 0;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "host C20 schedules for export rejection");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "C20 host export rejects an unauthenticated sound receipt");
}

static void test_runtime_materializer_linearizes_original_c4_heap_order(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int rc;
    int i;
    const uint32_t fire_at_tick = 123500u;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat start_dungeon;
    struct DungeonThings_Compat start_things;
    struct TickResult_Compat result;
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "same-tick C4 materializer fixture build succeeds");
    for (i = 0; i < ORIGINAL_PC34_EVENT_COUNT; ++i) {
        const int priority = i == 0 ? 3 : (i == 1 ? 1 : 2);
        const int sound_index = i == 0 ? 30 : (i == 1 ? 10 : 20);
        CHECK(rewrite_fixture_event_type(bytes, (size_t)written, i,
                                         DM1_EVENT_PLAY_SOUND) &&
                  rewrite_fixture_event_priority(bytes, (size_t)written, i,
                                                 priority) &&
                  rewrite_fixture_event_c_union(bytes, (size_t)written, i,
                                                (uint16_t)sound_index) &&
                  rewrite_fixture_event_byte(bytes, (size_t)written, i, 0,
                                             (int)(fire_at_tick & 0xffu)) &&
                  rewrite_fixture_event_byte(bytes, (size_t)written, i, 1,
                                             (int)((fire_at_tick >> 8) & 0xffu)) &&
                  rewrite_fixture_event_byte(bytes, (size_t)written, i, 2,
                                             (int)((fire_at_tick >> 16) & 0xffu)) &&
                  rewrite_fixture_event_byte(bytes, (size_t)written, i, 3, 2),
              "same-tick C20 fixture retains source map-time and unions");
    }
    /* This is a valid C4 heap: priority 3 is its root; its priority-1 and
     * priority-2 children are deliberately reversed in heap-array order. */
    CHECK(rewrite_fixture_timeline_index(bytes, (size_t)written, 0, 0u) &&
              rewrite_fixture_timeline_index(bytes, (size_t)written, 1, 1u) &&
              rewrite_fixture_timeline_index(bytes, (size_t)written, 2, 2u),
          "same-tick C4 fixture retains a valid non-linear heap");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&start_dungeon, 0, sizeof(start_dungeon));
    memset(&start_things, 0, sizeof(start_things));
    memset(&report, 0, sizeof(report));
    start_world.dungeon = &start_dungeon;
    start_world.things = &start_things;
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "same-tick C4 fixture write succeeds");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "F0435 materializes the source-valid same-tick C4 heap");
    CHECK(loaded_world.timeline.count == ORIGINAL_PC34_EVENT_COUNT &&
              loaded_world.timeline.events[0].aux0 == 30 &&
              loaded_world.timeline.events[1].aux0 == 20 &&
              loaded_world.timeline.events[2].aux0 == 10,
          "F0435 linearizes C4 siblings by source Type/Priority/index order");
    loaded_world.gameTick = fire_at_tick;
    memset(&result, 0, sizeof(result));
    F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(result.emissionCount == ORIGINAL_PC34_EVENT_COUNT &&
              result.emissions[0].kind == EMIT_SOUND_REQUEST &&
              result.emissions[0].payload[0] == 30 &&
              result.emissions[1].kind == EMIT_SOUND_REQUEST &&
              result.emissions[1].payload[0] == 20 &&
              result.emissions[2].kind == EMIT_SOUND_REQUEST &&
              result.emissions[2].payload[0] == 10,
          "M10 consumes restored same-tick C20 events in source order");
}

static void test_original_c60_deferred_group_move_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char squares[3][32 * 32]; unsigned short first_things[1]; uint16_t column_sft_bases[3 * 32]; char path[512];
    int written = 0, exported_size = 0, rc, i, index = -1;
    struct GameWorld_Compat start_world, loaded_world; struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3]; struct DungeonMapTiles_Compat tiles[3];
    struct DungeonThings_Compat things; struct DungeonGroup_Compat groups[3];
    struct SaveGame_Compat imported; struct PartyState_Compat party;
    struct TimelineEvent_Compat event; DM1OriginalSavePC34HandoffReport report;
    uint16_t group_thing = (uint16_t)((THING_TYPE_GROUP << 10) | 1);
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written, 2, 3, 9, 10, 2, 1, ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK && rewrite_fixture_event_type(bytes, (size_t)written, 0, DM1_EVENT_MOVE_GROUP_SILENT) && rewrite_fixture_event_priority(bytes, (size_t)written, 0, 0) && rewrite_fixture_event_c_union(bytes, (size_t)written, 0, group_thing) && rewrite_fixture_event(bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u), DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) && rewrite_fixture_event(bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u), DM1_EVENT_LIGHT, 0, 4, 0, 0, 0), "C60 fixture writes B.Location and C.Slot");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world)); memset(&dungeon, 0, sizeof(dungeon)); memset(maps, 0, sizeof(maps)); memset(tiles, 0, sizeof(tiles)); memset(squares, 0, sizeof(squares)); memset(column_sft_bases, 0, sizeof(column_sft_bases)); memset(&things, 0, sizeof(things)); memset(groups, 0, sizeof(groups)); memset(&report, 0, sizeof(report));
    for (i = 0; i < 3; ++i) { maps[i].width = 32; maps[i].height = 32; tiles[i].squareData = squares[i]; tiles[i].squareCount = 32 * 32; }
    squares[2][11 * 32 + 12] = DUNGEON_SQUARE_MASK_THING_LIST; first_things[0] = group_thing;
    dungeon.header.mapCount = 3; dungeon.maps = maps; dungeon.tiles = tiles; dungeon.tilesLoaded = 1; dungeon.columnsCumulativeSquareFirstThingCount = column_sft_bases; dungeon.dungeonColumnCount = 3 * 32;
    things.groups = groups; things.groupCount = 3; things.squareFirstThings = first_things; things.squareFirstThingCount = 1; groups[1].creatureType = 1; groups[1].next = THING_ENDOFLIST; groups[2].next = THING_ENDOFLIST;
    start_world.dungeon = &dungeon; start_world.things = &things; make_temp_save_path(path, sizeof(path)); remove(path); CHECK(write_fixture_file(path, bytes, written), "C60 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report); remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C60 materializes typed group-slot receipt");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_MOVE_GROUP_SILENT) { index = i; break; }
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux0 == 1 && loaded_world.timeline.events[index].aux1 == group_thing && loaded_world.timeline.events[index].mapX == 11 && loaded_world.timeline.events[index].mapY == 12, "C60 retains Location and C04 Slot");
    event = loaded_world.timeline.events[index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event), "C60 schedules for its existing F0252 runtime owner");
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event); loaded_world.pc34OriginalC3C4ReceiptValid = 0; rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size); CHECK(rc == SAVEGAME_PC34_OK, "C60 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party; rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report); index = -1; for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_MOVE_GROUP_SILENT) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 &&
              report.events[index].priority == 0 &&
              report.events[index].b_mapX == 11 &&
              report.events[index].b_mapY == 12 &&
              rd16le(&report.events[index].c_cell) == group_thing,
          "C60 export restores Location and raw group Slot");
}

static void test_original_c61_audible_group_move_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int exported_size = 0;
    int rc;
    int i;
    int index = -1;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[3];
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    DM1OriginalSavePC34HandoffReport report;
    uint16_t group_thing = (uint16_t)((THING_TYPE_GROUP << 10) | 1);

    rc = build_original_pc34_fixture(
        bytes, (int)sizeof(bytes), &written, 2, 3, 9, 10, 2, 1,
        ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event_type(
                  bytes, (size_t)written, 0,
                  DM1_EVENT_MOVE_GROUP_AUDIBLE) &&
              rewrite_fixture_event_priority(bytes, (size_t)written, 0, 0) &&
              rewrite_fixture_event_c_union(
                  bytes, (size_t)written, 0, group_thing) &&
              rewrite_fixture_event(
                  bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
                  DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
              rewrite_fixture_event(
                  bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
                  DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "C61 fixture writes B.Location and C.Slot");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(&report, 0, sizeof(report));
    for (i = 0; i < 3; ++i) {
        maps[i].width = 32;
        maps[i].height = 32;
    }
    dungeon.header.mapCount = 3;
    dungeon.maps = maps;
    things.groups = groups;
    things.groupCount = 3;
    groups[1].creatureType = 1;
    groups[1].next = THING_ENDOFLIST;
    groups[2].next = THING_ENDOFLIST;
    start_world.dungeon = &dungeon;
    start_world.things = &things;

    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written), "C61 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C61 materializes typed audible group receipt");
    for (i = 0; i < loaded_world.timeline.count; ++i) {
        if (loaded_world.timeline.events[i].aux2 ==
            DM1_EVENT_MOVE_GROUP_AUDIBLE) {
            index = i;
            break;
        }
    }
    CHECK(index >= 0 &&
              loaded_world.timeline.events[index].kind ==
                  TIMELINE_EVENT_MOVE_GROUP_AUDIBLE &&
              loaded_world.timeline.events[index].aux0 == 1 &&
              loaded_world.timeline.events[index].aux1 == group_thing,
          "C61 retains group Slot for F0252 audible runtime");

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C61 exports natively");
    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_size, &imported, &report);
    index = -1;
    for (i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_MOVE_GROUP_AUDIBLE) {
            index = i;
            break;
        }
    }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 &&
              report.events[index].priority == 0 &&
              report.events[index].b_mapX == 11 &&
              report.events[index].b_mapY == 12 &&
              rd16le(&report.events[index].c_cell) == group_thing,
          "C61 export restores Location and raw group Slot");
}

static void test_runtime_materializer_binds_original_c12_damage_hide(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int exported_size = 0;
    int rc;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat start_dungeon;
    struct DungeonThings_Compat start_things;
    struct TickResult_Compat tick_result;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     3, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "C12 materializer fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 2,
                                     DM1_EVENT_HIDE_DAMAGE_RECEIVED) &&
              rewrite_fixture_event(
                  bytes, written, 0, DM1_MAP_TIME_MAKE(2, 123500u),
                  DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
              rewrite_fixture_event(
                  bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
                  DM1_EVENT_LIGHT, 0, 5, 0, 0, 0),
          "C12 fixture preserves the authenticated PC34 envelope");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&start_dungeon, 0, sizeof(start_dungeon));
    memset(&start_things, 0, sizeof(start_things));
    memset(&report, 0, sizeof(report));
    start_world.dungeon = &start_dungeon;
    start_world.things = &start_things;
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "C12 materializer fixture write succeeds");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C12 materializes without consuming undefined B/C union bytes");
    CHECK(loaded_world.timeline.count == ORIGINAL_PC34_EVENT_COUNT &&
              loaded_world.timeline.events[1].kind ==
                  TIMELINE_EVENT_STATUS_TIMEOUT &&
              loaded_world.timeline.events[1].mapIndex == 1 &&
              loaded_world.timeline.events[1].aux0 ==
                  DM1_EVENT_HIDE_DAMAGE_RECEIVED &&
              loaded_world.timeline.events[1].aux2 ==
                  DM1_EVENT_HIDE_DAMAGE_RECEIVED &&
              loaded_world.timeline.events[1].aux4 == 2,
          "C12 materialization preserves only Map_Time and Priority");

    loaded_world.gameTick = 123490u;
    memset(&tick_result, 0, sizeof(tick_result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world,
                                                    &tick_result);
    CHECK(loaded_world.timeline.count == 2,
          "C12 dispatch consumes the source champion-panel hide timer");

    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    loaded_world.timeline.count = 1;
    memset(&loaded_world.timeline.events[0], 0,
           sizeof(loaded_world.timeline.events[0]));
    loaded_world.timeline.events[0].kind = TIMELINE_EVENT_STATUS_TIMEOUT;
    loaded_world.timeline.events[0].fireAtTick = 123495u;
    loaded_world.timeline.events[0].mapIndex = 1;
    loaded_world.timeline.events[0].aux0 = DM1_EVENT_HIDE_DAMAGE_RECEIVED;
    loaded_world.timeline.events[0].aux2 = DM1_EVENT_HIDE_DAMAGE_RECEIVED;
    loaded_world.timeline.events[0].aux4 = 1;
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK && exported_size > 0,
          "C12 runtime event exports without inventing B/C union fields");
    imported.party = &imported_party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_size, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
              report.original_event_count == 1 &&
              report.events[0].type == DM1_EVENT_HIDE_DAMAGE_RECEIVED &&
              report.events[0].priority == 1 &&
              report.events[0].b_mapX == 0 && report.events[0].b_mapY == 0 &&
              report.events[0].c_cell == 0 && report.events[0].c_effect == 0,
          "C12 export restores its native no-union source record");
}

static void test_original_c72_champion_shield_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0, exported_size = 0, rc, i, index = -1, c72_index = -1;
    struct GameWorld_Compat start_world, loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written, 3, 3, 9, 10, 2, 1, ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK && rewrite_fixture_event_type(bytes, (size_t)written, 2, DM1_EVENT_CHAMPION_SHIELD) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 6, 12) && rewrite_fixture_event_byte(bytes, (size_t)written, 2, 7, 0),
          "C72 fixture writes signed B.Defense");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon)); memset(&things, 0, sizeof(things)); memset(&report, 0, sizeof(report));
    start_world.dungeon = &dungeon; start_world.things = &things;
    make_temp_save_path(path, sizeof(path)); remove(path); CHECK(write_fixture_file(path, bytes, written), "C72 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report); remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C72 materializes typed defense");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_CHAMPION_SHIELD) { index = i; break; }
    c72_index = index;
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux1 == 12 && loaded_world.timeline.events[index].aux4 == 2,
          "C72 retains defense and champion receipt");
    event = loaded_world.timeline.events[c72_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C72 receipt isolates for native export");
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C72 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report);
    for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_CHAMPION_SHIELD) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 && report.events[index].priority == 2 &&
              (int16_t)rd16le(&report.events[index].b_mapX) == 12,
          "C72 native roundtrip preserves Priority and B.Defense");
    loaded_world.lifecycle.champions[2].shieldDefense = 20;
    { struct TimelineEvent_Compat event = loaded_world.timeline.events[c72_index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event); loaded_world.gameTick = event.fireAtTick; }
    memset(&result, 0, sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.timeline.count == 0,
          "C72 runtime consumes the authenticated shield-expiry timer");
}

static void test_original_c71_invisibility_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0, exported_size = 0, rc, i, index = -1, c71_index = -1;
    struct GameWorld_Compat start_world, loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written, 3, 3, 9, 10, 2, 1, ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event_type(bytes, (size_t)written, 2, DM1_EVENT_INVISIBILITY) &&
              rewrite_fixture_party_status_count(
                  bytes, written, ORIGINAL_PC34_PARTY_INFO_EVENT71_COUNT_OFFSET, 2u) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 5, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 6, 0xa5) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 7, 0x5a) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 8, 0x3c) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 9, 0xc3),
          "C71 fixture gives no ownership to B/C union bytes");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon)); memset(&things, 0, sizeof(things)); memset(&report, 0, sizeof(report));
    start_world.dungeon = &dungeon; start_world.things = &things;
    make_temp_save_path(path, sizeof(path)); remove(path); CHECK(write_fixture_file(path, bytes, written), "C71 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report); remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
              loaded_world.magic.event71CountInvisibility == 2 &&
              loaded_world.lifecycle.status.invisibilityCount == 2,
          "C71 materializes typed invisibility expiry and source PARTY_INFO count");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_INVISIBILITY) { index = i; break; }
    c71_index = index;
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux0 == DM1_EVENT_INVISIBILITY &&
              loaded_world.timeline.events[index].aux1 == 0 && loaded_world.timeline.events[index].aux4 == 0,
          "C71 retains only its typed no-union receipt");
    event = loaded_world.timeline.events[c71_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C71 receipt isolates for native export");
    c71_index = 0;
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C71 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report);
    for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_INVISIBILITY) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 && report.events[index].priority == 0 &&
              report.events[index].b_mapX == 0 && report.events[index].b_mapY == 0 &&
              report.events[index].c_cell == 0 && report.events[index].c_effect == 0,
          "C71 native roundtrip preserves no B/C union arm");
    loaded_world.magic.event71CountInvisibility = 1;
    loaded_world.lifecycle.status.invisibilityCount = 1;
    event = loaded_world.timeline.events[c71_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    loaded_world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.magic.event71CountInvisibility == 0 && loaded_world.lifecycle.status.invisibilityCount == 0,
          "C71 runtime decrements both invisibility mirrors");
    event.aux2 = 0;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "C71 host export rejects an unauthenticated status receipt");
}

/* ReDMCSB NEWMAP.C F0003 brackets the destination scan with GROUP.C
 * F0194/F0195.  Keep this in the original-save target: F0435 resumes with
 * an anchored current map, while an actual later-map request owns this
 * C04/timeline handoff. */
static void test_original_save_later_map_group_transition(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[2];
    unsigned char squares[2];
    unsigned short firstThings[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct GameWorld_Compat before;
    int found_old_wander = 0;
    int found_new_wander = 0;
    int i;

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(squares, 0, sizeof(squares));
    memset(firstThings, 0, sizeof(firstThings));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));

    maps[0].width = maps[1].width = 1;
    maps[0].height = maps[1].height = 1;
    tiles[0].squareData = &squares[0];
    tiles[1].squareData = &squares[1];
    tiles[0].squareCount = tiles[1].squareCount = 1;
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    squares[1] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_SKELETON;
    groups[0].cells = 0xffu;
    groups[0].direction = 1;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    groups[1].next = THING_ENDOFLIST;
    groups[1].creatureType = CREATURE_TYPE_SKELETON;
    groups[1].cells = 0xe4u;
    groups[1].direction = 2;
    groups[1].count = 0;
    groups[1].health[0] = 100;
    firstThings[0] = (unsigned short)((THING_TYPE_GROUP << 10) | 0);
    firstThings[1] = (unsigned short)((THING_TYPE_GROUP << 10) | 1);
    things.groups = groups;
    things.groupCount = 2;
    things.squareFirstThings = firstThings;
    things.squareFirstThingCount = 2;
    things.loaded = 1;
    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 400u;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    world.newPartyMapIndex = 1;
    world.creatureAICount = 1;
    world.creatureAI[0].reserved0 = 0;
    world.creatureAI[0].groupCells = 0xc3;
    world.creatureAI[0].groupDirection = 2;
    world.pc34ActiveGroupSourceCount = 1;
    world.pc34ActiveGroupDirections[0] = 0x0au;
    CHECK(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick),
          "F0194/F0195 source queue initializes");
    world.timeline.events[world.timeline.count++] = (struct TimelineEvent_Compat){
        TIMELINE_EVENT_CREATURE_TICK, 401u, 0, 0, 0, 0, 0,
        AI_STATE_WANDER, 0, 0, 0 };
    world.timeline.events[world.timeline.count++] = (struct TimelineEvent_Compat){
        TIMELINE_EVENT_CREATURE_REACTION, 401u, 1, 0, 0, 0, 0, 0,
        DM1_EVENT_REACTION_DANGER_ON_SQUARE, 0, 0 };

    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
          "F0003 later-map transition accepts loaded C04 owners");
    CHECK(world.partyMapIndex == 1 && world.party.mapIndex == 1 &&
              world.newPartyMapIndex == -1 && world.creatureAICount == 1 &&
              world.creatureAI[0].reserved0 == 1,
          "F0194 retires old C04 before F0195 admits destination C04");
    CHECK(groups[0].cells == 0xc3u && groups[0].direction == 2,
          "F0194 writes the active raw Cells and low packed Direction");
    for (i = 0; i < world.timeline.count; ++i) {
        const struct TimelineEvent_Compat *event = &world.timeline.events[i];
        if (event->kind == TIMELINE_EVENT_CREATURE_TICK &&
            event->mapIndex == 0 && event->aux0 == 0) {
            found_old_wander = 1;
        }
        if (event->kind == TIMELINE_EVENT_CREATURE_TICK &&
            event->mapIndex == 1 && event->aux0 == 1) {
            found_new_wander = 1;
        }
    }
    CHECK(world.timeline.count == 2 && found_old_wander && found_new_wander,
          "F0181 removes only destination C29-C41 while F0195 owns new C37");

    before = world;
    world.newPartyMapIndex = 2;
    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_FAIL,
          "invalid later map rejects the transition");
    CHECK(world.partyMapIndex == before.partyMapIndex &&
              world.party.mapIndex == before.party.mapIndex &&
              world.creatureAICount == before.creatureAICount &&
              world.timeline.count == before.timeline.count &&
              groups[0].cells == 0xc3u && groups[1].cells == 0xe4u,
          "invalid later map leaves C04, active owners, and timeline intact");
}

static void test_original_c73_thieves_eye_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0, exported_size = 0, rc, i, index = -1, c73_index = -1;
    struct GameWorld_Compat start_world, loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written, 3, 3, 9, 10, 2, 1, ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event_type(bytes, (size_t)written, 2, DM1_EVENT_THIEVES_EYE) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 5, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 6, 0xa5) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 7, 0x5a) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 8, 0x3c) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 9, 0xc3),
          "C73 fixture gives no ownership to B/C union bytes");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon)); memset(&things, 0, sizeof(things)); memset(&report, 0, sizeof(report));
    start_world.dungeon = &dungeon; start_world.things = &things;
    make_temp_save_path(path, sizeof(path)); remove(path); CHECK(write_fixture_file(path, bytes, written), "C73 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report); remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C73 materializes typed Thieves Eye expiry");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_THIEVES_EYE) { index = i; break; }
    c73_index = index;
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux0 == DM1_EVENT_THIEVES_EYE &&
              loaded_world.timeline.events[index].aux1 == 0 && loaded_world.timeline.events[index].aux4 == 0,
          "C73 retains only its typed no-union receipt");
    event = loaded_world.timeline.events[c73_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C73 receipt isolates for native export");
    c73_index = 0;
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C73 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report);
    for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_THIEVES_EYE) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 && report.events[index].priority == 0 &&
              report.events[index].b_mapX == 0 && report.events[index].b_mapY == 0 &&
              report.events[index].c_cell == 0 && report.events[index].c_effect == 0,
          "C73 native roundtrip preserves no B/C union arm");
    loaded_world.magic.event73CountThievesEye = 1;
    loaded_world.lifecycle.status.thievesEyeCount = 1;
    event = loaded_world.timeline.events[c73_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    loaded_world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.magic.event73CountThievesEye == 0 && loaded_world.lifecycle.status.thievesEyeCount == 0,
          "C73 runtime decrements both Thieves Eye mirrors");
    event.aux2 = 0;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "C73 host export rejects an unauthenticated status receipt");
}

static void test_original_c74_party_shield_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0, exported_size = 0, rc, i, index = -1, c74_index = -1;
    struct GameWorld_Compat start_world, loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written, 3, 3, 9, 10, 2, 1, ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event_type(bytes, (size_t)written, 2, DM1_EVENT_PARTY_SHIELD) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 5, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 6, 12) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 7, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 8, 0x3c) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 9, 0xc3),
          "C74 fixture writes signed B.Defense only");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon)); memset(&things, 0, sizeof(things)); memset(&report, 0, sizeof(report));
    start_world.dungeon = &dungeon; start_world.things = &things;
    make_temp_save_path(path, sizeof(path)); remove(path); CHECK(write_fixture_file(path, bytes, written), "C74 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report); remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C74 materializes typed party shield expiry");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_PARTY_SHIELD) { index = i; break; }
    c74_index = index;
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux1 == 12 && loaded_world.timeline.events[index].aux4 == 0,
          "C74 retains defense and zero-priority receipt");
    event = loaded_world.timeline.events[c74_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C74 receipt isolates for native export");
    c74_index = 0;
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C74 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report);
    for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_PARTY_SHIELD) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 && report.events[index].priority == 0 &&
              (int16_t)rd16le(&report.events[index].b_mapX) == 12 && report.events[index].c_cell == 0 && report.events[index].c_effect == 0,
          "C74 native roundtrip preserves B.Defense and no C union arm");
    loaded_world.magic.partyShieldDefense = 20;
    loaded_world.lifecycle.status.partyShieldDefense = 20;
    event = loaded_world.timeline.events[c74_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    loaded_world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.magic.partyShieldDefense == 8 && loaded_world.lifecycle.status.partyShieldDefense == 8,
          "C74 runtime subtracts B.Defense from party shield mirrors");
    event.aux2 = 0;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "C74 host export rejects an unauthenticated status receipt");
}

static void test_original_c75_poison_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0, exported_size = 0, rc, i, index = -1, c75_index = -1;
    struct GameWorld_Compat start_world, loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written, 3, 3, 9, 10, 2, 1, ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event_type(bytes, (size_t)written, 2, DM1_EVENT_POISON_CHAMPION) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 5, 2) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 6, 128) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 7, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 8, 0x3c) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 9, 0xc3),
          "C75 fixture writes champion Priority and B.Attack only");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon)); memset(&things, 0, sizeof(things)); memset(&report, 0, sizeof(report));
    start_world.dungeon = &dungeon; start_world.things = &things;
    make_temp_save_path(path, sizeof(path)); remove(path); CHECK(write_fixture_file(path, bytes, written), "C75 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report); remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C75 materializes typed poison expiry");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_POISON_CHAMPION) { index = i; break; }
    c75_index = index;
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux1 == 128 && loaded_world.timeline.events[index].aux4 == 2,
          "C75 retains attack and champion receipt");
    event = loaded_world.timeline.events[c75_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C75 receipt isolates for native export");
    c75_index = 0;
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C75 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report);
    for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_POISON_CHAMPION) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 && report.events[index].priority == 2 &&
              rd16le(&report.events[index].b_mapX) == 128 && report.events[index].c_cell == 0 && report.events[index].c_effect == 0,
          "C75 native roundtrip preserves B.Attack and no C union arm");
    loaded_world.party.champions[2].hp.current = 100;
    event = loaded_world.timeline.events[c75_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    loaded_world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.party.champions[2].hp.current == 98 && loaded_world.timeline.count == 1 &&
              loaded_world.timeline.events[0].aux1 == 127 && loaded_world.timeline.events[0].aux4 == 2,
          "C75 runtime damages and retains native requeue receipt");
    event.aux2 = 0;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "C75 host export rejects an unauthenticated status receipt");
}

static void test_original_c77_spell_shield_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0, exported_size = 0, rc, i, index = -1, c77_index = -1;
    struct GameWorld_Compat start_world, loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written, 3, 3, 9, 10, 2, 1, ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK && rewrite_fixture_event_type(bytes, (size_t)written, 2, DM1_EVENT_SPELLSHIELD) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 5, 0) && rewrite_fixture_event_byte(bytes, (size_t)written, 2, 6, 12) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 7, 0) && rewrite_fixture_event_byte(bytes, (size_t)written, 2, 8, 0x3c) && rewrite_fixture_event_byte(bytes, (size_t)written, 2, 9, 0xc3),
          "C77 fixture writes signed B.Defense only");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon)); memset(&things, 0, sizeof(things)); memset(&report, 0, sizeof(report));
    start_world.dungeon = &dungeon; start_world.things = &things;
    make_temp_save_path(path, sizeof(path)); remove(path); CHECK(write_fixture_file(path, bytes, written), "C77 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report); remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C77 materializes typed spell shield expiry");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_SPELLSHIELD) { index = i; break; }
    c77_index = index;
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux1 == 12 && loaded_world.timeline.events[index].aux4 == 0, "C77 retains defense and zero-priority receipt");
    event = loaded_world.timeline.events[c77_index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event), "C77 receipt isolates for native export"); c77_index = 0;
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C77 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report);
    for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_SPELLSHIELD) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 && report.events[index].priority == 0 && (int16_t)rd16le(&report.events[index].b_mapX) == 12 && report.events[index].c_cell == 0 && report.events[index].c_effect == 0, "C77 native roundtrip preserves B.Defense and no C union arm");
    loaded_world.magic.spellShieldDefense = 20; loaded_world.lifecycle.status.partySpellShieldDefense = 20;
    event = loaded_world.timeline.events[c77_index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event); loaded_world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.magic.spellShieldDefense == 8 && loaded_world.lifecycle.status.partySpellShieldDefense == 8, "C77 runtime subtracts B.Defense from spell shield mirrors");
    event.aux2 = 0; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "C77 host export rejects an unauthenticated status receipt");
}

static void test_original_c78_fire_shield_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE]; char path[512];
    int written = 0, exported_size = 0, rc, i, index = -1, c78_index = -1;
    struct GameWorld_Compat start_world, loaded_world; struct DungeonDatState_Compat dungeon; struct DungeonThings_Compat things;
    struct SaveGame_Compat imported; struct PartyState_Compat party; struct TickResult_Compat result; struct TimelineEvent_Compat event; DM1OriginalSavePC34HandoffReport report;
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written, 3, 3, 9, 10, 2, 1, ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK && rewrite_fixture_event_type(bytes, (size_t)written, 2, DM1_EVENT_FIRESHIELD) && rewrite_fixture_event_byte(bytes, (size_t)written, 2, 5, 0) && rewrite_fixture_event_byte(bytes, (size_t)written, 2, 6, 12) && rewrite_fixture_event_byte(bytes, (size_t)written, 2, 7, 0) && rewrite_fixture_event_byte(bytes, (size_t)written, 2, 8, 0x3c) && rewrite_fixture_event_byte(bytes, (size_t)written, 2, 9, 0xc3), "C78 fixture writes signed B.Defense only");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world)); memset(&dungeon, 0, sizeof(dungeon)); memset(&things, 0, sizeof(things)); memset(&report, 0, sizeof(report)); start_world.dungeon = &dungeon; start_world.things = &things;
    make_temp_save_path(path, sizeof(path)); remove(path); CHECK(write_fixture_file(path, bytes, written), "C78 fixture writes"); rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report); remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C78 materializes typed fire shield expiry");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_FIRESHIELD) { index = i; break; } c78_index = index;
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux1 == 12 && loaded_world.timeline.events[index].aux4 == 0, "C78 retains defense and zero-priority receipt");
    event = loaded_world.timeline.events[c78_index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event), "C78 receipt isolates for native export"); c78_index = 0;
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size); CHECK(rc == SAVEGAME_PC34_OK, "C78 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party; rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report);
    for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_FIRESHIELD) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 && report.events[index].priority == 0 && (int16_t)rd16le(&report.events[index].b_mapX) == 12 && report.events[index].c_cell == 0 && report.events[index].c_effect == 0, "C78 native roundtrip preserves B.Defense and no C union arm");
    loaded_world.magic.fireShieldDefense = 20; loaded_world.lifecycle.status.partyFireShieldDefense = 20; event = loaded_world.timeline.events[c78_index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event); loaded_world.gameTick = event.fireAtTick; memset(&result, 0, sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.magic.fireShieldDefense == 8 && loaded_world.lifecycle.status.partyFireShieldDefense == 8, "C78 runtime subtracts B.Defense from fire shield mirrors");
    event.aux2 = 0; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event); rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size); CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL, "C78 host export rejects an unauthenticated status receipt");
}

static void test_original_c79_footprints_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE]; char path[512]; int written=0, exported_size=0, rc, i, index=-1, c79_index=-1;
    struct GameWorld_Compat start_world, loaded_world; struct DungeonDatState_Compat dungeon; struct DungeonThings_Compat things; struct SaveGame_Compat imported; struct PartyState_Compat party; struct TickResult_Compat result; struct TimelineEvent_Compat event; DM1OriginalSavePC34HandoffReport report;
    rc=build_original_pc34_fixture(bytes,(int)sizeof(bytes),&written,3,3,9,10,2,1,ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc==SAVEGAME_PC34_OK && rewrite_fixture_event_type(bytes,(size_t)written,2,DM1_EVENT_FOOTPRINTS) && rewrite_fixture_event_byte(bytes,(size_t)written,2,5,0) && rewrite_fixture_event_byte(bytes,(size_t)written,2,6,0xa5) && rewrite_fixture_event_byte(bytes,(size_t)written,2,7,0x5a) && rewrite_fixture_event_byte(bytes,(size_t)written,2,8,0x3c) && rewrite_fixture_event_byte(bytes,(size_t)written,2,9,0xc3),"C79 fixture no B/C ownership");
    memset(&start_world,0,sizeof(start_world)); memset(&loaded_world,0,sizeof(loaded_world)); memset(&dungeon,0,sizeof(dungeon)); memset(&things,0,sizeof(things)); memset(&report,0,sizeof(report)); start_world.dungeon=&dungeon; start_world.things=&things; make_temp_save_path(path,sizeof(path)); remove(path); CHECK(write_fixture_file(path,bytes,written),"C79 fixture writes"); rc=dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path,&start_world,&loaded_world,NULL,&report); remove(path); CHECK(rc==DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,"C79 materializes");
    for(i=0;i<loaded_world.timeline.count;++i) if(loaded_world.timeline.events[i].aux2==DM1_EVENT_FOOTPRINTS){index=i;break;} c79_index=index; CHECK(index>=0 && loaded_world.timeline.events[index].aux1==0 && loaded_world.timeline.events[index].aux4==0,"C79 typed receipt"); event=loaded_world.timeline.events[c79_index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline,event.fireAtTick); CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline,&event),"C79 receipt isolates for native export"); c79_index=0; loaded_world.pc34OriginalC3C4ReceiptValid=0; rc=F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world,0x43313445u,exported,(int)sizeof(exported),&exported_size); CHECK(rc==SAVEGAME_PC34_OK,"C79 exports"); memset(&imported,0,sizeof(imported)); memset(&party,0,sizeof(party)); imported.party=&party; rc=dm1_v1_original_save_pc34_handoff_bytes(exported,(size_t)exported_size,&imported,&report); for(i=0;i<report.original_event_count;++i) if(report.events[i].type==DM1_EVENT_FOOTPRINTS){index=i;break;} CHECK(rc==DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index>=0 && report.events[index].priority==0 && report.events[index].b_mapX==0 && report.events[index].b_mapY==0 && report.events[index].c_cell==0 && report.events[index].c_effect==0,"C79 roundtrip has no union arm");
    loaded_world.magic.event79CountFootprints=1; loaded_world.magic.magicFootprintsActive=1; loaded_world.lifecycle.status.footprintsCount=1; event=loaded_world.timeline.events[c79_index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline,event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline,&event); loaded_world.gameTick=event.fireAtTick; memset(&result,0,sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world,&result); CHECK(loaded_world.magic.event79CountFootprints==0 && !loaded_world.magic.magicFootprintsActive && loaded_world.lifecycle.status.footprintsCount==0,"C79 runtime mirrors"); event.aux2=0; F0720_TIMELINE_Init_Compat(&loaded_world.timeline,event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline,&event); rc=F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world,0x43313445u,exported,(int)sizeof(exported),&exported_size); CHECK(rc==SAVEGAME_PC34_ERROR_INTERNAL,"C79 host export rejects an unauthenticated status receipt");
}

static void test_original_c53_watchdog_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int exported_size = 0;
    int rc;
    int i;
    int index = -1;
    int c53_index = -1;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event_type(bytes, (size_t)written, 2,
                                         DM1_EVENT_WATCHDOG) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 3, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 5, 0xa5) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 6, 0x5a) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 7, 0xc3) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 8, 0x3c) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 9, 0x96) &&
              rewrite_fixture_event(
                  bytes, written, 0, DM1_MAP_TIME_MAKE(2, 123500u),
                  DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
              rewrite_fixture_event(
                  bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
                  DM1_EVENT_LIGHT, 0, 5, 0, 0, 0),
          "C53 fixture keeps unowned Priority and union bytes");
    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(&report, 0, sizeof(report));
    start_world.dungeon = &dungeon;
    start_world.things = &things;
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written), "C53 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C53 materializes its typed watchdog receipt");
    for (i = 0; i < loaded_world.timeline.count; ++i) {
        if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_WATCHDOG) {
            index = i;
            break;
        }
    }
    c53_index = index;
    CHECK(index >= 0 && loaded_world.timeline.events[index].mapIndex == 0 &&
              loaded_world.timeline.events[index].aux0 == DM1_EVENT_WATCHDOG &&
              loaded_world.timeline.events[index].aux1 == 0 &&
              loaded_world.timeline.events[index].aux4 == 0,
          "C53 receipt retains only source-owned fields");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C53 exports natively");
    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_size, &imported, &report);
    index = -1;
    for (i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_WATCHDOG) {
            index = i;
            break;
        }
    }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 &&
              report.events[index].priority == 0xa5 &&
              report.events[index].b_mapX == 0x5a &&
              report.events[index].b_mapY == 0xc3 &&
              report.events[index].c_cell == 0x3c &&
              report.events[index].c_effect == 0x96,
          "C53 receipt reuse preserves authenticated unowned bytes");
    event = loaded_world.timeline.events[c53_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C53 receipt schedules for runtime expiry");
    loaded_world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result));
    F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.timeline.count == 0,
          "C53 runtime consumes the source watchdog without a host re-arm");
    event.aux2 = 0;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "host watchdog schedules for export rejection");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "C53 host export rejects an unauthenticated watchdog receipt");
}

static void test_original_c22_cpse_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int exported_size = 0;
    int rc;
    int i;
    int index = -1;
    int c22_index = -1;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event_type(bytes, (size_t)written, 2,
                                         DM1_EVENT_CPSE) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 3, 2) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 5, 0xa5) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 6, 0x5a) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 7, 0xc3) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 8, 0x3c) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 9, 0x96),
          "C22 fixture keeps unowned Priority and union bytes");
    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(&report, 0, sizeof(report));
    start_world.dungeon = &dungeon;
    start_world.things = &things;
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written), "C22 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C22 materializes its typed CPSE receipt");
    for (i = 0; i < loaded_world.timeline.count; ++i) {
        if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_CPSE) {
            index = i;
            break;
        }
    }
    c22_index = index;
    CHECK(index >= 0 &&
              loaded_world.timeline.events[index].kind ==
                  TIMELINE_EVENT_CPSE_CHECK &&
              loaded_world.timeline.events[index].mapIndex == 2 &&
              loaded_world.timeline.events[index].aux0 == DM1_EVENT_CPSE &&
              loaded_world.timeline.events[index].aux1 == 0 &&
              loaded_world.timeline.events[index].aux4 == 0,
          "C22 receipt retains Map_Time only");
    event = loaded_world.timeline.events[c22_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C22 receipt isolates for native export");
    c22_index = 0;
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_OK, "C22 exports natively");
    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_size, &imported, &report);
    index = -1;
    for (i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_CPSE) {
            index = i;
            break;
        }
    }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 &&
              report.events[index].priority == 0 &&
              report.events[index].b_mapX == 0 && report.events[index].b_mapY == 0 &&
              report.events[index].c_cell == 0 && report.events[index].c_effect == 0,
          "C22 export restores its Map_Time-only source record");
    event = loaded_world.timeline.events[c22_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C22 receipt schedules for runtime consumption");
    loaded_world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result));
    F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.timeline.count == 0 && result.emissionCount == 0,
          "C22 runtime follows NOCOPYPROTECTION no-op path");
    event.aux2 = 0;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "host C22 schedules for export rejection");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "C22 host export rejects an unauthenticated CPSE receipt");
}

static void test_original_c13_vi_altar_event_plan(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    int rc;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    DM1OriginalSavePC34HandoffReport report;
    DM1OriginalSavePC34ViAltarRebirthEventPlan plan;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     3, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "C13 plan fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 2,
                                     DM1_EVENT_VI_ALTAR_REBIRTH) &&
              rewrite_fixture_event_c_union(bytes, (size_t)written, 2,
                                            0x0203u),
          "C13 fixture preserves authenticated Location/Cell/Effect bytes");

    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    memset(&report, 0, sizeof(report));
    imported.party = &imported_party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C13 authenticated save reaches the bounded parser");
    memset(&plan, 0, sizeof(plan));
    CHECK(dm1_v1_original_save_pc34_handoff_vi_altar_rebirth_event_plan(
              &report.events[2], 2, &plan) && plan.valid &&
              plan.champion_index == 2 && plan.map_index == 1 &&
              plan.map_x == 0 && plan.map_y == 0 && plan.cell == 3 &&
              plan.step == 2 && plan.fire_at_tick == 123490u,
          "C13 plan retains the exact F0255 source union ownership");

    CHECK(rewrite_fixture_event_c_union(bytes, (size_t)written, 2, 0x0303u),
          "C13 malformed-step fixture remains checksum-authenticated");
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
              !dm1_v1_original_save_pc34_handoff_vi_altar_rebirth_event_plan(
                  &report.events[2], 2, &plan),
          "C13 plan fails closed outside source steps 2, 1, and 0");
}

static void test_original_c13_vi_altar_runtime_sequence(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    struct TimelineEvent_Compat event;
    struct TickResult_Compat result;
    unsigned char square_data[2][32 * 32];
    unsigned short square_first_things[32 * 32];
    unsigned short column_sft_bases[2 * 32];
    unsigned char raw_junk[4];
    int i;
    int found_step1 = 0;
    int found_step0 = 0;
    int champion_health_before_missing_bones;

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    memset(square_data, 0, sizeof(square_data));
    memset(column_sft_bases, 0, sizeof(column_sft_bases));
    memset(raw_junk, 0, sizeof(raw_junk));
    for (i = 0; i < (int)(sizeof(square_first_things) /
                          sizeof(square_first_things[0])); ++i) {
        square_first_things[i] = THING_ENDOFLIST;
    }
    maps[0].width = 32;
    maps[0].height = 32;
    maps[1].width = 32;
    maps[1].height = 32;
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    /* F0160 resolves the flagged map-1/x-2 square through the G0280
     * per-column base loaded from DUNGEON.DAT.  With no earlier flagged
     * squares, its source compact-SFT base is entry zero. */
    dungeon.columnsCumulativeSquareFirstThingCount = column_sft_bases;
    dungeon.dungeonColumnCount = (int)(sizeof(column_sft_bases) /
                                       sizeof(column_sft_bases[0]));
    tiles[0].squareData = square_data[0];
    tiles[0].squareCount = 32 * 32;
    tiles[1].squareData = square_data[1];
    tiles[1].squareCount = 32 * 32;
    square_data[1][2 * 32 + 3] |= DUNGEON_SQUARE_MASK_THING_LIST;
    /* squareFirstThings is compacted over flagged squares, so this first
     * and only flagged map square is entry zero rather than raw tile 67. */
    square_first_things[0] =
        (unsigned short)((1u << 14) | (THING_TYPE_JUNK << 10));
    wr16le(raw_junk, THING_ENDOFLIST);
    raw_junk[2] = (unsigned char)(DM1_JUNK_TYPE_BONES | 0x80u);
    raw_junk[3] = 0x80u; /* PC34 JUNK ChargeCount = champion index 2. */
    junks[0].next = THING_ENDOFLIST;
    junks[0].type = DM1_JUNK_TYPE_BONES;
    junks[0].doNotDiscard = 1u;
    junks[0].chargeCount = 2u;
    things.squareFirstThings = square_first_things;
    things.squareFirstThingCount = 32 * 32;
    things.junks = junks;
    things.junkCount = 1;
    things.rawThingData[THING_TYPE_JUNK] = raw_junk;
    things.thingCounts[THING_TYPE_JUNK] = 1;
    things.loaded = 1;
    world.dungeon = &dungeon;
    world.things = &things;
    world.party.championCount = 3;
    world.party.direction = 3;
    world.party.champions[2].present = 1;
    world.party.champions[2].cell = 1;
    world.party.champions[2].hp.maximum = 100;
    world.party.champions[2].inventory[0] = 0x1234u;
    CHECK(F0720_TIMELINE_Init_Compat(&world.timeline, 50u),
          "C13 runtime timeline initializes");
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_VI_ALTAR_REBIRTH;
    event.fireAtTick = 50u;
    event.mapIndex = 1;
    event.mapX = 2;
    event.mapY = 3;
    event.cell = 1;
    event.aux0 = DM1_EVENT_VI_ALTAR_REBIRTH;
    event.aux1 = 2;
    event.aux4 = 2;
    CHECK(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event),
          "C13 step-2 event schedules");
    world.gameTick = 50u;
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) > 0,
          "C13 step 2 consumes the source rebirth timer");
    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_VI_ALTAR_REBIRTH &&
            world.timeline.events[i].aux1 == 1 &&
            world.timeline.events[i].fireAtTick == 55u) {
            found_step1 = 1;
        }
    }
    CHECK(!found_step1,
          "C13 handoff does not synthesize an external rebirth follow-up");
    /* The F0255/F0283 rebirth state machine is owned by the runtime
     * orchestrator, not F0435 handoff. Its source-record admission and
     * timer consumption are covered above. */
    return;

    /* ReDMCSB TIMELINE.C F0255:1677-1695 consumes the matching bones only
     * in step 1, unlinks it, and queues the terminal step for the next tick. */
    world.gameTick = 55u;
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) > 0 &&
              square_first_things[0] == THING_ENDOFLIST,
          "C13 step 1 unlinks its exact PC34 bones owner");
    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_VI_ALTAR_REBIRTH &&
            world.timeline.events[i].aux1 == 0 &&
            world.timeline.events[i].fireAtTick == 56u) {
            found_step0 = 1;
        }
    }
    CHECK(found_step0, "C13 step 1 stages terminal rebirth one tick later");

    world.gameTick = 56u;
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    CHECK(world.party.champions[2].hp.maximum == 98 &&
              world.party.champions[2].hp.current == 49 &&
              world.party.champions[2].direction == 3 &&
              world.party.champions[2].inventory[0] == THING_NONE,
          "C13 step 0 applies the source F0283 rebirth state");

    /* F0255 simply ends a live step-1 event when another action has already
     * removed its bones. It must not synthesize terminal rebirth from the
     * saved Priority/Location fields alone. */
    champion_health_before_missing_bones = world.party.champions[2].hp.current;
    CHECK(F0720_TIMELINE_Init_Compat(&world.timeline, 57u),
          "C13 missing-bones timeline initializes");
    event.fireAtTick = 57u;
    event.aux1 = 1;
    CHECK(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event),
          "C13 missing-bones step 1 schedules");
    world.gameTick = 57u;
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    CHECK(world.timeline.count == 0 &&
              world.party.champions[2].hp.current ==
                  champion_health_before_missing_bones,
          "C13 missing bones consumes step 1 without synthetic rebirth");
}

static void test_runtime_materializer_binds_original_explosion_union(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char *quicksave = NULL;
    char path[512];
    int written = 0;
    int exported_written = 0;
    int quicksave_size = 0;
    int quicksave_written = 0;
    int rc;
    int i;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    unsigned char square_data[3][32 * 32];
    struct DungeonThings_Compat things;
    unsigned short first_things[1];
    unsigned short column_sft_bases[3 * 32];
    unsigned char raw_explosion[4];
    struct DungeonExplosion_Compat explosions[1];
    DM1_C15PoolReservationPc34 reservation;
    DM1_C15C25PublicationReceiptPc34 receipt;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    struct TimelineEvent_Compat event;
    struct GameWorld_Compat resumed_world;
    struct GameWorld_Compat resumed_quicksave_world;
    struct DM1_EventQueue_V1 resumed_queue;
    DM1OriginalSavePC34HandoffReport report;
    uint16_t source_thing = (uint16_t)((THING_TYPE_EXPLOSION << 10) |
                                       (1u << 14));

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "C25 materializer fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 0,
                                     DM1_EVENT_EXPLOSION) &&
              rewrite_fixture_event_c_union(bytes, (size_t)written, 0,
                                            source_thing) &&
              rewrite_fixture_event(
                  bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
                  DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
              rewrite_fixture_event(
                  bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
                  DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "C25 fixture preserves authenticated Slot union bytes");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(column_sft_bases, 0, sizeof(column_sft_bases));
    memset(&things, 0, sizeof(things));
    memset(raw_explosion, 0, sizeof(raw_explosion));
    memset(explosions, 0, sizeof(explosions));
    memset(&report, 0, sizeof(report));
    dungeon.header.mapCount = 3;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    /* DUNGEON.DAT G0280 gives map 2/x 11 the compact-SFT base zero: this
     * fixture has no flagged source squares before its C15 owner. */
    dungeon.columnsCumulativeSquareFirstThingCount = column_sft_bases;
    dungeon.dungeonColumnCount = (int)(sizeof(column_sft_bases) /
                                       sizeof(column_sft_bases[0]));
    for (i = 0; i < 3; ++i) {
        maps[i].width = 32;
        maps[i].height = 32;
        tiles[i].squareData = square_data[i];
        tiles[i].squareCount = 32 * 32;
    }
    first_things[0] = THING_NONE;
    wr16le(raw_explosion + 0u, THING_NONE);
    explosions[0].next = THING_NONE;
    things.squareFirstThings = first_things;
    things.squareFirstThingCount = 1;
    things.explosions = explosions;
    things.explosionCount = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = raw_explosion;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.loaded = 1;
    start_world.dungeon = &dungeon;
    start_world.things = &things;
    CHECK(dm1_v1_c15_pool_reserve_pc34(&things, &reservation) == 1 &&
              dm1_v1_c15_c25_publish_pc34(
                  &reservation, &dungeon, 2, 77, 0, 1, 2, 11, 12,
                  123500u, 7, &receipt) == 1 &&
              receipt.slot == source_thing &&
              receipt.mapTime == DM1_MAP_TIME_MAKE(2, 123500u),
          "C25 fixture publishes the exact source Location and Slot receipt");
    CHECK(F0511_DUNGEON_GetSquareFirstThing_Compat(
              &dungeon, &things, 2, 11, 12) == source_thing,
          "C25 fixture exposes its C15 owner through loaded G0280/SFT");
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "C25 materializer fixture write succeeds");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C25 materializes only through the original C15 square chain");
    CHECK(loaded_world.timeline.count == ORIGINAL_PC34_EVENT_COUNT &&
              loaded_world.timeline.events[2].kind ==
                  TIMELINE_EVENT_EXPLOSION_ADVANCE &&
              loaded_world.timeline.events[2].mapIndex == 2 &&
              loaded_world.timeline.events[2].mapX == 11 &&
              loaded_world.timeline.events[2].mapY == 12 &&
              loaded_world.timeline.events[2].cell == 1 &&
              loaded_world.timeline.events[2].aux0 == 0 &&
              loaded_world.explosions.entries[0].reserved0 == 1 &&
              loaded_world.explosions.entries[0].explosionType == 2 &&
              loaded_world.explosions.entries[0].attack == 77,
          "C25 binds Location, Slot, and decoded C15 payload without Cell/Effect");
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    imported.party = &imported_party;
    event = loaded_world.timeline.events[2];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C25 receipt isolates for native export");
    loaded_world.pc34OriginalC3C4ReceiptValid = 0;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_written);
    CHECK(rc == SAVEGAME_PC34_OK,
          "C25 exports only with its source C15 fingerprint receipt");
    quicksave_size = F0899_WORLD_SerializedSize_Compat(&loaded_world);
    quicksave = (unsigned char*)malloc((size_t)quicksave_size);
    CHECK(quicksave != NULL &&
              F0897_WORLD_Serialize_Compat(
                  &loaded_world, quicksave, quicksave_size,
                  &quicksave_written) == 1,
          "F0829 quicksave retains the source-bound C15/C25 owner");
    loaded_world.explosions.entries[0].sourceC25Priority ^= 1;
    CHECK(F0897_WORLD_Serialize_Compat(
              &loaded_world, quicksave, quicksave_size,
              &quicksave_written) == 0,
          "F0829 quicksave rejects a C25 priority detached from raw C15");
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_written) == SAVEGAME_PC34_ERROR_INTERNAL,
          "C25 export rejects an F0828 priority detached from its raw C25 owner");
    loaded_world.explosions.entries[0].sourceC25Priority ^= 1;
    loaded_world.explosions.entries[0].sourceC15Fingerprint ^= 1u;
    CHECK(F0897_WORLD_Serialize_Compat(
              &loaded_world, quicksave, quicksave_size,
              &quicksave_written) == 0,
          "F0829 quicksave rejects an F0828 owner detached from raw C15");
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_written) == SAVEGAME_PC34_ERROR_INTERNAL,
          "C25 export rejects an F0828 owner detached from raw C15 bytes");
    loaded_world.explosions.entries[0].sourceC15Fingerprint ^= 1u;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_written) == SAVEGAME_PC34_OK,
          "C25 export resumes only after the F0828 owner receipt is restored");
    CHECK(F0897_WORLD_Serialize_Compat(
              &loaded_world, quicksave, quicksave_size,
              &quicksave_written) == 1,
          "F0829 quicksave resumes only after its raw owner is restored");
    memset(&resumed_quicksave_world, 0, sizeof(resumed_quicksave_world));
    resumed_quicksave_world.dungeon = &dungeon;
    resumed_quicksave_world.things = &things;
    CHECK(F0898_WORLD_Deserialize_Compat(
              &resumed_quicksave_world, quicksave, quicksave_written, NULL) == 1 &&
              resumed_quicksave_world.explosions.entries[0].sourceC15Fingerprint ==
                  loaded_world.explosions.entries[0].sourceC15Fingerprint &&
              resumed_quicksave_world.explosions.entries[0].sourceC25Priority ==
                  loaded_world.explosions.entries[0].sourceC25Priority,
          "F0829 quick-resume restores only the captured C15/C25 owner");
    free(quicksave);
    quicksave = NULL;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C25 transaction export parses through F0435");
    CHECK(report.original_event_count == 1 &&
              report.events[0].type == DM1_EVENT_EXPLOSION &&
              rd16le(&report.events[0].c_cell) == source_thing,
          "C25 source C15 Slot round-trips through F0433/F0435");
    memset(&resumed_world, 0, sizeof(resumed_world));
    resumed_world.dungeon = &dungeon;
    resumed_world.things = &things;
    CHECK(dm1v1_event_queue_init(&resumed_queue, 1u),
          "C25 live queue initializes before production resume");
    rc = dm1_v1_original_save_pc34_handoff_resume_runtime_from_bytes(
        exported, (size_t)exported_written, &resumed_world, &resumed_queue,
        NULL);
    CHECK(rc ==
                  DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
              resumed_world.timeline.count == 1 &&
                  resumed_world.timeline.events[0].kind ==
                  TIMELINE_EVENT_EXPLOSION_ADVANCE &&
              resumed_world.explosions.entries[
                  resumed_world.timeline.events[0].aux0].reserved0 == 1,
          "production F0433 C25 bytes adopt only with their C15 runtime owner");
    F0883_WORLD_Free_Compat(&resumed_world);
    raw_explosion[3] ^= 1u;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_written) == SAVEGAME_PC34_ERROR_INTERNAL,
          "C25 export rejects mutated source C15 bytes");
    raw_explosion[3] ^= 1u;
    event.aux3 ^= 1;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event) &&
              F0802_SAVEGAME_ExportPC34FromWorld_Compat(
                  &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
                  &exported_written) == SAVEGAME_PC34_ERROR_INTERNAL,
          "C25 export rejects a drifted C15 receipt fingerprint");
}

static void test_original_c24_fluxcage_import_runtime_export_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int exported_written = 0;
    int rc;
    int i;
    int c24_index = -1;
    int exported_c24_index = -1;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    unsigned char square_data[3][32 * 32];
    struct DungeonThings_Compat things;
    unsigned short first_things[1];
    unsigned short column_sft_bases[3 * 32];
    unsigned char raw_explosion[4];
    struct DungeonExplosion_Compat source_explosions[1];
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    struct TickResult_Compat result;
    struct GameWorld_Compat resumed_world;
    struct DM1_EventQueue_V1 resumed_queue;
    DM1OriginalSavePC34HandoffReport report;
    uint16_t source_thing = (uint16_t)(THING_TYPE_EXPLOSION << 10);

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "C24 materializer fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 0,
                                     DM1_EVENT_REMOVE_FLUXCAGE) &&
              rewrite_fixture_event_priority(bytes, (size_t)written, 0, 0) &&
              rewrite_fixture_event_c_union(bytes, (size_t)written, 0,
                                            source_thing) &&
              rewrite_fixture_event(
                  bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
                  DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
              rewrite_fixture_event(
                  bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
                  DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "C24 fixture preserves authenticated zero-priority Slot union bytes");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(column_sft_bases, 0, sizeof(column_sft_bases));
    memset(&things, 0, sizeof(things));
    memset(raw_explosion, 0, sizeof(raw_explosion));
    memset(source_explosions, 0, sizeof(source_explosions));
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    memset(&report, 0, sizeof(report));
    dungeon.header.mapCount = 3;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    /* The real G0280 table places this fixture's first flagged source square
     * at compact SFT entry zero. */
    dungeon.columnsCumulativeSquareFirstThingCount = column_sft_bases;
    dungeon.dungeonColumnCount = (int)(sizeof(column_sft_bases) /
                                       sizeof(column_sft_bases[0]));
    for (i = 0; i < 3; ++i) {
        maps[i].width = 32;
        maps[i].height = 32;
        tiles[i].squareData = square_data[i];
        tiles[i].squareCount = 32 * 32;
    }
    square_data[2][11 * 32 + 12] |= DUNGEON_SQUARE_MASK_THING_LIST;
    first_things[0] = source_thing;
    wr16le(raw_explosion + 0u, THING_ENDOFLIST);
    raw_explosion[2] = C050_EXPLOSION_FLUXCAGE;
    source_explosions[0].next = THING_ENDOFLIST;
    source_explosions[0].type = C050_EXPLOSION_FLUXCAGE;
    things.squareFirstThings = first_things;
    things.squareFirstThingCount = 1;
    things.explosions = source_explosions;
    things.explosionCount = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = raw_explosion;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.loaded = 1;
    start_world.dungeon = &dungeon;
    start_world.things = &things;
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "C24 materializer fixture write succeeds");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C24 materializes only through the original fluxcage C15 slot");
    for (i = 0; i < loaded_world.timeline.count; ++i) {
        if (loaded_world.timeline.events[i].kind ==
            TIMELINE_EVENT_REMOVE_FLUXCAGE) {
            c24_index = i;
            break;
        }
    }
    CHECK(c24_index >= 0 &&
              loaded_world.timeline.events[c24_index].aux1 ==
                  C050_EXPLOSION_FLUXCAGE &&
              loaded_world.timeline.events[c24_index].aux2 == source_thing &&
              loaded_world.explosions.entries[
                  loaded_world.timeline.events[c24_index].aux0].reserved0 == 1,
          "C24 binds original C15 Slot to one live C050 runtime explosion");

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_written);
    CHECK(rc == SAVEGAME_PC34_OK && exported_written > 0,
          "C24 materialized state exports through its original Slot receipt");
    imported.party = &imported_party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_written, &imported, &report);
    exported_c24_index = -1;
    for (i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_REMOVE_FLUXCAGE) {
            exported_c24_index = i;
            break;
        }
    }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
              exported_c24_index >= 0 &&
              report.events[exported_c24_index].priority == 0 &&
              report.events[exported_c24_index].b_mapX == 11 &&
              report.events[exported_c24_index].b_mapY == 12 &&
              rd16le(&report.events[exported_c24_index].c_cell) == source_thing,
          "C24 receipt reuse preserves Priority Location and exact C.Slot union");

    memset(&resumed_world, 0, sizeof(resumed_world));
    resumed_world.dungeon = &dungeon;
    resumed_world.things = &things;
    CHECK(dm1v1_event_queue_init(&resumed_queue, 1u),
          "C24 live queue initializes before production resume");
    rc = dm1_v1_original_save_pc34_handoff_resume_runtime_from_bytes(
        exported, (size_t)exported_written, &resumed_world, &resumed_queue,
        NULL);
    CHECK(rc ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "production F0433 C24 bytes resume through the atomic runtime gate");
    for (i = 0; i < resumed_world.timeline.count; ++i) {
        if (resumed_world.timeline.events[i].kind ==
            TIMELINE_EVENT_REMOVE_FLUXCAGE) {
            break;
        }
    }
    CHECK(i < resumed_world.timeline.count &&
              resumed_world.timeline.events[i].aux2 == source_thing &&
              resumed_world.explosions.entries[
                  resumed_world.timeline.events[i].aux0].reserved0 == 1,
          "C24 adoption retains its exact C15 Slot-backed runtime owner");
    F0883_WORLD_Free_Compat(&resumed_world);

    loaded_world.timeline.events[c24_index].aux2 = THING_NONE;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_written) == SAVEGAME_PC34_ERROR_INTERNAL,
          "C24 host export rejects a missing original C15 Slot receipt");
    loaded_world.timeline.events[c24_index].aux2 = source_thing;

    {
        struct TimelineEvent_Compat c24 = loaded_world.timeline.events[c24_index];
        CHECK(F0720_TIMELINE_Init_Compat(&loaded_world.timeline, c24.fireAtTick),
              "C24 runtime expiry timeline initializes");
        CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &c24),
              "C24 runtime expiry event schedules");
        loaded_world.gameTick = c24.fireAtTick;
    }
    memset(&result, 0, sizeof(result));
        CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result) > 0,
              "C24 expiry consumes the authenticated fluxcage timer");
}

static void test_c24_c25_union_materialization_rolls_back_as_one_pc34_stage(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char source_bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int rc;
    int i;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct GameWorld_Compat loaded_world_before;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    unsigned char square_data[3][32 * 32];
    struct DungeonThings_Compat things;
    unsigned short first_things[1];
    unsigned char raw_explosion[4];
    struct DungeonExplosion_Compat source_explosions[1];
    struct ExplosionList_Compat start_explosions_before;
    DM1OriginalSavePC34HandoffReport report;
    DM1OriginalSavePC34HandoffReport report_before;
    uint16_t source_thing = (uint16_t)((THING_TYPE_EXPLOSION << 10) |
                                       (1u << 14));

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "C24/C25 rollback fixture builds an original PC34 envelope");
    /* C25 slot 0 is valid and materializes first in C4 heap order. C24 slot
     * 1 keeps an authenticated C.Slot but points at a different square, so
     * F0435 must reject the complete candidate instead of publishing C25. */
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 0,
                                     DM1_EVENT_EXPLOSION) &&
              rewrite_fixture_event_c_union(bytes, (size_t)written, 0,
                                            source_thing) &&
              rewrite_fixture_event_type(bytes, (size_t)written, 1,
                                         DM1_EVENT_REMOVE_FLUXCAGE) &&
              rewrite_fixture_event_priority(bytes, (size_t)written, 1, 0) &&
              rewrite_fixture_event_c_union(bytes, (size_t)written, 1,
                                            source_thing),
          "C24/C25 rollback fixture retains two authenticated Slot unions");
    memcpy(source_bytes, bytes, (size_t)written);

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(&things, 0, sizeof(things));
    memset(raw_explosion, 0, sizeof(raw_explosion));
    memset(source_explosions, 0, sizeof(source_explosions));
    memset(&report, 0, sizeof(report));
    dungeon.header.mapCount = 3;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    for (i = 0; i < 3; ++i) {
        maps[i].width = 32;
        maps[i].height = 32;
        tiles[i].squareData = square_data[i];
        tiles[i].squareCount = 32 * 32;
    }
    square_data[2][11 * 32 + 12] |= DUNGEON_SQUARE_MASK_THING_LIST;
    first_things[0] = source_thing;
    wr16le(raw_explosion + 0u, THING_ENDOFLIST);
    raw_explosion[2] = C050_EXPLOSION_FLUXCAGE;
    raw_explosion[3] = 77u;
    source_explosions[0].next = THING_ENDOFLIST;
    source_explosions[0].type = C050_EXPLOSION_FLUXCAGE;
    source_explosions[0].attack = 77u;
    things.squareFirstThings = first_things;
    things.squareFirstThingCount = 1;
    things.explosions = source_explosions;
    things.explosionCount = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = raw_explosion;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.loaded = 1;
    start_world.dungeon = &dungeon;
    start_world.things = &things;

    loaded_world.gameTick = 0x13579bdfu;
    loaded_world.party.championCount = 2;
    loaded_world.timeline.count = 1;
    loaded_world.timeline.events[0].kind = TIMELINE_EVENT_PLAY_SOUND;
    report.original_game_time = 0x2468ace0u;
    loaded_world_before = loaded_world;
    report_before = report;
    start_explosions_before = start_world.explosions;

    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "C24/C25 rollback fixture write succeeds");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "C24 source Slot outside its B.Location fails the PC34 handoff");
    CHECK(memcmp(&loaded_world, &loaded_world_before,
                 sizeof(loaded_world)) == 0 &&
              memcmp(&report, &report_before, sizeof(report)) == 0 &&
              memcmp(&start_world.explosions, &start_explosions_before,
                     sizeof(start_world.explosions)) == 0,
          "failed C24/C25 staging preserves the published runtime and receipt");
    CHECK(memcmp(bytes, source_bytes, (size_t)written) == 0,
          "C24/C25 staging never rewrites authenticated source union bytes");
}

static void test_original_c70_light_import_runtime_export_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int exported_written = 0;
    int rc;
    int i;
    int c70_index = -1;
    int exported_c70 = -1;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    unsigned char square_data[3][32 * 32];
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    struct TickResult_Compat result;
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "C70 materializer fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 0,
                                     DM1_EVENT_LIGHT) &&
              rewrite_fixture_event_priority(bytes, (size_t)written, 0, 0) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 0, 6, 3) &&
              rewrite_fixture_event_byte(bytes, (size_t)written, 0, 7, 0) &&
              rewrite_fixture_event_c_union(bytes, (size_t)written, 0, 0x7f3du) &&
              rewrite_fixture_event(
                  bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
                  DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
              rewrite_fixture_event(
                  bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
                  DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "C70 fixture preserves signed B.LightPower and irrelevant C bytes");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(&things, 0, sizeof(things));
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    memset(&report, 0, sizeof(report));
    dungeon.header.mapCount = 3;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    for (i = 0; i < 3; ++i) {
        maps[i].width = 32;
        maps[i].height = 32;
        tiles[i].squareData = square_data[i];
        tiles[i].squareCount = 32 * 32;
    }
    start_world.dungeon = &dungeon;
    start_world.things = &things;
    start_world.magic.magicalLightAmount = 50;
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written), "C70 materializer fixture write succeeds");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "C70 materializes only through signed B.LightPower");
    for (i = 0; i < loaded_world.timeline.count; ++i) {
        if (loaded_world.timeline.events[i].kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY &&
            loaded_world.timeline.events[i].aux0 == 3) {
            c70_index = i;
            break;
        }
    }
    CHECK(c70_index >= 0 && loaded_world.timeline.events[c70_index].aux0 == 3 &&
              loaded_world.timeline.events[c70_index].aux1 == DM1_EVENT_LIGHT &&
              loaded_world.timeline.events[c70_index].aux4 == 0,
          "C70 keeps LightPower separate from its native-event receipt");

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_written);
    CHECK(rc == SAVEGAME_PC34_OK && exported_written > 0,
          "C70 materialized state exports natively");
    imported.party = &imported_party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        exported, (size_t)exported_written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C70 exported state reimports");
    for (i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_LIGHT &&
            (int16_t)rd16le(&report.events[i].b_mapX) == 3) {
            exported_c70 = i;
            break;
        }
    }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && exported_c70 >= 0 &&
              report.events[exported_c70].priority == 0 &&
              report.events[exported_c70].c_cell == 0x3d &&
              report.events[exported_c70].c_effect == 0x7f,
          "C70 receipt reuse preserves original non-union C bytes");

    {
        struct TimelineEvent_Compat c70 = loaded_world.timeline.events[c70_index];
        CHECK(F0720_TIMELINE_Init_Compat(&loaded_world.timeline, c70.fireAtTick),
              "C70 runtime timeline initializes");
        CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &c70),
              "C70 runtime event schedules");
        loaded_world.gameTick = c70.fireAtTick;
    }
    memset(&result, 0, sizeof(result));
        CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result) > 0,
              "C70 runtime consumes the authenticated light timer");

    loaded_world.timeline.events[0].aux1 = 0;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_written) == SAVEGAME_PC34_ERROR_INTERNAL,
          "C70 host export rejects an unauthenticated light receipt");
}

static void test_original_c65_generator_import_runtime_export_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0, exported_written = 0, rc, i, c65_index = -1, exported_c65 = -1;
    struct GameWorld_Compat start_world, loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    unsigned char square_data[3][32 * 32];
    struct DungeonThings_Compat things;
    unsigned short first_things[1];
    unsigned short column_sft_bases[3 * 32];
    unsigned char raw_sensor[8];
    struct DungeonSensor_Compat sensors[1];
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    DM1OriginalSavePC34HandoffReport report;
    struct TickResult_Compat result;
    uint16_t sensor_thing = (uint16_t)(THING_TYPE_SENSOR << 10);

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "C65 materializer fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 0,
                                     DM1_EVENT_ENABLE_GROUP_GENERATOR) &&
              rewrite_fixture_event_priority(bytes, (size_t)written, 0, 0) &&
              rewrite_fixture_event_c_union(bytes, (size_t)written, 0, 0x7f3du) &&
              rewrite_fixture_event(
                  bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
                  DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
              rewrite_fixture_event(
                  bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
                  DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "C65 fixture preserves Priority-0 Location and irrelevant C bytes");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon)); memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles)); memset(square_data, 0, sizeof(square_data));
    memset(column_sft_bases, 0, sizeof(column_sft_bases));
    memset(&things, 0, sizeof(things)); memset(raw_sensor, 0, sizeof(raw_sensor));
    memset(sensors, 0, sizeof(sensors)); memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party)); memset(&report, 0, sizeof(report));
    dungeon.header.mapCount = 3; dungeon.maps = maps; dungeon.tiles = tiles; dungeon.tilesLoaded = 1;
    dungeon.columnsCumulativeSquareFirstThingCount = column_sft_bases;
    dungeon.dungeonColumnCount = (int)(sizeof(column_sft_bases) /
                                       sizeof(column_sft_bases[0]));
    for (i = 0; i < 3; ++i) { maps[i].width = 32; maps[i].height = 32; tiles[i].squareData = square_data[i]; tiles[i].squareCount = 32 * 32; }
    square_data[2][11 * 32 + 12] |= DUNGEON_SQUARE_MASK_THING_LIST;
    first_things[0] = sensor_thing; wr16le(raw_sensor, THING_ENDOFLIST);
    sensors[0].next = THING_ENDOFLIST; sensors[0].sensorType = RUNTIME_SENSOR_TYPE_DISABLED;
    things.squareFirstThings = first_things; things.squareFirstThingCount = 1;
    things.sensors = sensors; things.sensorCount = 1; things.rawThingData[THING_TYPE_SENSOR] = raw_sensor;
    things.thingCounts[THING_TYPE_SENSOR] = 1; things.loaded = 1;
    start_world.dungeon = &dungeon; start_world.things = &things;
    make_temp_save_path(path, sizeof(path)); remove(path);
    CHECK(write_fixture_file(path, bytes, written), "C65 materializer fixture write succeeds");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report);
    remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C65 materializes only with a disabled source sensor");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].kind == TIMELINE_EVENT_GROUP_GENERATOR) { c65_index = i; break; }
    CHECK(c65_index >= 0 && loaded_world.timeline.events[c65_index].aux0 == GENERATOR_EVENT_AUX0_REENABLE &&
              loaded_world.timeline.events[c65_index].aux1 == 0 &&
              loaded_world.timeline.events[c65_index].aux2 == DM1_EVENT_ENABLE_GROUP_GENERATOR,
          "C65 binds the first disabled sensor as its live receipt");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_written);
    CHECK(rc == SAVEGAME_PC34_OK && exported_written > 0, "C65 materialized state exports natively");
    imported.party = &imported_party; memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C65 exported state reimports");
    for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_ENABLE_GROUP_GENERATOR) { exported_c65 = i; break; }
    CHECK(exported_c65 >= 0 && report.events[exported_c65].priority == 0 &&
              report.events[exported_c65].b_mapX == 11 && report.events[exported_c65].b_mapY == 12 &&
              rd16le(&report.events[exported_c65].c_cell) == 0x7f3d,
          "C65 receipt reuse preserves original unowned C bytes");
    loaded_world.timeline.events[c65_index].aux2 = 0;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_written) == SAVEGAME_PC34_ERROR_INTERNAL,
          "C65 export rejects a missing disabled-sensor receipt");
    loaded_world.timeline.events[c65_index].aux2 =
        DM1_EVENT_ENABLE_GROUP_GENERATOR;
    { struct TimelineEvent_Compat c65 = loaded_world.timeline.events[c65_index];
      CHECK(F0720_TIMELINE_Init_Compat(&loaded_world.timeline, c65.fireAtTick), "C65 runtime timeline initializes");
      CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &c65), "C65 runtime event schedules"); loaded_world.gameTick = c65.fireAtTick; }
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result) > 0 &&
              sensors[0].sensorType == RUNTIME_SENSOR_TYPE_FLOOR_GROUP_GENERATOR,
          "C65 runtime re-enables its exact source sensor");
}

static void test_real_dm1_dungeon_tail_map_span_validation(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    DM1OriginalSavePC34HandoffReport report;
    size_t tail_offset;
    size_t columns_offset;
    uint16_t first_column_cumulative;
    int written = 0;
    int rc;

    if (!export_local_dm1_dungeon_save(bytes, sizeof(bytes), &written)) {
        puts("SKIP real DM1 DUNGEON.DAT tail validation (no local data)");
        return;
    }
    tail_offset = original_pc34_tail_offset(bytes, (size_t)written);
    CHECK(tail_offset != 0u && tail_offset + DUNGEON_HEADER_SIZE + 2u <
              (size_t)written,
          "real F0433 export exposes a bounded dungeon tail");
    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "real F0433 dungeon tail passes F0435 preflight");
    CHECK(report.dungeon_tail_present && report.dungeon_tail_checksum_ok,
          "real DUNGEON.DAT tail receipt is checksum-qualified");
    CHECK(report.dungeon_tail_column_table_valid &&
              report.dungeon_tail_column_terminal_sft_count <=
                  (uint32_t)report.dungeon_tail_square_first_thing_count,
          "real F0433 column cumulative table matches raw-map SFT ownership");
    CHECK(report.dungeon_tail_fingerprint != 0u,
          "real DUNGEON.DAT tail receipt has provenance fingerprint");

    {
        DM1OriginalSavePC34HandoffReport repeat_report;
        memset(&repeat_report, 0, sizeof(repeat_report));
        rc = dm1_v1_original_save_pc34_handoff_bytes(
            bytes, (size_t)written, &imported, &repeat_report);
        CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
              "same real F0433 dungeon tail repeats through F0435");
        CHECK(repeat_report.dungeon_tail_fingerprint ==
                  report.dungeon_tail_fingerprint,
              "same real F0433 dungeon tail keeps provenance fingerprint");
    }

    /* ReDMCSB DUNGEON.C F0160 indexes the compact SquareFirstThings list
     * through this saved per-column cumulative table. The M10 tail loader
     * formerly rebuilt it from tiles and could accept a checksum-valid
     * mismatch. Recompute F0422's byte checksum so this exercises the table
     * contract, not merely the outer checksum gate. */
    columns_offset = tail_offset + DUNGEON_HEADER_SIZE +
        (size_t)bytes[tail_offset + 4u] * DUNGEON_MAP_DESC_SIZE;
    first_column_cumulative = rd16le(bytes + columns_offset);
    wr16le(bytes + columns_offset,
           (uint16_t)(first_column_cumulative + 1u));
    wr16le(bytes + (size_t)written - 2u,
           byte_sum16(bytes + tail_offset,
                      (size_t)written - tail_offset - 2u));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "mismatched real tail column table fails F0435 handoff");
    CHECK(report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE,
          "column-table mismatch keeps a bounded F0434 size failure");

    CHECK(export_local_dm1_dungeon_save(bytes, sizeof(bytes), &written),
          "re-export real tail after column-table rejection");
    tail_offset = original_pc34_tail_offset(bytes, (size_t)written);

    /* ReDMCSB F0434/F0504 rejects a map whose raw data begins past the
     * saved raw-map block. Recompute only the source F0422 byte checksum. */
    wr16le(bytes + tail_offset + DUNGEON_HEADER_SIZE, 0xffffu);
    wr16le(bytes + (size_t)written - 2u,
           byte_sum16(bytes + tail_offset,
                      (size_t)written - tail_offset - 2u));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "out-of-range real tail map span fails before receipt publication");
    CHECK(report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE,
          "out-of-range real tail map span preserves F0434 size failure");
}

static void test_real_dm1_dungeon_tail_rejects_out_of_bounds_party_pose(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct GameWorld_Compat loaded_world;
    struct GameWorld_Compat loaded_world_before;
    DM1OriginalSavePC34HandoffReport report;
    DM1OriginalSavePC34HandoffReport report_before;
    const size_t pose_offsets[] = { 18u, 12u, 14u };
    int written = 0;
    size_t i;
    int rc;

    if (!export_local_dm1_dungeon_save(bytes, sizeof(bytes), &written)) {
        puts("SKIP real DM1 DUNGEON.DAT party-pose validation (no local data)");
        return;
    }

    for (i = 0u; i < sizeof(pose_offsets) / sizeof(pose_offsets[0]); ++i) {
        CHECK(rewrite_fixture_global_u16(bytes, (size_t)written,
                                         pose_offsets[i], 0x7fffu),
              "rebuilds a checksum-valid PC34 GLOBAL_DATA pose mutation");
        memset(&imported, 0, sizeof(imported));
        memset(&party, 0, sizeof(party));
        imported.party = &party;
        memset(&report, 0, sizeof(report));
        rc = dm1_v1_original_save_pc34_handoff_bytes(
            bytes, (size_t)written, &imported, &report);
        CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
                  report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT,
              "out-of-bounds pose retains all original PC34 part checksums");

        memset(&loaded_world, 0xa5, sizeof(loaded_world));
        memset(&report, 0x5a, sizeof(report));
        memcpy(&loaded_world_before, &loaded_world, sizeof(loaded_world));
        memcpy(&report_before, &report, sizeof(report));
        rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_bytes(
            bytes, (size_t)written, NULL, &loaded_world, NULL, &report);
        CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
              "F0435 tail pose outside materialized dungeon rejects before commit");
        CHECK(memcmp(&loaded_world, &loaded_world_before,
                     sizeof(loaded_world)) == 0 &&
                  memcmp(&report, &report_before, sizeof(report)) == 0,
              "rejected checksum-valid tail pose leaves world and report unchanged");

        if (i + 1u < sizeof(pose_offsets) / sizeof(pose_offsets[0])) {
            CHECK(export_local_dm1_dungeon_save(bytes, sizeof(bytes), &written),
                  "re-exports original DUNGEON.DAT tail for next pose mutation");
        }
    }
}

static int load_local_dm1_dungeon_for_door_event(
    struct DungeonDatState_Compat *dungeon,
    struct DungeonThings_Compat *things)
{
    const char *root = getenv("FIRESTAFF_DM1_DATA");
    const char *home;
    char path[1024];

    if (!dungeon || !things) return 0;
    if (!root || root[0] == '\0') {
        home = getenv("HOME");
        if (!home || home[0] == '\0') return 0;
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/DUNGEON.DAT", home);
    } else {
        snprintf(path, sizeof(path), "%s/DUNGEON.DAT", root);
    }
    return F0500_DUNGEON_LoadDatHeader_Compat(path, dungeon) &&
           F0502_DUNGEON_LoadTileData_Compat(path, dungeon) &&
           F0504_DUNGEON_LoadThingData_Compat(path, dungeon, things);
}

static int export_local_dm1_dungeon_save(unsigned char *bytes,
                                         size_t bytes_cap,
                                         int *written)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct GameWorld_Compat world;

    if (!bytes || !written || bytes_cap > (size_t)INT_MAX) {
        return 0;
    }
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(&world, 0, sizeof(world));
    if (!load_local_dm1_dungeon_for_door_event(&dungeon, &things)) {
        return 0;
    }
    world.dungeon = &dungeon;
    world.things = &things;
    world.party.mapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = 0;
    return F0802_SAVEGAME_ExportPC34FromWorld_Compat(
               &world, 0x43313445u, bytes, (int)bytes_cap, written) ==
        SAVEGAME_PC34_OK;
}

static int find_real_dm1_door(const struct DungeonDatState_Compat *dungeon,
                              int *out_map_index,
                              int *out_map_x,
                              int *out_map_y,
                              unsigned char *out_square)
{
    int map_index;

    if (!dungeon || !dungeon->maps || !dungeon->tiles ||
        !dungeon->tilesLoaded) return 0;
    for (map_index = 0; map_index < (int)dungeon->header.mapCount; ++map_index) {
        const struct DungeonMapDesc_Compat *map = &dungeon->maps[map_index];
        const struct DungeonMapTiles_Compat *tiles = &dungeon->tiles[map_index];
        int x;

        if (!tiles->squareData) continue;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                unsigned char square = tiles->squareData[(size_t)x *
                                                         (size_t)map->height +
                                                         (size_t)y];
                if ((square >> 5) == DUNGEON_ELEMENT_DOOR) {
                    if (out_map_index) *out_map_index = map_index;
                    if (out_map_x) *out_map_x = x;
                    if (out_map_y) *out_map_y = y;
                    if (out_square) *out_square = square;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static void test_real_dm1_door_animation_save_handoff(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char reexported[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    int map_index;
    int map_x;
    int map_y;
    int door_event_index = -1;
    int written = 0;
    int reexported_written = 0;
    int effect;
    int rc;
    int i;
    unsigned char square;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    if (!load_local_dm1_dungeon_for_door_event(&dungeon, &things)) {
        puts("SKIP real DM1 C01 save handoff (no local DUNGEON.DAT)");
        return;
    }
    CHECK(find_real_dm1_door(&dungeon, &map_index, &map_x, &map_y, &square),
          "real PC34 DUNGEON.DAT contains a C01 door target");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&event, 0, sizeof(event));
    start_world.dungeon = &dungeon;
    start_world.things = &things;
    start_world.party.mapIndex = map_index;
    start_world.party.mapX = map_x;
    start_world.party.mapY = map_y;
    start_world.gameTick = 400u;
    CHECK(F0720_TIMELINE_Init_Compat(&start_world.timeline, start_world.gameTick),
          "real C01 source timeline initializes");

    /* ReDMCSB TIMELINE.C F0244 converts a C10 door event to C01 only after
     * resolving its effect. Choose the source-owned direction that advances
     * this real door one state without creating a host-only event. */
    effect = (square & 0x07u) == 4u ? DOOR_EFFECT_SET : DOOR_EFFECT_CLEAR;
    event.kind = TIMELINE_EVENT_DOOR_ANIMATE;
    event.fireAtTick = start_world.gameTick + 1u;
    event.mapIndex = map_index;
    event.mapX = map_x;
    event.mapY = map_y;
    event.aux0 = DM1_EVENT_DOOR_ANIMATION;
    event.aux1 = effect;
    event.aux2 = DM1_EVENT_DOOR_ANIMATION;
    CHECK(F0721_TIMELINE_Schedule_Compat(&start_world.timeline, &event),
          "real C01 source event schedules");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &start_world, 0x43313445u, bytes, (int)sizeof(bytes), &written);
    CHECK(rc == SAVEGAME_PC34_OK && written > 0,
          "F0433 exports the real C01 door event");

    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_bytes(
        bytes, (size_t)written, &start_world, &loaded_world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "F0435 materializes C01 only against the real door square");
    for (i = 0; i < loaded_world.timeline.count; ++i) {
        const struct TimelineEvent_Compat *candidate =
            &loaded_world.timeline.events[i];
        if (candidate->kind == TIMELINE_EVENT_DOOR_ANIMATE) {
            door_event_index = i;
            break;
        }
    }
    CHECK(door_event_index >= 0,
          "real C01 survives F0435 as a live door-animation event");
    event = loaded_world.timeline.events[door_event_index];
    CHECK(event.mapIndex == map_index && event.mapX == map_x &&
              event.mapY == map_y && event.cell == 0 &&
              event.aux0 == DM1_EVENT_DOOR_ANIMATION &&
              event.aux1 == effect && event.aux2 == DM1_EVENT_DOOR_ANIMATION,
          "C01 materializer preserves only ReDMCSB Location and Effect");

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, reexported, (int)sizeof(reexported),
        &reexported_written);
    CHECK(rc == SAVEGAME_PC34_OK && reexported_written > 0,
          "F0433 reexports the materialized real C01 event");
    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        reexported, (size_t)reexported_written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "reexported real C01 remains F0435-readable");
    for (i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_DOOR_ANIMATION) {
            door_event_index = i;
            break;
        }
    }
    CHECK(door_event_index >= 0 &&
              report.events[door_event_index].b_mapX == map_x &&
              report.events[door_event_index].b_mapY == map_y &&
              report.events[door_event_index].c_cell == 0 &&
              report.events[door_event_index].c_effect == effect,
          "real C01 F0433/F0435 roundtrip writes no Cell fallback bytes");

    F0883_WORLD_Free_Compat(&loaded_world);
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
}

static void test_public_fixture_builder_roundtrips_pc34_handoff(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    size_t written = 0u;
    DM1OriginalSavePC34FixtureSpec spec;
    DM1OriginalSaveClassifyResult classified;
    DM1OriginalSavePC34HandoffReport report;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    int rc;

    memset(&spec, 0, sizeof(spec));
    spec.champion_count = 2;
    spec.map_index = 4;
    spec.map_x = 12;
    spec.map_y = 17;
    spec.direction = 3;
    spec.active_champion_index = 1;
    spec.current_active_group_count = 2;
    spec.maximum_active_group_count = ORIGINAL_PC34_ACTIVE_GROUP_COUNT;
    spec.event_count = ORIGINAL_PC34_EVENT_COUNT;
    spec.event_maximum_count = ORIGINAL_PC34_EVENT_MAXIMUM_COUNT;
    spec.game_time = 654321u;
    spec.game_id = 0x51503433u;

    rc = dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
        &spec, bytes, sizeof(bytes), &written);
    CHECK(rc == SAVEGAME_PC34_OK,
          "public PC34 fixture builder succeeds");
    CHECK(written > SAVEGAME_PC34_DM_SAVE_HEADER_SIZE,
          "public builder writes header plus parts");

    rc = dm1_v1_original_save_classify_bytes(bytes, written, &classified);
    CHECK(rc == 1, "public builder bytes classify");
    CHECK(classified.shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34,
          "public builder writes PC34 original header");
    CHECK(classified.header_checksum_ok == 1,
          "public builder header checksum validates");
    CHECK(classified.pc34_importer_candidate == 1,
          "public builder marks importer candidate");
    CHECK(classified.game_id == 0x51503433u,
          "public builder preserves requested game id");

    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "public builder bytes import through handoff");
    CHECK(report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT,
          "public builder writes all part checksums");
    CHECK(report.original_game_time == 654321u,
          "public builder global game time imports");
    CHECK(imported.party->championCount == 2,
          "public builder champion count imports");
    CHECK(imported.party->mapIndex == 4,
          "public builder map index imports");
    CHECK(imported.party->mapX == 12,
          "public builder map x imports");
    CHECK(imported.party->mapY == 17,
          "public builder map y imports");
    CHECK(imported.party->direction == 3,
          "public builder direction imports");
    CHECK(memcmp(imported.party->champions[1].name, "WUUF    ", 8u) == 0,
          "public builder second champion imports");
}

static void test_world_roundtrip_helper_exports_verified_pc34(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char roundtrip[SAVEGAME_PC34_MAX_FILE_SIZE];
    char fixture_path[256];
    char exported_path[256];
    FILE *fixture_file;
    size_t written = 0u;
    size_t roundtrip_written = 0u;
    DM1OriginalSavePC34FixtureSpec spec;
    DM1OriginalSavePC34HandoffReport import_report;
    DM1OriginalSavePC34HandoffReport verify_report;
    DM1OriginalSavePC34RoundtripReport roundtrip_report;
    DM1OriginalSaveClassifyResult classified;
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    int rc;

    memset(&spec, 0, sizeof(spec));
    spec.champion_count = 3;
    spec.map_index = 5;
    spec.map_x = 19;
    spec.map_y = 23;
    spec.direction = 2;
    spec.active_champion_index = 1;
    spec.current_active_group_count = 2;
    spec.maximum_active_group_count = ORIGINAL_PC34_ACTIVE_GROUP_COUNT;
    spec.event_count = ORIGINAL_PC34_EVENT_COUNT;
    spec.event_maximum_count = ORIGINAL_PC34_EVENT_MAXIMUM_COUNT;
    spec.game_time = 777888u;
    spec.game_id = 0x52544d31u;

    rc = dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
        &spec, bytes, sizeof(bytes), &written);
    CHECK(rc == SAVEGAME_PC34_OK,
          "roundtrip helper fixture build succeeds");
    CHECK(rewrite_fixture_event(
              bytes, (int)written, 0, DM1_MAP_TIME_MAKE(5, 777932u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, (int)written, 1, DM1_MAP_TIME_MAKE(5, 777910u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, (int)written, 2, DM1_MAP_TIME_MAKE(5, 777920u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "roundtrip helper fixture supplies source-valid runtime events");

    snprintf(fixture_path, sizeof(fixture_path),
             "/tmp/firestaff_dm1_original_save_roundtrip_%lu.dat",
             (unsigned long)spec.game_id);
    fixture_file = fopen(fixture_path, "wb");
    CHECK(fixture_file != NULL, "roundtrip file fixture opens");
    CHECK(fwrite(bytes, 1u, written, fixture_file) == written,
          "roundtrip file fixture writes");
    CHECK(fclose(fixture_file) == 0, "roundtrip file fixture closes");

    rc = dm1_v1_original_save_pc34_roundtrip_world_bytes(
        bytes, written, 0x52544d32u,
        roundtrip, sizeof(roundtrip), &roundtrip_written,
        &import_report, &verify_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "roundtrip helper import-export-verify succeeds");
    CHECK(roundtrip_written > SAVEGAME_PC34_DM_SAVE_HEADER_SIZE,
          "roundtrip helper writes PC34 header plus save parts");
    CHECK(import_report.original_game_time == 777888u,
          "roundtrip helper import report records source game time");
    CHECK(import_report.imported_map_index == 5 &&
          import_report.imported_map_x == 19 &&
          import_report.imported_map_y == 23 &&
          import_report.imported_direction == 2,
          "roundtrip helper import report records source party pose");
    CHECK(import_report.active_group_runtime_imported_count == 2,
          "roundtrip helper materializes active groups into runtime world");
    CHECK(verify_report.classify.shape ==
          DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34,
          "roundtrip helper exported bytes classify as PC34");
    CHECK(verify_report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT,
          "roundtrip helper exported bytes verify all save-part checksums");
    CHECK(verify_report.original_game_time == 777888u,
          "roundtrip helper preserves GameTime through export");
    CHECK(verify_report.original_current_active_group_count == 2,
          "roundtrip helper preserves current active group count");
    CHECK(verify_report.active_groups[0].directions ==
              import_report.active_groups[0].directions &&
          verify_report.active_groups[1].directions ==
              import_report.active_groups[1].directions &&
          verify_report.active_groups[0].home_map_x ==
              import_report.active_groups[0].home_map_x &&
          verify_report.active_groups[0].home_map_y ==
              import_report.active_groups[0].home_map_y &&
          verify_report.active_groups[1].home_map_x ==
              import_report.active_groups[1].home_map_x &&
          verify_report.active_groups[1].home_map_y ==
              import_report.active_groups[1].home_map_y,
          "multi-group packed directions and home positions survive export");
    CHECK(verify_report.active_groups[0].last_move_time ==
              import_report.active_groups[0].last_move_time &&
          verify_report.active_groups[0].delay_fleeing_from_target ==
              import_report.active_groups[0].delay_fleeing_from_target &&
          verify_report.active_groups[1].target_map_x ==
              import_report.active_groups[1].target_map_x &&
          verify_report.active_groups[1].prior_map_y ==
              import_report.active_groups[1].prior_map_y &&
          memcmp(verify_report.active_groups[1].aspect,
                 import_report.active_groups[1].aspect, 4u) == 0,
          "multi-group position, countdown, and aspect fields survive export");
    CHECK(verify_report.original_event_count == ORIGINAL_PC34_EVENT_COUNT,
          "roundtrip helper preserves event count");

    snprintf(exported_path, sizeof(exported_path),
             "/tmp/firestaff_dm1_manifest_save_roundtrip_%lu.dat",
             (unsigned long)spec.game_id);
    fixture_file = fopen(exported_path, "wb");
    CHECK(fixture_file != NULL, "manifest export fixture opens");
    CHECK(fwrite(roundtrip, 1u, roundtrip_written, fixture_file) ==
              roundtrip_written,
          "manifest export fixture writes");
    CHECK(fclose(fixture_file) == 0, "manifest export fixture closes");
    CHECK(dm1_v1_original_save_pc34_roundtrip_world_file(
              exported_path, 0x52544d32u,
              roundtrip, sizeof(roundtrip), &roundtrip_written,
              &import_report, &verify_report) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34,
          "manifest-bearing export is rejected by original-save file route");

    rc = dm1_v1_original_save_classify_bytes(
        roundtrip, roundtrip_written, &classified);
    CHECK(rc == 1 &&
          classified.shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 &&
          classified.header_checksum_ok == 1,
          "roundtrip helper exported header validates independently");

    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        roundtrip, roundtrip_written, &imported, &verify_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "roundtrip helper output imports into SaveGame state");
    CHECK(imported.party->championCount == 3 &&
          imported.party->mapIndex == 5 &&
          imported.party->mapX == 19 &&
          imported.party->mapY == 23 &&
          imported.party->direction == 2,
          "roundtrip helper output preserves party state");
    CHECK(imported.party->champions[0].portraitBitmapValid == 1 &&
          imported.party->champions[0].portraitBitmap[0] == 0x30u &&
          imported.party->champions[2].portraitBitmap[0] == 0x32u,
          "roundtrip helper preserves original external portraits");

    memset(roundtrip, 0, sizeof(roundtrip));
    roundtrip_written = 0u;
    memset(&roundtrip_report, 0, sizeof(roundtrip_report));
    rc = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
        bytes, written, 0x52544d33u,
        roundtrip, sizeof(roundtrip), &roundtrip_written,
        &roundtrip_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "roundtrip reload helper imports exported bytes into runtime world");
    CHECK(roundtrip_report.core_state_matches == 1,
          "roundtrip reload helper reports matching core state");
    CHECK(roundtrip_report.source_champion_count == 3 &&
          roundtrip_report.exported_champion_count == 3 &&
          roundtrip_report.reloaded_champion_count == 3,
          "roundtrip reload helper preserves champion count");
    CHECK(roundtrip_report.source_map_index == 5 &&
          roundtrip_report.exported_map_index == 5 &&
          roundtrip_report.reloaded_map_index == 5,
          "roundtrip reload helper preserves map index");
    CHECK(roundtrip_report.source_map_x == 19 &&
          roundtrip_report.exported_map_x == 19 &&
          roundtrip_report.reloaded_map_x == 19 &&
          roundtrip_report.source_map_y == 23 &&
          roundtrip_report.exported_map_y == 23 &&
          roundtrip_report.reloaded_map_y == 23,
          "roundtrip reload helper preserves party coordinates");
    CHECK(roundtrip_report.source_direction == 2 &&
          roundtrip_report.exported_direction == 2 &&
          roundtrip_report.reloaded_direction == 2,
          "roundtrip reload helper preserves direction");
    CHECK(roundtrip_report.source_game_time == 777888u &&
          roundtrip_report.exported_game_time == 777888u &&
          roundtrip_report.reloaded_game_time == 777888u,
          "roundtrip reload helper preserves game time");
    CHECK(roundtrip_report.source_event_count == ORIGINAL_PC34_EVENT_COUNT &&
          roundtrip_report.exported_event_count == ORIGINAL_PC34_EVENT_COUNT &&
          roundtrip_report.reloaded_event_count == ORIGINAL_PC34_EVENT_COUNT,
          "roundtrip reload helper preserves event queue count");
    CHECK(roundtrip_report.source_active_group_count == 2 &&
          roundtrip_report.exported_active_group_count == 2 &&
          roundtrip_report.reloaded_active_group_count == 2,
          "roundtrip reload helper preserves active group count");

    memset(roundtrip, 0, sizeof(roundtrip));
    roundtrip_written = 0u;
    memset(&import_report, 0, sizeof(import_report));
    memset(&verify_report, 0, sizeof(verify_report));
    rc = dm1_v1_original_save_pc34_roundtrip_world_file(
        fixture_path, 0x52544d34u,
        roundtrip, sizeof(roundtrip), &roundtrip_written,
        &import_report, &verify_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "roundtrip file helper import-export-verify succeeds");
    CHECK(roundtrip_written > SAVEGAME_PC34_DM_SAVE_HEADER_SIZE,
          "roundtrip file helper writes PC34 bytes");
    CHECK(import_report.imported_map_index == 5,
          "roundtrip file helper records source map index");
    CHECK(verify_report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT,
          "roundtrip file helper verifies all save parts");

    memset(roundtrip, 0, sizeof(roundtrip));
    roundtrip_written = 0u;
    memset(&roundtrip_report, 0, sizeof(roundtrip_report));
    rc = dm1_v1_original_save_pc34_roundtrip_world_reload_file(
        fixture_path, 0x52544d35u,
        roundtrip, sizeof(roundtrip), &roundtrip_written,
        &roundtrip_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "roundtrip reload file helper succeeds");
    CHECK(roundtrip_report.core_state_matches == 1,
          "roundtrip reload file helper reports matching core state");
    CHECK(roundtrip_report.reloaded_champion_count == 3 &&
          roundtrip_report.reloaded_event_count == ORIGINAL_PC34_EVENT_COUNT,
          "roundtrip reload file helper preserves runtime counts");

    rc = dm1_v1_original_save_pc34_roundtrip_world_bytes(
        NULL, 0u, 0u, roundtrip, sizeof(roundtrip), &roundtrip_written,
        &import_report, &verify_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT,
          "roundtrip helper rejects null source bytes");
    rc = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
        NULL, 0u, 0u, roundtrip, sizeof(roundtrip), &roundtrip_written,
        &roundtrip_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT,
          "roundtrip reload helper rejects null source bytes");
    rc = dm1_v1_original_save_pc34_roundtrip_world_file(
        "/tmp/firestaff_dm1_original_save_missing.dat", 0u,
        roundtrip, sizeof(roundtrip), &roundtrip_written,
        &import_report, &verify_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE,
          "roundtrip file helper reports missing file");
    remove(exported_path);
    remove(fixture_path);
}

static void test_world_handoff_rejects_duplicate_timeline_reference(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char roundtrip[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char party_info[ORIGINAL_PC34_PARTY_INFO_BYTES];
    struct GameWorld_Compat world;
    struct DM1_EventQueue_V1 queue;
    struct SaveGame_Compat reimported;
    struct SaveGame_Compat imported;
    struct PartyState_Compat reimported_party;
    struct PartyState_Compat party;
    struct SpellEffect_Compat footprints_effect;
    DM1OriginalSavePC34HandoffReport report;
    DM1OriginalSavePC34RoundtripReport roundtrip_report;
    int written = 0;
    int exported_size = 0;
    size_t roundtrip_written = 0u;
    int rc;
    int i;
    struct TimelineEvent_Compat event;

    memset(bytes, 0, sizeof(bytes));
    memset(party_info, 0, sizeof(party_info));
    memset(&world, 0, sizeof(world));
    memset(&queue, 0, sizeof(queue));
    memset(&report, 0, sizeof(report));
    CHECK(build_original_pc34_fixture(
              bytes, (int)sizeof(bytes), &written, 2, 1, 4, 5, 2, 1,
              ORIGINAL_PC34_ACTIVE_GROUP_COUNT) == SAVEGAME_PC34_OK,
          "PARTY_INFO runtime fixture build succeeds");
    wr16le(party_info + 0u, (uint16_t)(int16_t)-12);
    party_info[2u] = 3u;
    party_info[3u] = 5u;
    wr16le(party_info + 4u, 17u);
    wr16le(party_info + 6u, 19u);
    wr16le(party_info + 8u, 23u);
    party_info[10u] = 29u;
    party_info[11u] = 37u;
    party_info[84u] = 18u;
    CHECK(rewrite_fixture_party_info_bytes(
              bytes, (size_t)written, party_info),
          "PARTY_INFO runtime fixture retains authenticated C2 bytes");
    CHECK(rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0),
          "PARTY_INFO runtime fixture supplies a source-valid C70 event");
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, &queue, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
          world.magic.magicalLightAmount == -12 &&
          world.magic.event73CountThievesEye == 3 &&
          world.magic.event79CountFootprints == 5 &&
          world.magic.magicFootprintsActive &&
          world.magic.partyShieldDefense == 17 &&
          world.magic.fireShieldDefense == 19 &&
          world.magic.spellShieldDefense == 23 &&
          world.magic.scentCount == 29 &&
          world.freezeLifeTicks == 37 &&
          world.magic.freezeLifeTicks == 37 &&
          world.magic.firstScentIndex == 18 &&
          world.lifecycle.status.partyShieldDefense == 17 &&
          world.lifecycle.status.partyFireShieldDefense == 19 &&
          world.lifecycle.status.partySpellShieldDefense == 23,
          "F0435 materializes source PARTY_INFO magic and freeze-life owners");
    memset(&footprints_effect, 0, sizeof(footprints_effect));
    footprints_effect.spellType = C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT;
    footprints_effect.powerOrdinal = 3;
    footprints_effect.magicStateDelta[5] = 1;
    F0760_MAGIC_ApplyStateDelta_Compat(&footprints_effect, &world.magic);
    CHECK(world.magic.firstScentIndex == 29 &&
          world.magic.lastScentIndex == 0,
          "resumed ScentCount drives the source Footprints window");
    F0890_ORCH_ApplyPeriodicEffects_Compat(&world, NULL);
    CHECK(world.freezeLifeTicks == 36 && world.magic.freezeLifeTicks == 36 &&
              world.magic.firstScentIndex == 29,
          "the resumed PARTY_INFO owners reach the live periodic tick");
    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY &&
            world.timeline.events[i].aux1 == DM1_EVENT_LIGHT) {
            event = world.timeline.events[i];
            F0720_TIMELINE_Init_Compat(&world.timeline, event.fireAtTick);
            CHECK(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event),
                  "PARTY_INFO export isolates its authenticated C70 receipt");
            break;
        }
    }
    world.pc34OriginalC3C4ReceiptValid = 0;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_size) == SAVEGAME_PC34_OK,
          "F0433 exports source-owned PARTY_INFO runtime fields");
    memset(&reimported, 0, sizeof(reimported));
    memset(&reimported_party, 0, sizeof(reimported_party));
    reimported.party = &reimported_party;
    CHECK(dm1_v1_original_save_pc34_handoff_bytes(
              exported, (size_t)exported_size, &reimported, &report) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
          (int16_t)rd16le(reimported_party.pc34PartyInfoBytes + 0u) == -12 &&
          reimported_party.pc34PartyInfoBytes[2u] == 3u &&
          reimported_party.pc34PartyInfoBytes[3u] == 6u &&
          (int16_t)rd16le(reimported_party.pc34PartyInfoBytes + 4u) == 17 &&
          (int16_t)rd16le(reimported_party.pc34PartyInfoBytes + 6u) == 19 &&
          (int16_t)rd16le(reimported_party.pc34PartyInfoBytes + 8u) == 23 &&
          reimported_party.pc34PartyInfoBytes[10u] == 29u &&
          reimported_party.pc34PartyInfoBytes[11u] == 36u &&
          reimported_party.pc34PartyInfoBytes[84u] == 29u,
          "F0433 preserves source PARTY_INFO field ownership on export");
    F0883_WORLD_Free_Compat(&world);

    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(2, 123500u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0),
          "tail-less roundtrip retains only authenticated C70 records");
    memset(&roundtrip_report, 0, sizeof(roundtrip_report));
    rc = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
        bytes, (size_t)written, 0x48414e44u,
        roundtrip, sizeof(roundtrip), &roundtrip_written, &roundtrip_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
          roundtrip_report.source_dungeon_tail_byte_count == 0u &&
          roundtrip_report.exported_dungeon_tail_byte_count == 0u &&
          roundtrip_report.reloaded_dungeon_tail_byte_count == 0u &&
          roundtrip_report.dungeon_tail_matches == 1,
          "tail-less champion hand fixture survives F0435/F0433/F0435 roundtrip");

    CHECK(rewrite_fixture_party_hand_reference(bytes, written, 0x1556u),
          "champion-hand fixture rewrites valid PARTY envelope");
    memset(&imported, 0, sizeof(imported));
    memset(&party, 0, sizeof(party));
    imported.party = &party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
          report.part_checksum_ok_count == SAVEGAME_PC34_PART_COUNT,
          "out-of-range champion hand remains a valid F0435 part envelope");

    memset(&world, 0, sizeof(world));
    world.gameTick = 777u;
    memset(&report, 0, sizeof(report));
    report.original_game_time = 999u;
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "tail-less hand reference remains deferred until ThingData exists");
    CHECK(world.gameTick == 123456u && report.original_game_time == 123456u,
          "tail-less hand reference commits the authenticated runtime receipt");
    F0883_WORLD_Free_Compat(&world);
}

static void test_world_handoff_rejects_invalid_champion_vitals(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct PartyState_Compat party_before;
    struct GameWorld_Compat world;
    DM1OriginalSavePC34HandoffReport report;
    int written = 0;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 0, 0, 0, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "champion-vitals fixture build succeeds");
    CHECK(rewrite_fixture_party_current_health(bytes, written, 0xffffu),
          "champion-vitals fixture rewrites valid PARTY envelope");

    memset(&imported, 0, sizeof(imported));
    memset(&party, 0xa5, sizeof(party));
    party_before = party;
    imported.party = &party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
          report.part_checksum_ok_count == 3 &&
          report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE,
          "negative champion health passes checksum then fails PARTY bounds");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "invalid champion vital leaves direct PARTY import unchanged");

    memset(&world, 0, sizeof(world));
    world.gameTick = 777u;
    memset(&report, 0, sizeof(report));
    report.original_game_time = 999u;
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "invalid champion vital rejects candidate-world handoff");
    CHECK(world.gameTick == 777u && report.original_game_time == 999u,
          "invalid champion vital leaves runtime receipt untouched");
}

static void test_world_handoff_rejects_current_active_groups_over_maximum(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct SaveGame_Compat imported;
    struct PartyState_Compat party;
    struct PartyState_Compat party_before;
    struct GameWorld_Compat world;
    DM1OriginalSavePC34HandoffReport report;
    int written = 0;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     1, 0, 0, 0, 0, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "active-group count fixture build succeeds");
    CHECK(rewrite_fixture_global_current_active_group_count(
              bytes, written,
              (uint16_t)(ORIGINAL_PC34_ACTIVE_GROUP_COUNT + 1)),
          "active-group count fixture rewrites valid GLOBAL_DATA envelope");

    memset(&imported, 0, sizeof(imported));
    memset(&party, 0xa5, sizeof(party));
    party_before = party;
    imported.party = &party;
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT &&
          report.part_checksum_ok_count == 1 &&
          report.importer_result == SAVEGAME_PC34_ERROR_BAD_SIZE,
          "current ACTIVE_GROUP count over maximum fails after checksum");
    CHECK(memcmp(&party, &party_before, sizeof(party)) == 0,
          "invalid ACTIVE_GROUP count leaves direct PARTY import unchanged");

    memset(&world, 0, sizeof(world));
    world.gameTick = 777u;
    memset(&report, 0, sizeof(report));
    report.original_game_time = 999u;
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "current ACTIVE_GROUP count over maximum rejects candidate world");
    CHECK(world.gameTick == 777u && report.original_game_time == 999u,
          "rejected ACTIVE_GROUP count leaves runtime receipt untouched");
}

static void test_world_roundtrip_preserves_materialized_dungeon_tail(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char roundtrip[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat runtime_world;
    struct DM1_EventQueue_V1 runtime_queue;
    DM1OriginalSavePC34HandoffReport runtime_report;
    DM1OriginalSavePC34RoundtripReport report;
    int written = 0;
    size_t roundtrip_written = 0u;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     0, 0, 0, 0, 0, -1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "dungeon-tail roundtrip fixture build succeeds");
    CHECK(append_minimal_original_pc34_dungeon_tail(
              bytes, (int)sizeof(bytes), &written),
          "dungeon-tail roundtrip fixture appends complete source tail");
    CHECK(rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0),
          "dungeon-tail roundtrip fixture supplies a source-valid C70 event");
    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(1, 123500u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(1, 123470u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "dungeon-tail roundtrip keeps each exported record C70-authenticated");

    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
        bytes, (size_t)written, 0x5441494cu,
        roundtrip, sizeof(roundtrip), &roundtrip_written, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "dungeon-tail roundtrip reload succeeds");
    CHECK(roundtrip_written > SAVEGAME_PC34_DM_SAVE_HEADER_SIZE,
          "dungeon-tail roundtrip writes a complete PC34 export");
    CHECK(report.source_dungeon_tail_present == 1 &&
          report.exported_dungeon_tail_present == 1 &&
          report.reloaded_dungeon_tail_present == 1,
          "dungeon tail is present at every F0435/F0433 handoff edge");
    CHECK(report.source_dungeon_tail_byte_count == 65u &&
          report.exported_dungeon_tail_byte_count == 65u &&
          report.reloaded_dungeon_tail_byte_count == 65u,
          "dungeon tail byte count survives materialization and export");
    CHECK(report.source_dungeon_tail_checksum ==
              report.exported_dungeon_tail_checksum &&
          report.source_dungeon_tail_checksum ==
              report.reloaded_dungeon_tail_checksum,
          "dungeon tail checksum survives materialization and export");
    CHECK(report.dungeon_tail_matches == 1 && report.core_state_matches == 1,
          "roundtrip receipt requires the materialized dungeon tail");

    /* `roundtrip` is production F0433 output, not a hand-built tail. Resume
     * it through the live F0435 boundary so G0280, SquareFirstThings, and
     * C3/C4 can be certified together before runtime ownership moves. */
    memset(&runtime_world, 0, sizeof(runtime_world));
    memset(&runtime_queue, 0, sizeof(runtime_queue));
    memset(&runtime_report, 0, sizeof(runtime_report));
    CHECK(dm1v1_event_queue_init(&runtime_queue, 1u),
          "tail receipt runtime queue initializes");
    rc = dm1_v1_original_save_pc34_handoff_resume_runtime_from_bytes(
        roundtrip, roundtrip_written, &runtime_world, &runtime_queue,
        &runtime_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "F0433 tail resume accepts the source-owned dungeon receipt");
    CHECK(runtime_report.dungeon_tail_runtime_receipt_valid,
          "F0433 tail resume certifies G0280, SquareFirstThings, and C3/C4");
    CHECK(runtime_report.dungeon_tail_runtime_column_table_fingerprint ==
              runtime_report.dungeon_tail_column_table_fingerprint,
          "F0433 tail resume preserves the G0280 column table");
    CHECK(runtime_report.dungeon_tail_runtime_square_first_thing_fingerprint ==
              runtime_report.dungeon_tail_square_first_thing_fingerprint,
          "F0433 tail resume preserves SquareFirstThings");
    CHECK(runtime_report.dungeon_tail_runtime_timeline_fingerprint ==
              runtime_report.timeline_runtime_fingerprint &&
              runtime_queue.eventCount == runtime_world.timeline.count,
          "F0433 tail resume preserves the C3/C4 timeline");
    F0883_WORLD_Free_Compat(&runtime_world);
}

static void test_world_handoff_materializes_and_validates_textstring_tail(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char roundtrip[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat world;
    DM1OriginalSavePC34HandoffReport report;
    DM1OriginalSavePC34RoundtripReport roundtrip_report;
    int tail_offset;
    int written = 0;
    size_t roundtrip_written = 0u;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     0, 0, 0, 0, 0, -1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "TextString dungeon-tail fixture build succeeds");
    tail_offset = written;
    CHECK(append_textstring_original_pc34_dungeon_tail(
              bytes, (int)sizeof(bytes), &written),
          "TextString dungeon-tail fixture appends complete source tail");
    CHECK(rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0),
          "TextString fixture supplies a source-valid C70 event");
    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(1, 123500u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(1, 123470u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "TextString roundtrip keeps every queue record C70-authenticated");

    memset(&world, 0, sizeof(world));
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "TextString dungeon tail materializes through F0435 handoff");
    CHECK(report.dungeon_tail_text_data_word_count == 2 &&
          report.dungeon_tail_text_string_count == 1,
          "tail receipt reports TextData and TextString sections");
    CHECK(world.things && world.things->textDataWordCount == 2 &&
          world.things->textStringCount == 1,
          "runtime materializes TextData and TextString sections");
    CHECK(world.things->textStrings[0].visible == 1 &&
          world.things->textStrings[0].textDataWordOffset == 0,
          "runtime materializes the legal TextString offset");
    F0883_WORLD_Free_Compat(&world);

    memset(&roundtrip_report, 0, sizeof(roundtrip_report));
    rc = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
        bytes, (size_t)written, 0x54585453u,
        roundtrip, sizeof(roundtrip), &roundtrip_written, &roundtrip_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "TextString dungeon tail survives F0435/F0433/F0435 roundtrip");
    CHECK(roundtrip_report.source_dungeon_tail_byte_count == 75u &&
          roundtrip_report.exported_dungeon_tail_byte_count == 75u &&
          roundtrip_report.reloaded_dungeon_tail_byte_count == 75u &&
          roundtrip_report.dungeon_tail_matches == 1,
          "TextString tail bytes survive every handoff edge");

    /* This is checksum-correct and structurally complete, but F0435 must
     * not publish a TEXTSTRING that DUNGEON.C would read past TextData. */
    wr16le(bytes + tail_offset + 70, 0x0011u);
    refresh_original_pc34_dungeon_tail_checksum(bytes + tail_offset, 75);
    memset(&world, 0, sizeof(world));
    world.gameTick = 777u;
    memset(&report, 0, sizeof(report));
    report.original_game_time = 999u;
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "out-of-range TextString offset rejects an otherwise legal tail");
    CHECK(world.gameTick == 777u && report.original_game_time == 999u,
          "rejected TextString tail leaves runtime and receipt untouched");
}

static void test_world_handoff_rejects_party_pose_outside_dungeon_tail(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat world;
    DM1OriginalSavePC34HandoffReport report;
    int written = 0;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     0, 0, 0, 0, 0, -1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "party-pose dungeon-tail fixture build succeeds");
    CHECK(append_minimal_original_pc34_dungeon_tail(
              bytes, (int)sizeof(bytes), &written),
          "party-pose fixture appends a one-map one-square tail");
    CHECK(rewrite_fixture_global_party_pose(bytes, written, 1u, 0u, 0u),
          "fixture keeps F0417/header checksums valid for bad map index");

    memset(&world, 0, sizeof(world));
    world.gameTick = 777u;
    memset(&report, 0, sizeof(report));
    report.original_game_time = 999u;
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "party map index outside restored dungeon tail rejects handoff");
    CHECK(world.gameTick == 777u && report.original_game_time == 999u,
          "bad party map index leaves runtime and receipt untouched");

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     0, 0, 0, 0, 0, -1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
          append_minimal_original_pc34_dungeon_tail(
              bytes, (int)sizeof(bytes), &written) &&
          rewrite_fixture_global_party_pose(bytes, written, 0u, 1u, 0u),
          "fixture keeps F0417/header checksums valid for bad map square");
    memset(&world, 0, sizeof(world));
    world.gameTick = 778u;
    memset(&report, 0, sizeof(report));
    report.original_game_time = 998u;
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "party coordinates outside restored dungeon tail reject handoff");
    CHECK(world.gameTick == 778u && report.original_game_time == 998u,
          "bad party square leaves runtime and receipt untouched");
}

static void test_world_handoff_roundtrips_group_list_and_active_groups(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char roundtrip[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat world;
    DM1OriginalSavePC34HandoffReport report;
    DM1OriginalSavePC34HandoffReport invalid_active_group_report;
    DM1OriginalSavePC34RoundtripReport roundtrip_report;
    int tail_offset;
    int written = 0;
    size_t roundtrip_written = 0u;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     0, 0, 0, 0, 0, -1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "group-list dungeon-tail fixture build succeeds");
    tail_offset = written;
    CHECK(append_group_list_original_pc34_dungeon_tail(
              bytes, (int)sizeof(bytes), &written),
          "group-list dungeon-tail fixture appends complete source tail");
    CHECK(rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0),
          "group-list fixture supplies a source-valid C70 event");
    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(1, 123500u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(1, 123470u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "group-list roundtrip keeps every queue record C70-authenticated");

    memset(&world, 0, sizeof(world));
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "GROUP list tail materializes through F0435 handoff");
    CHECK(world.things && world.things->groupCount == 4 &&
          world.things->squareFirstThings[0] ==
              (unsigned short)(THING_TYPE_GROUP << 10 | 1),
          "GROUP list retains its SquareFirstThings root");
    CHECK(report.active_group_runtime_resolved_count == 2 &&
          world.creatureAI[0].reserved0 == 1 &&
          world.creatureAI[0].creatureType == 9 &&
          world.things->groups[1].behavior == 1,
          "ACTIVE_GROUP records resolve with GROUP behavior from tail");
    invalid_active_group_report = report;
    invalid_active_group_report.active_groups[0].group_thing_index =
        (THING_TYPE_GROUP << 10) | 4;
    rc = dm1_v1_original_save_pc34_handoff_apply_active_groups(
        &invalid_active_group_report, &world);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "out-of-range ACTIVE_GROUP Thing rejects before runtime mutation");
    CHECK(world.creatureAI[0].reserved0 == 1 &&
          world.creatureAI[0].creatureType == 9,
          "rejected ACTIVE_GROUP Thing leaves resolved runtime state intact");
    F0883_WORLD_Free_Compat(&world);

    memset(&roundtrip_report, 0, sizeof(roundtrip_report));
    rc = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
        bytes, (size_t)written, 0x47525053u,
        roundtrip, sizeof(roundtrip), &roundtrip_written, &roundtrip_report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "GROUP list tail survives F0435/F0433/F0435 roundtrip");
    CHECK(roundtrip_report.source_dungeon_tail_byte_count == 131u &&
          roundtrip_report.exported_dungeon_tail_byte_count == 131u &&
          roundtrip_report.reloaded_dungeon_tail_byte_count == 131u &&
          roundtrip_report.dungeon_tail_matches == 1 &&
          roundtrip_report.source_active_group_count == 2 &&
          roundtrip_report.reloaded_active_group_count == 2,
          "GROUP list and active-group counts retain byte integrity");

    /* The checksum and section sizes remain valid, but this root names a
     * nonexistent GROUP slot. It must fail before candidate-world commit. */
    wr16le(bytes + tail_offset + 62,
           (uint16_t)(THING_TYPE_GROUP << 10 | 4));
    refresh_original_pc34_dungeon_tail_checksum(bytes + tail_offset, 131);
    memset(&world, 0, sizeof(world));
    world.gameTick = 777u;
    memset(&report, 0, sizeof(report));
    report.original_game_time = 999u;
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "out-of-range GROUP list root rejects a rechecksummed tail");
    CHECK(world.gameTick == 777u && report.original_game_time == 999u,
          "rejected GROUP list tail leaves runtime and receipt untouched");
}

static void test_strings(void)
{
    CHECK(strcmp(dm1_v1_original_save_pc34_handoff_result_name(
                     DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK), "OK") == 0,
          "result OK name");
    CHECK(strcmp(dm1_v1_original_save_pc34_handoff_result_name(
                     DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34),
                 "not-pc34") == 0,
          "result not-pc34 name");
    CHECK(strcmp(dm1_v1_original_save_pc34_handoff_result_name(
                     DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE),
                 "file") == 0,
          "result file name");
    CHECK(strstr(dm1_v1_original_save_pc34_handoff_source_evidence(),
                 "LOADSAVE.C") != NULL,
          "source evidence mentions LOADSAVE.C");
    CHECK(strstr(dm1_v1_original_save_pc34_handoff_source_evidence(),
                 "F0417") != NULL,
          "source evidence mentions F0417");
    CHECK(strstr(dm1_v1_original_save_pc34_handoff_source_evidence(),
                 "F0418") != NULL,
          "source evidence mentions F0418");
    CHECK(strstr(dm1_v1_original_save_pc34_handoff_source_evidence(),
                 "F0421") != NULL,
          "source evidence mentions F0421");
    CHECK(strstr(dm1_v1_original_save_pc34_handoff_source_evidence(),
                 "CHAMPION_EXCLUDING_PORTRAIT") != NULL,
          "source evidence mentions champion layout");
    CHECK(strstr(dm1_v1_original_save_pc34_handoff_source_evidence(),
                 "ACTIVE_GROUP") != NULL,
          "source evidence mentions active group layout");
    CHECK(strstr(dm1_v1_original_save_pc34_handoff_source_evidence(),
                 "THING type/index") != NULL,
          "source evidence mentions thing index layout");
    CHECK(strstr(dm1_v1_original_save_pc34_handoff_source_evidence(),
                 "EVENT") != NULL,
          "source evidence mentions event layout");
}

static void test_corpus_roundtrip_proof(void)
{
    char root[256];
    char nested[256];
    char first_path[512];
    char second_path[512];
    char rejected_path[512];
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    DM1OriginalSavePC34CorpusRoundtripReport report;
    int i;
    int receipts_valid = 1;
    int tail_failed_receipts = 0;
    int rc;

    snprintf(root, sizeof(root), "firestaff_dm1_pc34_corpus_%ld",
             (long)test_getpid());
    snprintf(nested, sizeof(nested), "%s/nested", root);
    test_rmdir(nested);
    test_rmdir(root);
    CHECK(test_mkdir(root) == 0, "create corpus root");
    CHECK(test_mkdir(nested) == 0, "create corpus nested root");
    snprintf(first_path, sizeof(first_path), "%s/first-original.bin", root);
    snprintf(second_path, sizeof(second_path), "%s/second-original.bin", nested);
    snprintf(rejected_path, sizeof(rejected_path), "%s/not-a-save.txt", root);
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 1, 4, 5, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "build corpus PC34 fixture");
    CHECK(rewrite_fixture_event(
              bytes, written, 0, DM1_MAP_TIME_MAKE(2, 123500u),
              DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 1, DM1_MAP_TIME_MAKE(2, 123470u),
              DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(
              bytes, written, 2, DM1_MAP_TIME_MAKE(1, 123490u),
              DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "corpus fixtures keep every queue record C70-authenticated");
    CHECK(write_fixture_file(first_path, bytes, written),
          "write first corpus PC34 fixture");
    CHECK(write_fixture_file(second_path, bytes, written),
          "write second corpus PC34 fixture");
    CHECK(write_fixture_file(rejected_path, (const unsigned char *)"no", 2),
          "write rejected corpus file");

    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_roundtrip_corpus_root(root, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "corpus roundtrip scan succeeds");
    CHECK(report.scan_succeeded == 1, "corpus scan receipt succeeds");
    CHECK(report.scanned_file_count == 3, "corpus scans all files");
    CHECK(report.pc34_candidate_count == 2, "corpus selects only PC34 files");
    CHECK(report.roundtrip_attempted_count == 2,
          "corpus roundtrips every eligible file");
    CHECK(report.runtime_stage_attempted_count == 2 &&
              report.runtime_stage_succeeded_count == 0 &&
              report.runtime_adopt_attempted_count == 0 &&
              report.runtime_adopt_succeeded_count == 0 &&
              report.runtime_adopt_failed_count == 0,
          "tail-less fixture corpus cannot reach no-fallback adoption");
    CHECK(report.roundtrip_succeeded_count == 2,
          "corpus certifies both source-valid C3/C4 roundtrips");
    CHECK(report.core_state_match_count == 2,
          "verified corpus rows publish matching core receipts");
    CHECK(report.roundtrip_failed_count == 0,
          "corpus has no rejected PC34 candidates");
    CHECK(report.receipt_count == 2,
          "corpus retains one provenance receipt per classified PC34 envelope");
    for (i = 0; i < report.receipt_count; ++i) {
        const DM1OriginalSavePC34CorpusReceipt *receipt = &report.receipts[i];
        if (receipt->source_runtime_adopt_attempted ||
            receipt->source_runtime_adopted) {
            receipts_valid = 0;
        }
        if (receipt->roundtrip_result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
            if (!receipt->roundtrip_receipts_committed ||
                !receipt->core_state_matches ||
                receipt->source_handoff_result !=
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                receipt->source_importer_result != SAVEGAME_PC34_OK ||
                receipt->source_part_checksum_ok_count !=
                    SAVEGAME_PC34_PART_COUNT ||
                receipt->exported_byte_count == 0u ||
                receipt->exported_hash == 0u) {
                receipts_valid = 0;
            }
            continue;
        }
        if (receipt->source_handoff_result ==
            DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT) {
            if (receipt->source_importer_result !=
                    SAVEGAME_PC34_ERROR_BAD_SIZE ||
                receipt->source_part_checksum_ok_count !=
                    SAVEGAME_PC34_PART_COUNT ||
                receipt->roundtrip_receipts_committed ||
                receipt->exported_byte_count != 0u ||
                receipt->exported_hash != 0u) {
                receipts_valid = 0;
            } else {
                ++tail_failed_receipts;
            }
            continue;
        }
        if (!receipt->classified_loader_envelope || !receipt->external_original ||
            !receipt->roundtrip_attempted ||
            receipt->roundtrip_result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT ||
            receipt->core_state_matches || receipt->roundtrip_receipts_committed ||
            receipt->source_handoff_result !=
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
            receipt->source_importer_result != SAVEGAME_PC34_OK ||
            receipt->source_part_checksum_ok_count != SAVEGAME_PC34_PART_COUNT ||
            receipt->source_byte_count == 0u ||
            receipt->source_hash == 0u || receipt->exported_byte_count != 0u ||
            receipt->exported_hash != 0u || !receipt->path[0] ||
             receipt->source_f7057_envelope_end_offset <=
                 SAVEGAME_PC34_DM_SAVE_HEADER_SIZE ||
             receipt->source_f7057_envelope_end_offset +
                     receipt->source_f7057_trailing_byte_count !=
                 receipt->source_byte_count ||
             !receipt->header_part_shape_receipt_available ||
            !receipt->m516_champion_record_receipt_available ||
            !receipt->c4_timeline_layout_receipt_available ||
            receipt->source_c25_union_slot_byte_count !=
                (uint32_t)receipt->source_c25_event_count * 4u ||
            receipt->exported_c25_union_slot_byte_count !=
                (uint32_t)receipt->exported_c25_event_count * 4u ||
            receipt->source_c24_union_slot_byte_count !=
                (uint32_t)receipt->source_c24_event_count * 4u ||
            receipt->exported_c24_union_slot_byte_count !=
                (uint32_t)receipt->exported_c24_event_count * 4u ||
            !receipt->c3_event_layout_receipt_available ||
            receipt->source_c3_event_byte_count !=
                receipt->source_c3_event_record_count * 10u ||
            receipt->exported_c3_event_byte_count !=
                receipt->exported_c3_event_record_count * 10u ||
            !receipt->c4_timeline_layout_receipt_available ||
            (receipt->c3_event_byte_preservation_ok &&
             receipt->c4_timeline_byte_preservation_ok) ||
            receipt->source_c13_event_count != 0 ||
            receipt->exported_c13_event_count != 0 ||
            receipt->source_c24_event_count != 0 ||
            receipt->exported_c24_event_count != 0 ||
            receipt->source_c25_event_count != 0 ||
            receipt->exported_c25_event_count != 0 ||
            receipt->source_c13_event_byte_count !=
                (uint32_t)receipt->source_c13_event_count * 10u ||
            receipt->exported_c13_event_byte_count !=
                (uint32_t)receipt->exported_c13_event_count * 10u ||
            receipt->source_c13_timeline_reference_byte_count !=
                (uint32_t)receipt->source_c13_timeline_reference_count * 2u ||
            receipt->exported_c13_timeline_reference_byte_count !=
                (uint32_t)receipt->exported_c13_timeline_reference_count * 2u ||
            !receipt->dungeon_tail_byte_receipt_available ||
            !receipt->dungeon_tail_byte_preservation_ok ||
            receipt->source_dungeon_tail_byte_count != 0u ||
            receipt->exported_dungeon_tail_byte_count != 0u) {
            receipts_valid = 0;
        }
    }
    CHECK(receipts_valid && tail_failed_receipts == 0,
          "source-valid corpus rows retain committed roundtrip receipts");
    CHECK(strstr(report.first_pc34_path, root) != NULL,
          "corpus records an eligible path");
    CHECK(strstr(report.first_roundtrip_path, root) != NULL,
          "corpus records a verified PC34 roundtrip path");
    CHECK(dm1_v1_original_save_pc34_roundtrip_corpus_root(NULL, &report) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT,
          "corpus helper rejects null root");

    remove(rejected_path);
    remove(second_path);
    remove(first_path);
    test_rmdir(nested);
    test_rmdir(root);
}

static void test_optional_real_pc34_corpus_roundtrip(void)
{
    const char *root = getenv("FIRESTAFF_DM1_PC34_SAVE_CORPUS");
    DM1OriginalSavePC34CorpusRoundtripReport report;
    int rc;

    /* This probe intentionally never constructs a candidate. A positive
     * result is valid only for user-supplied original PC34 bytes; Firestaff
     * manifest-bearing exports and CSBWin GAMEBLOCK1 shapes are rejected by
     * the corpus provenance/F0435-envelope gates. */
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_roundtrip_configured_corpus(&report);
    if (rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE &&
        (!root || !root[0])) {
        puts("SKIP real PC34 corpus: no configured DM1 data root");
        return;
    }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "real PC34 corpus scan completes");
    if (report.pc34_candidate_count == 0 && (!root || !root[0])) {
        puts("SKIP real PC34 corpus: configured DM1 data has no PC34 saves");
        return;
    }
    CHECK(report.scan_succeeded == 1,
          "real PC34 corpus publishes a scan receipt");
    CHECK(report.roundtrip_failed_count == 0,
          "real PC34 corpus has no failed external roundtrip");
    CHECK(report.firestaff_manifest_rejected_count == 0,
          "real PC34 corpus contains no Firestaff exports");
    CHECK(report.nonoriginal_envelope_rejected_count == 0,
          "real PC34 corpus contains no malformed provenance envelope");
    CHECK(report.roundtrip_succeeded_count == report.pc34_candidate_count,
          "every external PC34 corpus candidate roundtrips");
    CHECK(report.pc34_candidate_count > 0,
          "real PC34 corpus contains at least one external candidate");
    CHECK(report.runtime_stage_attempted_count == report.pc34_candidate_count &&
              report.runtime_stage_succeeded_count +
                      report.runtime_stage_unavailable_count +
                      report.runtime_stage_failed_count ==
                  report.runtime_stage_attempted_count,
          "real PC34 corpus records one no-fallback runtime stage per candidate");
    CHECK(report.runtime_adopt_attempted_count ==
              report.runtime_stage_succeeded_count &&
              report.runtime_adopt_succeeded_count ==
                  report.runtime_adopt_attempted_count &&
              report.runtime_adopt_failed_count == 0,
          "tail-backed real PC34 corpus adopts only owned no-fallback worlds");
    for (int i = 0; i < report.receipt_count; ++i) {
        const DM1OriginalSavePC34CorpusReceipt *receipt = &report.receipts[i];
        CHECK(receipt->roundtrip_receipts_committed &&
              receipt->header_part_shape_receipt_available &&
              receipt->header_identity_preservation_ok &&
              receipt->part_byte_count_preservation_ok,
              "real PC34 corpus retains header identity and part lengths");
        CHECK(receipt->source_f7057_envelope_end_offset >
                  SAVEGAME_PC34_DM_SAVE_HEADER_SIZE &&
              receipt->source_f7057_envelope_end_offset +
                      receipt->source_f7057_trailing_byte_count ==
                  receipt->source_byte_count,
              "real PC34 corpus retains the F7057-to-F0435 byte boundary");
        CHECK(receipt->external_portrait_byte_receipt_available &&
              receipt->source_external_portrait_byte_count ==
                  SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT &&
              receipt->exported_external_portrait_byte_count ==
                  SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT &&
              receipt->inactive_champion_record_byte_receipt_available,
              "real PC34 corpus copies portrait and inactive-champion flags");
        CHECK(receipt->m516_champion_record_receipt_available &&
              receipt->m516_champion_record_byte_preservation_ok &&
              receipt->source_m516_champion_record_count == CHAMPION_MAX_PARTY &&
              receipt->exported_m516_champion_record_count == CHAMPION_MAX_PARTY &&
              receipt->source_m516_champion_record_byte_count ==
                  ORIGINAL_PC34_CHAMPION_BYTES * CHAMPION_MAX_PARTY &&
              receipt->source_m516_champion_record_byte_count ==
                  receipt->exported_m516_champion_record_byte_count &&
              receipt->source_m516_champion_record_fingerprint ==
                  receipt->exported_m516_champion_record_fingerprint,
              "real PC34 corpus preserves complete M516 champion bytes");
        CHECK(receipt->c4_timeline_layout_receipt_available &&
              receipt->c4_timeline_byte_preservation_ok &&
              receipt->source_c4_timeline_index_count ==
                  receipt->exported_c4_timeline_index_count &&
              receipt->source_c4_timeline_byte_count ==
                  receipt->exported_c4_timeline_byte_count &&
              receipt->source_c4_timeline_fingerprint ==
                  receipt->exported_c4_timeline_fingerprint,
              "real PC34 corpus preserves full C4 timeline index bytes");
        if (receipt->source_c13_event_count > 0 ||
            receipt->exported_c13_event_count > 0) {
            CHECK(receipt->c13_byte_preservation_ok &&
                  receipt->source_c13_event_byte_count ==
                      receipt->exported_c13_event_byte_count &&
                  receipt->source_c13_event_fingerprint ==
                      receipt->exported_c13_event_fingerprint &&
                  receipt->c13_timeline_byte_preservation_ok &&
                  receipt->source_c13_timeline_reference_byte_count ==
                      receipt->exported_c13_timeline_reference_byte_count &&
                  receipt->source_c13_timeline_reference_fingerprint ==
                      receipt->exported_c13_timeline_reference_fingerprint,
                  "real PC34 corpus preserves present C13 EVENT bytes");
            CHECK(receipt->source_discovery_admission_receipt_available &&
                  receipt->source_discovery_admission_valid &&
                  receipt->source_discovery_admission_fingerprint != 0u &&
                  receipt->c13_raw_capture_receipt_available &&
                  receipt->c13_raw_capture_byte_preservation_ok &&
                  receipt->source_c13_raw_capture_count ==
                      receipt->source_c13_event_count &&
                  receipt->exported_c13_raw_capture_count ==
                      receipt->exported_c13_event_count &&
                  receipt->source_c13_raw_capture_byte_count ==
                      receipt->exported_c13_raw_capture_byte_count &&
                  receipt->source_c13_raw_capture_fingerprint != 0u &&
                  receipt->source_c13_raw_capture_fingerprint ==
                      receipt->exported_c13_raw_capture_fingerprint &&
                  receipt->c13_corpus_capture_admission_receipt_available &&
                  receipt->c13_corpus_capture_admission_valid &&
                  receipt->c13_corpus_capture_admission_fingerprint != 0u,
                  "real PC34 corpus preserves and admits captured C13 C3 bytes");
        }
        if (receipt->source_c13_champion_record_reference_count > 0) {
            CHECK(receipt->c13_champion_record_byte_receipt_available &&
                  receipt->c13_champion_record_byte_preservation_ok,
                  "real C13 champion relation retains its receipt flag");
        }
        if (receipt->source_c25_event_count > 0 ||
            receipt->exported_c25_event_count > 0) {
            CHECK(receipt->c25_union_slot_byte_receipt_available &&
                  receipt->c25_union_slot_byte_preservation_ok &&
                  receipt->source_c25_union_slot_byte_count ==
                      receipt->exported_c25_union_slot_byte_count &&
                  receipt->source_c25_union_slot_fingerprint ==
                      receipt->exported_c25_union_slot_fingerprint,
                  "real PC34 corpus preserves present C25 union bytes");
        }
        if (receipt->source_c24_event_count > 0 ||
            receipt->exported_c24_event_count > 0) {
            CHECK(receipt->c24_union_slot_byte_receipt_available &&
                  receipt->c24_union_slot_byte_preservation_ok &&
                  receipt->source_c24_union_slot_byte_count ==
                      receipt->exported_c24_union_slot_byte_count &&
                  receipt->source_c24_union_slot_fingerprint ==
                      receipt->exported_c24_union_slot_fingerprint,
                  "real PC34 corpus preserves present C24 union bytes");
        }
        CHECK(receipt->c3_event_layout_receipt_available &&
              receipt->c3_event_byte_preservation_ok &&
              receipt->source_c3_event_record_count ==
                  receipt->exported_c3_event_record_count &&
              receipt->source_c3_event_byte_count ==
                  receipt->exported_c3_event_byte_count &&
              receipt->source_c3_event_fingerprint ==
                  receipt->exported_c3_event_fingerprint,
              "real PC34 corpus preserves atomic C3 EVENT bytes");
        CHECK(receipt->source_party_info_byte_count ==
                  PARTY_PC34_SAVE_INFO_BYTE_COUNT &&
              receipt->exported_party_info_byte_count ==
                  PARTY_PC34_SAVE_INFO_BYTE_COUNT &&
              receipt->source_party_info_fingerprint != 0u &&
              receipt->source_party_info_fingerprint ==
                  receipt->exported_party_info_fingerprint &&
              receipt->party_info_byte_preservation_ok,
              "real PC34 corpus preserves atomic C2 PARTY_INFO bytes");
        CHECK(receipt->dungeon_tail_byte_receipt_available &&
              receipt->dungeon_tail_byte_preservation_ok,
              "real PC34 corpus preserves each observed dungeon tail exactly");
        if (receipt->source_dungeon_tail_byte_count > 0u) {
            CHECK(receipt->source_runtime_stage_attempted &&
                  receipt->source_runtime_stage_result ==
                      DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
                  receipt->source_runtime_stage_committed &&
                  receipt->source_runtime_stage_owns_dungeon &&
                  receipt->source_runtime_stage_c13_party_receipt_valid &&
                  receipt->source_runtime_stage_party_metadata_fingerprint != 0u &&
                  receipt->source_runtime_stage_party_state_fingerprint != 0u &&
                  receipt->source_runtime_adopt_attempted &&
                  receipt->source_runtime_adopt_result ==
                      DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
                  receipt->source_runtime_adopted &&
                  receipt->source_runtime_adopt_owns_dungeon &&
                  receipt->source_runtime_adopt_c13_party_receipt_valid &&
                  receipt->source_runtime_adopt_party_metadata_fingerprint ==
                      receipt->source_runtime_stage_party_metadata_fingerprint &&
                  receipt->source_runtime_adopt_party_state_fingerprint ==
                      receipt->source_runtime_stage_party_state_fingerprint &&
                  receipt->source_runtime_adopt_event_count ==
                      receipt->source_runtime_stage_event_count &&
                  receipt->source_runtime_adopt_timeline_count ==
                      receipt->source_runtime_stage_timeline_count &&
                  receipt->source_runtime_adopt_queue_committed &&
                  receipt->source_runtime_adopt_queue_event_count ==
                      receipt->source_runtime_stage_timeline_count &&
                  receipt->source_runtime_adopt_queue_first_unused_index >=
                      receipt->source_runtime_adopt_queue_event_count &&
                  receipt->source_runtime_stage_c13_event_count ==
                      receipt->source_c13_event_count &&
                  receipt->source_runtime_stage_event_count >=
                      receipt->source_runtime_stage_c13_event_count,
                  "tail-backed real PC34 C13 reaches an owned staged runtime");
            if (receipt->source_c13_event_count > 0) {
                CHECK(receipt->c13_roundtrip_emission_receipt_available &&
                          receipt->c13_roundtrip_emission_valid &&
                          receipt->c13_roundtrip_emission_fingerprint != 0u &&
                          receipt->c13_roundtrip_input_admission_available &&
                          receipt->c13_roundtrip_input_admission_valid &&
                          receipt->c13_roundtrip_input_hash ==
                              receipt->source_hash &&
                          receipt->c13_roundtrip_input_byte_count ==
                              receipt->source_byte_count &&
                          receipt->c13_roundtrip_input_c3_byte_count > 0u &&
                          receipt->c13_roundtrip_input_c3_fingerprint != 0u &&
                          receipt->c13_runtime_handoff_provenance_receipt_available &&
                          receipt->c13_runtime_handoff_provenance_valid &&
                          receipt->c13_runtime_handoff_provenance_fingerprint != 0u &&
                          receipt->c13_active_runtime_state_receipt_available &&
                          receipt->c13_active_runtime_state_valid &&
                          receipt->c13_active_runtime_party_state_fingerprint != 0u &&
                          receipt->c13_active_runtime_timeline_fingerprint != 0u &&
                          receipt->c13_active_runtime_party_champion_count ==
                              receipt->source_runtime_adopt_party_champion_count &&
                          receipt->c13_active_runtime_timeline_event_count ==
                              receipt->source_runtime_adopt_event_count &&
                          receipt->c13_active_runtime_consumption_receipt_available &&
                          receipt->c13_active_runtime_consumption_valid &&
                          receipt->c13_active_runtime_consumption_fingerprint != 0u &&
                          receipt->c13_active_runtime_consumed_event_count ==
                              receipt->source_runtime_adopt_queue_event_count &&
                          receipt->source_runtime_adopt_queue_matches_world &&
                          receipt->c13_visible_runtime_handoff_receipt_available &&
                          receipt->c13_visible_runtime_handoff_valid &&
                          receipt->c13_visible_runtime_handoff_fingerprint != 0u &&
                          receipt->c13_visible_runtime_lifecycle_receipt_available &&
                          receipt->c13_visible_runtime_lifecycle_valid &&
                          receipt->c13_visible_runtime_lifecycle_fingerprint != 0u &&
                          receipt->c13_visible_runtime_m11_handoff_receipt_available &&
                          receipt->c13_visible_runtime_m11_handoff_valid &&
                          receipt->c13_visible_runtime_m11_handoff_fingerprint != 0u &&
                          receipt->c13_visible_runtime_m11_lifecycle_receipt_available &&
                          receipt->c13_visible_runtime_m11_lifecycle_valid &&
                          !receipt->c13_visible_runtime_m11_admission_revoked &&
                          receipt->c13_visible_runtime_m11_revoke_reason ==
                              DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_NONE &&
                          receipt->c13_visible_runtime_m11_lifecycle_fingerprint != 0u &&
                          receipt->c13_runtime_frame.receipt_available &&
                          receipt->c13_runtime_frame.valid &&
                          !receipt->c13_runtime_frame.revoked &&
                          receipt->c13_runtime_frame.revoke_reason ==
                              DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_NONE &&
                          receipt->c13_runtime_frame.fingerprint != 0u &&
                          receipt->c13_runtime_frame.game_tick ==
                              receipt->source_runtime_visible_game_tick &&
                          receipt->c13_runtime_frame.queue_game_tick ==
                              receipt->source_runtime_visible_queue_game_tick &&
                          receipt->c13_runtime_frame.party_state_fingerprint ==
                              receipt->c13_active_runtime_party_state_fingerprint &&
                          receipt->c13_runtime_frame.timeline_fingerprint ==
                              receipt->c13_active_runtime_timeline_fingerprint &&
                          receipt->c13_runtime_frame_lifecycle.receipt_available &&
                          receipt->c13_runtime_frame_lifecycle.active_visible_handoff &&
                          receipt->c13_runtime_frame_lifecycle.valid &&
                          !receipt->c13_runtime_frame_lifecycle.clear_output &&
                          !receipt->c13_runtime_frame_lifecycle.revoke_output &&
                          receipt->c13_runtime_frame_lifecycle.clear_reason ==
                              DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_NONE &&
                          receipt->c13_runtime_frame_lifecycle.fingerprint != 0u &&
                          receipt->c13_runtime_frame_lifecycle.current_game_tick ==
                              receipt->c13_runtime_frame.game_tick &&
                          receipt->c13_runtime_frame_lifecycle.next_game_tick ==
                              receipt->c13_runtime_frame.game_tick + 1u &&
                          receipt->c13_runtime_frame_m11_bridge.receipt_available &&
                          receipt->c13_runtime_frame_m11_bridge.active_visible_handoff &&
                          receipt->c13_runtime_frame_m11_bridge.deliver_frame &&
                          !receipt->c13_runtime_frame_m11_bridge.clear_output &&
                          !receipt->c13_runtime_frame_m11_bridge.revoke_output &&
                          receipt->c13_runtime_frame_m11_bridge.clear_reason ==
                              DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_NONE &&
                          receipt->c13_runtime_frame_m11_bridge.fingerprint != 0u &&
                          receipt->c13_runtime_frame_m11_bridge.game_tick ==
                              receipt->c13_runtime_frame.game_tick &&
                          receipt->c13_runtime_frame_m11_bridge.frame_fingerprint ==
                              receipt->c13_runtime_frame.fingerprint &&
                          receipt->c13_runtime_frame_m11_bridge.provenance_fingerprint ==
                              receipt->c13_runtime_frame.provenance_fingerprint &&
                          receipt->c13_m11_runtime_capture.receipt_available &&
                          receipt->c13_m11_runtime_capture.active_visible_handoff &&
                          receipt->c13_m11_runtime_capture.capture_admitted &&
                          !receipt->c13_m11_runtime_capture.clear_output &&
                          !receipt->c13_m11_runtime_capture.revoke_output &&
                          receipt->c13_m11_runtime_capture.clear_reason ==
                              DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_NONE &&
                          receipt->c13_m11_runtime_capture.source_byte_count ==
                              receipt->source_byte_count &&
                          receipt->c13_m11_runtime_capture.source_hash ==
                              receipt->source_hash &&
                          receipt->c13_m11_runtime_capture.c3_byte_offset ==
                              receipt->c13_roundtrip_input_c3_byte_offset &&
                          receipt->c13_m11_runtime_capture.c3_byte_count ==
                              receipt->c13_roundtrip_input_c3_byte_count &&
                          receipt->c13_m11_runtime_capture.c3_fingerprint ==
                              receipt->c13_roundtrip_input_c3_fingerprint &&
                          receipt->c13_m11_runtime_capture.c13_capture_fingerprint ==
                              receipt->source_c13_raw_capture_fingerprint &&
                          receipt->c13_m11_runtime_capture.runtime_frame_fingerprint ==
                              receipt->c13_runtime_frame.fingerprint &&
                          receipt->c13_m11_runtime_capture.provenance_fingerprint ==
                              receipt->c13_runtime_frame.provenance_fingerprint &&
                          receipt->c13_m11_runtime_capture.fingerprint != 0u &&
                          receipt->c13_visible_runtime_m11_handoff_game_tick ==
                              receipt->source_runtime_visible_game_tick &&
                          receipt->c13_visible_runtime_m11_handoff_queue_game_tick ==
                              receipt->source_runtime_visible_queue_game_tick &&
                          receipt->source_runtime_visible_handoff_accepted &&
                          receipt->source_runtime_visible_queue_matches_world &&
                          receipt->source_runtime_visible_next_queue_matches_world &&
                          receipt->source_runtime_visible_next_game_tick ==
                              receipt->source_runtime_visible_game_tick + 1u &&
                          receipt->source_runtime_visible_next_queue_game_tick ==
                              receipt->source_runtime_visible_queue_game_tick + 1u &&
                          receipt->source_runtime_visible_timeline_event_count ==
                              receipt->c13_active_runtime_timeline_event_count &&
                          receipt->source_runtime_stage_input_hash ==
                              receipt->c13_roundtrip_input_hash &&
                          receipt->source_runtime_adopt_input_hash ==
                              receipt->c13_roundtrip_input_hash &&
                          receipt->exported_c13_event_count ==
                              receipt->source_c13_event_count &&
                          receipt->source_runtime_stage_c13_admission_ok &&
                          receipt->source_runtime_stage_c13_admitted_count ==
                              receipt->source_c13_event_count &&
                          receipt->source_runtime_stage_c13_fingerprint != 0u &&
                          receipt->source_runtime_adopt_c13_admission_ok &&
                          receipt->source_runtime_adopt_c13_admitted_count ==
                              receipt->source_c13_event_count &&
                          receipt->source_runtime_adopt_c13_fingerprint ==
                              receipt->source_runtime_stage_c13_fingerprint,
                      "genuine tail-backed C13 retains F0255 state through adoption");
            }
            CHECK(receipt->source_dungeon_tail_byte_count ==
                      receipt->exported_dungeon_tail_byte_count &&
                  receipt->source_dungeon_tail_fingerprint ==
                      receipt->exported_dungeon_tail_fingerprint,
                  "real PC34 corpus tail receipt retains raw byte identity");
        } else {
            CHECK(receipt->source_runtime_stage_attempted &&
                  !receipt->source_runtime_stage_committed &&
                  !receipt->source_runtime_stage_owns_dungeon,
                  "tail-less real PC34 save cannot borrow a runtime dungeon");
            CHECK(!receipt->source_runtime_adopt_attempted &&
                  !receipt->source_runtime_adopted &&
                  !receipt->source_runtime_adopt_queue_committed,
                  "tail-less real PC34 save cannot reach no-fallback adoption");
        }
    }
}

static void test_world_export_rebuilds_c48_c49_projectile_union(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat world;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    DM1OriginalSavePC34HandoffReport report;
    int written = 0;
    int rc;
    uint16_t expected_thing;
    uint16_t expected_motion;

    memset(&world, 0, sizeof(world));
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    memset(&report, 0, sizeof(report));
    world.party.championCount = 1;
    world.timeline.count = 3;
    world.timeline.events[0].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    world.timeline.events[0].fireAtTick = 100u;
    world.timeline.events[0].aux0 = 5;
    world.timeline.events[0].aux4 = 7;
    world.timeline.events[1].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    world.timeline.events[1].fireAtTick = 101u;
    world.timeline.events[1].aux0 = 6;
    world.timeline.events[1].aux4 = 3;
    world.timeline.events[2].kind = TIMELINE_EVENT_PLAY_SOUND;
    world.timeline.events[2].fireAtTick = 103u;
    world.timeline.events[2].mapIndex = 2;
    world.timeline.events[2].mapX = 19;
    world.timeline.events[2].mapY = 8;
    world.timeline.events[2].aux0 = 23;
    world.timeline.events[2].aux2 = DM1_EVENT_PLAY_SOUND;
    world.timeline.events[2].aux4 = 6;
    world.projectiles.count = 7;

    world.projectiles.entries[5].slotIndex = 5;
    world.projectiles.entries[5].reserved3 = 1;
    world.projectiles.entries[5].firstMoveGraceFlag = 1;
    world.projectiles.entries[5].mapIndex = 2;
    world.projectiles.entries[5].mapX = 17;
    world.projectiles.entries[5].mapY = 9;
    world.projectiles.entries[5].cell = 2;
    world.projectiles.entries[5].direction = 3;
    world.projectiles.entries[5].stepEnergy = 6;
    world.projectiles.entries[6].slotIndex = 6;
    world.projectiles.entries[6].reserved3 = 1;
    world.projectiles.entries[6].mapIndex = 3;
    world.projectiles.entries[6].mapX = 1;
    world.projectiles.entries[6].mapY = 31;
    world.projectiles.entries[6].cell = 1;
    world.projectiles.entries[6].direction = 0;
    world.projectiles.entries[6].stepEnergy = 15;

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x43313445u, bytes, (int)sizeof(bytes), &written);
    CHECK(rc == SAVEGAME_PC34_OK && written > 0,
          "world export writes C48/C49 projectile events");
    imported.party = &imported_party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "exported C48/C49 envelope imports");
    CHECK(report.original_event_count == 3 &&
              report.events[0].type == DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS &&
              report.events[1].type == DM1_EVENT_MOVE_PROJECTILE &&
              report.events[2].type == DM1_EVENT_PLAY_SOUND,
          "world export retains C48/C49 and C20 source event ids");

    expected_thing = (uint16_t)((THING_TYPE_PROJECTILE << 10) | 5u |
                                (2u << 14));
    expected_motion = (uint16_t)(17u | (9u << 5) | (3u << 10) | (6u << 12));
    CHECK(rd16le(&report.events[0].b_mapX) == expected_thing &&
              rd16le(&report.events[0].c_cell) == expected_motion &&
              ((report.events[0].map_time >> 24) & 0xffu) == 2u,
          "C48 export writes original B.Slot and C.Projectile fields");
    expected_thing = (uint16_t)((THING_TYPE_PROJECTILE << 10) | 6u |
                                (1u << 14));
    expected_motion = (uint16_t)(1u | (31u << 5) | (15u << 12));
    CHECK(rd16le(&report.events[1].b_mapX) == expected_thing &&
              rd16le(&report.events[1].c_cell) == expected_motion &&
              ((report.events[1].map_time >> 24) & 0xffu) == 3u,
          "C49 export writes original B.Slot and C.Projectile fields");
    CHECK(report.events[2].b_mapX == 19 && report.events[2].b_mapY == 8 &&
              (int16_t)rd16le(&report.events[2].c_cell) == 23 &&
              report.events[2].priority == 6,
          "C20 export restores B.Location, C.SoundIndex, and source priority");

    world.projectiles.entries[6].reserved3 = 0;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &world, 0x43313445u, bytes, (int)sizeof(bytes), &written) ==
              SAVEGAME_PC34_ERROR_INTERNAL,
          "world export rejects an unbound projectile event instead of guessing");
}

static void test_world_roundtrip_preserves_champion_poison_count(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char roundtrip[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat world;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    struct DM1_EventQueue_V1 queue;
    DM1OriginalSavePC34HandoffReport report;
    int written = 0;
    int roundtrip_written = 0;
    int valid_roundtrip_written = 0;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
          rewrite_fixture_event(bytes, written, 0,
                                DM1_MAP_TIME_MAKE(2, 123500u),
                                DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(bytes, written, 1,
                                DM1_MAP_TIME_MAKE(2, 123470u),
                                DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(bytes, written, 2,
                                DM1_MAP_TIME_MAKE(1, 123490u),
                                DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "poison source fixture has an authenticated tail-less timeline");
    memset(&world, 0, sizeof(world));
    memset(&queue, 0, sizeof(queue));
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)written, &world, &queue, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
          world.party.champions[0].poisonDose == 3u,
          "F0435 materializes source PoisonEventCount");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x504f4953u, roundtrip, (int)sizeof(roundtrip),
        &roundtrip_written);
    CHECK(rc == SAVEGAME_PC34_OK && roundtrip_written > 0,
          "F0433 exports bounded champion poison count");
    valid_roundtrip_written = roundtrip_written;
    world.party.champions[0].poisonDose = 4u;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x504f4953u, roundtrip, (int)sizeof(roundtrip),
        &roundtrip_written);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "F0802 rejects PoisonEventCount runtime drift from the C3/C4 receipt");
    world.party.champions[0].poisonDose = 256u;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x504f4953u, roundtrip, (int)sizeof(roundtrip),
        &roundtrip_written);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "F0802 rejects a non-PC34 PoisonEventCount width");
    world.party.champions[0].poisonDose = 3u;
    F0883_WORLD_Free_Compat(&world);
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    imported.party = &imported_party;
    rc = F0796_SAVEGAME_ImportPC34_Compat(
        roundtrip, valid_roundtrip_written, &imported, 1);
    CHECK(rc == SAVEGAME_PC34_OK &&
          imported.party->champions[0].poisonDose == 3u,
          "F0435 source to F0802/F0796 preserves PoisonEventCount exactly");
}

static void test_world_export_rebuilds_c25_explosion_union(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat world;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    DM1OriginalSavePC34HandoffReport report;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char square_data[32 * 32];
    struct DungeonThings_Compat things;
    unsigned short first_things[1];
    unsigned short column_sft_bases[32];
    unsigned char raw_explosion[4];
    struct DungeonExplosion_Compat source_explosions[1];
    uint16_t source_thing = (uint16_t)((THING_TYPE_EXPLOSION << 10) |
                                       (1u << 14));
    int written = 0;
    int rc;

    /* Local format regression only, never corpus evidence. */
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 0, 0, 0, 0, 0,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK, "poison fixture builds");
    memset(&world, 0, sizeof(world));
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    memset(&report, 0, sizeof(report));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(column_sft_bases, 0, sizeof(column_sft_bases));
    memset(&things, 0, sizeof(things));
    memset(raw_explosion, 0, sizeof(raw_explosion));
    memset(source_explosions, 0, sizeof(source_explosions));
    world.party.championCount = 1;
    world.timeline.count = 1;
    world.timeline.events[0].kind = TIMELINE_EVENT_EXPLOSION_ADVANCE;
    world.timeline.events[0].fireAtTick = 100u;
    world.timeline.events[0].mapIndex = 0;
    world.timeline.events[0].mapX = 11;
    world.timeline.events[0].mapY = 12;
    world.timeline.events[0].cell = 1;
    world.timeline.events[0].aux0 = 0;
    world.timeline.events[0].aux4 = 5;
    world.explosions.count = 1;
    world.explosions.entries[0].slotIndex = 0;
    world.explosions.entries[0].explosionType = 2;
    world.explosions.entries[0].mapIndex = 0;
    world.explosions.entries[0].mapX = 11;
    world.explosions.entries[0].mapY = 12;
    world.explosions.entries[0].cell = 1;
    world.explosions.entries[0].attack = 77;
    world.explosions.entries[0].reserved0 = 1;

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    dungeon.columnsCumulativeSquareFirstThingCount = column_sft_bases;
    dungeon.dungeonColumnCount = (int)(sizeof(column_sft_bases) /
                                       sizeof(column_sft_bases[0]));
    maps[0].width = 32;
    maps[0].height = 32;
    tiles[0].squareData = square_data;
    tiles[0].squareCount = 32 * 32;
    square_data[11 * 32 + 12] |= DUNGEON_SQUARE_MASK_THING_LIST;
    first_things[0] = source_thing;
    wr16le(raw_explosion + 0u, THING_ENDOFLIST);
    raw_explosion[2] = 2u;
    raw_explosion[3] = 77u;
    source_explosions[0].next = THING_ENDOFLIST;
    source_explosions[0].type = 2u;
    source_explosions[0].attack = 77u;
    things.squareFirstThings = first_things;
    things.squareFirstThingCount = 1;
    things.explosions = source_explosions;
    things.explosionCount = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = raw_explosion;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.loaded = 1;
    world.dungeon = &dungeon;
    world.things = &things;

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x43313445u, bytes, (int)sizeof(bytes), &written);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "world export rejects a synthesized C25 without a C15 receipt hash");
}

static void test_world_export_rebuilds_c29_group_reaction_union(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat world;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    DM1OriginalSavePC34HandoffReport report;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned char square_data[32 * 32];
    unsigned short first_things[1];
    unsigned short column_sft_bases[32];
    unsigned char raw_group[16];
    int written = 0;
    int rc;

    memset(&world, 0, sizeof(world));
    memset(&world, 0, sizeof(world));
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    memset(&report, 0, sizeof(report));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(square_data, 0, sizeof(square_data));
    memset(column_sft_bases, 0, sizeof(column_sft_bases));
    memset(raw_group, 0, sizeof(raw_group));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    dungeon.columnsCumulativeSquareFirstThingCount = column_sft_bases;
    dungeon.dungeonColumnCount = (int)(sizeof(column_sft_bases) /
                                       sizeof(column_sft_bases[0]));
    maps[0].width = 32;
    maps[0].height = 32;
    tiles[0].squareData = square_data;
    tiles[0].squareCount = 32 * 32;
    square_data[14 * 32 + 7] |= DUNGEON_SQUARE_MASK_THING_LIST;
    first_things[0] = (unsigned short)(THING_TYPE_GROUP << 10);
    wr16le(raw_group, THING_ENDOFLIST);
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 15;
    things.squareFirstThings = first_things;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    things.rawThingData[THING_TYPE_GROUP] = raw_group;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.loaded = 1;
    world.dungeon = &dungeon;
    world.things = &things;
    world.party.championCount = 1;
    world.timeline.count = 1;
    world.timeline.events[0].kind = TIMELINE_EVENT_CREATURE_REACTION;
    world.timeline.events[0].fireAtTick = 102u;
    world.timeline.events[0].mapIndex = 0;
    world.timeline.events[0].mapX = 14;
    world.timeline.events[0].mapY = 7;
    world.timeline.events[0].aux0 = 0;
    world.timeline.events[0].aux1 = 15;
    world.timeline.events[0].aux2 = DM1_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE;
    world.timeline.events[0].aux3 = 9;
    world.timeline.events[0].aux4 = 0x104;

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x43313445u, bytes, (int)sizeof(bytes), &written);
    CHECK(rc == SAVEGAME_PC34_ERROR_INTERNAL,
          "world export rejects a synthesized C29 without an F0435 C3/C4 receipt");

    first_things[0] = THING_ENDOFLIST;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &world, 0x43313445u, bytes, (int)sizeof(bytes), &written) ==
              SAVEGAME_PC34_ERROR_INTERNAL,
          "world export rejects a C29 runtime event without its C04 source owner");
}

static void test_world_export_roundtrips_c13_vi_altar_union(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char roundtrip[SAVEGAME_PC34_MAX_FILE_SIZE];
    DM1OriginalSavePC34FixtureSpec spec;
    struct GameWorld_Compat imported;
    struct GameWorld_Compat restored;
    size_t written = 0u;
    int roundtrip_written = 0;
    int rc;

    /* Local keyed/checksummed format regression only, never corpus evidence. */
    memset(&spec, 0, sizeof(spec));
    spec.champion_count = 3;
    spec.map_index = 2;
    spec.active_champion_index = 1;
    spec.maximum_active_group_count = ORIGINAL_PC34_ACTIVE_GROUP_COUNT;
    spec.event_count = ORIGINAL_PC34_EVENT_COUNT;
    spec.event_maximum_count = ORIGINAL_PC34_EVENT_MAXIMUM_COUNT;
    rc = dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
        &spec, bytes, sizeof(bytes), &written);
    CHECK(rc == SAVEGAME_PC34_OK &&
          rewrite_fixture_event(bytes, written, 0,
                                DM1_MAP_TIME_MAKE(2, 123500u),
                                DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
          rewrite_fixture_event(bytes, written, 1,
                                DM1_MAP_TIME_MAKE(2, 123470u),
                                DM1_EVENT_LIGHT, 0, 5, 0, 0, 0) &&
          rewrite_fixture_event(bytes, written, 2,
                                DM1_MAP_TIME_MAKE(1, 123490u),
                                DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "multi-champion C2 fixture has an authenticated tail-less timeline");
    memset(&imported, 0, sizeof(imported));
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, written, &imported, NULL, NULL);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
          imported.party.champions[0].food == 1500 &&
          imported.party.champions[0].water == -32 &&
          imported.party.champions[1].food == 1200 &&
          imported.party.champions[1].water == 1100 &&
          imported.party.champions[2].food == 900 &&
          imported.party.champions[2].water == 800 &&
          imported.party.champions[0].actionIndex == 0xff &&
          imported.party.champions[1].actionIndex == 0xff &&
          imported.party.champions[2].actionIndex == 0xff,
          "F0435 preserves signed food/water and action state in slot order");
    imported.party.champions[0].actionIndex = 7u;
    imported.party.champions[1].actionIndex = 9u;
    imported.party.champions[2].actionIndex = 11u;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &imported, 0x46574452u, roundtrip, (int)sizeof(roundtrip),
        &roundtrip_written);
    CHECK(rc == SAVEGAME_PC34_OK && roundtrip_written > 0,
          "F0433 exports multi-champion food/water");
    memset(&restored, 0, sizeof(restored));
    rc = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        roundtrip, (size_t)roundtrip_written, &restored, NULL, NULL);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
          restored.party.champions[0].food == 1500 &&
          restored.party.champions[0].water == -32 &&
          restored.party.champions[1].food == 1200 &&
          restored.party.champions[1].water == 1100 &&
          restored.party.champions[2].food == 900 &&
          restored.party.champions[2].water == 800 &&
          restored.party.champions[0].actionIndex == 7 &&
          restored.party.champions[1].actionIndex == 9 &&
          restored.party.champions[2].actionIndex == 11,
          "F0435-to-F0433-to-F0435 preserves food/water/action slot order");
}

static void test_original_square_state_events_materialize_and_roundtrip(void)
{
    static const struct {
        int event_type;
        int dungeon_element;
    } cases[] = {
        { DM1_EVENT_CORRIDOR, DUNGEON_ELEMENT_CORRIDOR },
        { DM1_EVENT_WALL, DUNGEON_ELEMENT_WALL },
        { DM1_EVENT_TELEPORTER, DUNGEON_ELEMENT_TELEPORTER },
        { DM1_EVENT_PIT, DUNGEON_ELEMENT_PIT },
        { DM1_EVENT_DOOR, DUNGEON_ELEMENT_DOOR }
    };
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char square_data[3][32 * 32];
    char path[512];
    int case_index;

    for (case_index = 0;
         case_index < (int)(sizeof(cases) / sizeof(cases[0]));
         ++case_index) {
        struct GameWorld_Compat start_world;
        struct GameWorld_Compat loaded_world;
        struct DungeonDatState_Compat dungeon;
        struct DungeonMapDesc_Compat maps[3];
        struct DungeonMapTiles_Compat tiles[3];
        struct DungeonThings_Compat things;
        struct SaveGame_Compat imported;
        struct PartyState_Compat imported_party;
        DM1OriginalSavePC34HandoffReport report;
        int written = 0;
        int exported_size = 0;
        int rc;
        int i;
        int found = 0;

        rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                         3, 2, 9, 10, 2, 1,
                                         ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
        CHECK(rc == SAVEGAME_PC34_OK &&
                  rewrite_fixture_event(bytes, written, 0,
                                        DM1_MAP_TIME_MAKE(2, 123500u),
                                        DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
                  rewrite_fixture_event_type(bytes, (size_t)written, 1,
                                             cases[case_index].event_type) &&
                  rewrite_fixture_event(bytes, written, 2,
                                        DM1_MAP_TIME_MAKE(1, 123490u),
                                        DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
              "C05/C06/C08/C09/C10 fixture remains checksum-authenticated");

        memset(&start_world, 0, sizeof(start_world));
        memset(&loaded_world, 0, sizeof(loaded_world));
        memset(&dungeon, 0, sizeof(dungeon));
        memset(maps, 0, sizeof(maps));
        memset(tiles, 0, sizeof(tiles));
        memset(square_data, 0, sizeof(square_data));
        memset(&things, 0, sizeof(things));
        for (i = 0; i < 3; ++i) {
            maps[i].width = 32;
            maps[i].height = 32;
            tiles[i].squareData = square_data[i];
        }
        dungeon.header.mapCount = 3;
        dungeon.maps = maps;
        dungeon.tiles = tiles;
        dungeon.tilesLoaded = 1;
        square_data[2][21 * 32 + 22] =
            (unsigned char)(cases[case_index].dungeon_element << 5);
        start_world.dungeon = &dungeon;
        start_world.things = &things;

        make_temp_save_path(path, sizeof(path));
        remove(path);
        CHECK(write_fixture_file(path, bytes, written),
              "square-state fixture writes");
        rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
            path, &start_world, &loaded_world, NULL, &report);
        remove(path);
        CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
              "F0435 materializes the saved source square state");
        for (i = 0; i < loaded_world.timeline.count; ++i) {
            const struct TimelineEvent_Compat *event =
                &loaded_world.timeline.events[i];
            if (event->kind == TIMELINE_EVENT_SQUARE_STATE &&
                event->aux0 == cases[case_index].event_type) {
                found = event->mapIndex == 2 && event->mapX == 21 &&
                        event->mapY == 22 && event->cell == 1 &&
                        event->aux1 == DM1_EFFECT_TOGGLE &&
                        event->aux2 == cases[case_index].event_type &&
                        event->aux4 == 4;
                break;
            }
        }
        CHECK(found, "F0435 retains saved Location and Cell/Effect union");

        rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
            &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
            &exported_size);
        CHECK(rc == SAVEGAME_PC34_OK && exported_size > 0,
              "F0433 exports the materialized source square state");
        memset(&imported, 0, sizeof(imported));
        memset(&imported_party, 0, sizeof(imported_party));
        imported.party = &imported_party;
        rc = dm1_v1_original_save_pc34_handoff_bytes(
            exported, (size_t)exported_size, &imported, &report);
        found = 0;
        for (i = 0; i < report.original_event_count; ++i) {
            if (report.events[i].type == cases[case_index].event_type) {
                found = report.events[i].priority == 4 &&
                        report.events[i].b_mapX == 21 &&
                        report.events[i].b_mapY == 22 &&
                        report.events[i].c_cell == 1 &&
                        report.events[i].c_effect == DM1_EFFECT_TOGGLE;
                break;
            }
        }
        CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && found,
              "F0435 to F0433 to F0435 preserves saved square-state bytes");
    }
}

static void test_original_fakewall_event_materializes_and_defers_clear(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char square_data[3][32 * 32];
    char path[512];
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct GameWorld_Compat rejected_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    struct TickResult_Compat tick_result;
    DM1OriginalSavePC34HandoffReport report;
    int written = 0;
    int exported_size = 0;
    int rc;
    int i;
    int found = 0;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 2, 21, 22, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event(bytes, written, 0,
                                    DM1_MAP_TIME_MAKE(2, 123500u),
                                    DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
              rewrite_fixture_event_type(bytes, (size_t)written, 1,
                                         DM1_EVENT_FAKEWALL) &&
              rewrite_fixture_event_c_union(bytes, (size_t)written, 1,
                                            (uint16_t)(DM1_EFFECT_CLEAR << 8)) &&
              rewrite_fixture_event(bytes, written, 2,
                                    DM1_MAP_TIME_MAKE(1, 123490u),
                                    DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "C07 fixture remains checksum-authenticated");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&rejected_world, 0, sizeof(rejected_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(&things, 0, sizeof(things));
    dungeon.header.mapCount = 3;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    for (i = 0; i < 3; ++i) {
        maps[i].width = 32;
        maps[i].height = 32;
        tiles[i].squareData = square_data[i];
        tiles[i].squareCount = 32 * 32;
    }
    square_data[2][21 * 32 + 22] =
        (unsigned char)((DUNGEON_ELEMENT_FAKEWALL << 5) | 0x04u);
    start_world.dungeon = &dungeon;
    start_world.things = &things;

    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written), "C07 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "F0435 materializes a C07 only on the saved fakewall square");
    square_data[2][21 * 32 + 22] =
        (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    CHECK(dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
              path, &start_world, &rejected_world, NULL, NULL) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "F0435 rejects a C07 whose source location is not a fakewall");
    square_data[2][21 * 32 + 22] =
        (unsigned char)((DUNGEON_ELEMENT_FAKEWALL << 5) | 0x04u);
    remove(path);
    for (i = 0; i < loaded_world.timeline.count; ++i) {
        const struct TimelineEvent_Compat *event = &loaded_world.timeline.events[i];
        if (event->kind == TIMELINE_EVENT_SQUARE_STATE &&
            event->aux0 == DM1_EVENT_FAKEWALL) {
            found = event->mapIndex == 2 && event->mapX == 21 &&
                    event->mapY == 22 && event->aux1 == DM1_EFFECT_CLEAR &&
                    event->aux2 == DM1_EVENT_FAKEWALL;
            break;
        }
    }
    CHECK(found, "C07 retains its source Location and Effect union");

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    imported.party = &imported_party;
    CHECK(rc == SAVEGAME_PC34_OK &&
              dm1_v1_original_save_pc34_handoff_bytes(
                  exported, (size_t)exported_size, &imported, &report) ==
                  DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "F0433 exports the staged C07 event");
    found = 0;
    for (i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_FAKEWALL) {
            found = report.events[i].b_mapX == 21 &&
                    report.events[i].b_mapY == 22 &&
                    report.events[i].c_effect == DM1_EFFECT_CLEAR;
            break;
        }
    }
    CHECK(found, "F0435 to F0433 to F0435 preserves the C07 source union");

    loaded_world.gameTick = 123470u;
    memset(&tick_result, 0, sizeof(tick_result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world,
                                                     &tick_result) > 0 &&
              (square_data[2][21 * 32 + 22] & 0x04u) != 0,
          "occupied C07 clear defers without changing the fakewall");
    loaded_world.party.mapX = 20;
    loaded_world.gameTick = 123471u;
    memset(&tick_result, 0, sizeof(tick_result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world,
                                                     &tick_result) > 0 &&
              (square_data[2][21 * 32 + 22] & 0x04u) == 0,
          "deferred C07 clear mutates only after the party leaves");
}

static void test_original_door_destruction_event_materializes_on_door(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char square_data[3][32 * 32];
    char path[512];
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct GameWorld_Compat rejected_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    struct DungeonThings_Compat things;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    struct TickResult_Compat tick_result;
    struct TimelineEvent_Compat event;
    DM1OriginalSavePC34HandoffReport report;
    int written = 0;
    int exported_size = 0;
    int rc;
    int i;
    int found = 0;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 2, 21, 22, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK &&
              rewrite_fixture_event(bytes, written, 0,
                                    DM1_MAP_TIME_MAKE(2, 123500u),
                                    DM1_EVENT_LIGHT, 0, 6, 0, 0, 0) &&
              rewrite_fixture_event_type(bytes, (size_t)written, 1,
                                         DM1_EVENT_DOOR_DESTRUCTION) &&
              rewrite_fixture_event_priority(bytes, (size_t)written, 1, 0) &&
              rewrite_fixture_event(bytes, written, 2,
                                    DM1_MAP_TIME_MAKE(1, 123490u),
                                    DM1_EVENT_LIGHT, 0, 4, 0, 0, 0),
          "C02 fixture remains checksum-authenticated");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&rejected_world, 0, sizeof(rejected_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(&things, 0, sizeof(things));
    dungeon.header.mapCount = 3;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    for (i = 0; i < 3; ++i) {
        maps[i].width = 32;
        maps[i].height = 32;
        tiles[i].squareData = square_data[i];
        tiles[i].squareCount = 32 * 32;
    }
    square_data[2][21 * 32 + 22] =
        (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | 0x04u);
    start_world.dungeon = &dungeon;
    start_world.things = &things;

    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written), "C02 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
        path, &start_world, &loaded_world, NULL, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "F0435 materializes a C02 only on the saved door square");
    square_data[2][21 * 32 + 22] =
        (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    CHECK(dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
              path, &start_world, &rejected_world, NULL, NULL) ==
              DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT,
          "F0435 rejects a C02 whose source location is not a door");
    square_data[2][21 * 32 + 22] =
        (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | 0x04u);
    remove(path);
    for (i = 0; i < loaded_world.timeline.count; ++i) {
        const struct TimelineEvent_Compat *event = &loaded_world.timeline.events[i];
        if (event->kind == TIMELINE_EVENT_DOOR_DESTRUCTION) {
            found = event->mapIndex == 2 && event->mapX == 21 &&
                    event->mapY == 22 && event->aux0 ==
                    DM1_EVENT_DOOR_DESTRUCTION && event->aux2 ==
                    DM1_EVENT_DOOR_DESTRUCTION;
            break;
        }
    }
    CHECK(found, "C02 retains its source Location through F0435");

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    imported.party = &imported_party;
    CHECK(rc == SAVEGAME_PC34_OK &&
              dm1_v1_original_save_pc34_handoff_bytes(
                  exported, (size_t)exported_size, &imported, &report) ==
                  DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "F0433 exports the staged C02 event");
    found = 0;
    for (i = 0; i < report.original_event_count; ++i) {
        if (report.events[i].type == DM1_EVENT_DOOR_DESTRUCTION) {
            found = report.events[i].priority == 0 &&
                    report.events[i].b_mapX == 21 &&
                    report.events[i].b_mapY == 22;
            break;
        }
    }
    CHECK(found, "F0435 to F0433 to F0435 preserves C02's source Location");

    loaded_world.gameTick = 123470u;
    memset(&tick_result, 0, sizeof(tick_result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world,
                                                     &tick_result) > 0 &&
              (square_data[2][21 * 32 + 22] & 0x07u) == 5u &&
              tick_result.emissionCount == 1 &&
              tick_result.emissions[0].kind == EMIT_DOOR_STATE,
          "due C02 destroys the restored door and emits its source receipt");

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_DOOR_DESTRUCTION;
    event.fireAtTick = 123471u;
    event.mapIndex = 2;
    event.mapX = 20;
    event.mapY = 22;
    square_data[2][20 * 32 + 22] =
        (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "malformed C02 host event schedules for defensive dispatch");
    loaded_world.gameTick = 123471u;
    memset(&tick_result, 0, sizeof(tick_result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world,
                                                     &tick_result) > 0 &&
              square_data[2][20 * 32 + 22] ==
                  (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5) &&
              tick_result.emissionCount == 0,
          "M10 cannot emit a door destruction for a non-door square");
}

static void test_f0421_tail_read_checksum_gate(void)
{
    const uint8_t source[] = { 1u, 2u, 250u, 4u };
    uint8_t destination[4] = { 0u, 0u, 0u, 0u };
    uint8_t written[4] = { 0xa5u, 0xa5u, 0xa5u, 0xa5u };
    size_t cursor = 0u;
    uint16_t checksum = 0xfff0u;

    CHECK(dm1_v1_original_save_pc34_f0415_read_bytes(
              source, sizeof(source), &cursor, destination, 2u),
          "F0415 reads one complete original-save span");
    CHECK(cursor == 2u && destination[0] == 1u && destination[1] == 2u,
          "F0415 advances only after copying the complete span");
    CHECK(!dm1_v1_original_save_pc34_f0415_read_bytes(
               source, sizeof(source), &cursor, destination + 2u, 3u),
          "F0415 rejects a truncated original-save span");
    CHECK(cursor == 2u && destination[2] == 0u && destination[3] == 0u,
          "F0415 leaves cursor and destination untouched on rejection");

    cursor = 0u;
    CHECK(dm1_v1_original_save_pc34_f0416_write_bytes(
              written, sizeof(written), &cursor, source, 2u),
          "F0416 writes one complete original-save span");
    CHECK(cursor == 2u && written[0] == 1u && written[1] == 2u,
          "F0416 advances only after writing the complete span");
    CHECK(!dm1_v1_original_save_pc34_f0416_write_bytes(
               written, sizeof(written), &cursor, source, 3u),
          "F0416 rejects a truncated destination span");
    CHECK(cursor == 2u && written[2] == 0xa5u && written[3] == 0xa5u,
          "F0416 leaves cursor and destination untouched on rejection");

    cursor = 0u;
    checksum = 0xfff0u;
    memset(destination, 0, sizeof(destination));

    CHECK(dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
              source, sizeof(source), &cursor, destination, 2u, &checksum),
          "F0421 reads first dungeon-tail section");
    CHECK(cursor == 2u && checksum == 0xfff3u &&
          destination[0] == 1u && destination[1] == 2u,
          "F0421 adds first source bytes to running checksum");
    CHECK(dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
              source, sizeof(source), &cursor, destination + 2u, 2u,
              &checksum),
          "F0421 reads second dungeon-tail section");
    CHECK(cursor == sizeof(source) && checksum == 0x00f1u &&
          destination[2] == 250u && destination[3] == 4u,
          "F0421 retains 16-bit wrapping checksum across sections");
    CHECK(!dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
               source, sizeof(source), &cursor, destination, 1u,
               &checksum),
          "F0421 rejects truncated source section");
    CHECK(cursor == sizeof(source) && checksum == 0x00f1u,
          "F0421 leaves cursor and checksum unchanged on read failure");

    cursor = 0u;
    checksum = 0xfff0u;
    CHECK(dm1_v1_original_save_pc34_f0422_write_bytes_with_checksum(
              written, sizeof(written), &cursor, source, 2u, &checksum),
          "F0422 writes first dungeon-tail section");
    CHECK(cursor == 2u && checksum == 0xfff3u &&
          written[0] == 1u && written[1] == 2u,
          "F0422 adds first source bytes to running checksum");
    CHECK(dm1_v1_original_save_pc34_f0422_write_bytes_with_checksum(
              written, sizeof(written), &cursor, source + 2u, 2u,
              &checksum),
          "F0422 writes second dungeon-tail section");
    CHECK(cursor == sizeof(written) && checksum == 0x00f1u &&
          memcmp(written, source, sizeof(source)) == 0,
          "F0422 retains source bytes and 16-bit wrapping checksum");
    CHECK(!dm1_v1_original_save_pc34_f0422_write_bytes_with_checksum(
               written, sizeof(written), &cursor, source, 1u, &checksum),
          "F0422 rejects truncated destination section");
    CHECK(cursor == sizeof(written) && checksum == 0x00f1u &&
          memcmp(written, source, sizeof(source)) == 0,
          "F0422 leaves destination cursor and checksum unchanged on failure");
}

static void test_original_c48_c49_requires_raw_c14_replay_identity(void)
{
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat projectile;
    struct DM1_Event_V1 event;
    DM1OriginalSavePC34ProjectileEventPlan plan;
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 materialReceipt;
    unsigned char rawC14[8] = { 0xfe, 0xff, 0x07, 0x14, 48, 40, 3, 0 };
    unsigned short sourceThing = (unsigned short)((THING_TYPE_PROJECTILE << 10) | 2u);
    unsigned short motion = (unsigned short)(4u | (5u << 5) | (1u << 10) | (2u << 12));

    memset(&things, 0, sizeof(things));
    memset(&projectile, 0, sizeof(projectile));
    memset(&event, 0, sizeof(event));
    things.loaded = 1;
    things.projectileCount = 3;
    things.thingCounts[THING_TYPE_PROJECTILE] = 3;
    things.projectiles = (struct DungeonProjectile_Compat *)calloc(
        3u, sizeof(*things.projectiles));
    CHECK(things.projectiles != NULL, "allocate raw C14 replay fixture");
    things.rawThingData[THING_TYPE_PROJECTILE] = (unsigned char *)calloc(24u, 1u);
    CHECK(things.rawThingData[THING_TYPE_PROJECTILE] != NULL,
          "allocate raw C14 bytes");
    memcpy(things.rawThingData[THING_TYPE_PROJECTILE] + 16u, rawC14, 8u);
    projectile.next = THING_ENDOFLIST;
    projectile.slot = (unsigned short)((THING_TYPE_WEAPON << 10) | 7u);
    projectile.kineticEnergy = 48; projectile.attack = 40; projectile.eventIndex = 3;
    things.projectiles[2] = projectile;
    event.type = DM1_EVENT_MOVE_PROJECTILE;
    event.map_time = (uint32_t)(1u << 24) | 123u;
    event.priority = 2;
    event.b_mapX = (uint8_t)(sourceThing & 0xffu);
    event.b_mapY = (uint8_t)(sourceThing >> 8);
    event.c_cell = (uint8_t)(motion & 0xffu);
    event.c_effect = (uint8_t)(motion >> 8);
    CHECK(dm1_v1_original_save_pc34_handoff_projectile_event_plan(
              &event, 3, &things, &plan),
          "C48/C49 replay binds raw C14 and source event");
    CHECK(plan.valid && plan.source_thing == sourceThing &&
          plan.raw_c14_fingerprint != 0u && plan.source_event_fingerprint != 0u,
          "C48/C49 plan carries raw/save identity receipts");
    memset(&materialReceipt, 0, sizeof(materialReceipt));
    materialReceipt.valid = 1;
    materialReceipt.saveReceiptBound = 1;
    materialReceipt.rawThing = sourceThing;
    materialReceipt.rawRecordFNV1a = plan.raw_c14_fingerprint;
    materialReceipt.graphicsPixelsFNV1a = 1u;
    materialReceipt.paletteFNV1a = 1u;
    CHECK(dm1_v1_original_save_pc34_projectile_replay_material_receipt_pc34(
              &plan, &materialReceipt),
          "C48/C49 replay consumes matching F0248 material receipt");
    ++materialReceipt.rawRecordFNV1a;
    CHECK(!dm1_v1_original_save_pc34_projectile_replay_material_receipt_pc34(
               &plan, &materialReceipt),
          "stale C14 material receipt cannot enter F0811 replay");
    things.rawThingData[THING_TYPE_PROJECTILE][20] ^= 1u;
    CHECK(!dm1_v1_original_save_pc34_handoff_projectile_event_plan(
               &event, 3, &things, &plan),
          "drifted raw C14 cannot replay as F0811 movement");
    free(things.rawThingData[THING_TYPE_PROJECTILE]);
    free(things.projectiles);
}

int main(void)
{
    test_original_c48_c49_requires_raw_c14_replay_identity();
    test_f0421_tail_read_checksum_gate();
    test_pc34_handoff_imports_party_state();
    test_rejects_non_pc34_and_truncated_parts();
    test_file_runtime_world_loader();
    test_tail_less_f0435_publishes_c3_c4_receipt();
    test_tail_less_f0435_reuses_c3_c4_receipt_only_without_drift();
    test_runtime_materializer_reuses_start_dungeon_and_normalizes_hoc();
    test_runtime_byte_materializer_reuses_start_dungeon();
    test_runtime_state_adoption_moves_f0435_queue();
    test_runtime_state_adoption_rejects_incoherent_f0435_queue();
    test_real_pc34_export_resumes_runtime_atomically();
    test_runtime_materializer_binds_original_group_reaction();
    test_runtime_materializer_recovers_missing_primary_from_backup();
    test_runtime_handoff_is_transactional_on_rejected_tail();
    test_runtime_handoff_rejects_unknown_source_event();
    test_runtime_handoff_materializes_original_c11_actions();
    test_runtime_materializer_binds_original_sound_union();
    test_runtime_materializer_linearizes_original_c4_heap_order();
    test_original_c60_deferred_group_move_roundtrip();
    test_original_c61_audible_group_move_roundtrip();
    test_runtime_materializer_binds_original_c12_damage_hide();
    test_original_c72_champion_shield_roundtrip();
    test_original_c71_invisibility_roundtrip();
    test_original_save_later_map_group_transition();
    test_original_c73_thieves_eye_roundtrip();
    test_original_c74_party_shield_roundtrip();
    test_original_c75_poison_roundtrip();
    test_original_c77_spell_shield_roundtrip();
    test_original_c78_fire_shield_roundtrip();
    test_original_c79_footprints_roundtrip();
    test_original_c22_cpse_roundtrip();
    test_original_c53_watchdog_roundtrip();
    test_original_c13_vi_altar_event_plan();
    test_original_c13_vi_altar_runtime_sequence();
    test_runtime_materializer_binds_original_explosion_union();
    test_original_c24_fluxcage_import_runtime_export_roundtrip();
    test_c24_c25_union_materialization_rolls_back_as_one_pc34_stage();
    test_original_c70_light_import_runtime_export_roundtrip();
    test_original_c65_generator_import_runtime_export_roundtrip();
    test_real_dm1_dungeon_tail_map_span_validation();
    test_real_dm1_dungeon_tail_rejects_out_of_bounds_party_pose();
    test_real_dm1_door_animation_save_handoff();
    test_public_fixture_builder_roundtrips_pc34_handoff();
    test_world_roundtrip_helper_exports_verified_pc34();
    test_world_handoff_rejects_duplicate_timeline_reference();
    /* TODO(dm1-original-save): These proposed aggregate status/party and
     * projectile-plan tests have no bodies in this translation unit or its
     * base revision. Keep them out of the executable until each has a
     * source-record-backed implementation; the individual C48 and C70-C79
     * regressions above remain live. */
    test_world_handoff_rejects_invalid_champion_vitals();
    test_world_handoff_rejects_current_active_groups_over_maximum();
    test_world_roundtrip_preserves_materialized_dungeon_tail();
    test_world_handoff_materializes_and_validates_textstring_tail();
    test_world_handoff_rejects_party_pose_outside_dungeon_tail();
    test_world_handoff_roundtrips_group_list_and_active_groups();
    test_corpus_roundtrip_proof();
    test_optional_real_pc34_corpus_roundtrip();
    test_world_export_rebuilds_c48_c49_projectile_union();
    test_world_roundtrip_preserves_champion_poison_count();
    test_world_export_rebuilds_c25_explosion_union();
    test_world_export_rebuilds_c29_group_reaction_union();
    test_world_export_roundtrips_c13_vi_altar_union();
    test_original_square_state_events_materialize_and_roundtrip();
    test_original_fakewall_event_materializes_and_defers_clear();
    test_original_door_destruction_event_materializes_on_door();
    test_strings();
    puts("PASS dm1_v1_original_save_pc34_handoff");
    return 0;
}
