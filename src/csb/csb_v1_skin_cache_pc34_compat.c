#include "csb_v1_skin_cache_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CSBWin CSB.h:181-194 SKIN_CACHE; data.cpp:2053-2077 Load "
    "uses (EDT_Skins<<24)+(level<<4)+(x/2), 16 cached columns, "
    "64-byte cap; data.cpp:2080-2105 GetSkin bounds-checks level/x/y "
    "and indexes (2*y)+(x&1); data.cpp:2107-2125 GetDefaultSkin reads "
    "(EDT_Skins<<24)+0x800000 and caches 64 defaults; "
    "DSA.cpp:3118-3134 exposes GETSKIN/SETSKIN to DSA opcodes.";

void csb_v1_skin_cache_init(CSB_V1_SkinCache *cache)
{
    int i;

    if (!cache) return;
    memset(cache, 0, sizeof(*cache));
    cache->loaded_level = -1;
    for (i = 0; i < CSB_V1_SKIN_CACHE_COLUMNS; ++i) {
        cache->column_sizes[i] = -1;
    }
}

void csb_v1_skin_cache_cleanup(CSB_V1_SkinCache *cache)
{
    csb_v1_skin_cache_init(cache);
}

uint32_t csb_v1_skin_cache_column_record_id(int level, int x)
{
    if (level < 0 || x < 0) return 0u;
    return (CSB_V1_SKIN_CACHE_EDT_SKINS << 24) |
           ((uint32_t)level << 4) |
           (uint32_t)(x / 2);
}

uint32_t csb_v1_skin_cache_default_record_id(void)
{
    return (CSB_V1_SKIN_CACHE_EDT_SKINS << 24) | 0x800000u;
}

static void reset_level_columns(CSB_V1_SkinCache *cache, int level)
{
    int i;

    cache->loaded_level = level;
    for (i = 0; i < CSB_V1_SKIN_CACHE_COLUMNS; ++i) {
        cache->column_sizes[i] = -1;
        memset(cache->columns[i], 0, sizeof(cache->columns[i]));
    }
}

static int load_column(
    CSB_V1_SkinCache *cache,
    CSB_V1_SkinCacheRecordLookup lookup,
    void *lookup_user,
    int level,
    int x)
{
    const uint8_t *record = NULL;
    size_t record_size = 0u;
    size_t copy_size;
    int column;

    if (!cache || level < 0 || x < 0) return -1;
    column = x / 2;
    if (column < 0 || column >= CSB_V1_SKIN_CACHE_COLUMNS) return -1;
    if (cache->loaded_level != level) {
        reset_level_columns(cache, level);
    }
    if (cache->column_sizes[column] != -1) {
        return column;
    }

    if (!lookup ||
        !lookup(csb_v1_skin_cache_column_record_id(level, x),
                &record,
                &record_size,
                lookup_user) ||
        !record ||
        record_size == 0u) {
        cache->column_sizes[column] = 0;
        return column;
    }

    copy_size = record_size;
    if (copy_size > CSB_V1_SKIN_CACHE_COLUMN_BYTES) {
        copy_size = CSB_V1_SKIN_CACHE_COLUMN_BYTES;
    }
    memcpy(cache->columns[column], record, copy_size);
    cache->column_sizes[column] = (int)copy_size;
    return column;
}

uint8_t csb_v1_skin_cache_get_skin(
    CSB_V1_SkinCache *cache,
    CSB_V1_SkinCacheRecordLookup lookup,
    void *lookup_user,
    int level,
    int level_width,
    int level_height,
    int x,
    int y)
{
    int column;
    int index;

    if (!cache || level < 0 || x < 0 || y < 0 ||
        x >= level_width || y >= level_height) {
        return 0u;
    }
    column = load_column(cache, lookup, lookup_user, level, x);
    if (column < 0 || cache->column_sizes[column] <= 0) {
        return 0u;
    }
    index = 2 * y + (x & 1);
    if (index < 0 || index >= cache->column_sizes[column]) {
        return 0u;
    }
    return cache->columns[column][index];
}

uint8_t csb_v1_skin_cache_get_default_skin(
    CSB_V1_SkinCache *cache,
    CSB_V1_SkinCacheRecordLookup lookup,
    void *lookup_user,
    int level)
{
    const uint8_t *record = NULL;
    size_t record_size = 0u;
    size_t copy_size;

    if (!cache || level < 0 || level >= CSB_V1_SKIN_CACHE_MAX_LEVELS) {
        return 0u;
    }
    if (!cache->default_skins_loaded) {
        memset(cache->default_skins, 0, sizeof(cache->default_skins));
        if (lookup &&
            lookup(csb_v1_skin_cache_default_record_id(),
                   &record,
                   &record_size,
                   lookup_user) &&
            record &&
            record_size > 0u) {
            copy_size = record_size;
            if (copy_size > CSB_V1_SKIN_CACHE_MAX_LEVELS) {
                copy_size = CSB_V1_SKIN_CACHE_MAX_LEVELS;
            }
            memcpy(cache->default_skins, record, copy_size);
        }
        cache->default_skins_loaded = 1;
    }
    return cache->default_skins[level];
}

int csb_v1_skin_cache_set_skin(
    CSB_V1_SkinCache *cache,
    int level,
    int level_width,
    int level_height,
    int x,
    int y,
    uint8_t skin_num)
{
    int column;
    int index;

    if (!cache || level < 0 || x < 0 || y < 0 ||
        x >= level_width || y >= level_height) {
        return 0;
    }
    if (cache->loaded_level != level) {
        reset_level_columns(cache, level);
    }
    column = x / 2;
    if (column < 0 || column >= CSB_V1_SKIN_CACHE_COLUMNS) return 0;
    index = 2 * y + (x & 1);
    if (index < 0 || index >= CSB_V1_SKIN_CACHE_COLUMN_BYTES) return 0;
    if (cache->column_sizes[column] < 0) {
        cache->column_sizes[column] = 0;
    }
    if (cache->column_sizes[column] <= index) {
        cache->column_sizes[column] = index + 1;
    }
    cache->columns[column][index] = skin_num;
    return 1;
}

const char *csb_v1_skin_cache_source_evidence(void)
{
    return s_source_evidence;
}
