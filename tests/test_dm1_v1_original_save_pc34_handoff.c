#include "dm1_v1_original_save_pc34_handoff.h"

#include "memory_champion_state_pc34_compat.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(void)
{
    test_pc34_handoff_imports_party_state();
    test_rejects_non_pc34_and_truncated_parts();
    test_file_runtime_world_loader();
    test_public_fixture_builder_roundtrips_pc34_handoff();
    test_strings();
    puts("PASS dm1_v1_original_save_pc34_handoff");
    return 0;
}
