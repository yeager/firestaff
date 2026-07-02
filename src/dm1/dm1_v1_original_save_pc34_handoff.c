#include "dm1_v1_original_save_pc34_handoff.h"

#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT 319u
#define DM1_PC34_ORIGINAL_PARTY_INFO_BYTE_COUNT 128u
#define DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT \
    ((DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY) + \
     DM1_PC34_ORIGINAL_PARTY_INFO_BYTE_COUNT)
#define DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT 16u
#define DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT 10u

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

static int16_t read_i16_le(const uint8_t *p)
{
    return (int16_t)read_u16_le(p);
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

    if (part_size < DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT) {
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
    case DM1_EVENT_DOOR_ANIMATION:
        return TIMELINE_EVENT_DOOR_ANIMATE;
    case DM1_EVENT_DOOR_DESTRUCTION:
        return TIMELINE_EVENT_DOOR_DESTRUCTION;
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

static int materialize_original_pc34_timeline(
    const DM1OriginalSavePC34HandoffReport *report,
    struct TimelineQueue_Compat *timeline)
{
    int i;

    if (!report || !timeline) {
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
            continue;
        }
        memset(&ev, 0, sizeof(ev));
        ev.kind = kind;
        ev.fireAtTick = src->map_time & 0x00ffffffu;
        ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
        ev.aux0 = src->type;
        ev.aux4 = src->priority;
        if (original_pc34_event_type_is_status_timeout(src->type)) {
            ev.aux1 = (int)read_u16_le(&src->b_mapX);
            ev.cell = src->c_cell;
        } else if (src->type == DM1_EVENT_REMOVE_FLUXCAGE) {
            uint16_t thing = read_u16_le(&src->c_cell);
            ev.mapX = src->b_mapX;
            ev.mapY = src->b_mapY;
            ev.cell = EXPLOSION_CELL_CENTERED;
            ev.aux0 =
                (((thing >> DM1_PC34_THING_TYPE_SHIFT) &
                  DM1_PC34_THING_TYPE_MASK) == THING_TYPE_EXPLOSION)
                    ? (int)(thing & DM1_PC34_THING_INDEX_MASK)
                    : (int)thing;
            ev.aux1 = C050_EXPLOSION_FLUXCAGE;
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

static size_t import_original_pc34_external_portraits(
    const uint8_t *bytes,
    size_t size,
    size_t cursor,
    struct SaveGame_Compat *out_state)
{
    int slot;
    int count;
    if (!bytes) {
        return cursor;
    }
    count = (out_state && out_state->party) ? out_state->party->championCount : 0;
    if (count < 0) count = 0;
    if (count > CHAMPION_MAX_PARTY) count = CHAMPION_MAX_PARTY;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        if (cursor + CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT > size) {
            return cursor;
        }
        if (out_state && out_state->party && slot < count) {
            memcpy(out_state->party->champions[slot].portraitBitmap,
                   bytes + cursor,
                   CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
            out_state->party->champions[slot].portraitBitmapValid = 1;
        }
        cursor += CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
    }
    return cursor;
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
    off = DUNGEON_HEADER_SIZE;
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
        off += (size_t)raw_map_bytes;
        expected_checksum = read_u16_le(tail + off);
        actual_checksum = original_pc34_byte_checksum(tail, off);
        out_report->dungeon_tail_present = 1;
        out_report->dungeon_tail_byte_count = (uint32_t)tail_size;
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

    cursor = import_original_pc34_external_portraits(bytes, size, cursor,
                                                     out_state);
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

int dm1_v1_original_save_pc34_handoff_bytes(
    const uint8_t *bytes,
    size_t size,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    DM1OriginalSaveClassifyResult classify;
    int rc;

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
    rc = import_original_pc34_global_data(bytes, size, out_state, out_report);
    if (out_report) {
        out_report->importer_result = rc;
    }
    if (rc != SAVEGAME_PC34_OK) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_file(
    const char *path,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    FILE *file;
    long file_size;
    uint8_t *bytes;
    size_t read_count;
    int result;

    if (!path || !out_state) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

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

    result = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, (size_t)file_size, out_state, out_report);
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
    int i;

    if (!report || !queue) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (report->original_event_count < 0 ||
        report->original_event_count > DM1_EVENT_MAX_COUNT ||
        report->original_event_count > report->decoded_event_count ||
        report->original_event_count > report->decoded_timeline_index_count ||
        report->original_first_unused_event_index < 0 ||
        report->original_first_unused_event_index > DM1_EVENT_MAX_COUNT) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    if (!dm1v1_event_queue_init(queue, report->original_game_time)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    queue->eventCount = report->original_event_count;
    queue->firstUnusedIndex = report->original_first_unused_event_index;
    queue->maxEvents = DM1_EVENT_MAX_COUNT;
    for (i = 0; i < report->decoded_event_count; ++i) {
        queue->events[i] = report->events[i];
    }
    for (i = 0; i < report->original_event_count; ++i) {
        if (report->timeline_indices[i] >=
            (uint16_t)report->decoded_event_count) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        queue->timeline[i] = report->timeline_indices[i];
    }
    return queue->eventCount;
}

int dm1_v1_original_save_pc34_handoff_load_world_from_file(
    const char *path,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    FILE *file;
    long file_size;
    uint8_t *bytes;
    size_t read_count;
    int result;

    if (!path || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

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

    result = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, (size_t)file_size, world, event_queue, out_report);
    free(bytes);
    return result;
}

int dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    struct SaveGame_Compat state;
    DM1OriginalSavePC34HandoffReport local_report;
    DM1OriginalSavePC34HandoffReport *report =
        out_report ? out_report : &local_report;
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
    result = materialize_original_pc34_timeline(report, &world->timeline);
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
