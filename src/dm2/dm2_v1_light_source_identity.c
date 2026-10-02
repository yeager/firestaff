#include "dm2_v1_light_source_identity.h"

#include <stddef.h>
#include <string.h>

#define HASH_START UINT64_C(14695981039346656037)
#define HASH_PRIME UINT64_C(1099511628211)

uint32_t dm2_v1_light_source_identity_fold(uint64_t identity)
{
    uint32_t hash = 2166136261u;
    for (unsigned i = 0u; i < 8u; ++i) {
        hash = (hash ^ (uint8_t)identity) * 16777619u;
        identity >>= 8u;
    }
    return hash;
}

static uint64_t hash_bytes(uint64_t h, const void *data, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    for (size_t i = 0; i < size; ++i) h = (h ^ bytes[i]) * HASH_PRIME;
    return h;
}

static uint64_t hash_word(uint64_t h, uint64_t value)
{
    for (unsigned i = 0; i < 8u; ++i) {
        h = (h ^ (uint8_t)value) * HASH_PRIME;
        value >>= 8u;
    }
    return h;
}

int dm2_v1_light_source_identity(
    const DM2_V1_LightSourceIdentityInputs *in,
    DM2_V1_LightSourceIdentity *out)
{
    uint64_t h;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!in || !in->map || !in->map->valid || !in->dungeon ||
        !in->dungeon->raw_data || in->dungeon->raw_size <= 0 ||
        !in->dungeon->record_graph_complete || !in->records ||
        !in->records->valid || !in->records->record_graph_complete ||
        !in->gdat || !in->gdat->loaded || !in->gdat->data ||
        !in->gdat->data_size || !in->gdat->entries ||
        !in->gdat->entry_count ||
        (in->gdat->raw_data_count &&
         (!in->gdat->raw_offsets || !in->gdat->raw_sizes)) ||
        !in->game || !in->party || !in->weather || !in->weather_chain ||
        !in->weather_light_valid || !in->savegames1 ||
        !in->savegames1_size || !in->champion_inventory_objects ||
        !in->champion_inventory_count ||
        in->graphics_style < 0 || in->graphics_style > 255 ||
        in->map->level < 0 || in->map->level >= in->dungeon->level_count ||
        in->game->current_level != in->map->level)
        return 0;

    h = hash_word(HASH_START, 0x4d4150u);
    h = hash_word(h, (uint32_t)in->map->level);
    h = hash_word(h, in->map->difficulty);
    h = hash_word(h, in->map->dynamic_light);
    h = hash_word(h, (uint32_t)in->graphics_style);
    out->map = hash_word(h, in->map->descriptor_hash);

    h = hash_word(HASH_START, 0x44554e47454f4eu);
    h = hash_word(h, (uint32_t)in->dungeon->level_count);
    h = hash_word(h, (uint32_t)in->dungeon->raw_size);
    h = hash_word(h, (uint32_t)in->dungeon->square_bytes);
    h = hash_word(h, (uint32_t)in->dungeon->raw_map_data_base);
    h = hash_word(h, (uint32_t)in->dungeon->square_first_thing_base);
    h = hash_word(h, (uint32_t)in->dungeon->square_first_thing_count);
    for (int map = 0; map < in->dungeon->level_count; ++map) {
        h = hash_word(h, (uint32_t)in->dungeon->level_widths[map]);
        h = hash_word(h, (uint32_t)in->dungeon->level_heights[map]);
        h = hash_word(h, (uint32_t)in->dungeon->level_offsets[map]);
    }
    out->dungeon = hash_bytes(h, in->dungeon->raw_data,
                              (size_t)in->dungeon->raw_size);

    h = hash_word(HASH_START, 0x5245434f524453u);
    h = hash_word(h, (uint32_t)in->records->words_big_endian);
    for (unsigned i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        const DM2_V1_RecordPool *pool = &in->records->pools[i];
        size_t bytes;
        if (pool->record_count < 0 || pool->record_size < 0 ||
            pool->extension_count < 0 ||
            (pool->record_count && (!pool->bytes || !pool->record_size)) ||
            (pool->extension_count &&
             (!pool->extension_bytes || !pool->record_size)) ||
            (size_t)pool->record_count > SIZE_MAX /
                (size_t)(pool->record_size ? pool->record_size : 1) ||
            (size_t)pool->extension_count > SIZE_MAX /
                (size_t)(pool->record_size ? pool->record_size : 1))
            return 0;
        h = hash_word(h, i);
        h = hash_word(h, (uint32_t)pool->record_count);
        h = hash_word(h, (uint32_t)pool->record_size);
        h = hash_word(h, (uint32_t)pool->source_base);
        h = hash_word(h, (uint32_t)pool->extension_count);
        h = hash_word(h, (uint32_t)pool->extension_base);
        bytes = (size_t)pool->record_count * (size_t)pool->record_size;
        if (bytes) h = hash_bytes(h, pool->bytes, bytes);
        bytes = (size_t)pool->extension_count * (size_t)pool->record_size;
        if (bytes) h = hash_bytes(h, pool->extension_bytes, bytes);
    }
    out->records = h;

    h = hash_word(HASH_START, 0x47444154u);
    h = hash_word(h, in->gdat->data_size);
    h = hash_word(h, in->gdat->entry_count);
    h = hash_word(h, in->gdat->raw_data_count);
    h = hash_word(h, in->gdat->big_endian);
    for (unsigned i = 0; i < in->gdat->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &in->gdat->entries[i];
        h = hash_word(h, entry->cls1);
        h = hash_word(h, entry->cls2);
        h = hash_word(h, entry->cls3);
        h = hash_word(h, entry->cls4);
        h = hash_word(h, entry->cls5);
        h = hash_word(h, entry->cls6);
        h = hash_word(h, entry->data_index);
    }
    for (unsigned i = 0; i < in->gdat->raw_data_count; ++i) {
        h = hash_word(h, in->gdat->raw_offsets[i]);
        h = hash_word(h, in->gdat->raw_sizes[i]);
    }
    out->gdat = hash_bytes(h, in->gdat->data, in->gdat->data_size);

    h = hash_word(HASH_START, 0x5041525459u);
    h = hash_word(h, (uint32_t)in->game->current_level);
    h = hash_word(h, (uint32_t)in->game->party_x);
    h = hash_word(h, (uint32_t)in->game->party_y);
    h = hash_word(h, (uint32_t)in->game->party_dir);
    h = hash_word(h, (uint16_t)in->party->heros_in_party);
    h = hash_word(h, (uint16_t)in->party->absdir);
    h = hash_word(h, (uint16_t)in->party->curacthero);
    h = hash_word(h, (uint16_t)in->party->curactmode);
    h = hash_bytes(h, in->party->hero, sizeof(in->party->hero));
    h = hash_bytes(h, in->party->hand_container,
                   sizeof(in->party->hand_container));
    h = hash_bytes(h, in->party->handitems, sizeof(in->party->handitems));
    h = hash_bytes(h, in->savegames1, in->savegames1_size);
    h = hash_word(h, in->leader_hand_object);
    h = hash_word(h, (uint16_t)in->source_light_level);
    for (uint32_t i = 0; i < in->champion_inventory_count; ++i)
        h = hash_word(h, in->champion_inventory_objects[i]);
    out->party_light = h;

    h = hash_word(HASH_START, 0x57454154484552u);
    h = hash_word(h, (uint32_t)in->weather->weather);
    h = hash_word(h, (uint32_t)in->weather->time_of_day);
    h = hash_bytes(h, &in->weather->time_fraction,
                   sizeof(in->weather->time_fraction));
    h = hash_word(h, in->weather->weather_seed);
    h = hash_word(h, (uint32_t)in->weather->weather_intensity);
    h = hash_word(h, (uint32_t)in->weather_chain_started);
    h = hash_word(h, (uint16_t)in->weather_chain->zone_index);
    h = hash_word(h, (uint8_t)in->weather_chain->weather_allowed);
    h = hash_word(h, (uint8_t)in->weather_chain->retry);
    h = hash_word(h, (uint8_t)in->weather_chain->pattern_row);
    h = hash_word(h, (uint8_t)in->weather_chain->step);
    h = hash_word(h, (uint16_t)in->weather_chain->intensity);
    h = hash_word(h, (uint16_t)in->weather_chain->previous_intensity);
    h = hash_word(h, (uint32_t)in->weather_chain->day_tick);
    h = hash_word(h, (uint32_t)in->weather_chain->day_offset);
    h = hash_word(h, (uint16_t)in->weather_chain->cloud_timer);
    h = hash_word(h, (uint16_t)in->weather_chain->day_word);
    h = hash_word(h, (uint8_t)in->weather_chain->rain_counter);
    h = hash_word(h, (uint8_t)in->weather_chain->cloud_state);
    h = hash_word(h, (uint8_t)in->weather_chain->storm_active);
    h = hash_word(h, (uint8_t)in->weather_chain->lightning_flag);
    h = hash_word(h, (uint8_t)in->weather_chain->wind_dir);
    h = hash_word(h, (uint32_t)in->weather_chain->storm_request);
    h = hash_word(h, (uint8_t)in->weather_chain->clouds_enabled);
    h = hash_word(h, (uint8_t)in->weather_chain->rain_enabled);
    h = hash_word(h, (uint8_t)in->weather_chain->lightning_enabled);
    h = hash_word(h, (uint8_t)in->weather_chain->light_pending);
    h = hash_word(h, (uint8_t)in->weather_chain->thunder_latch);
    h = hash_word(h, in->gdat_scene_rain);
    h = hash_word(h, in->gdat_scene_highest_light_level);
    h = hash_word(h, in->gdat_ambient_darkness);
    out->weather = hash_word(h, in->gdat_scene_ambient_light);

    h = hash_word(HASH_START, out->map);
    h = hash_word(h, out->dungeon);
    h = hash_word(h, out->records);
    h = hash_word(h, out->gdat);
    h = hash_word(h, out->party_light);
    out->combined = hash_word(h, out->weather);
    out->valid = 1;
    return 1;
}
