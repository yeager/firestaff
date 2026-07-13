#include "dm1_v1_original_save_pc34_handoff.h"

#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

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
                         DM1_EVENT_ENABLE_CHAMPION_ACTION, 2, 0, 0, 0, 0);
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

static uint16_t byte_sum16(const unsigned char *bytes, size_t count)
{
    uint16_t sum = 0u;
    size_t i;

    for (i = 0u; i < count; ++i) {
        sum = (uint16_t)(sum + bytes[i]);
    }
    return sum;
}

static size_t original_pc34_tail_offset(const unsigned char *bytes, size_t size)
{
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int part;

    if (!bytes || size < cursor) return 0u;
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;
        if (cursor + 2u > size) return 0u;
        part_size = rd16le(bytes + cursor);
        cursor += 2u;
        if (cursor + part_size > size) return 0u;
        cursor += part_size;
    }
    if (cursor + SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT >= size) {
        return 0u;
    }
    return cursor + SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT;
}

static int export_local_dm1_dungeon_save(unsigned char *out,
                                         size_t capacity,
                                         int *out_written)
{
    const char *root = getenv("FIRESTAFF_DM1_DATA");
    char path[1024];
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct GameWorld_Compat world;
    int rc;

    if (!root || root[0] == '\0') {
        const char *home = getenv("HOME");
        if (!home || home[0] == '\0') return 0;
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/DUNGEON.DAT", home);
    } else {
        snprintf(path, sizeof(path), "%s/DUNGEON.DAT", root);
    }
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(&world, 0, sizeof(world));
    if (!F0500_DUNGEON_LoadDatHeader_Compat(path, &dungeon) ||
        !F0502_DUNGEON_LoadTileData_Compat(path, &dungeon) ||
        !F0504_DUNGEON_LoadThingData_Compat(path, &dungeon, &things)) {
        F0504_DUNGEON_FreeThingData_Compat(&things);
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        return 0;
    }
    world.dungeon = &dungeon;
    world.things = &things;
    world.party.championCount = 1;
    world.party.mapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.gameTick = 1u;
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x44554e31u, out, (int)capacity, out_written);
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
    return rc == SAVEGAME_PC34_OK;
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
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[4];
    struct DM1_EventQueue_V1 event_queue;
    DM1OriginalSavePC34HandoffReport report;
    int i;
    int found_enable_action = 0;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "file loader fixture build succeeds");
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "fixture file write succeeds");

    memset(&world, 0, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    groups[1].creatureType = 15;
    groups[2].creatureType = 16;
    things.groups = groups;
    things.groupCount = 4;
    world.things = &things;

    rc = dm1_v1_original_save_pc34_handoff_load_world_from_file(
        path, &world, &event_queue, &report);
    remove(path);

    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "file world loader succeeds");
    CHECK(world.gameTick == 123456u,
          "world game tick imported from original GameTime");
    CHECK(world.timeline.nowTick == 123456u,
          "world M10 timeline tick follows original GameTime");
    for (i = 0; i < world.timeline.count; ++i) {
        const struct TimelineEvent_Compat *event = &world.timeline.events[i];
        if (event->kind == TIMELINE_EVENT_ENABLE_CHAMPION_ACTION) {
            CHECK(event->aux0 == DM1_EVENT_ENABLE_CHAMPION_ACTION,
                  "C11 source type survives runtime materialization");
            CHECK(event->aux4 == 2 && event->cell == 0,
                  "C11 priority and zero SlotOrdinal survive runtime materialization");
            found_enable_action = 1;
        }
    }
    CHECK(found_enable_action,
          "C11 enable-action event materializes into the live timeline");
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
    CHECK(world.creatureAI[0].creatureType == 15,
          "world active group resolved through things");
    CHECK(world.creatureAI[1].creatureType == 16,
          "world second active group resolved through things");
    CHECK(report.active_group_runtime_imported_count == 2,
          "file loader report active group import count");
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

static void test_runtime_materializer_reuses_start_dungeon_and_normalizes_hoc(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat start_dungeon;
    struct DungeonThings_Compat start_things;
    struct DungeonGroup_Compat groups[4];
    DM1OriginalSavePC34HoCResumeState hoc;
    int rc;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "runtime materializer fixture build succeeds");
    make_temp_save_path(path, sizeof(path));
    remove(path);
    CHECK(write_fixture_file(path, bytes, written),
          "runtime materializer fixture write succeeds");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&start_dungeon, 0, sizeof(start_dungeon));
    memset(&start_things, 0, sizeof(start_things));
    memset(groups, 0, sizeof(groups));
    groups[1].creatureType = 15;
    groups[2].creatureType = 16;
    start_things.groups = groups;
    start_things.groupCount = 4;
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

    hoc.candidate_party_index = 0;
    hoc.inventory_panel_active = 1;
    dm1_v1_original_save_pc34_handoff_normalize_hoc_resume_state(
        &start_world, &hoc);
    CHECK(hoc.candidate_panel_active == 0,
          "stale HoC candidate cannot reopen over an older party slot");
    CHECK(hoc.candidate_mirror_ordinal == -1 && hoc.candidate_party_index == -1,
          "stale HoC candidate clears both source indices");
    CHECK(hoc.inventory_panel_active == 0,
          "stale HoC candidate closes dependent inventory render panel");

    hoc.candidate_mirror_ordinal = 7;
    hoc.candidate_party_index = CHAMPION_MAX_PARTY;
    hoc.candidate_panel_active = 1;
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
    struct DungeonThings_Compat things;
    unsigned short first_things[1];
    struct DungeonGroup_Compat groups[1];
    DM1OriginalSavePC34HandoffReport report;

    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written,
                                     2, 3, 9, 10, 2, 1,
                                     ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK,
          "group reaction fixture build succeeds");
    CHECK(rewrite_fixture_event_type(
              bytes, (size_t)written, 0,
              DM1_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE),
          "fixture rewrites an authenticated source C29 event");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
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
    /* The fixture's event 0 is B.Location=(11,12) on map 2.  This is
     * the exact original SFT lookup used by TIMELINE.C -> GROUP.C F0209. */
    square_data[2][11 * 32 + 12] |= DUNGEON_SQUARE_MASK_THING_LIST;
    first_things[0] = (unsigned short)(THING_TYPE_GROUP << 10);
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 15;
    things.squareFirstThings = first_things;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
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

static void test_runtime_handoff_rejects_unmaterialized_source_event(void)
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
          "unmaterialized-event fixture build succeeds");
    CHECK(rewrite_fixture_event_type(bytes, (size_t)written, 0,
                                     DM1_EVENT_ENABLE_CHAMPION_ACTION),
          "C11 event rewrite preserves the PC34 envelope");

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
          "active C11 without a materializer rejects runtime handoff");
    CHECK(world.gameTick == 777u && world.party.championCount == 1 &&
          world.party.mapIndex == 6,
          "unmaterialized source event leaves live world untouched");
    CHECK(event_queue.gameTick == 888u && event_queue.eventCount == 1,
          "unmaterialized source event leaves live queue untouched");
    CHECK(report.original_game_time == 999u,
          "unmaterialized source event leaves receipt untouched");
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
              rewrite_fixture_event_c_union(bytes, (size_t)written, 0, 23u),
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
              rd16le(&report.events[c20_index].c_cell) == 23u,
          "C20 native roundtrip preserves Priority Location SoundIndex");
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
    CHECK(rc != SAVEGAME_PC34_OK,
          "C20 export rejects a host sound event without the receipt");
}

static void test_original_c60_deferred_group_move_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char squares[3][32 * 32]; unsigned short first_things[1]; char path[512];
    int written = 0, exported_size = 0, rc, i, index = -1;
    struct GameWorld_Compat start_world, loaded_world; struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3]; struct DungeonMapTiles_Compat tiles[3];
    struct DungeonThings_Compat things; struct DungeonGroup_Compat groups[2];
    struct SaveGame_Compat imported; struct PartyState_Compat party;
    struct TimelineEvent_Compat event; DM1OriginalSavePC34HandoffReport report;
    uint16_t group_thing = (uint16_t)((THING_TYPE_GROUP << 10) | 1);
    rc = build_original_pc34_fixture(bytes, (int)sizeof(bytes), &written, 2, 3, 9, 10, 2, 1, ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc == SAVEGAME_PC34_OK && rewrite_fixture_event_type(bytes, (size_t)written, 0, DM1_EVENT_MOVE_GROUP_SILENT) && rewrite_fixture_event_priority(bytes, (size_t)written, 0, 0) && rewrite_fixture_event_c_union(bytes, (size_t)written, 0, group_thing), "C60 fixture writes B.Location and C.Slot");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world)); memset(&dungeon, 0, sizeof(dungeon)); memset(maps, 0, sizeof(maps)); memset(tiles, 0, sizeof(tiles)); memset(squares, 0, sizeof(squares)); memset(&things, 0, sizeof(things)); memset(groups, 0, sizeof(groups)); memset(&report, 0, sizeof(report));
    for (i = 0; i < 3; ++i) { maps[i].width = 32; maps[i].height = 32; tiles[i].squareData = squares[i]; tiles[i].squareCount = 32 * 32; }
    squares[2][11 * 32 + 12] = DUNGEON_SQUARE_MASK_THING_LIST; first_things[0] = THING_ENDOFLIST;
    dungeon.header.mapCount = 3; dungeon.maps = maps; dungeon.tiles = tiles; dungeon.tilesLoaded = 1;
    things.groups = groups; things.groupCount = 2; things.squareFirstThings = first_things; things.squareFirstThingCount = 1; groups[1].creatureType = 1; groups[1].next = THING_ENDOFLIST;
    start_world.dungeon = &dungeon; start_world.things = &things; make_temp_save_path(path, sizeof(path)); remove(path); CHECK(write_fixture_file(path, bytes, written), "C60 fixture writes");
    rc = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path, &start_world, &loaded_world, NULL, &report); remove(path);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C60 materializes typed group-slot receipt");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_MOVE_GROUP_SILENT) { index = i; break; }
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux0 == 1 && loaded_world.timeline.events[index].aux1 == group_thing && loaded_world.timeline.events[index].mapX == 11 && loaded_world.timeline.events[index].mapY == 12, "C60 retains Location and C04 Slot");
    event = loaded_world.timeline.events[index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event), "C60 schedules for its existing F0252 runtime owner");
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event); rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size); CHECK(rc == SAVEGAME_PC34_OK, "C60 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party; rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report); index = -1; for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_MOVE_GROUP_SILENT) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 && report.events[index].priority == 0 && report.events[index].b_mapX == 11 && report.events[index].b_mapY == 12 && rd16le(&report.events[index].c_cell) == group_thing, "C60 native roundtrip preserves Location and Slot");
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
    struct DungeonGroup_Compat groups[2];
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
                  bytes, (size_t)written, 0, group_thing),
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
    things.groupCount = 2;
    groups[1].creatureType = 1;
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
          "C61 native roundtrip preserves Location and Slot");
}

static void test_runtime_materializer_binds_original_c12_damage_hide(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported[SAVEGAME_PC34_MAX_FILE_SIZE];
    char path[512];
    int written = 0;
    int exported_size = 0;
    int rc;
    int i;
    int found_hide = 0;
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
                                     DM1_EVENT_HIDE_DAMAGE_RECEIVED),
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
              loaded_world.timeline.events[1].aux4 == 2,
          "C12 materialization preserves only Map_Time and Priority");

    loaded_world.gameTick = 123490u;
    memset(&tick_result, 0, sizeof(tick_result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world,
                                                    &tick_result);
    for (i = 0; i < tick_result.emissionCount; ++i) {
        if (tick_result.emissions[i].kind == EMIT_CHAMPION_DAMAGE_HIDDEN &&
            tick_result.emissions[i].payload[0] == 2) {
            found_hide = 1;
        }
    }
    CHECK(found_hide,
          "C12 dispatch reaches the source champion-panel hide receipt");

    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    loaded_world.timeline.count = 1;
    memset(&loaded_world.timeline.events[0], 0,
           sizeof(loaded_world.timeline.events[0]));
    loaded_world.timeline.events[0].kind = TIMELINE_EVENT_STATUS_TIMEOUT;
    loaded_world.timeline.events[0].fireAtTick = 123495u;
    loaded_world.timeline.events[0].mapIndex = 1;
    loaded_world.timeline.events[0].aux0 = DM1_EVENT_HIDE_DAMAGE_RECEIVED;
    loaded_world.timeline.events[0].aux4 = 1;
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
          "C12 export retains priority and zeroes unowned union bytes");
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
    CHECK(loaded_world.lifecycle.champions[2].shieldDefense == 8, "C72 runtime subtracts B.Defense from selected champion");
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
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK, "C71 materializes typed invisibility expiry");
    for (i = 0; i < loaded_world.timeline.count; ++i) if (loaded_world.timeline.events[i].aux2 == DM1_EVENT_INVISIBILITY) { index = i; break; }
    c71_index = index;
    CHECK(index >= 0 && loaded_world.timeline.events[index].aux0 == DM1_EVENT_INVISIBILITY &&
              loaded_world.timeline.events[index].aux1 == 0 && loaded_world.timeline.events[index].aux4 == 0,
          "C71 retains only its typed no-union receipt");
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
    CHECK(rc != SAVEGAME_PC34_OK, "C71 export rejects an unproven host timeout");
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
    CHECK(rc != SAVEGAME_PC34_OK, "C73 export rejects an unproven host timeout");
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
    CHECK(rc != SAVEGAME_PC34_OK, "C74 export rejects an unproven host timeout");
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
              loaded_world.timeline.events[0].aux2 == DM1_EVENT_POISON_CHAMPION &&
              loaded_world.timeline.events[0].aux1 == 127 && loaded_world.timeline.events[0].aux4 == 2,
          "C75 runtime damages and retains native requeue receipt");
    event.aux2 = 0;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event);
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size);
    CHECK(rc != SAVEGAME_PC34_OK, "C75 export rejects an unproven host timeout");
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
    CHECK(rc != SAVEGAME_PC34_OK, "C77 export rejects an unproven host timeout");
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
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size); CHECK(rc == SAVEGAME_PC34_OK, "C78 exports natively");
    memset(&imported, 0, sizeof(imported)); memset(&party, 0, sizeof(party)); imported.party = &party; rc = dm1_v1_original_save_pc34_handoff_bytes(exported, (size_t)exported_size, &imported, &report);
    for (i = 0; i < report.original_event_count; ++i) if (report.events[i].type == DM1_EVENT_FIRESHIELD) { index = i; break; }
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index >= 0 && report.events[index].priority == 0 && (int16_t)rd16le(&report.events[index].b_mapX) == 12 && report.events[index].c_cell == 0 && report.events[index].c_effect == 0, "C78 native roundtrip preserves B.Defense and no C union arm");
    loaded_world.magic.fireShieldDefense = 20; loaded_world.lifecycle.status.partyFireShieldDefense = 20; event = loaded_world.timeline.events[c78_index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event); loaded_world.gameTick = event.fireAtTick; memset(&result, 0, sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.magic.fireShieldDefense == 8 && loaded_world.lifecycle.status.partyFireShieldDefense == 8, "C78 runtime subtracts B.Defense from fire shield mirrors");
    event.aux2 = 0; F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event); rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world, 0x43313445u, exported, (int)sizeof(exported), &exported_size); CHECK(rc != SAVEGAME_PC34_OK, "C78 export rejects an unproven host timeout");
}

static void test_original_c79_footprints_roundtrip(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE], exported[SAVEGAME_PC34_MAX_FILE_SIZE]; char path[512]; int written=0, exported_size=0, rc, i, index=-1, c79_index=-1;
    struct GameWorld_Compat start_world, loaded_world; struct DungeonDatState_Compat dungeon; struct DungeonThings_Compat things; struct SaveGame_Compat imported; struct PartyState_Compat party; struct TickResult_Compat result; struct TimelineEvent_Compat event; DM1OriginalSavePC34HandoffReport report;
    rc=build_original_pc34_fixture(bytes,(int)sizeof(bytes),&written,3,3,9,10,2,1,ORIGINAL_PC34_ACTIVE_GROUP_COUNT);
    CHECK(rc==SAVEGAME_PC34_OK && rewrite_fixture_event_type(bytes,(size_t)written,2,DM1_EVENT_FOOTPRINTS) && rewrite_fixture_event_byte(bytes,(size_t)written,2,5,0) && rewrite_fixture_event_byte(bytes,(size_t)written,2,6,0xa5) && rewrite_fixture_event_byte(bytes,(size_t)written,2,7,0x5a) && rewrite_fixture_event_byte(bytes,(size_t)written,2,8,0x3c) && rewrite_fixture_event_byte(bytes,(size_t)written,2,9,0xc3),"C79 fixture no B/C ownership");
    memset(&start_world,0,sizeof(start_world)); memset(&loaded_world,0,sizeof(loaded_world)); memset(&dungeon,0,sizeof(dungeon)); memset(&things,0,sizeof(things)); memset(&report,0,sizeof(report)); start_world.dungeon=&dungeon; start_world.things=&things; make_temp_save_path(path,sizeof(path)); remove(path); CHECK(write_fixture_file(path,bytes,written),"C79 fixture writes"); rc=dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(path,&start_world,&loaded_world,NULL,&report); remove(path); CHECK(rc==DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,"C79 materializes");
    for(i=0;i<loaded_world.timeline.count;++i) if(loaded_world.timeline.events[i].aux2==DM1_EVENT_FOOTPRINTS){index=i;break;} c79_index=index; CHECK(index>=0 && loaded_world.timeline.events[index].aux1==0 && loaded_world.timeline.events[index].aux4==0,"C79 typed receipt"); rc=F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world,0x43313445u,exported,(int)sizeof(exported),&exported_size); CHECK(rc==SAVEGAME_PC34_OK,"C79 exports"); memset(&imported,0,sizeof(imported)); memset(&party,0,sizeof(party)); imported.party=&party; rc=dm1_v1_original_save_pc34_handoff_bytes(exported,(size_t)exported_size,&imported,&report); for(i=0;i<report.original_event_count;++i) if(report.events[i].type==DM1_EVENT_FOOTPRINTS){index=i;break;} CHECK(rc==DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK && index>=0 && report.events[index].priority==0 && report.events[index].b_mapX==0 && report.events[index].b_mapY==0 && report.events[index].c_cell==0 && report.events[index].c_effect==0,"C79 roundtrip has no union arm");
    loaded_world.magic.event79CountFootprints=1; loaded_world.magic.magicFootprintsActive=1; loaded_world.lifecycle.status.footprintsCount=1; event=loaded_world.timeline.events[c79_index]; F0720_TIMELINE_Init_Compat(&loaded_world.timeline,event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline,&event); loaded_world.gameTick=event.fireAtTick; memset(&result,0,sizeof(result)); F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world,&result); CHECK(loaded_world.magic.event79CountFootprints==0 && !loaded_world.magic.magicFootprintsActive && loaded_world.lifecycle.status.footprintsCount==0,"C79 runtime mirrors"); event.aux2=0; F0720_TIMELINE_Init_Compat(&loaded_world.timeline,event.fireAtTick); F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline,&event); rc=F0802_SAVEGAME_ExportPC34FromWorld_Compat(&loaded_world,0x43313445u,exported,(int)sizeof(exported),&exported_size); CHECK(rc!=SAVEGAME_PC34_OK,"C79 rejects host timeout");
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
              rewrite_fixture_event_byte(bytes, (size_t)written, 2, 9, 0x96),
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
              report.events[index].priority == 0 &&
              report.events[index].b_mapX == 0 &&
              report.events[index].b_mapY == 0 &&
              report.events[index].c_cell == 0 &&
              report.events[index].c_effect == 0,
          "C53 native roundtrip canonicalizes unowned union bytes");
    event = loaded_world.timeline.events[c53_index];
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "C53 receipt schedules for runtime expiry");
    loaded_world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result));
    F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result);
    CHECK(loaded_world.timeline.count == 1 &&
              loaded_world.timeline.events[0].kind == TIMELINE_EVENT_WATCHDOG &&
              loaded_world.timeline.events[0].fireAtTick ==
                  ((event.fireAtTick + 300u) & 0x00ffffffu) &&
              loaded_world.timeline.events[0].aux2 == DM1_EVENT_WATCHDOG,
          "C53 runtime re-arms the source watchdog interval");
    event.aux2 = 0;
    F0720_TIMELINE_Init_Compat(&loaded_world.timeline, event.fireAtTick);
    CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &event),
          "host watchdog schedules for export rejection");
    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
        &exported_size);
    CHECK(rc != SAVEGAME_PC34_OK,
          "C53 export rejects a watchdog without the source receipt");
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
              ((report.events[index].map_time >> 24) & 0xffu) == 2u &&
              report.events[index].priority == 0 &&
              report.events[index].b_mapX == 0 && report.events[index].b_mapY == 0 &&
              report.events[index].c_cell == 0 && report.events[index].c_effect == 0,
          "C22 native roundtrip preserves Map_Time and canonicalizes unowned bytes");
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
    CHECK(rc != SAVEGAME_PC34_OK,
          "C22 export rejects an unproven host CPSE event");
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
    struct DungeonThings_Compat things;
    struct TimelineEvent_Compat event;
    struct TickResult_Compat result;
    int i;
    int found_step1 = 0;

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(&things, 0, sizeof(things));
    maps[1].width = 32;
    maps[1].height = 32;
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
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
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) > 0 &&
              world.explosions.count == 1,
          "C13 step 2 materializes the source rebirth explosion");
    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_VI_ALTAR_REBIRTH &&
            world.timeline.events[i].aux1 == 1 &&
            world.timeline.events[i].fireAtTick == 55u) {
            found_step1 = 1;
        }
    }
    CHECK(found_step1, "C13 step 2 stages its five-tick bones transition");

    memset(&world.timeline, 0, sizeof(world.timeline));
    CHECK(F0720_TIMELINE_Init_Compat(&world.timeline, 56u),
          "C13 rebirth terminal timeline initializes");
    event.fireAtTick = 56u;
    event.aux1 = 0;
    CHECK(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event),
          "C13 terminal event schedules");
    world.gameTick = 56u;
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    CHECK(world.party.champions[2].hp.maximum == 98 &&
              world.party.champions[2].hp.current == 49 &&
              world.party.champions[2].direction == 3 &&
              world.party.champions[2].inventory[0] == THING_NONE,
          "C13 step 0 applies the source F0283 rebirth state");
}

static void test_runtime_materializer_binds_original_explosion_union(void)
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
    struct DungeonThings_Compat things;
    unsigned short first_things[1];
    unsigned char raw_explosion[4];
    struct DungeonExplosion_Compat explosions[1];
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
                                            source_thing),
          "C25 fixture preserves authenticated Slot union bytes");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
    memset(&things, 0, sizeof(things));
    memset(raw_explosion, 0, sizeof(raw_explosion));
    memset(explosions, 0, sizeof(explosions));
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
    raw_explosion[2] = 2u; /* lightning-bolt explosion type */
    raw_explosion[3] = 77u;
    explosions[0].next = THING_ENDOFLIST;
    explosions[0].type = 2u;
    explosions[0].attack = 77u;
    things.squareFirstThings = first_things;
    things.squareFirstThingCount = 1;
    things.explosions = explosions;
    things.explosionCount = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = raw_explosion;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.loaded = 1;
    start_world.dungeon = &dungeon;
    start_world.things = &things;
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
    struct GameWorld_Compat start_world;
    struct GameWorld_Compat loaded_world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    unsigned char square_data[3][32 * 32];
    struct DungeonThings_Compat things;
    unsigned short first_things[1];
    unsigned char raw_explosion[4];
    struct DungeonExplosion_Compat source_explosions[1];
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    struct TickResult_Compat result;
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
                                            source_thing),
          "C24 fixture preserves authenticated zero-priority Slot union bytes");

    memset(&start_world, 0, sizeof(start_world));
    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
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
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
              report.events[2].type == DM1_EVENT_REMOVE_FLUXCAGE &&
              report.events[2].priority == 0 &&
              report.events[2].b_mapX == 11 && report.events[2].b_mapY == 12 &&
              rd16le(&report.events[2].c_cell) == source_thing,
          "C24 roundtrip preserves Priority Location and exact C.Slot union");

    loaded_world.timeline.events[c24_index].aux2 = THING_NONE;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_written) == SAVEGAME_PC34_ERROR_INTERNAL,
          "C24 export rejects a missing original C.Slot receipt");
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
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result) > 0 &&
              first_things[0] == THING_ENDOFLIST &&
              source_explosions[0].next == THING_NONE &&
              loaded_world.explosions.entries[0].reserved0 == 0,
          "C24 expiry removes the exact C15 fluxcage and its live counterpart");
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
              rewrite_fixture_event_c_union(bytes, (size_t)written, 0, 0x7f3du),
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
        if (loaded_world.timeline.events[i].kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY) {
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
        if (report.events[i].type == DM1_EVENT_LIGHT) {
            exported_c70 = i;
            break;
        }
    }
    CHECK(exported_c70 >= 0 && report.events[exported_c70].priority == 0 &&
              (int16_t)rd16le(&report.events[exported_c70].b_mapX) == 3 &&
              rd16le(&report.events[exported_c70].c_cell) == 0,
          "C70 roundtrip preserves B.LightPower and invents no C union arm");

    {
        struct TimelineEvent_Compat c70 = loaded_world.timeline.events[c70_index];
        CHECK(F0720_TIMELINE_Init_Compat(&loaded_world.timeline, c70.fireAtTick),
              "C70 runtime timeline initializes");
        CHECK(F0721_TIMELINE_Schedule_Compat(&loaded_world.timeline, &c70),
              "C70 runtime event schedules");
        loaded_world.gameTick = c70.fireAtTick;
    }
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&loaded_world, &result) > 0 &&
              loaded_world.magic.magicalLightAmount == 12 &&
              loaded_world.timeline.count == 1 &&
              loaded_world.timeline.events[0].aux0 == 2 &&
              loaded_world.timeline.events[0].aux1 == DM1_EVENT_LIGHT &&
              loaded_world.timeline.events[0].aux4 == 0,
          "C70 runtime follows F0257 and retains native export provenance");

    loaded_world.timeline.events[0].aux1 = 0;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &loaded_world, 0x43313445u, exported, (int)sizeof(exported),
              &exported_written) == SAVEGAME_PC34_ERROR_INTERNAL,
          "C70 export rejects an unproven host light event");
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
              rewrite_fixture_event_c_union(bytes, (size_t)written, 0, 0x7f3du),
          "C65 fixture preserves Priority-0 Location and irrelevant C bytes");
    memset(&start_world, 0, sizeof(start_world)); memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&dungeon, 0, sizeof(dungeon)); memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles)); memset(square_data, 0, sizeof(square_data));
    memset(&things, 0, sizeof(things)); memset(raw_sensor, 0, sizeof(raw_sensor));
    memset(sensors, 0, sizeof(sensors)); memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party)); memset(&report, 0, sizeof(report));
    dungeon.header.mapCount = 3; dungeon.maps = maps; dungeon.tiles = tiles; dungeon.tilesLoaded = 1;
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
              rd16le(&report.events[exported_c65].c_cell) == 0,
          "C65 roundtrip preserves Location and invents no C union arm");
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
    CHECK(report.roundtrip_succeeded_count == 2,
          "corpus roundtrips both PC34 files");
    CHECK(report.core_state_match_count == 2,
          "corpus preserves core state for every PC34 file");
    CHECK(report.roundtrip_failed_count == 0,
          "corpus has no failed PC34 roundtrip");
    CHECK(strstr(report.first_pc34_path, "first-original.bin") != NULL,
          "corpus records first eligible path");
    CHECK(strstr(report.first_roundtrip_path, "first-original.bin") != NULL,
          "corpus records first verified path");
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
    if (!root || !root[0]) {
        puts("SKIP real PC34 corpus: FIRESTAFF_DM1_PC34_SAVE_CORPUS unset");
        return;
    }
    memset(&report, 0, sizeof(report));
    rc = dm1_v1_original_save_pc34_roundtrip_corpus_root(root, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "real PC34 corpus scan completes");
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
}

static void test_original_projectile_event_plan_preserves_c48_bits(void)
{
    struct DungeonProjectile_Compat projectiles[6];
    struct DungeonThings_Compat things;
    struct DM1_Event_V1 event;
    DM1OriginalSavePC34ProjectileEventPlan plan;
    uint16_t projectile_thing;
    uint16_t motion;

    memset(projectiles, 0, sizeof(projectiles));
    memset(&things, 0, sizeof(things));
    memset(&event, 0, sizeof(event));
    memset(&plan, 0, sizeof(plan));
    things.projectiles = projectiles;
    things.projectileCount = 6;
    projectiles[5].eventIndex = 7;
    projectiles[5].slot = 0xff80u; /* ReDMCSB fireball explosion thing */
    projectiles[5].kineticEnergy = 83;
    projectiles[5].attack = 91;
    projectile_thing = (uint16_t)((THING_TYPE_PROJECTILE << 10) | 5u |
                                  (2u << 14));
    motion = (uint16_t)(17u | (9u << 5) | (3u << 10) | (6u << 12));
    event.map_time = DM1_MAP_TIME_MAKE(2, 123456u);
    event.type = DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS;
    event.priority = 4;
    wr16le(&event.b_mapX, projectile_thing);
    wr16le(&event.c_cell, motion);

    CHECK(dm1_v1_original_save_pc34_handoff_projectile_event_plan(
              &event, 7, &things, &plan) == 1,
          "C48 projectile event binds its original projectile record");
    CHECK(plan.valid && plan.projectile_index == 5 && plan.map_index == 2,
          "C48 plan preserves event identity");
    CHECK(plan.map_x == 17 && plan.map_y == 9 && plan.cell == 2 &&
              plan.direction == 3 && plan.step_energy == 6,
          "C48 plan decodes ReDMCSB C.Projectile bitfields");
    CHECK(plan.first_move_grace == 1 &&
              plan.projectile_category == PROJECTILE_CATEGORY_MAGICAL &&
              plan.projectile_subtype == PROJECTILE_SUBTYPE_FIREBALL,
          "C48 plan preserves first-move and fireball identity");
    CHECK(plan.kinetic_energy == 83 && plan.attack == 91,
          "C48 plan binds the original projectile kinetic fields");

    event.type = DM1_EVENT_MOVE_PROJECTILE;
    CHECK(dm1_v1_original_save_pc34_handoff_projectile_event_plan(
              &event, 7, &things, &plan) == 1 &&
              plan.first_move_grace == 0,
          "C49 preserves the source immediate-impact state");

    projectiles[5].eventIndex = 8;
    CHECK(dm1_v1_original_save_pc34_handoff_projectile_event_plan(
              &event, 7, &things, &plan) == 0,
          "mismatched original EventIndex rejects projectile materialization");
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
    world.timeline.count = 4;
    world.timeline.events[0].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    world.timeline.events[0].fireAtTick = 100u;
    world.timeline.events[0].aux0 = 5;
    world.timeline.events[0].aux4 = 7;
    world.timeline.events[1].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    world.timeline.events[1].fireAtTick = 101u;
    world.timeline.events[1].aux0 = 6;
    world.timeline.events[1].aux4 = 3;
    world.timeline.events[2].kind = TIMELINE_EVENT_CREATURE_REACTION;
    world.timeline.events[2].fireAtTick = 102u;
    world.timeline.events[2].mapIndex = 2;
    world.timeline.events[2].mapX = 14;
    world.timeline.events[2].mapY = 7;
    world.timeline.events[2].aux0 = 3;
    world.timeline.events[2].aux1 = 15;
    world.timeline.events[2].aux2 = DM1_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE;
    world.timeline.events[2].aux3 = 9;
    world.timeline.events[2].aux4 = 0x104;
    world.timeline.events[3].kind = TIMELINE_EVENT_PLAY_SOUND;
    world.timeline.events[3].fireAtTick = 103u;
    world.timeline.events[3].mapIndex = 2;
    world.timeline.events[3].mapX = 19;
    world.timeline.events[3].mapY = 8;
    world.timeline.events[3].aux0 = 23;
    world.timeline.events[3].aux2 = DM1_EVENT_PLAY_SOUND;
    world.timeline.events[3].aux4 = 6;
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
    CHECK(report.original_event_count == 4 &&
              report.events[0].type == DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS &&
              report.events[1].type == DM1_EVENT_MOVE_PROJECTILE &&
              report.events[2].type == DM1_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE &&
              report.events[3].type == DM1_EVENT_PLAY_SOUND,
          "world export retains C48/C49, C29, and C20 source event ids");

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
    CHECK(report.events[2].b_mapX == 14 && report.events[2].b_mapY == 7 &&
              rd16le(&report.events[2].c_cell) == 9u &&
              report.events[2].priority == 4,
          "C29 export restores B.Location, C.Ticks, and source priority");
    CHECK(report.events[3].b_mapX == 19 && report.events[3].b_mapY == 8 &&
              (int16_t)rd16le(&report.events[3].c_cell) == 23 &&
              report.events[3].priority == 6,
          "C20 export restores B.Location, C.SoundIndex, and source priority");

    world.projectiles.entries[6].reserved3 = 0;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &world, 0x43313445u, bytes, (int)sizeof(bytes), &written) ==
              SAVEGAME_PC34_ERROR_INTERNAL,
          "world export rejects an unbound projectile event instead of guessing");
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
    unsigned char raw_explosion[4];
    struct DungeonExplosion_Compat source_explosions[1];
    uint16_t source_thing = (uint16_t)((THING_TYPE_EXPLOSION << 10) |
                                       (1u << 14));
    int written = 0;
    int rc;

    memset(&world, 0, sizeof(world));
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    memset(&report, 0, sizeof(report));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, 0, sizeof(square_data));
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
    CHECK(rc == SAVEGAME_PC34_OK && written > 0,
          "world export writes a C25 event only with an original C15 source");
    imported.party = &imported_party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
              report.original_event_count == 1 &&
              report.events[0].type == DM1_EVENT_EXPLOSION &&
              report.events[0].b_mapX == 11 && report.events[0].b_mapY == 12 &&
              rd16le(&report.events[0].c_cell) == source_thing &&
              report.events[0].priority == 5,
          "C25 export restores original Location, Slot, and priority union");

    first_things[0] = THING_ENDOFLIST;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &world, 0x43313445u, bytes, (int)sizeof(bytes), &written) ==
              SAVEGAME_PC34_ERROR_INTERNAL,
          "world export rejects an unbound C25 instead of inventing Cell/Effect");
}

static void test_world_export_roundtrips_c13_vi_altar_union(void)
{
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat world;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    DM1OriginalSavePC34HandoffReport report;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    int written = 0;
    int rc;

    memset(&world, 0, sizeof(world));
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    memset(&report, 0, sizeof(report));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    maps[0].width = 32;
    maps[0].height = 32;
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    world.dungeon = &dungeon;
    world.party.championCount = 3;
    world.party.champions[2].present = 1;
    world.timeline.count = 1;
    world.timeline.events[0].kind = TIMELINE_EVENT_VI_ALTAR_REBIRTH;
    world.timeline.events[0].fireAtTick = 1234u;
    world.timeline.events[0].mapIndex = 0;
    world.timeline.events[0].mapX = 11;
    world.timeline.events[0].mapY = 12;
    world.timeline.events[0].cell = 1;
    world.timeline.events[0].aux0 = DM1_EVENT_VI_ALTAR_REBIRTH;
    world.timeline.events[0].aux1 = 2;
    world.timeline.events[0].aux4 = 2;

    rc = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x43313445u, bytes, (int)sizeof(bytes), &written);
    CHECK(rc == SAVEGAME_PC34_OK && written > 0,
          "world export writes source-proven C13 Vi Altar state");
    imported.party = &imported_party;
    rc = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)written, &imported, &report);
    CHECK(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
              report.original_event_count == 1 &&
              report.events[0].type == DM1_EVENT_VI_ALTAR_REBIRTH &&
              report.events[0].priority == 2 &&
              report.events[0].b_mapX == 11 && report.events[0].b_mapY == 12 &&
              report.events[0].c_cell == 1 && report.events[0].c_effect == 2,
          "C13 export roundtrips original Priority Location Cell Effect union");

    world.timeline.events[0].aux1 = 3;
    CHECK(F0802_SAVEGAME_ExportPC34FromWorld_Compat(
              &world, 0x43313445u, bytes, (int)sizeof(bytes), &written) ==
              SAVEGAME_PC34_ERROR_INTERNAL,
          "world export rejects an unproven C13 rebirth step");
}

int main(void)
{
    test_pc34_handoff_imports_party_state();
    test_rejects_non_pc34_and_truncated_parts();
    test_file_runtime_world_loader();
    test_runtime_materializer_reuses_start_dungeon_and_normalizes_hoc();
    test_runtime_materializer_binds_original_group_reaction();
    test_runtime_materializer_recovers_missing_primary_from_backup();
    test_runtime_handoff_is_transactional_on_rejected_tail();
    test_runtime_handoff_rejects_unmaterialized_source_event();
    test_runtime_materializer_binds_original_sound_union();
    test_original_c60_deferred_group_move_roundtrip();
    test_original_c61_audible_group_move_roundtrip();
    test_runtime_materializer_binds_original_c12_damage_hide();
    test_original_c72_champion_shield_roundtrip();
    test_original_c71_invisibility_roundtrip();
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
    test_original_c70_light_import_runtime_export_roundtrip();
    test_original_c65_generator_import_runtime_export_roundtrip();
    test_real_dm1_dungeon_tail_map_span_validation();
    test_public_fixture_builder_roundtrips_pc34_handoff();
    test_world_roundtrip_helper_exports_verified_pc34();
    test_corpus_roundtrip_proof();
    test_optional_real_pc34_corpus_roundtrip();
    test_original_projectile_event_plan_preserves_c48_bits();
    test_world_export_rebuilds_c48_c49_projectile_union();
    test_world_export_rebuilds_c25_explosion_union();
    test_world_export_roundtrips_c13_vi_altar_union();
    test_strings();
    puts("PASS dm1_v1_original_save_pc34_handoff");
    return 0;
}
