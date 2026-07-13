#include "dm1_v1_original_save_pc34_handoff.h"

#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_original_pc34_file_bytes(
    const char *path,
    uint8_t **out_bytes,
    size_t *out_size);

static int dm1_original_save_backup_path(const char *path,
                                         char out_path[DM1_ORIGINAL_SAVE_PATH_MAX])
{
    size_t length;
    if (!path || !path[0] || !out_path) return 0;
    length = strlen(path);
    if (length + 4u >= DM1_ORIGINAL_SAVE_PATH_MAX) return 0;
    memcpy(out_path, path, length);
    memcpy(out_path + length, ".bak", 5u);
    return 1;
}

static int dm1_original_save_file_opens_for_read(const char *path)
{
    FILE *file;
    if (!path || !path[0]) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    fclose(file);
    return 1;
}

/* A corpus round-trip is evidence for external original bytes only. The
 * Firestaff exporter deliberately stamps AdditionalData with LSV01RDM, which
 * ReDMCSB ignores but must not certify its own output as an original-save
 * corpus. CSBWin's 512-byte GAMEBLOCK1 also cannot pass this gate by header
 * shape alone: the classifier has already required F0435/F7057's five
 * length-prefixed, keyed, checksummed parts before this provenance check. */
static int dm1_original_save_corpus_external_pc34_file(
    const char *path,
    int *out_firestaff_manifest)
{
    uint8_t *bytes = NULL;
    size_t size = 0u;
    int manifest_result;
    int result;

    if (out_firestaff_manifest) {
        *out_firestaff_manifest = 0;
    }
    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return 0;
    }
    if (size > (size_t)((int)0x7fffffff)) {
        free(bytes);
        return 0;
    }
    manifest_result = F0799_SAVEGAME_PC34PeekManifest_Compat(
        bytes, (int)size, NULL, NULL, NULL);
    free(bytes);
    if (manifest_result == SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT) {
        return 1;
    }
    if (manifest_result == SAVEGAME_PC34_MANIFEST_OK &&
        out_firestaff_manifest) {
        *out_firestaff_manifest = 1;
    }
    return 0;
}

#define DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT 319u
#define DM1_PC34_ORIGINAL_PARTY_INFO_BYTE_COUNT 128u
#define DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT \
    ((DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY) + \
     DM1_PC34_ORIGINAL_PARTY_INFO_BYTE_COUNT)
#define DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT 16u
#define DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT 10u
#define DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT 3u
#define DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT 4u

#define DM1_PC34_CHAMPION_NAME_OFFSET 0u
#define DM1_PC34_CHAMPION_TITLE_OFFSET 8u
#define DM1_PC34_CHAMPION_DIRECTION_OFFSET 28u
#define DM1_PC34_CHAMPION_ATTRIBUTES_OFFSET 48u
#define DM1_PC34_CHAMPION_WOUNDS_OFFSET 50u
#define DM1_PC34_CHAMPION_CURRENT_HEALTH_OFFSET 52u
#define DM1_PC34_CHAMPION_MAXIMUM_HEALTH_OFFSET 54u
#define DM1_PC34_CHAMPION_CURRENT_STAMINA_OFFSET 56u
#define DM1_PC34_CHAMPION_MAXIMUM_STAMINA_OFFSET 58u
#define DM1_PC34_CHAMPION_CURRENT_MANA_OFFSET 60u
#define DM1_PC34_CHAMPION_MAXIMUM_MANA_OFFSET 62u
#define DM1_PC34_CHAMPION_FOOD_OFFSET 66u
#define DM1_PC34_CHAMPION_WATER_OFFSET 68u
#define DM1_PC34_CHAMPION_STATISTICS_OFFSET 70u
#define DM1_PC34_CHAMPION_SKILLS_OFFSET 91u
#define DM1_PC34_CHAMPION_SLOTS_OFFSET 211u
#define DM1_PC34_CHAMPION_LOAD_OFFSET 271u

#define DM1_PC34_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET 30u
#define DM1_PC34_GLOBAL_EVENT_COUNT_OFFSET 24u
#define DM1_PC34_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET 26u
#define DM1_PC34_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET 28u
#define DM1_PC34_GLOBAL_MAXIMUM_ACTIVE_GROUP_COUNT_OFFSET 46u

#define DM1_PC34_THING_TYPE_GROUP 4
#define DM1_PC34_THING_INDEX_MASK 0x03ffu
#define DM1_PC34_THING_TYPE_SHIFT 10u
#define DM1_PC34_THING_TYPE_MASK 0x000fu

static uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void write_u16_le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static void write_u32_le(uint8_t *p, uint32_t v)
{
    write_u16_le(p, (uint16_t)(v & 0xffffu));
    write_u16_le(p + 2u, (uint16_t)((v >> 16) & 0xffffu));
}

static int16_t read_i16_le(const uint8_t *p)
{
    return (int16_t)read_u16_le(p);
}

static int read_original_pc34_file_bytes(
    const char *path,
    uint8_t **out_bytes,
    size_t *out_size)
{
    FILE *file;
    long file_size;
    uint8_t *bytes;
    size_t read_count;

    if (!path || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    *out_bytes = NULL;
    *out_size = 0u;

    file = fopen(path, "rb");
    if (!file) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    file_size = ftell(file);
    if (file_size <= 0 ||
        file_size > (long)SAVEGAME_PC34_MAX_FILE_SIZE ||
        file_size > (long)((int)0x7fffffff)) {
        fclose(file);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    bytes = (uint8_t *)malloc((size_t)file_size);
    if (!bytes) {
        fclose(file);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    read_count = fread(bytes, 1u, (size_t)file_size, file);
    fclose(file);
    if (read_count != (size_t)file_size) {
        free(bytes);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    *out_bytes = bytes;
    *out_size = (size_t)file_size;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static uint16_t original_pc34_header_first_half_checksum(const uint8_t *header)
{
    uint16_t acc = 0u;
    size_t i;
    for (i = 0u; i < 32u; ++i) {
        acc = (uint16_t)(acc + read_u16_le(header + (i * 8u) + 0u));
        acc = (uint16_t)(acc ^ read_u16_le(header + (i * 8u) + 2u));
        acc = (uint16_t)(acc - read_u16_le(header + (i * 8u) + 4u));
        acc = (uint16_t)(acc ^ read_u16_le(header + (i * 8u) + 6u));
    }
    return acc;
}

static uint16_t original_pc34_header_second_half_plain_sum(const uint8_t *header)
{
    uint16_t sum = 0u;
    size_t i;
    for (i = SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS; i < 256u; ++i) {
        sum = (uint16_t)(sum + read_u16_le(header + (i * 2u)));
    }
    return sum;
}

static uint32_t read_skill_experience_le(const uint8_t *p)
{
    return read_u32_le(p + 2u);
}

static uint16_t skill_level_from_base_experience(uint32_t experience)
{
    uint16_t level = 1u;

    /* ReDMCSB CHAMPION.C F0303 lines 765-769: base skill level starts
     * at 1 and halves experience while it remains >= 500. Object
     * modifiers and temporary experience are not folded into saved base
     * runtime levels here; they remain live-runtime concerns. */
    while (experience >= 500u) {
        experience >>= 1;
        ++level;
    }
    return level;
}

static uint16_t f0417_xor_checksum_bytes(uint8_t *bytes,
                                         size_t word_count,
                                         uint16_t key)
{
    size_t i;
    uint16_t checksum = key;
    uint16_t rolling_key = key;
    for (i = 0u; i < word_count; ++i) {
        uint8_t *word = bytes + i * 2u;
        uint16_t value = read_u16_le(word);
        checksum = (uint16_t)(checksum + value);
        value = (uint16_t)(value ^ rolling_key);
        word[0] = (uint8_t)(value & 0xffu);
        word[1] = (uint8_t)((value >> 8) & 0xffu);
        checksum = (uint16_t)(checksum + value);
        rolling_key = (uint16_t)(rolling_key + (uint16_t)word_count);
    }
    return checksum;
}

static int write_original_part(uint8_t *dst,
                               size_t dst_capacity,
                               const uint8_t *plain,
                               size_t byte_count,
                               uint16_t key,
                               uint16_t *out_checksum)
{
    uint16_t checksum;

    if (!dst || !plain || !out_checksum) {
        return SAVEGAME_PC34_ERROR_NULL_ARG;
    }
    if ((byte_count & 1u) != 0u ||
        byte_count > 0xffffu ||
        dst_capacity < 2u + byte_count) {
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    }

    write_u16_le(dst, (uint16_t)byte_count);
    memcpy(dst + 2u, plain, byte_count);
    checksum = f0417_xor_checksum_bytes(dst + 2u, byte_count / 2u, key);
    *out_checksum = checksum;
    return (int)(2u + byte_count);
}

static void write_original_pc34_fixture_champion(uint8_t *dst,
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

    memset(dst, 0, DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT);
    memset(dst + DM1_PC34_CHAMPION_NAME_OFFSET, ' ', CHAMPION_NAME_LENGTH);
    memset(dst + DM1_PC34_CHAMPION_TITLE_OFFSET, ' ', CHAMPION_TITLE_LENGTH);
    if (name) {
        size_t n = strlen(name);
        if (n > CHAMPION_NAME_LENGTH) n = CHAMPION_NAME_LENGTH;
        memcpy(dst + DM1_PC34_CHAMPION_NAME_OFFSET, name, n);
    }
    if (title) {
        size_t n = strlen(title);
        if (n > CHAMPION_TITLE_LENGTH) n = CHAMPION_TITLE_LENGTH;
        memcpy(dst + DM1_PC34_CHAMPION_TITLE_OFFSET, title, n);
    }
    dst[DM1_PC34_CHAMPION_DIRECTION_OFFSET] = (uint8_t)(direction & 3);
    write_u16_le(dst + DM1_PC34_CHAMPION_ATTRIBUTES_OFFSET, 0x1234u);
    write_u16_le(dst + DM1_PC34_CHAMPION_WOUNDS_OFFSET, wounds);
    write_u16_le(dst + DM1_PC34_CHAMPION_CURRENT_HEALTH_OFFSET, (uint16_t)hp_current);
    write_u16_le(dst + DM1_PC34_CHAMPION_MAXIMUM_HEALTH_OFFSET, (uint16_t)hp_maximum);
    write_u16_le(dst + DM1_PC34_CHAMPION_CURRENT_STAMINA_OFFSET, (uint16_t)stamina_current);
    write_u16_le(dst + DM1_PC34_CHAMPION_MAXIMUM_STAMINA_OFFSET, (uint16_t)stamina_maximum);
    write_u16_le(dst + DM1_PC34_CHAMPION_CURRENT_MANA_OFFSET, (uint16_t)mana_current);
    write_u16_le(dst + DM1_PC34_CHAMPION_MAXIMUM_MANA_OFFSET, (uint16_t)mana_maximum);
    write_u16_le(dst + DM1_PC34_CHAMPION_FOOD_OFFSET, (uint16_t)food);
    write_u16_le(dst + DM1_PC34_CHAMPION_WATER_OFFSET, (uint16_t)water);
    for (i = 0; i < 7; ++i) {
        uint8_t *stat = dst + DM1_PC34_CHAMPION_STATISTICS_OFFSET +
                        (size_t)i * 3u;
        stat[0] = (uint8_t)(40 + i);
        stat[1] = (uint8_t)(30 + i);
        stat[2] = (uint8_t)(10 + i);
    }
    for (i = 0; i < 20; ++i) {
        uint8_t *skill = dst + DM1_PC34_CHAMPION_SKILLS_OFFSET +
                         (size_t)i * 6u;
        write_u16_le(skill, (uint16_t)(0x0100u + (uint16_t)i));
        write_u32_le(skill + 2u, 1000u + (uint32_t)i * 111u);
    }
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        write_u16_le(dst + DM1_PC34_CHAMPION_SLOTS_OFFSET +
                     (size_t)i * 2u, 0xffffu);
    }
    write_u16_le(dst + DM1_PC34_CHAMPION_SLOTS_OFFSET +
                 (size_t)CHAMPION_SLOT_HAND_RIGHT * 2u, hand_item);
    write_u16_le(dst + DM1_PC34_CHAMPION_LOAD_OFFSET, 345u);
}

static void write_original_pc34_fixture_active_group(uint8_t *dst,
                                                     uint16_t group_thing_index,
                                                     int directions,
                                                     int cells,
                                                     int target_x,
                                                     int target_y)
{
    write_u16_le(dst + 0u, group_thing_index);
    dst[2u] = (uint8_t)directions;
    dst[3u] = (uint8_t)cells;
    dst[4u] = 12u;
    dst[5u] = 3u;
    dst[6u] = (uint8_t)target_x;
    dst[7u] = (uint8_t)target_y;
    dst[8u] = 5u;
    dst[9u] = 6u;
    dst[10u] = 7u;
    dst[11u] = 8u;
    dst[12u] = 0x41u;
    dst[13u] = 0x42u;
    dst[14u] = 0x43u;
    dst[15u] = 0x44u;
}

static void write_original_pc34_fixture_event(uint8_t *dst,
                                              uint32_t map_time,
                                              int type,
                                              int priority,
                                              int map_x,
                                              int map_y,
                                              int cell,
                                              int effect)
{
    write_u32_le(dst + 0u, map_time);
    dst[4u] = (uint8_t)type;
    dst[5u] = (uint8_t)priority;
    dst[6u] = (uint8_t)map_x;
    dst[7u] = (uint8_t)map_y;
    dst[8u] = (uint8_t)cell;
    dst[9u] = (uint8_t)effect;
}

static int read_original_part(const uint8_t *bytes,
                              size_t size,
                              size_t *cursor,
                              uint16_t key,
                              uint16_t expected_checksum,
                              uint8_t *out_plain,
                              size_t out_capacity,
                              size_t *out_size,
                              uint16_t *out_actual_checksum)
{
    uint16_t byte_count;
    uint16_t actual_checksum;

    if (!bytes || !cursor || !out_plain || !out_size) {
        return SAVEGAME_PC34_ERROR_NULL_ARG;
    }
    *out_size = 0u;
    if (out_actual_checksum) {
        *out_actual_checksum = 0u;
    }
    if (*cursor + 2u > size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    byte_count = read_u16_le(bytes + *cursor);
    *cursor += 2u;
    if ((byte_count & 1u) != 0u ||
        *cursor + (size_t)byte_count > size ||
        (size_t)byte_count > out_capacity) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    memcpy(out_plain, bytes + *cursor, (size_t)byte_count);
    actual_checksum = f0417_xor_checksum_bytes(
        out_plain, (size_t)byte_count / 2u, key);
    *cursor += (size_t)byte_count;
    *out_size = (size_t)byte_count;
    if (out_actual_checksum) {
        *out_actual_checksum = actual_checksum;
    }
    if (actual_checksum != expected_checksum) {
        return SAVEGAME_PC34_ERROR_BAD_CHECKSUM;
    }
    return SAVEGAME_PC34_OK;
}

static void import_original_pc34_champion(const uint8_t *src,
                                          int slot,
                                          struct ChampionState_Compat *champ)
{
    int i;

    F0600_CHAMPION_InitEmpty_Compat(champ);
    champ->present = 1;
    champ->portraitIndex = slot;
    memcpy(champ->name, src + DM1_PC34_CHAMPION_NAME_OFFSET,
           CHAMPION_NAME_LENGTH);
    memcpy(champ->title, src + DM1_PC34_CHAMPION_TITLE_OFFSET,
           CHAMPION_TITLE_LENGTH);
    champ->direction = src[DM1_PC34_CHAMPION_DIRECTION_OFFSET];
    champ->wounds = read_u16_le(src + DM1_PC34_CHAMPION_WOUNDS_OFFSET);
    champ->hp.current =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_CURRENT_HEALTH_OFFSET);
    champ->hp.maximum =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_MAXIMUM_HEALTH_OFFSET);
    champ->hp.shifted = (uint16_t)(champ->hp.maximum << 1);
    champ->stamina.current =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_CURRENT_STAMINA_OFFSET);
    champ->stamina.maximum =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_MAXIMUM_STAMINA_OFFSET);
    champ->stamina.shifted = (uint16_t)(champ->stamina.maximum << 1);
    champ->mana.current =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_CURRENT_MANA_OFFSET);
    champ->mana.maximum =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_MAXIMUM_MANA_OFFSET);
    champ->mana.shifted = (uint16_t)(champ->mana.maximum << 1);
    champ->food = read_i16_le(src + DM1_PC34_CHAMPION_FOOD_OFFSET);
    champ->water = read_i16_le(src + DM1_PC34_CHAMPION_WATER_OFFSET);

    for (i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
        const uint8_t *stat = src + DM1_PC34_CHAMPION_STATISTICS_OFFSET +
                              (size_t)(i + 1) * 3u;
        champ->attributeMaximums[i] = stat[0];
        champ->attributes[i] = stat[1];
    }

    /* ReDMCSB DEFS.H lines 608-622 stores each SKILL as
     * TemporaryExperience + Experience. Firestaff currently exposes four
     * base skill experience buckets, matching DEFS.H lines 756-760. */
    for (i = 0; i < CHAMPION_SKILL_COUNT; ++i) {
        const uint8_t *skill = src + DM1_PC34_CHAMPION_SKILLS_OFFSET +
                               (size_t)i * 6u;
        champ->skillExperience[i] = read_skill_experience_le(skill);
        champ->skillLevels[i] =
            skill_level_from_base_experience((uint32_t)champ->skillExperience[i]);
    }
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = read_u16_le(src + DM1_PC34_CHAMPION_SLOTS_OFFSET +
                                          (size_t)i * 2u);
    }
    champ->load = read_u16_le(src + DM1_PC34_CHAMPION_LOAD_OFFSET);
}

static int import_original_pc34_party_part(const uint8_t *part,
                                           size_t part_size,
                                           struct SaveGame_Compat *out_state,
                                           DM1OriginalSavePC34HandoffReport *out_report)
{
    int slot_count;
    int i;

    /* ReDMCSB LOADSAVE.C F0435:2766-2777 reads one fixed PC34 PARTY
     * save part: M516_CHAMPIONS (4 * 319 bytes) followed by PARTY_INFO
     * (128 bytes). Do not treat excess bytes as a private extension. */
    if (part_size != DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (!out_state->party) {
        return SAVEGAME_PC34_OK;
    }

    slot_count = out_state->party->championCount;
    if (slot_count < 0) {
        slot_count = 0;
    }
    if (slot_count > CHAMPION_MAX_PARTY) {
        slot_count = CHAMPION_MAX_PARTY;
    }
    for (i = 0; i < slot_count; ++i) {
        import_original_pc34_champion(
            part + (size_t)i * DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT,
            i,
            &out_state->party->champions[i]);
    }
    for (; i < CHAMPION_MAX_PARTY; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&out_state->party->champions[i]);
    }
    if (out_report) {
        out_report->imported_champion_block_count = CHAMPION_MAX_PARTY;
        out_report->imported_champion_slot_count = slot_count;
        out_report->imported_skill_level_count =
            slot_count * CHAMPION_SKILL_COUNT;
    }
    return SAVEGAME_PC34_OK;
}

static void decode_original_pc34_active_group(
    const uint8_t *src,
    DM1OriginalSavePC34ActiveGroupRecord *dst)
{
    int i;

    dst->group_thing_index = read_i16_le(src + 0u);
    dst->directions = src[2u];
    dst->cells = src[3u];
    dst->last_move_time = src[4u];
    dst->delay_fleeing_from_target = src[5u];
    dst->target_map_x = src[6u];
    dst->target_map_y = src[7u];
    dst->prior_map_x = src[8u];
    dst->prior_map_y = src[9u];
    dst->home_map_x = src[10u];
    dst->home_map_y = src[11u];
    for (i = 0; i < 4; ++i) {
        dst->aspect[i] = src[12u + (size_t)i];
    }
}

static int import_original_pc34_active_group_part(
    const uint8_t *part,
    size_t part_size,
    int maximum_active_group_count,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    size_t expected_size;
    int i;
    int report_count;

    if (maximum_active_group_count < 0) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    expected_size = (size_t)maximum_active_group_count *
                    DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT;
    if (part_size != expected_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (!out_report) {
        return SAVEGAME_PC34_OK;
    }

    out_report->decoded_active_group_count = maximum_active_group_count;
    report_count = maximum_active_group_count;
    if (report_count > DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP) {
        report_count = DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP;
    }
    out_report->reported_active_group_count = report_count;
    out_report->active_group_decode_truncated_count =
        maximum_active_group_count - report_count;
    for (i = 0; i < report_count; ++i) {
        decode_original_pc34_active_group(
            part + (size_t)i * DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT,
            &out_report->active_groups[i]);
    }
    return SAVEGAME_PC34_OK;
}

static void decode_original_pc34_event(const uint8_t *src,
                                       struct DM1_Event_V1 *dst)
{
    dst->map_time = read_u32_le(src + 0u);
    dst->type = src[4u];
    dst->priority = src[5u];
    dst->b_mapX = src[6u];
    dst->b_mapY = src[7u];
    dst->c_cell = src[8u];
    dst->c_effect = src[9u];
}

static int import_original_pc34_events_part(
    const uint8_t *part,
    size_t part_size,
    int event_maximum_count,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    size_t expected_size;
    int i;
    int decode_count;

    if (event_maximum_count < 0) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    expected_size = (size_t)event_maximum_count *
                    DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT;
    if (part_size != expected_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (!out_report) {
        return SAVEGAME_PC34_OK;
    }

    decode_count = event_maximum_count;
    if (decode_count > DM1_EVENT_MAX_COUNT) {
        decode_count = DM1_EVENT_MAX_COUNT;
    }
    out_report->decoded_event_count = decode_count;
    out_report->event_decode_truncated_count =
        event_maximum_count - decode_count;
    for (i = 0; i < decode_count; ++i) {
        decode_original_pc34_event(
            part + (size_t)i * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT,
            &out_report->events[i]);
    }
    return SAVEGAME_PC34_OK;
}

static int import_original_pc34_timeline_part(
    const uint8_t *part,
    size_t part_size,
    int event_maximum_count,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    size_t expected_size;
    int i;
    int decode_count;

    if (event_maximum_count < 0) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    expected_size = (size_t)event_maximum_count * 2u;
    if (part_size != expected_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (!out_report) {
        return SAVEGAME_PC34_OK;
    }

    decode_count = event_maximum_count;
    if (decode_count > DM1_EVENT_MAX_COUNT) {
        decode_count = DM1_EVENT_MAX_COUNT;
    }
    out_report->decoded_timeline_index_count = decode_count;
    for (i = 0; i < decode_count; ++i) {
        out_report->timeline_indices[i] = read_u16_le(part + (size_t)i * 2u);
    }
    return SAVEGAME_PC34_OK;
}

static int timeline_kind_from_original_pc34_event_type(int type)
{
    switch (type) {
    case DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE:
    case DM1_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE:
    case DM1_EVENT_GROUP_REACTION_PARTY_IS_ADJACENT:
    case DM1_EVENT_UPDATE_ASPECT_GROUP:
    case DM1_EVENT_UPDATE_ASPECT_CREATURE_0:
    case DM1_EVENT_UPDATE_ASPECT_CREATURE_1:
    case DM1_EVENT_UPDATE_ASPECT_CREATURE_2:
    case DM1_EVENT_UPDATE_ASPECT_CREATURE_3:
    case DM1_EVENT_UPDATE_BEHAVIOR_GROUP:
    case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0:
    case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1:
    case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_2:
    case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3:
        return TIMELINE_EVENT_CREATURE_REACTION;
    case DM1_EVENT_DOOR_ANIMATION:
        return TIMELINE_EVENT_DOOR_ANIMATE;
    case DM1_EVENT_DOOR_DESTRUCTION:
        return TIMELINE_EVENT_DOOR_DESTRUCTION;
    case DM1_EVENT_ENABLE_CHAMPION_ACTION:
        return TIMELINE_EVENT_ENABLE_CHAMPION_ACTION;
    case DM1_EVENT_HIDE_DAMAGE_RECEIVED:
        return TIMELINE_EVENT_STATUS_TIMEOUT;
    case DM1_EVENT_VI_ALTAR_REBIRTH:
        return TIMELINE_EVENT_VI_ALTAR_REBIRTH;
    case DM1_EVENT_MOVE_PROJECTILE:
    case DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS:
        return TIMELINE_EVENT_PROJECTILE_MOVE;
    case DM1_EVENT_EXPLOSION:
        return TIMELINE_EVENT_EXPLOSION_ADVANCE;
    case DM1_EVENT_LIGHT:
        return TIMELINE_EVENT_MAGIC_LIGHT_DECAY;
    case DM1_EVENT_ENABLE_GROUP_GENERATOR:
        return TIMELINE_EVENT_GROUP_GENERATOR;
    case DM1_EVENT_REMOVE_FLUXCAGE:
        return TIMELINE_EVENT_REMOVE_FLUXCAGE;
    case DM1_EVENT_PLAY_SOUND:
        return TIMELINE_EVENT_PLAY_SOUND;
    case DM1_EVENT_WATCHDOG:
        return TIMELINE_EVENT_WATCHDOG;
    case DM1_EVENT_MOVE_GROUP_SILENT:
        return TIMELINE_EVENT_MOVE_GROUP_SILENT;
    case DM1_EVENT_MOVE_GROUP_AUDIBLE:
        return TIMELINE_EVENT_MOVE_GROUP_AUDIBLE;
    case DM1_EVENT_CORRIDOR:
    case DM1_EVENT_WALL:
    case DM1_EVENT_FAKEWALL:
    case DM1_EVENT_TELEPORTER:
    case DM1_EVENT_PIT:
    case DM1_EVENT_DOOR:
        return TIMELINE_EVENT_SQUARE_STATE;
    case DM1_EVENT_INVISIBILITY:
    case DM1_EVENT_CHAMPION_SHIELD:
    case DM1_EVENT_THIEVES_EYE:
    case DM1_EVENT_PARTY_SHIELD:
    case DM1_EVENT_POISON_CHAMPION:
    case DM1_EVENT_SPELLSHIELD:
    case DM1_EVENT_FIRESHIELD:
    case DM1_EVENT_FOOTPRINTS:
        return TIMELINE_EVENT_STATUS_TIMEOUT;
    default:
        return TIMELINE_EVENT_INVALID;
    }
}

static int original_pc34_event_type_is_group_reaction(int type)
{
    return type >= DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE &&
           type <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3;
}

static uint16_t original_pc34_next_thing(
    const struct DungeonThings_Compat *things,
    uint16_t thing)
{
    int type;
    int index;
    const unsigned char *raw;
    int byte_count;

    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_ENDOFLIST;
    }
    type = (int)THING_GET_TYPE(thing);
    index = (int)THING_GET_INDEX(thing);
    if (type < 0 || type >= 16 || index < 0 ||
        index >= things->thingCounts[type]) {
        return THING_ENDOFLIST;
    }
    raw = things->rawThingData[type];
    byte_count = s_thingDataByteCount[type];
    if (!raw || byte_count < 2) {
        return THING_ENDOFLIST;
    }
    return read_u16_le(raw + (size_t)index * (size_t)byte_count);
}

static int original_pc34_group_on_square(
    const struct GameWorld_Compat *world,
    int map_index,
    int map_x,
    int map_y,
    int *out_group_index)
{
    uint16_t thing;
    int safety = 0;

    if (out_group_index) *out_group_index = -1;
    if (!world || !world->dungeon || !world->things ||
        !world->things->loaded || !world->things->groups) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, map_index, map_x, map_y);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = (int)THING_GET_TYPE(thing);
        int index = (int)THING_GET_INDEX(thing);
        if (type == THING_TYPE_GROUP && index >= 0 &&
            index < world->things->groupCount) {
            if (out_group_index) *out_group_index = index;
            return 1;
        }
        thing = original_pc34_next_thing(world->things, thing);
    }
    return 0;
}

static int original_pc34_explosion_on_square(
    const struct GameWorld_Compat *world,
    int map_index,
    int map_x,
    int map_y,
    uint16_t expected_thing,
    int *out_explosion_index)
{
    uint16_t thing;
    int safety = 0;

    if (out_explosion_index) *out_explosion_index = -1;
    if (!world || !world->dungeon || !world->things ||
        !world->things->loaded || !world->things->explosions ||
        THING_GET_TYPE(expected_thing) != THING_TYPE_EXPLOSION) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, map_index, map_x, map_y);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if (thing == expected_thing) {
            int index = (int)THING_GET_INDEX(thing);
            if (index < 0 || index >= world->things->explosionCount) {
                return 0;
            }
            if (out_explosion_index) *out_explosion_index = index;
            return 1;
        }
        thing = original_pc34_next_thing(world->things, thing);
    }
    return 0;
}

static int original_pc34_event_type_is_status_timeout(int type)
{
    return type == DM1_EVENT_INVISIBILITY ||
           type == DM1_EVENT_CHAMPION_SHIELD ||
           type == DM1_EVENT_THIEVES_EYE ||
           type == DM1_EVENT_PARTY_SHIELD ||
           type == DM1_EVENT_POISON_CHAMPION ||
           type == DM1_EVENT_SPELLSHIELD ||
           type == DM1_EVENT_FIRESHIELD ||
           type == DM1_EVENT_FOOTPRINTS;
}

int dm1_v1_original_save_pc34_handoff_projectile_event_plan(
    const struct DM1_Event_V1 *src,
    int source_index,
    const struct DungeonThings_Compat *things,
    DM1OriginalSavePC34ProjectileEventPlan *out_plan)
{
    const struct DungeonProjectile_Compat *source_projectile;
    uint16_t source_thing;
    uint16_t projectile_motion;
    int projectile_index;
    int projectile_type;

    if (!src || !things || !out_plan ||
        (src->type != DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS &&
         src->type != DM1_EVENT_MOVE_PROJECTILE)) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));

    /* ReDMCSB DEFS.H EVENT stores B.Slot as a C14 THING and packs
     * C.Projectile as MapX:5, MapY:5, Direction:2, StepEnergy:4.  An
     * original C48/C49 must bind both records before it can enter M10. */
    source_thing = read_u16_le(&src->b_mapX);
    projectile_motion = read_u16_le(&src->c_cell);
    if (THING_GET_TYPE(source_thing) != THING_TYPE_PROJECTILE) {
        return 0;
    }
    projectile_index = (int)THING_GET_INDEX(source_thing);
    if (projectile_index < 0 || projectile_index >= PROJECTILE_LIST_CAPACITY ||
        projectile_index >= things->projectileCount || !things->projectiles) {
        return 0;
    }
    source_projectile = &things->projectiles[projectile_index];
    if ((int)source_projectile->eventIndex != source_index) {
        return 0;
    }
    projectile_type = THING_GET_TYPE(source_projectile->slot);
    out_plan->valid = 1;
    out_plan->source_event_type = src->type;
    out_plan->source_event_index = source_index;
    out_plan->projectile_index = projectile_index;
    out_plan->projectile_category =
        projectile_type == THING_TYPE_EXPLOSION
            ? PROJECTILE_CATEGORY_MAGICAL : PROJECTILE_CATEGORY_KINETIC;
    out_plan->projectile_subtype =
        projectile_type == THING_TYPE_EXPLOSION
            ? (int)(source_projectile->slot & 0xffu)
            : PROJECTILE_SUBTYPE_KINETIC_ARROW;
    out_plan->map_index = (int)((src->map_time >> 24) & 0xffu);
    out_plan->map_x = (int)(projectile_motion & 0x001fu);
    out_plan->map_y = (int)((projectile_motion >> 5) & 0x001fu);
    out_plan->cell = (int)THING_GET_CELL(source_thing);
    out_plan->direction = (int)((projectile_motion >> 10) & 0x03u);
    out_plan->step_energy = (int)((projectile_motion >> 12) & 0x0fu);
    out_plan->first_move_grace =
        src->type == DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS;
    out_plan->kinetic_energy = source_projectile->kineticEnergy;
    out_plan->attack = source_projectile->attack;
    out_plan->associated_thing = source_projectile->slot;
    return 1;
}

int dm1_v1_original_save_pc34_handoff_vi_altar_rebirth_event_plan(
    const struct DM1_Event_V1 *src,
    int source_event_index,
    DM1OriginalSavePC34ViAltarRebirthEventPlan *out_plan)
{
    if (!src || !out_plan || source_event_index < 0 ||
        src->type != DM1_EVENT_VI_ALTAR_REBIRTH ||
        src->priority >= CHAMPION_MAX_PARTY || src->c_cell > 3u ||
        src->c_effect > 2u) {
        return 0;
    }

    /* ReDMCSB CLIKVIEW.C F0374 lines 179-186 creates C13 from the bones
     * location/cell and ChargeCount champion index. TIMELINE.C F0255 lines
     * 1665-1699 consumes B.Location, C.A.Cell, C.A.Effect and Priority for
     * its exact 2 -> 1 -> 0 sequence. Do not collapse this union into the
     * generic Location/Cell/Effect handoff before that transaction exists. */
    memset(out_plan, 0, sizeof(*out_plan));
    out_plan->valid = 1;
    out_plan->source_event_index = source_event_index;
    out_plan->champion_index = src->priority;
    out_plan->map_index = (int)((src->map_time >> 24) & 0xffu);
    out_plan->map_x = src->b_mapX;
    out_plan->map_y = src->b_mapY;
    out_plan->cell = src->c_cell;
    out_plan->step = src->c_effect;
    out_plan->fire_at_tick = src->map_time & 0x00ffffffu;
    return 1;
}

static int materialize_original_pc34_projectile_event(
    const struct DM1_Event_V1 *src,
    int source_index,
    struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    struct ProjectileInstance_Compat *runtime_projectile;
    DM1OriginalSavePC34ProjectileEventPlan plan;

    if (!src || !world || !world->dungeon || !world->things || !out_event ||
        !dm1_v1_original_save_pc34_handoff_projectile_event_plan(
            src, source_index, world->things, &plan)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    if (plan.map_index < 0 ||
        plan.map_index >= (int)world->dungeon->header.mapCount ||
        plan.map_x >= (int)world->dungeon->maps[plan.map_index].width ||
        plan.map_y >= (int)world->dungeon->maps[plan.map_index].height) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    runtime_projectile = &world->projectiles.entries[plan.projectile_index];
    if (runtime_projectile->reserved3 != 0) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    memset(runtime_projectile, 0, sizeof(*runtime_projectile));
    runtime_projectile->slotIndex = plan.projectile_index;
    runtime_projectile->projectileCategory = plan.projectile_category;
    runtime_projectile->projectileSubtype = plan.projectile_subtype;
    runtime_projectile->ownerKind = -1;
    runtime_projectile->ownerIndex = -1;
    runtime_projectile->mapIndex = plan.map_index;
    runtime_projectile->mapX = plan.map_x;
    runtime_projectile->mapY = plan.map_y;
    runtime_projectile->cell = plan.cell;
    runtime_projectile->direction = plan.direction;
    runtime_projectile->kineticEnergy = plan.kinetic_energy;
    runtime_projectile->attack = plan.attack;
    runtime_projectile->stepEnergy = plan.step_energy;
    runtime_projectile->firstMoveGraceFlag = plan.first_move_grace;
    runtime_projectile->launchedAtTick =
        (int)((src->map_time & 0x00ffffffu) - 1u);
    runtime_projectile->scheduledAtTick =
        (int)(src->map_time & 0x00ffffffu);
    runtime_projectile->attackTypeCode = COMBAT_ATTACK_BLUNT;
    runtime_projectile->flags = PROJECTILE_FLAG_IGNORE_DOOR_PASS_THROUGH;
    runtime_projectile->launcherStrength = plan.attack;
    runtime_projectile->reserved1 = (int)plan.associated_thing;
    runtime_projectile->reserved3 = 1;
    if (world->projectiles.count <= plan.projectile_index) {
        world->projectiles.count = plan.projectile_index + 1;
    }

    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = plan.map_index;
    out_event->mapX = plan.map_x;
    out_event->mapY = plan.map_y;
    out_event->cell = plan.cell;
    out_event->aux0 = plan.projectile_index;
    out_event->aux3 = runtime_projectile->projectileSubtype;
    out_event->aux4 = src->priority;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_group_reaction_event(
    const struct DM1_Event_V1 *src,
    struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    int group_index;

    if (!src || !world || !out_event ||
        !original_pc34_event_type_is_group_reaction(src->type) ||
        !original_pc34_group_on_square(world,
                                       (int)((src->map_time >> 24) & 0xffu),
                                       (int)src->b_mapX, (int)src->b_mapY,
                                       &group_index)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    /* ReDMCSB TIMELINE.C F0261:1858-1863 extracts C29..C41 and calls
     * GROUP.C F0209 with B.Location and C.Ticks.  Resolve the group from
     * the original SFT chain rather than inventing an M10 group identity. */
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_CREATURE_REACTION;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = (int)((src->map_time >> 24) & 0xffu);
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->aux0 = group_index;
    out_event->aux1 = world->things->groups[group_index].creatureType;
    out_event->aux2 = src->type;
    out_event->aux3 = (int)read_u16_le(&src->c_cell);
    /* Keep the source byte while marking aux3 as an original C.Ticks
     * payload; M10-generated reactions retain their legacy tick route. */
    out_event->aux4 = (int)src->priority | 0x100;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_explosion_event(
    const struct DM1_Event_V1 *src,
    struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    const struct DungeonExplosion_Compat *source_explosion;
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_event;
    uint16_t source_thing;
    int source_index;
    int runtime_index;
    int map_index;

    if (!src || !world || !world->dungeon || !world->things || !out_event ||
        src->type != DM1_EVENT_EXPLOSION) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    source_thing = read_u16_le(&src->c_cell);

    /* ReDMCSB PROJEXPL.C F0213:157-165 creates C25 with B.Location and
     * C.Slot. TIMELINE.C F0261:1872 forwards that same EVENT to F0220.
     * A C25 cannot be reconstructed from Cell/Effect: its C15 reference
     * must be present in the original square chain before M10 publishes it. */
    if (map_index < 0 || map_index >= (int)world->dungeon->header.mapCount ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height ||
        !original_pc34_explosion_on_square(world, map_index,
                                           src->b_mapX, src->b_mapY,
                                           source_thing, &source_index)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    source_explosion = &world->things->explosions[source_index];
    memset(&input, 0, sizeof(input));
    input.explosionType = source_explosion->type;
    input.attack = source_explosion->attack;
    input.mapIndex = map_index;
    input.mapX = src->b_mapX;
    input.mapY = src->b_mapY;
    input.cell = (int)THING_GET_CELL(source_thing);
    input.centered = source_explosion->centered;
    input.currentTick = (int)((src->map_time & 0x00ffffffu) - 1u);
    input.ownerKind = -1;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    if (!F0821_EXPLOSION_Create_Compat(&input, &world->explosions,
                                       &runtime_index, &first_event)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    (void)first_event;
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_EXPLOSION_ADVANCE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->cell = (int)THING_GET_CELL(source_thing);
    out_event->aux0 = runtime_index;
    out_event->aux1 = source_explosion->type;
    out_event->aux2 = source_explosion->attack;
    out_event->aux4 = src->priority;
    world->explosions.entries[runtime_index].scheduledAtTick =
        (int)(src->map_time & 0x00ffffffu);
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_remove_fluxcage_event(
    const struct DM1_Event_V1 *src,
    struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    const struct DungeonExplosion_Compat *source_explosion;
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_event;
    uint16_t source_thing;
    int source_index;
    int runtime_index;
    int map_index;

    if (!src || !world || !world->dungeon || !world->things || !out_event ||
        src->type != DM1_EVENT_REMOVE_FLUXCAGE || src->priority != 0u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    source_thing = read_u16_le(&src->c_cell);

    /* ReDMCSB PROJEXPL.C F0224:983-994 creates C24 only for a newly
     * linked C15 fluxcage: Priority=0, B.Location, C.Slot. TIMELINE.C
     * F0261:1906-1916 later unlinks that exact Thing unless the game is
     * won. Do not reinterpret C.Slot as a host ExplosionList index. */
    if (map_index < 0 || map_index >= (int)world->dungeon->header.mapCount ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height ||
        THING_GET_CELL(source_thing) != 0 ||
        !original_pc34_explosion_on_square(world, map_index,
                                           src->b_mapX, src->b_mapY,
                                           source_thing, &source_index) ||
        world->things->explosions[source_index].type !=
            C050_EXPLOSION_FLUXCAGE) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    source_explosion = &world->things->explosions[source_index];
    memset(&input, 0, sizeof(input));
    input.explosionType = C050_EXPLOSION_FLUXCAGE;
    input.attack = source_explosion->attack;
    input.mapIndex = map_index;
    input.mapX = src->b_mapX;
    input.mapY = src->b_mapY;
    input.cell = 0;
    input.centered = source_explosion->centered;
    input.currentTick = (int)((src->map_time & 0x00ffffffu) - 1u);
    input.ownerKind = -1;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    if (!F0821_EXPLOSION_Create_Compat(&input, &world->explosions,
                                       &runtime_index, &first_event)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    (void)first_event;
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_REMOVE_FLUXCAGE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->cell = 0;
    out_event->aux0 = runtime_index;
    out_event->aux1 = C050_EXPLOSION_FLUXCAGE;
    out_event->aux2 = (int)source_thing;
    out_event->aux4 = 0;
    world->explosions.entries[runtime_index].scheduledAtTick =
        (int)(src->map_time & 0x00ffffffu);
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_light_event(
    const struct DM1_Event_V1 *src,
    struct TimelineEvent_Compat *out_event)
{
    int light_power;
    int abs_power;

    if (!src || !out_event || src->type != DM1_EVENT_LIGHT ||
        src->priority != 0u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    light_power = (int)(int16_t)read_u16_le(&src->b_mapX);
    abs_power = light_power < 0 ? -light_power : light_power;
    /* ReDMCSB TIMELINE.C F0257:1747-1765 consumes only B.LightPower,
     * decrements its signed magnitude, and queues C70 at Priority=0.
     * DATA.C G0039 has 16 entries, so zero or magnitudes above 15 are
     * not a live source sequence Firestaff can faithfully materialize. */
    if (light_power == 0 || abs_power > RUNTIME_LIGHT_POWER_MAX) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_MAGIC_LIGHT_DECAY;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = (int)((src->map_time >> 24) & 0xffu);
    out_event->aux0 = light_power;
    /* aux0 is live LightPower; retain C70 separately as F0802's receipt. */
    out_event->aux1 = DM1_EVENT_LIGHT;
    out_event->aux4 = 0;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_generator_reenable_event(
    const struct DM1_Event_V1 *src, const struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    uint16_t thing;
    int map_index;
    int sensor_index = -1;
    int safety = 0;
    if (!src || !world || !world->dungeon || !world->things ||
        !world->things->loaded || !world->things->sensors || !out_event ||
        src->type != DM1_EVENT_ENABLE_GROUP_GENERATOR || src->priority != 0u) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    map_index = (int)((src->map_time >> 24) & 0xffu);
    if (map_index < 0 || map_index >= (int)world->dungeon->header.mapCount ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    /* ReDMCSB TIMELINE.C F0246:1020-1027 reads B.Location only and
     * re-enables the first disabled sensor found on that exact square. */
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(world->dungeon, world->things,
                                                      map_index, src->b_mapX, src->b_mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int index = (int)THING_GET_INDEX(thing);
        if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR && index >= 0 &&
            index < world->things->sensorCount &&
            world->things->sensors[index].sensorType == RUNTIME_SENSOR_TYPE_DISABLED) {
            sensor_index = index;
            break;
        }
        thing = original_pc34_next_thing(world->things, thing);
    }
    if (sensor_index < 0) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_GROUP_GENERATOR;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->aux0 = GENERATOR_EVENT_AUX0_REENABLE;
    out_event->aux1 = sensor_index;
    out_event->aux2 = DM1_EVENT_ENABLE_GROUP_GENERATOR;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_timeline(
    const DM1OriginalSavePC34HandoffReport *report,
    struct GameWorld_Compat *world,
    struct TimelineQueue_Compat *timeline)
{
    int i;

    if (!report || !world || !timeline) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (report->original_event_count < 0 ||
        report->original_event_count > DM1_EVENT_MAX_COUNT ||
        report->original_event_count > TIMELINE_QUEUE_CAPACITY ||
        report->original_event_count > report->decoded_event_count ||
        report->original_event_count > report->decoded_timeline_index_count) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    /* ReDMCSB LOADSAVE.C F0435 lines 2780-2800 loads the EVENT array
     * and timeline heap immediately after PARTY, then initializes the
     * optimized timeline management. Mirror that runtime handoff here:
     * the report preserves the raw source EVENT/TIMELINE bytes, while
     * GameWorld_Compat needs the equivalent M10 TimelineQueue. */
    (void)F0720_TIMELINE_Init_Compat(timeline, report->original_game_time);
    for (i = 0; i < report->original_event_count; ++i) {
        uint16_t source_index = report->timeline_indices[i];
        const struct DM1_Event_V1 *src;
        struct TimelineEvent_Compat ev;
        int kind;
        if (source_index >= (uint16_t)report->decoded_event_count) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        src = &report->events[source_index];
        kind = timeline_kind_from_original_pc34_event_type(src->type);
        if (kind == TIMELINE_EVENT_INVALID) {
            /* ReDMCSB LOADSAVE.C F0435:2781-2800 restores the complete
             * EVENTS/TIMELINE pair, then TIMELINE.C F0651:100-124 rebuilds
             * its live management over every non-NONE event.  Dropping an
             * active source event here would publish a different runtime;
             * reject the candidate world until that event family has a real
             * M10 materialization route. */
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        if (src->type == DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS ||
            src->type == DM1_EVENT_MOVE_PROJECTILE) {
            if (materialize_original_pc34_projectile_event(
                    src, (int)source_index, world, &ev) !=
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_ENABLE_GROUP_GENERATOR) {
            if (materialize_original_pc34_generator_reenable_event(
                    src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (original_pc34_event_type_is_group_reaction(src->type)) {
            if (materialize_original_pc34_group_reaction_event(
                    src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_EXPLOSION) {
            if (materialize_original_pc34_explosion_event(src, world, &ev) !=
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_REMOVE_FLUXCAGE) {
            if (materialize_original_pc34_remove_fluxcage_event(
                    src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_LIGHT) {
            if (materialize_original_pc34_light_event(src, &ev) !=
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_INVISIBILITY) {
            /* ReDMCSB MENU.C F0412:1922-1964 creates C71 with Priority
             * zero, while TIMELINE.C C71:1953-1964 consumes no B/C union
             * arm. Keep those bytes outside the live contract. */
            if (src->priority != 0u) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_INVISIBILITY;
            ev.aux2 = DM1_EVENT_INVISIBILITY;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_THIEVES_EYE) {
            /* ReDMCSB MENU.C action F0407:1542-1546 and spell F0412 set
             * C73 Priority to zero. TIMELINE.C C73:1972-1974 consumes no
             * B/C union arm, so those source bytes have no live owner. */
            if (src->priority != 0u) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_THIEVES_EYE;
            ev.aux2 = DM1_EVENT_THIEVES_EYE;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_CHAMPION_SHIELD) {
            int defense = (int)(int16_t)read_u16_le(&src->b_mapX);
            if (src->priority >= CHAMPION_MAX_PARTY ||
                !world->party.champions[src->priority].present) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            /* ReDMCSB TIMELINE.C C72:1964-1967 consumes Priority and
             * signed B.Defense only. C is not part of this event union. */
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_CHAMPION_SHIELD;
            ev.aux1 = defense;
            ev.aux2 = DM1_EVENT_CHAMPION_SHIELD;
            ev.aux4 = src->priority;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        memset(&ev, 0, sizeof(ev));
        ev.kind = kind;
        ev.fireAtTick = src->map_time & 0x00ffffffu;
        ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
        ev.aux0 = src->type;
        ev.aux4 = src->priority;
        /* ReDMCSB TIMELINE.C C11 first clears the champion action lock,
         * then conditionally moves a weapon from a quiver when SlotOrdinal
         * is non-zero.  Firestaff has a source-locked live action-lock
         * route, but no proven original quiver transfer handoff yet.  Do
         * not silently reinterpret that byte as a generic inventory slot. */
        if (src->type == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            src->c_cell != 0u) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        if (src->type == DM1_EVENT_HIDE_DAMAGE_RECEIVED) {
            /* ReDMCSB CHAMPION.C F0320 writes only Map_Time, Type and
             * Priority for C12. TIMELINE.C F0254 consumes only Priority.
             * B/C are an uninitialised union arm in the source event and
             * must never be reinterpreted as Location/Cell/Effect. */
            if (src->priority >= CHAMPION_MAX_PARTY ||
                !world->party.champions[src->priority].present) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
        } else if (src->type == DM1_EVENT_VI_ALTAR_REBIRTH) {
            DM1OriginalSavePC34ViAltarRebirthEventPlan plan;

            if (!dm1_v1_original_save_pc34_handoff_vi_altar_rebirth_event_plan(
                    src, (int)source_index, &plan) || !world->dungeon ||
                plan.map_index >= (int)world->dungeon->header.mapCount ||
                plan.map_x >= (int)world->dungeon->maps[plan.map_index].width ||
                plan.map_y >= (int)world->dungeon->maps[plan.map_index].height ||
                !world->party.champions[plan.champion_index].present) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            ev.mapX = plan.map_x;
            ev.mapY = plan.map_y;
            ev.cell = plan.cell;
            ev.aux1 = plan.step;
        } else if (original_pc34_event_type_is_status_timeout(src->type)) {
            ev.aux1 = (int)read_u16_le(&src->b_mapX);
            ev.cell = src->c_cell;
        } else if (src->type == DM1_EVENT_PLAY_SOUND) {
            /* ReDMCSB SOUND.C:1536-1543 writes B.Location and the signed
             * C.SoundIndex union member; TIMELINE.C:1903-1905 passes those
             * three values directly to F0064_SOUND_RequestPlay. */
            ev.mapX = src->b_mapX;
            ev.mapY = src->b_mapY;
            ev.aux0 = (int)(int16_t)read_u16_le(&src->c_cell);
        } else {
            ev.mapX = src->b_mapX;
            ev.mapY = src->b_mapY;
            ev.cell = src->c_cell;
            ev.aux1 = src->c_effect;
        }
        if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int import_original_pc34_external_portraits(
    const uint8_t *bytes,
    size_t size,
    size_t cursor,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    int slot;
    int count;
    const size_t portrait_bytes = SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT;

    /* ReDMCSB LOADSAVE.C F0435 lines ~2810-2816 reads all four fixed
     * 32x29 portrait payloads after the five save parts. Validate the
     * whole section before copying portrait 0, so truncation cannot leave
     * a partially updated party. */
    if (!bytes || cursor > size || portrait_bytes > size - cursor) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    count = (out_state && out_state->party) ? out_state->party->championCount : 0;
    if (count < 0) count = 0;
    if (count > CHAMPION_MAX_PARTY) count = CHAMPION_MAX_PARTY;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        if (out_state && out_state->party && slot < count) {
            memcpy(out_state->party->champions[slot].portraitBitmap,
                   bytes + cursor,
                   CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
            out_state->party->champions[slot].portraitBitmapValid = 1;
        }
        cursor += CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
    }
    if (out_report) {
        out_report->external_portrait_byte_count = (uint32_t)portrait_bytes;
        out_report->external_portrait_payload_count = CHAMPION_MAX_PARTY;
        out_report->external_portrait_imported_count = count;
    }
    return SAVEGAME_PC34_OK;
}

static uint16_t original_pc34_byte_checksum(const uint8_t *bytes,
                                            size_t count)
{
    uint16_t checksum = 0u;
    size_t i;
    for (i = 0u; i < count; ++i) {
        checksum = (uint16_t)(checksum + bytes[i]);
    }
    return checksum;
}

static uint32_t original_pc34_tail_fingerprint(const uint8_t *bytes,
                                               size_t count)
{
    uint32_t fingerprint = 2166136261u;
    size_t i;

    for (i = 0u; i < count; ++i) {
        fingerprint ^= bytes[i];
        fingerprint *= 16777619u;
    }
    return fingerprint;
}

/* ReDMCSB LOADSAVE.C F0435:2826 calls F0434 after the five save parts.
 * F0434's dungeon loader rejects map descriptors whose raw-map span falls
 * outside the saved raw-map block. Keep the receipt fail-closed before it
 * can describe a tail that the actual F0434/F0504 materialization rejects. */
static int validate_original_pc34_dungeon_tail_map_spans(
    const uint8_t *tail,
    int map_count,
    size_t map_descriptors_offset,
    size_t raw_map_offset,
    size_t raw_map_byte_count)
{
    int map_index;

    if (!tail || map_count <= 0 || map_count > DUNGEON_MAX_MAPS) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    for (map_index = 0; map_index < map_count; ++map_index) {
        const uint8_t *map = tail + map_descriptors_offset +
            (size_t)map_index * DUNGEON_MAP_DESC_SIZE;
        uint16_t raw_bitfield_a = read_u16_le(map + 8u);
        uint16_t raw_bitfield_c = read_u16_le(map + 12u);
        size_t raw_map_data_offset = (size_t)read_u16_le(map);
        size_t width = (size_t)((raw_bitfield_a >> 6) & 0x1fu) + 1u;
        size_t height = (size_t)((raw_bitfield_a >> 11) & 0x1fu) + 1u;
        size_t creature_type_count = (size_t)((raw_bitfield_c >> 4) & 0x0fu);
        size_t map_span = width * height + creature_type_count;

        if (raw_map_offset > SIZE_MAX - raw_map_data_offset ||
            raw_map_data_offset > raw_map_byte_count ||
            map_span > raw_map_byte_count - raw_map_data_offset) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
    }
    return SAVEGAME_PC34_OK;
}

static int decode_original_pc34_dungeon_tail(
    const uint8_t *bytes,
    size_t size,
    size_t cursor,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    const uint8_t *tail;
    size_t tail_size;
    size_t off;
    int map_count;
    int column_count = 0;
    int thing_data_bytes = 0;
    size_t map_descriptors_offset;
    int type;

    if (!bytes || !out_report || cursor >= size) {
        return SAVEGAME_PC34_OK;
    }
    tail = bytes + cursor;
    tail_size = size - cursor;
    if (tail_size < DUNGEON_HEADER_SIZE + 2u) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }

    map_count = (int)tail[4u];
    if (map_count <= 0 || map_count > DUNGEON_MAX_MAPS) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    map_descriptors_offset = DUNGEON_HEADER_SIZE;
    off = map_descriptors_offset;
    if (off + (size_t)map_count * DUNGEON_MAP_DESC_SIZE + 2u > tail_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    for (type = 0; type < map_count; ++type) {
        const uint8_t *map = tail + off + (size_t)type * DUNGEON_MAP_DESC_SIZE;
        uint16_t bitfield_a = read_u16_le(map + 8u);
        int width = (int)((bitfield_a >> 6) & 0x1fu) + 1;
        column_count += width;
    }
    off += (size_t)map_count * DUNGEON_MAP_DESC_SIZE;
    if (off + (size_t)column_count * 2u + 2u > tail_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    off += (size_t)column_count * 2u;

    {
        int square_first_thing_count = (int)read_u16_le(tail + 10u);
        int text_data_word_count = (int)read_u16_le(tail + 6u);
        int raw_map_bytes = (int)read_u16_le(tail + 2u);
        uint16_t expected_checksum;
        uint16_t actual_checksum;
        if (square_first_thing_count < 0 || text_data_word_count < 0 ||
            raw_map_bytes < 0) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        if (off + (size_t)square_first_thing_count * 2u +
                  (size_t)text_data_word_count * 2u + 2u > tail_size) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        off += (size_t)square_first_thing_count * 2u;
        off += (size_t)text_data_word_count * 2u;
        for (type = 0; type < DUNGEON_THING_TYPE_COUNT; ++type) {
            int count = (int)read_u16_le(tail + 12u + (size_t)type * 2u);
            int bytes_for_type = count * (int)s_thingDataByteCount[type];
            if (count < 0 || bytes_for_type < 0) {
                return SAVEGAME_PC34_ERROR_BAD_SIZE;
            }
            thing_data_bytes += bytes_for_type;
        }
        if (off + (size_t)thing_data_bytes + 2u > tail_size) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        off += (size_t)thing_data_bytes;
        if (off + (size_t)raw_map_bytes + 2u != tail_size) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        if (validate_original_pc34_dungeon_tail_map_spans(
                tail, map_count, map_descriptors_offset, off,
                (size_t)raw_map_bytes) != SAVEGAME_PC34_OK) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        off += (size_t)raw_map_bytes;
        expected_checksum = read_u16_le(tail + off);
        actual_checksum = original_pc34_byte_checksum(tail, off);
        out_report->dungeon_tail_present = 1;
        out_report->dungeon_tail_byte_count = (uint32_t)tail_size;
        out_report->dungeon_tail_fingerprint =
            original_pc34_tail_fingerprint(tail, tail_size);
        out_report->dungeon_tail_expected_checksum = expected_checksum;
        out_report->dungeon_tail_actual_checksum = actual_checksum;
        out_report->dungeon_tail_checksum_ok =
            (expected_checksum == actual_checksum);
        out_report->dungeon_tail_map_count = map_count;
        out_report->dungeon_tail_column_count = column_count;
        out_report->dungeon_tail_square_first_thing_count =
            square_first_thing_count;
        out_report->dungeon_tail_text_data_word_count =
            text_data_word_count;
        out_report->dungeon_tail_thing_data_byte_count =
            (uint32_t)thing_data_bytes;
        out_report->dungeon_tail_raw_map_data_byte_count =
            (uint32_t)raw_map_bytes;
        if (!out_report->dungeon_tail_checksum_ok) {
            return SAVEGAME_PC34_ERROR_BAD_CHECKSUM;
        }
    }
    return SAVEGAME_PC34_OK;
}

static size_t original_pc34_dungeon_tail_cursor(const uint8_t *bytes,
                                                size_t size)
{
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int part;

    if (!bytes || size < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        return 0u;
    }
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;
        if (cursor + 2u > size) {
            return 0u;
        }
        part_size = read_u16_le(bytes + cursor);
        cursor += 2u;
        if (cursor + (size_t)part_size > size) {
            return 0u;
        }
        cursor += (size_t)part_size;
    }
    if (cursor + CHAMPION_MAX_PARTY * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT >
        size) {
        return 0u;
    }
    cursor += CHAMPION_MAX_PARTY * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
    return cursor;
}

static int materialize_original_pc34_dungeon_tail(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *world,
    DM1OriginalSavePC34HandoffReport *report)
{
    size_t cursor;
    struct DungeonDatState_Compat *dungeon;
    struct DungeonThings_Compat *things;

    if (!bytes || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    cursor = original_pc34_dungeon_tail_cursor(bytes, size);
    if (cursor == 0u || cursor >= size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
    }
    if (size - cursor > (size_t)((int)0x7fffffff)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    dungeon = (struct DungeonDatState_Compat *)calloc(1u, sizeof(*dungeon));
    things = (struct DungeonThings_Compat *)calloc(1u, sizeof(*things));
    if (!dungeon || !things) {
        free(dungeon);
        free(things);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    if (!F0504_DUNGEON_LoadTailBuffer_Compat(
            bytes + cursor, (int)(size - cursor), dungeon, things)) {
        F0504_DUNGEON_FreeThingData_Compat(things);
        F0500_DUNGEON_FreeDatHeader_Compat(dungeon);
        free(things);
        free(dungeon);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    if (world->ownsDungeon) {
        F0883_WORLD_Free_Compat(world);
    }
    world->dungeon = dungeon;
    world->things = things;
    world->ownsDungeon = 1;
    world->dungeonFingerprint =
        report ? (((uint32_t)report->dungeon_tail_actual_checksum << 16) ^
                  report->dungeon_tail_byte_count) : 0u;
    if (report) {
        report->dungeon_tail_runtime_imported = 1;
    }
    (void)F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat(dungeon, things);
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int import_original_pc34_global_data(
    const uint8_t *bytes,
    size_t size,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    uint8_t meta[256];
    uint16_t key;
    uint16_t part_keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t part_checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    uint8_t part[SAVEGAME_PC34_TIMELINE_BYTE_COUNT];
    size_t part_size = 0u;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint32_t game_id;
    uint16_t actual_checksum = 0u;
    int maximum_active_group_count = 0;
    int event_maximum_count = 0;
    int i;
    int rc;

    if (size < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }

    key = read_u16_le(bytes + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u);
    memcpy(meta, bytes + 256u, sizeof(meta));
    (void)f0417_xor_checksum_bytes(
        meta, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);

    game_id = read_u32_le(meta + 50u);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        part_keys[i] = read_u16_le(meta + 54u + (size_t)i * 2u);
        part_checksums[i] = read_u16_le(meta + 86u + (size_t)i * 2u);
    }
    if (out_report) {
        for (i = 0; i < SAVEGAME_PC34_PART_COUNT; ++i) {
            out_report->part_expected_checksums[i] = part_checksums[i];
        }
    }

    rc = read_original_part(bytes, size, &cursor,
                            part_keys[SAVEGAME_PC34_PART_GLOBAL_DATA],
                            part_checksums[SAVEGAME_PC34_PART_GLOBAL_DATA],
                            part, sizeof(part), &part_size,
                            &actual_checksum);
    if (out_report) {
        out_report->part_byte_counts[SAVEGAME_PC34_PART_GLOBAL_DATA] =
            (uint32_t)part_size;
        out_report->part_actual_checksums[SAVEGAME_PC34_PART_GLOBAL_DATA] =
            actual_checksum;
        if (rc == SAVEGAME_PC34_OK) {
            out_report->part_checksum_ok_count++;
        }
    }
    if (rc != SAVEGAME_PC34_OK) {
        return rc;
    }
    if (part_size < SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    maximum_active_group_count = (int)read_u16_le(
        part + DM1_PC34_GLOBAL_MAXIMUM_ACTIVE_GROUP_COUNT_OFFSET);
    event_maximum_count = (int)read_u16_le(
        part + DM1_PC34_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET);
    if (out_report) {
        out_report->original_game_time = read_u32_le(part + 0u);
        out_report->original_current_active_group_count = (int)read_u16_le(
            part + DM1_PC34_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET);
        out_report->original_maximum_active_group_count =
            maximum_active_group_count;
        out_report->original_event_count = (int)read_u16_le(
            part + DM1_PC34_GLOBAL_EVENT_COUNT_OFFSET);
        out_report->original_first_unused_event_index = (int)read_u16_le(
            part + DM1_PC34_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET);
        out_report->original_event_maximum_count = event_maximum_count;
    }
    if (out_state->party) {
        out_state->party->championCount = (int)read_u16_le(part + 10u);
        out_state->party->mapX = (int)read_i16_le(part + 12u);
        out_state->party->mapY = (int)read_i16_le(part + 14u);
        out_state->party->direction = (int)read_i16_le(part + 16u);
        out_state->party->mapIndex = (int)read_i16_le(part + 18u);
        out_state->party->activeChampionIndex = (int)read_i16_le(part + 20u);
    }

    for (i = 1; i < SAVEGAME_PC34_PART_COUNT; ++i) {
        rc = read_original_part(bytes, size, &cursor, part_keys[i],
                                part_checksums[i],
                                part, sizeof(part), &part_size,
                                &actual_checksum);
        if (out_report) {
            out_report->part_byte_counts[i] = (uint32_t)part_size;
            out_report->part_actual_checksums[i] = actual_checksum;
            if (rc == SAVEGAME_PC34_OK) {
                out_report->part_checksum_ok_count++;
            }
        }
        if (rc != SAVEGAME_PC34_OK) {
            return rc;
        }
        if (i == SAVEGAME_PC34_PART_ACTIVE_GROUP) {
            rc = import_original_pc34_active_group_part(
                part, part_size, maximum_active_group_count, out_report);
            if (rc != SAVEGAME_PC34_OK) {
                return rc;
            }
        }
        if (i == SAVEGAME_PC34_PART_PARTY) {
            rc = import_original_pc34_party_part(part, part_size, out_state,
                                                 out_report);
            if (rc != SAVEGAME_PC34_OK) {
                return rc;
            }
        }
        if (i == SAVEGAME_PC34_PART_EVENTS) {
            rc = import_original_pc34_events_part(
                part, part_size, event_maximum_count, out_report);
            if (rc != SAVEGAME_PC34_OK) {
                return rc;
            }
        }
        if (i == SAVEGAME_PC34_PART_TIMELINE) {
            rc = import_original_pc34_timeline_part(
                part, part_size, event_maximum_count, out_report);
            if (rc != SAVEGAME_PC34_OK) {
                return rc;
            }
        }
        if (i == SAVEGAME_PC34_PART_TIMELINE && out_state->timeline && part_size > 0u) {
            size_t copy_n = part_size;
            if (copy_n > sizeof(*out_state->timeline)) {
                copy_n = sizeof(*out_state->timeline);
            }
            memcpy(out_state->timeline, part, copy_n);
        }
    }

    rc = import_original_pc34_external_portraits(bytes, size, cursor,
                                                 out_state, out_report);
    if (rc != SAVEGAME_PC34_OK) {
        return rc;
    }
    cursor += SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT;
    rc = decode_original_pc34_dungeon_tail(bytes, size, cursor, out_report);
    if (rc != SAVEGAME_PC34_OK) {
        return rc;
    }

    out_state->header.reserved[SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET + 0] =
        (unsigned char)(game_id & 0xffu);
    out_state->header.reserved[SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET + 1] =
        (unsigned char)((game_id >> 8) & 0xffu);
    out_state->header.reserved[SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET + 2] =
        (unsigned char)((game_id >> 16) & 0xffu);
    out_state->header.reserved[SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET + 3] =
        (unsigned char)((game_id >> 24) & 0xffu);

    if (out_report && out_state->party) {
        out_report->imported_champion_count =
            out_state->party->championCount;
        out_report->imported_map_index = out_state->party->mapIndex;
        out_report->imported_map_x = out_state->party->mapX;
        out_report->imported_map_y = out_state->party->mapY;
        out_report->imported_direction = out_state->party->direction;
        out_report->imported_active_champion_index =
            out_state->party->activeChampionIndex;
    }
    return SAVEGAME_PC34_OK;
}

int dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
    const DM1OriginalSavePC34FixtureSpec *spec,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size)
{
    uint8_t header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint8_t global[SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT];
    uint8_t active_group[DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT *
                         DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT];
    uint8_t party[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    uint8_t events[DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT *
                   DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT];
    uint8_t timeline[2u * DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT];
    uint8_t portraits[SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int champion_count;
    int current_active_group_count;
    int maximum_active_group_count;
    int event_count;
    int event_maximum_count;
    int rc;
    int i;

    if (!spec || !out_bytes || !out_size) {
        return SAVEGAME_PC34_ERROR_NULL_ARG;
    }
    *out_size = 0u;
    if (out_capacity < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    }

    champion_count = spec->champion_count;
    if (champion_count < 0) champion_count = 0;
    if (champion_count > CHAMPION_MAX_PARTY) champion_count = CHAMPION_MAX_PARTY;

    current_active_group_count = spec->current_active_group_count;
    if (current_active_group_count < 0) current_active_group_count = 0;
    if (current_active_group_count > (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT) {
        current_active_group_count = (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT;
    }
    maximum_active_group_count = spec->maximum_active_group_count;
    if (maximum_active_group_count <= 0) {
        maximum_active_group_count =
            (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT;
    }
    if (maximum_active_group_count >
        (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT) {
        maximum_active_group_count =
            (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT;
    }

    event_count = spec->event_count;
    if (event_count <= 0) event_count = 3;
    if (event_count > (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT) {
        event_count = (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT;
    }
    event_maximum_count = spec->event_maximum_count;
    if (event_maximum_count <= 0) {
        event_maximum_count = (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT;
    }
    if (event_maximum_count < event_count) {
        event_maximum_count = event_count;
    }
    if (event_maximum_count > (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT) {
        event_maximum_count = (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT;
    }

    memset(out_bytes, 0, out_capacity);
    memset(header, 0, sizeof(header));
    memset(global, 0, sizeof(global));
    memset(active_group, 0, sizeof(active_group));
    memset(party, 0, sizeof(party));
    memset(events, 0, sizeof(events));
    memset(timeline, 0, sizeof(timeline));
    memset(portraits, 0, sizeof(portraits));
    memset(checksums, 0, sizeof(checksums));

    for (i = 0; i < 127; ++i) {
        write_u16_le(header + (size_t)i * 2u,
                     (uint16_t)(0x4321u + (uint16_t)(i * 17u)));
    }
    write_u16_le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u,
                 0x2468u);
    header[298u] = 1u;
    header[299u] = SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC;
    write_u16_le(header + 304u, 1u);
    write_u32_le(header + 306u,
                 spec->game_id ? spec->game_id : 0x50433334u);

    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = (uint16_t)(0x2000u + (uint16_t)(i * 0x101u));
    }

    write_u32_le(global + 0u, spec->game_time ? spec->game_time : 123456u);
    write_u16_le(global + 10u, (uint16_t)champion_count);
    write_u16_le(global + 12u, (uint16_t)spec->map_x);
    write_u16_le(global + 14u, (uint16_t)spec->map_y);
    write_u16_le(global + 16u, (uint16_t)spec->direction);
    write_u16_le(global + 18u, (uint16_t)spec->map_index);
    write_u16_le(global + 20u, (uint16_t)spec->active_champion_index);
    write_u16_le(global + DM1_PC34_GLOBAL_EVENT_COUNT_OFFSET,
                 (uint16_t)event_count);
    write_u16_le(global + DM1_PC34_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET,
                 (uint16_t)event_count);
    write_u16_le(global + DM1_PC34_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET,
                 (uint16_t)event_maximum_count);
    write_u16_le(global + DM1_PC34_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET,
                 (uint16_t)current_active_group_count);
    write_u16_le(global + DM1_PC34_GLOBAL_MAXIMUM_ACTIVE_GROUP_COUNT_OFFSET,
                 (uint16_t)maximum_active_group_count);

    write_original_pc34_fixture_active_group(active_group + 0u, 0x1001u,
                                             0x5a, 0xc3, 21, 22);
    write_original_pc34_fixture_active_group(
        active_group + DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT,
        0x1002u, 0x6b, 0xd4, 23, 24);
    write_original_pc34_fixture_active_group(
        active_group + 2u * DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT,
        0x1003u, 0x7c, 0xe5, 25, 26);

    if (champion_count > 0) {
        write_original_pc34_fixture_champion(
            party + 0u, "TIGGY", "APPRENTICE", spec->direction,
            44, 55, 66, 77, 8, 9, 1500, -32, 0x0021u, 0x1555u);
    }
    if (champion_count > 1) {
        write_original_pc34_fixture_champion(
            party + DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT,
            "WUUF", "BIKA", (spec->direction + 1) & 3,
            88, 99, 111, 122, 33, 44, 1200, 1100, 0x0002u, 0x1666u);
    }
    if (champion_count > 2) {
        write_original_pc34_fixture_champion(
            party + 2u * DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT,
            "HALK", "BARBARIAN", (spec->direction + 2) & 3,
            101, 202, 303, 404, 55, 66, 900, 800, 0x0010u, 0x1777u);
    }

    write_original_pc34_fixture_event(events + 0u,
                                      DM1_MAP_TIME_MAKE(2, 123500u),
                                      DM1_EVENT_MOVE_GROUP_AUDIBLE, 7,
                                      11, 12, 3, DM1_EFFECT_SET);
    write_original_pc34_fixture_event(
        events + DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT,
        DM1_MAP_TIME_MAKE(2, 123470u),
        DM1_EVENT_DOOR, 4, 21, 22, 1, DM1_EFFECT_TOGGLE);
    write_original_pc34_fixture_event(
        events + 2u * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT,
        DM1_MAP_TIME_MAKE(1, 123490u),
        DM1_EVENT_ENABLE_CHAMPION_ACTION, 2, 0, 0, 0, 0);
    write_u16_le(timeline + 0u, 1u);
    write_u16_le(timeline + 2u, 2u);
    write_u16_le(timeline + 4u, 0u);
    write_u16_le(timeline + 6u, 3u);
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        memset(portraits + (size_t)i * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT,
               0x30 + i, CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
    }

    rc = write_original_part(out_bytes + cursor, out_capacity - cursor,
                             global, sizeof(global),
                             keys[SAVEGAME_PC34_PART_GLOBAL_DATA],
                             &checksums[SAVEGAME_PC34_PART_GLOBAL_DATA]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;
    rc = write_original_part(
        out_bytes + cursor, out_capacity - cursor,
        active_group,
        (size_t)maximum_active_group_count *
            DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT,
        keys[SAVEGAME_PC34_PART_ACTIVE_GROUP],
        &checksums[SAVEGAME_PC34_PART_ACTIVE_GROUP]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;
    rc = write_original_part(out_bytes + cursor, out_capacity - cursor,
                             party, sizeof(party),
                             keys[SAVEGAME_PC34_PART_PARTY],
                             &checksums[SAVEGAME_PC34_PART_PARTY]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;
    rc = write_original_part(
        out_bytes + cursor, out_capacity - cursor,
        events,
        (size_t)event_maximum_count * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT,
        keys[SAVEGAME_PC34_PART_EVENTS],
        &checksums[SAVEGAME_PC34_PART_EVENTS]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;
    rc = write_original_part(out_bytes + cursor, out_capacity - cursor,
                             timeline, (size_t)event_maximum_count * 2u,
                             keys[SAVEGAME_PC34_PART_TIMELINE],
                             &checksums[SAVEGAME_PC34_PART_TIMELINE]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;

    if (out_capacity - cursor < sizeof(portraits)) {
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    }
    memcpy(out_bytes + cursor, portraits, sizeof(portraits));
    cursor += sizeof(portraits);

    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        write_u16_le(header + 310u + (size_t)i * 2u, keys[i]);
        write_u16_le(header + 342u + (size_t)i * 2u, checksums[i]);
    }
    write_u16_le(header + 374u, SAVEGAME_PC34_PLATFORM_PC);
    write_u16_le(header + 376u, SAVEGAME_PC34_DUNGEON_ID_DM);
    {
        uint16_t second_sum = original_pc34_header_second_half_plain_sum(header);
        uint16_t first_before_last =
            original_pc34_header_first_half_checksum(header);
        uint16_t last =
            (uint16_t)(read_u16_le(header + 254u) ^
                       first_before_last ^
                       second_sum);
        write_u16_le(header + 254u, last);
    }
    (void)f0417_xor_checksum_bytes(
        header + 256u, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS,
        read_u16_le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(out_bytes, header, sizeof(header));
    *out_size = cursor;
    return SAVEGAME_PC34_OK;
}

int dm1_v1_original_save_pc34_handoff_bytes(
    const uint8_t *bytes,
    size_t size,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    DM1OriginalSaveClassifyResult classify;
    DM1OriginalSavePC34HandoffReport staged_report;
    struct SaveGame_Compat staged_state;
    struct PartyState_Compat staged_party;
    struct TimelineQueue_Compat staged_timeline;
    int rc;

    memset(&staged_report, 0, sizeof(staged_report));
    staged_report.importer_result = SAVEGAME_PC34_ERROR_INTERNAL;
    if (out_report) {
        memset(out_report, 0, sizeof(*out_report));
        out_report->importer_result = SAVEGAME_PC34_ERROR_INTERNAL;
    }
    if (!bytes || !out_state) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (size > (size_t)SAVEGAME_PC34_MAX_FILE_SIZE ||
        size > (size_t)((int)0x7fffffff)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    }

    if (!dm1_v1_original_save_classify_bytes(bytes, size, &classify)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (out_report) {
        out_report->classify = classify;
    }
    if (classify.shape != DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 ||
        !classify.pc34_importer_candidate) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    }

    /* ReDMCSB LOADSAVE.C F0435 lines ~2665-2722 reads the original
     * DM_SAVE_HEADER layout from DEFS.H lines 468-480
     * (Noise[149], then metadata at byte 298) before reading
     * GLOBAL_DATA and save parts through READWRIT.C F0419 lines
     * ~232-242. LOADSAVE.C F0435 lines ~2746-2754 reads
     * sizeof(ACTIVE_GROUP) * GLOBAL_DATA.MaximumActiveGroupCount
     * before LOADSAVE.C F0435 lines ~2766-2777 copies the
     * PC34 PARTY block as four 319-byte CHAMPION_EXCLUDING_PORTRAIT
     * records plus 128 PARTY_INFO bytes, then lines ~2810-2816 read
     * four external 32x29 portrait bitmap payloads. F0796 handles Firestaff's
     * PC34 native-export layout;
     * this path handles the real original header envelope classified
     * above. */
    staged_report.classify = classify;
    staged_state = *out_state;
    if (out_state->party) {
        staged_party = *out_state->party;
        staged_state.party = &staged_party;
    }
    if (out_state->timeline) {
        staged_timeline = *out_state->timeline;
        staged_state.timeline = &staged_timeline;
    }
    rc = import_original_pc34_global_data(bytes, size, &staged_state,
                                          &staged_report);
    staged_report.importer_result = rc;
    if (out_report) *out_report = staged_report;
    if (rc != SAVEGAME_PC34_OK) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    out_state->header = staged_state.header;
    if (out_state->party) *out_state->party = staged_party;
    if (out_state->timeline) *out_state->timeline = staged_timeline;

    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_file(
    const char *path,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    uint8_t *bytes;
    size_t size;
    int result;

    if (!path || !out_state) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    result = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, size, out_state, out_report);
    free(bytes);
    return result;
}

int dm1_v1_original_save_pc34_handoff_apply_active_groups(
    DM1OriginalSavePC34HandoffReport *report,
    struct GameWorld_Compat *world)
{
    int i;
    int import_count;

    if (!report || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (report->reported_active_group_count < 0 ||
        report->reported_active_group_count >
            DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    import_count = report->original_current_active_group_count;
    if (import_count < 0) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    if (import_count > report->reported_active_group_count) {
        import_count = report->reported_active_group_count;
    }
    if (import_count > GAMEWORLD_CREATURE_AI_CAPACITY) {
        import_count = GAMEWORLD_CREATURE_AI_CAPACITY;
    }

    memset(world->creatureAI, 0, sizeof(world->creatureAI));
    world->creatureAICount = import_count;
    report->active_group_runtime_imported_count = import_count;
    report->active_group_runtime_truncated_count =
        report->original_current_active_group_count - import_count;
    report->active_group_runtime_resolved_count = 0;
    report->active_group_runtime_unresolved_count = 0;
    for (i = 0; i < import_count; ++i) {
        const DM1OriginalSavePC34ActiveGroupRecord *src =
            &report->active_groups[i];
        struct CreatureAIState_Compat *dst = &world->creatureAI[i];
        const struct DungeonGroup_Compat *resolved_group = 0;
        unsigned int thing = (unsigned int)(uint16_t)src->group_thing_index;
        int thing_type = (int)((thing >> DM1_PC34_THING_TYPE_SHIFT) &
                               DM1_PC34_THING_TYPE_MASK);
        int thing_index = (int)(thing & DM1_PC34_THING_INDEX_MASK);
        int first_direction = src->directions & 0x03;

        /* ReDMCSB DEFS.H ACTIVE_GROUP carries GroupThingIndex, packed
         * directions/cells, target/prior/home coordinates, and aspect
         * bytes. DEFS.H THING encodes Bits 13-10 as type and Bits 9-0
         * as index. If Firestaff has decoded DungeonThings_Compat, a type
         * 4 GROUP thing resolves directly to things->groups[index]. */
        if (thing_type == DM1_PC34_THING_TYPE_GROUP &&
            world->things &&
            thing_index >= 0 &&
            thing_index < world->things->groupCount &&
            world->things->groups) {
            resolved_group = &world->things->groups[thing_index];
        }

        dst->stateKind = AI_STATE_WANDER;
        dst->creatureType = resolved_group ? resolved_group->creatureType : -1;
        dst->groupMapIndex = world->partyMapIndex;
        dst->groupMapX = src->prior_map_x;
        dst->groupMapY = src->prior_map_y;
        dst->groupCells = src->cells;
        dst->groupDirection = first_direction;
        dst->targetChampionIndex = -1;
        dst->lastSeenPartyMapX = src->target_map_x;
        dst->lastSeenPartyMapY = src->target_map_y;
        dst->lastSeenPartyTick = src->last_move_time;
        dst->fearCounter = src->delay_fleeing_from_target;
        dst->turnCounter = 0;
        dst->attackCooldownTicks = 0;
        dst->movementCooldownTicks = 0;
        dst->aggressionScore = 0;
        dst->rngCallCount = 0;
        dst->reserved0 = resolved_group ? thing_index : src->group_thing_index;
        memcpy(dst->aspect, src->aspect, sizeof(dst->aspect));
        if (resolved_group) {
            report->active_group_runtime_resolved_count++;
        } else {
            report->active_group_runtime_unresolved_count++;
        }
    }

    return import_count;
}

int dm1_v1_original_save_pc34_handoff_apply_event_queue(
    const DM1OriginalSavePC34HandoffReport *report,
    struct DM1_EventQueue_V1 *queue)
{
    struct DM1_EventQueue_V1 candidate_queue;
    int i;

    if (!report || !queue) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (report->original_event_count < 0 ||
        report->original_event_count > DM1_EVENT_MAX_COUNT ||
        report->decoded_event_count < 0 ||
        report->decoded_event_count > DM1_EVENT_MAX_COUNT ||
        report->decoded_timeline_index_count < 0 ||
        report->decoded_timeline_index_count > DM1_EVENT_MAX_COUNT ||
        report->original_event_count > report->decoded_event_count ||
        report->original_event_count > report->decoded_timeline_index_count ||
        report->original_first_unused_event_index < 0 ||
        report->original_first_unused_event_index > DM1_EVENT_MAX_COUNT) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    /* ReDMCSB LOADSAVE.C F0435:2780-2800 reads EVENTS and TIMELINE before
     * F0651 publishes optimized timeline management. Validate and build the
     * complete Firestaff queue first, so a malformed timeline cannot replace
     * the runtime's previously valid queue with a partial load. */
    for (i = 0; i < report->original_event_count; ++i) {
        if (report->timeline_indices[i] >=
            (uint16_t)report->decoded_event_count) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
    }

    if (!dm1v1_event_queue_init(&candidate_queue, report->original_game_time)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    candidate_queue.eventCount = report->original_event_count;
    candidate_queue.firstUnusedIndex = report->original_first_unused_event_index;
    candidate_queue.maxEvents = DM1_EVENT_MAX_COUNT;
    for (i = 0; i < report->decoded_event_count; ++i) {
        candidate_queue.events[i] = report->events[i];
    }
    for (i = 0; i < report->original_event_count; ++i) {
        candidate_queue.timeline[i] = report->timeline_indices[i];
    }
    *queue = candidate_queue;
    return queue->eventCount;
}

int dm1_v1_original_save_pc34_handoff_load_world_from_file(
    const char *path,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    uint8_t *bytes;
    size_t size;
    int result;

    if (!path || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    result = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, size, world, event_queue, out_report);
    free(bytes);
    return result;
}

static int load_world_from_bytes_uncommitted(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *report)
{
    struct SaveGame_Compat state;
    int result;

    if (!bytes || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    memset(&state, 0, sizeof(state));
    state.party = &world->party;
    state.timeline = &world->timeline;

    result = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, size, &state, report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    world->gameTick = report->original_game_time;
    world->partyMapIndex = world->party.mapIndex;
    world->newPartyMapIndex = world->party.mapIndex;
    world->timeline.nowTick = report->original_game_time;

    result = dm1_v1_original_save_pc34_handoff_apply_active_groups(
        report, world);
    if (result < 0) {
        return result;
    }
    result = materialize_original_pc34_dungeon_tail(
        bytes, size, world, report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }
    result = dm1_v1_original_save_pc34_handoff_apply_active_groups(
        report, world);
    if (result < 0) {
        return result;
    }
    result = materialize_original_pc34_timeline(report, world, &world->timeline);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }
    if (event_queue) {
        result = dm1_v1_original_save_pc34_handoff_apply_event_queue(
            report, event_queue);
        if (result < 0) {
            return result;
        }
    }

    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    struct GameWorld_Compat candidate_world;
    struct DM1_EventQueue_V1 candidate_queue;
    DM1OriginalSavePC34HandoffReport candidate_report;
    int reuses_existing_dungeon;
    int result;

    if (!bytes || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    /* ReDMCSB LOADSAVE.C F0435 restores GLOBAL_DATA, PARTY, EVENT,
     * TIMELINE, portraits, and finally the dungeon before the resumed
     * runtime is exposed. Keep that sequence private to a candidate world:
     * a rejected final tail/checksum must not leak a partially loaded party
     * or event heap into the running HoC session. */
    memset(&candidate_world, 0, sizeof(candidate_world));
    /* The byte-loader also serves callers that have already materialized a
     * start dungeon. It may resolve ACTIVE_GROUP records against that data,
     * but never owns it while validation is still in progress. */
    candidate_world.dungeon = world->dungeon;
    candidate_world.things = world->things;
    candidate_world.ownsDungeon = 0;
    memset(&candidate_queue, 0, sizeof(candidate_queue));
    memset(&candidate_report, 0, sizeof(candidate_report));
    result = load_world_from_bytes_uncommitted(
        bytes, size, &candidate_world,
        event_queue ? &candidate_queue : NULL, &candidate_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&candidate_world);
        return result;
    }

    reuses_existing_dungeon =
        candidate_world.dungeon == world->dungeon &&
        candidate_world.things == world->things;
    if (reuses_existing_dungeon) {
        candidate_world.ownsDungeon = world->ownsDungeon;
        world->dungeon = NULL;
        world->things = NULL;
        world->ownsDungeon = 0;
    }
    F0883_WORLD_Free_Compat(world);
    *world = candidate_world;
    memset(&candidate_world, 0, sizeof(candidate_world));
    if (event_queue) {
        *event_queue = candidate_queue;
    }
    if (out_report) {
        *out_report = candidate_report;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
    const char *path,
    const struct GameWorld_Compat *start_world,
    struct GameWorld_Compat *out_world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    struct GameWorld_Compat candidate_world;
    struct DM1_EventQueue_V1 candidate_queue;
    DM1OriginalSavePC34HandoffReport candidate_report;
    char backup_path[DM1_ORIGINAL_SAVE_PATH_MAX];
    const char *load_path = path;
    int resumed_from_backup = 0;
    int result;

    if (!path || !out_world || out_world == start_world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    memset(&candidate_world, 0, sizeof(candidate_world));
    memset(&candidate_queue, 0, sizeof(candidate_queue));
    memset(&candidate_report, 0, sizeof(candidate_report));
    /* ReDMCSB LOADSAVE.C F0435 lines 2560-2583 only opens the backup when
     * the primary cannot be opened. A malformed primary must fail closed;
     * it must never be replaced by an older backup. */
    if (!dm1_original_save_file_opens_for_read(path)) {
        if (!dm1_original_save_backup_path(path, backup_path) ||
            !dm1_original_save_file_opens_for_read(backup_path)) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
        }
        load_path = backup_path;
        resumed_from_backup = 1;
    }
    /* ReDMCSB LOADSAVE.C F0435 restores EVENTS/TIMELINE before exposing
     * the resumed game, but a tail-less PC34 save still owns its original
     * start DUNGEON.DAT backing.  Bind that backing before materializing
     * source event unions: C29-C41 resolve B.Location through the real SFT
     * chain and C48/C49 resolve their C14 records. */
    if (start_world) {
        candidate_world.dungeon = start_world->dungeon;
        candidate_world.things = start_world->things;
        candidate_world.ownsDungeon = 0;
    }
    result = dm1_v1_original_save_pc34_handoff_load_world_from_file(
        load_path, &candidate_world, event_queue ? &candidate_queue : NULL,
        &candidate_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&candidate_world);
        return result;
    }

    /* ReDMCSB LOADSAVE.C F0435 loads the dungeon after PARTY/EVENT/TIMELINE.
     * A PC34 stream without that optional tail is therefore resumed against
     * the already materialized DM1 start dungeon, never a host-made HoC
     * substitute. */
    if (!candidate_world.dungeon || !candidate_world.things) {
        F0883_WORLD_Free_Compat(&candidate_world);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    candidate_report.resumed_from_backup = resumed_from_backup;
    /* The save source is promoted only after every part, optional dungeon
     * tail, and borrowed-start-dungeon invariant has passed. If promotion
     * fails, leave both the destination world and the backup untouched. */
    if (resumed_from_backup) {
        if (rename(backup_path, path) != 0) {
            F0883_WORLD_Free_Compat(&candidate_world);
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
        }
        candidate_report.backup_promoted_to_primary = 1;
    }
    F0883_WORLD_Free_Compat(out_world);
    *out_world = candidate_world;
    memset(&candidate_world, 0, sizeof(candidate_world));
    if (event_queue) {
        *event_queue = candidate_queue;
    }
    if (out_report) {
        *out_report = candidate_report;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_adopt_runtime_world(
    struct GameWorld_Compat *runtime_world,
    struct GameWorld_Compat *loaded_world)
{
    int reuses_start_dungeon;

    if (!runtime_world || !loaded_world || runtime_world == loaded_world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    /* Native quicksaves serialize runtime data but retain the start dungeon
     * outside that blob.  Give them the same DM1-owned materialized backing
     * used by a tail-less original save before transferring ownership. */
    if (!loaded_world->dungeon && runtime_world->dungeon) {
        loaded_world->dungeon = runtime_world->dungeon;
        loaded_world->things = runtime_world->things;
        loaded_world->ownsDungeon = 0;
    }
    reuses_start_dungeon = loaded_world->dungeon == runtime_world->dungeon &&
                         loaded_world->things == runtime_world->things;
    if (reuses_start_dungeon) {
        loaded_world->ownsDungeon = runtime_world->ownsDungeon;
        runtime_world->dungeon = NULL;
        runtime_world->things = NULL;
        runtime_world->ownsDungeon = 0;
    }
    F0883_WORLD_Free_Compat(runtime_world);
    *runtime_world = *loaded_world;
    memset(loaded_world, 0, sizeof(*loaded_world));
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

void dm1_v1_original_save_pc34_handoff_normalize_hoc_resume_state(
    const struct GameWorld_Compat *world,
    DM1OriginalSavePC34HoCResumeState *state)
{
    if (!state) {
        return;
    }
    if (!world || !state->candidate_panel_active ||
        state->candidate_mirror_ordinal < 0 ||
        state->candidate_party_index < 0 ||
        state->candidate_party_index >= world->party.championCount ||
        state->candidate_party_index >= CHAMPION_MAX_PARTY ||
        state->candidate_party_index != world->party.championCount - 1 ||
        !world->party.champions[state->candidate_party_index].present) {
        state->candidate_mirror_ordinal = -1;
        state->candidate_party_index = -1;
        state->candidate_panel_active = 0;
        /* ReDMCSB LOADSAVE.C F0435 restores PARTY and dungeon state, not
         * Firestaff's transient C040 panel. A rejected/stale candidate must
         * therefore also close its dependent inventory surface; otherwise a
         * quicksave sidecar can paint a false HoC panel after the restored
         * world has no live mirror candidate. */
        state->inventory_panel_active = 0;
        return;
    }
    /* ReDMCSB REVIVE.C F0280 appends the candidate and F0282:744 reads
     * PartyChampionCount - 1. A sidecar cannot reopen C040 over an older
     * live party slot, even if that slot happens to contain a champion. */
    state->candidate_panel_active = 1;
    state->inventory_panel_active = 1;
}

int dm1_v1_original_save_pc34_roundtrip_world_bytes(
    const uint8_t *bytes,
    size_t size,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34HandoffReport *import_report,
    DM1OriginalSavePC34HandoffReport *verify_report)
{
    struct GameWorld_Compat world;
    struct DM1_EventQueue_V1 event_queue;
    struct SaveGame_Compat verify_state;
    int written = 0;
    int result;

    if (!bytes || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    *out_size = 0u;
    if (size > (size_t)((int)0x7fffffff) ||
        out_capacity > (size_t)((int)0x7fffffff)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    memset(&world, 0, sizeof(world));
    memset(&event_queue, 0, sizeof(event_queue));

    result = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, size, &world, &event_queue, import_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&world);
        return result;
    }

    /* ReDMCSB LOADSAVE.C F0435 materializes GLOBAL_DATA, ACTIVE_GROUP,
     * PARTY, EVENT/TIMELINE, and optional dungeon bytes before runtime
     * resumes; F0433 then writes those same save parts back through
     * READWRIT.C F0420. This helper pins that import-export-import
     * contract for Firestaff's bounded DM1 world handoff. */
    result = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, game_id, out_bytes, (int)out_capacity, &written);
    F0883_WORLD_Free_Compat(&world);
    if (result != SAVEGAME_PC34_OK) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    memset(&verify_state, 0, sizeof(verify_state));
    result = dm1_v1_original_save_pc34_handoff_bytes(
        out_bytes, (size_t)written, &verify_state, verify_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    *out_size = (size_t)written;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static void fill_roundtrip_core_report(
    const DM1OriginalSavePC34HandoffReport *source_report,
    const DM1OriginalSavePC34HandoffReport *export_report,
    const struct GameWorld_Compat *reloaded_world,
    const struct DM1_EventQueue_V1 *reloaded_queue,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    if (!out_report) {
        return;
    }
    memset(out_report, 0, sizeof(*out_report));
    if (source_report) {
        out_report->source_champion_count =
            source_report->imported_champion_count;
        out_report->source_map_index = source_report->imported_map_index;
        out_report->source_map_x = source_report->imported_map_x;
        out_report->source_map_y = source_report->imported_map_y;
        out_report->source_direction = source_report->imported_direction;
        out_report->source_game_time = source_report->original_game_time;
        out_report->source_event_count = source_report->original_event_count;
        out_report->source_active_group_count =
            source_report->original_current_active_group_count;
    }
    if (export_report) {
        out_report->exported_champion_count =
            export_report->imported_champion_count;
        out_report->exported_map_index = export_report->imported_map_index;
        out_report->exported_map_x = export_report->imported_map_x;
        out_report->exported_map_y = export_report->imported_map_y;
        out_report->exported_direction = export_report->imported_direction;
        out_report->exported_game_time = export_report->original_game_time;
        out_report->exported_event_count = export_report->original_event_count;
        out_report->exported_active_group_count =
            export_report->original_current_active_group_count;
    }
    if (reloaded_world) {
        out_report->reloaded_champion_count =
            reloaded_world->party.championCount;
        out_report->reloaded_map_index = reloaded_world->party.mapIndex;
        out_report->reloaded_map_x = reloaded_world->party.mapX;
        out_report->reloaded_map_y = reloaded_world->party.mapY;
        out_report->reloaded_direction = reloaded_world->party.direction;
        out_report->reloaded_game_time = reloaded_world->gameTick;
        out_report->reloaded_active_group_count =
            reloaded_world->creatureAICount;
    }
    if (reloaded_queue) {
        out_report->reloaded_event_count = reloaded_queue->eventCount;
    }

    out_report->core_state_matches =
        out_report->source_champion_count ==
            out_report->exported_champion_count &&
        out_report->source_champion_count ==
            out_report->reloaded_champion_count &&
        out_report->source_map_index == out_report->exported_map_index &&
        out_report->source_map_index == out_report->reloaded_map_index &&
        out_report->source_map_x == out_report->exported_map_x &&
        out_report->source_map_x == out_report->reloaded_map_x &&
        out_report->source_map_y == out_report->exported_map_y &&
        out_report->source_map_y == out_report->reloaded_map_y &&
        out_report->source_direction == out_report->exported_direction &&
        out_report->source_direction == out_report->reloaded_direction &&
        out_report->source_game_time == out_report->exported_game_time &&
        out_report->source_game_time == out_report->reloaded_game_time &&
        out_report->source_event_count == out_report->exported_event_count &&
        out_report->source_event_count == out_report->reloaded_event_count &&
        out_report->source_active_group_count ==
            out_report->exported_active_group_count &&
        out_report->source_active_group_count ==
            out_report->reloaded_active_group_count;
}

int dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
    const uint8_t *bytes,
    size_t size,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    DM1OriginalSavePC34HandoffReport import_report;
    DM1OriginalSavePC34HandoffReport export_report;
    struct GameWorld_Compat reloaded_world;
    struct DM1_EventQueue_V1 reloaded_queue;
    int result;

    if (out_report) {
        memset(out_report, 0, sizeof(*out_report));
    }
    if (!bytes || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    result = dm1_v1_original_save_pc34_roundtrip_world_bytes(
        bytes, size, game_id, out_bytes, out_capacity, out_size,
        &import_report, &export_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    memset(&reloaded_world, 0, sizeof(reloaded_world));
    memset(&reloaded_queue, 0, sizeof(reloaded_queue));
    result = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        out_bytes, *out_size, &reloaded_world, &reloaded_queue,
        &export_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&reloaded_world);
        return result;
    }

    fill_roundtrip_core_report(&import_report, &export_report,
                               &reloaded_world, &reloaded_queue,
                               out_report);
    F0883_WORLD_Free_Compat(&reloaded_world);
    if (out_report && !out_report->core_state_matches) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_roundtrip_world_file(
    const char *path,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34HandoffReport *import_report,
    DM1OriginalSavePC34HandoffReport *verify_report)
{
    uint8_t *bytes;
    size_t size;
    int result;

    if (!path || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (!dm1_original_save_file_opens_for_read(path)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    /* Product-facing file round trips accept only external PC34 envelopes.
     * F0433 verification output carries Firestaff's manifest and must never
     * re-enter the original-save corpus/product import route as evidence. */
    if (!dm1_original_save_corpus_external_pc34_file(path, NULL)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    }
    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    /* ReDMCSB LOADSAVE.C F0435 reads original PC34 bytes from disk before
     * materializing runtime GLOBAL_DATA/ACTIVE_GROUP/PARTY/EVENT state.
     * This corpus-facing wrapper keeps Firestaff's file edge on the same
     * bounded import-export verification path as the byte helper. */
    result = dm1_v1_original_save_pc34_roundtrip_world_bytes(
        bytes, size, game_id, out_bytes, out_capacity, out_size,
        import_report, verify_report);
    free(bytes);
    return result;
}

int dm1_v1_original_save_pc34_roundtrip_world_reload_file(
    const char *path,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    uint8_t *bytes;
    size_t size;
    int result;

    if (!path || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (!dm1_original_save_file_opens_for_read(path)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    if (!dm1_original_save_corpus_external_pc34_file(path, NULL)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    }
    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    result = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
        bytes, size, game_id, out_bytes, out_capacity, out_size, out_report);
    free(bytes);
    return result;
}

int dm1_v1_original_save_pc34_roundtrip_corpus_root(
    const char *root,
    DM1OriginalSavePC34CorpusRoundtripReport *out_report)
{
    DM1OriginalSaveCorpusManifest corpus;
    DM1OriginalSavePC34CorpusRoundtripReport report;
    uint8_t *exported_bytes;
    int i;

    if (!root || !root[0] || !out_report) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    memset(&report, 0, sizeof(report));
    report.first_failure_result = DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
    memset(&corpus, 0, sizeof(corpus));
    if (!dm1_v1_original_save_classify_corpus_root(root, &corpus)) {
        *out_report = report;
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    report.scan_succeeded = 1;
    report.scanned_file_count = corpus.scanned_file_count;
    /* The classifier intentionally keeps arbitrary files out of the PC34
     * importer. Preserve that decision in the corpus receipt instead of
     * silently losing evidence of a truncated/non-PC34 neighbour. */
    report.rejected_count = corpus.scanned_file_count -
        corpus.pc34_loader_part_envelope_count;
    report.roundtrip_hash = 2166136261u;
    exported_bytes = (uint8_t *)malloc(SAVEGAME_PC34_MAX_FILE_SIZE);
    if (!exported_bytes) {
        *out_report = report;
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    /* ReDMCSB LOADSAVE.C F0435 reads only a valid header plus the five
     * checksum-protected parts. F0433 subsequently serializes the live
     * state. Keep corpus proof in memory so validation cannot create or
     * replace a sibling DMSAVE.DAT. */
    for (i = 0; i < corpus.present_count &&
                i < (int)DM1_ORIGINAL_SAVE_CORPUS_CANDIDATE_CAP; ++i) {
        DM1OriginalSavePC34RoundtripReport roundtrip;
        size_t exported_size = 0u;
        int result;
        int firestaff_manifest = 0;

        if (!corpus.results[i].pc34_loader_part_envelope_candidate) {
            continue;
        }
        if (!dm1_original_save_corpus_external_pc34_file(
                corpus.paths[i], &firestaff_manifest)) {
            if (firestaff_manifest) {
                ++report.firestaff_manifest_rejected_count;
            } else {
                ++report.nonoriginal_envelope_rejected_count;
            }
            if (report.first_failure_result ==
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                report.first_failure_result =
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        ++report.pc34_candidate_count;
        if (!report.first_pc34_path[0]) {
            snprintf(report.first_pc34_path, sizeof(report.first_pc34_path),
                     "%s", corpus.paths[i]);
        }
        memset(&roundtrip, 0, sizeof(roundtrip));
        ++report.roundtrip_attempted_count;
        result = dm1_v1_original_save_pc34_roundtrip_world_reload_file(
            corpus.paths[i],
            corpus.results[i].game_id,
            exported_bytes,
            SAVEGAME_PC34_MAX_FILE_SIZE,
            &exported_size,
            &roundtrip);
        if (result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
            roundtrip.core_state_matches) {
            size_t byte_index;
            ++report.roundtrip_succeeded_count;
            ++report.core_state_match_count;
            for (byte_index = 0u; byte_index < exported_size; ++byte_index) {
                report.roundtrip_hash ^= exported_bytes[byte_index];
                report.roundtrip_hash *= 16777619u;
            }
            if (!report.first_roundtrip_path[0]) {
                snprintf(report.first_roundtrip_path,
                         sizeof(report.first_roundtrip_path), "%s",
                         corpus.paths[i]);
            }
        } else {
            ++report.roundtrip_failed_count;
            if (result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                result = DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            if (report.first_failure_result ==
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                report.first_failure_result = result;
            }
        }
    }
    free(exported_bytes);
    if (report.roundtrip_succeeded_count == 0) {
        report.roundtrip_hash = 0u;
    }
    *out_report = report;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

const char *dm1_v1_original_save_pc34_handoff_result_name(int result)
{
    switch (result) {
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK: return "OK";
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT: return "argument";
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34: return "not-pc34";
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT: return "import";
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE: return "file";
    default: return "unknown";
    }
}

const char *dm1_v1_original_save_pc34_handoff_source_evidence(void)
{
    return "ReDMCSB DEFS.H:468-480 DM_SAVE_HEADER and 503-521 constants; "
           "SAVEHEAD.C F0429/F0430 header obfuscation; "
           "LOADSAVE.C F0435 PC save load path; "
           "LOADSAVE.C F0435 dungeon tail read path and F0421 checksum; "
           "DEFS.H:394-418 THING type/index layout; "
           "DEFS.H:574-587 ACTIVE_GROUP; "
           "DEFS.H:880-920 EVENT and timeline save arrays; "
           "DEFS.H:661-705 CHAMPION_EXCLUDING_PORTRAIT; "
           "READWRIT.C F0417/F0418/F0419 save-part checksum and obfuscation";
}
