#ifndef FIRESTAFF_CSB_V1_SKIN_CACHE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_SKIN_CACHE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_SKIN_CACHE_EDT_SKINS 4u
#define CSB_V1_SKIN_CACHE_MAX_LEVELS 64
#define CSB_V1_SKIN_CACHE_COLUMNS 16
#define CSB_V1_SKIN_CACHE_COLUMN_BYTES 64

typedef int (*CSB_V1_SkinCacheRecordLookup)(
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size,
    void *user);

typedef struct {
    int loaded_level;
    int column_sizes[CSB_V1_SKIN_CACHE_COLUMNS];
    uint8_t columns[CSB_V1_SKIN_CACHE_COLUMNS][CSB_V1_SKIN_CACHE_COLUMN_BYTES];
    int default_skins_loaded;
    uint8_t default_skins[CSB_V1_SKIN_CACHE_MAX_LEVELS];
} CSB_V1_SkinCache;

void csb_v1_skin_cache_init(CSB_V1_SkinCache *cache);

void csb_v1_skin_cache_cleanup(CSB_V1_SkinCache *cache);

uint32_t csb_v1_skin_cache_column_record_id(int level, int x);

uint32_t csb_v1_skin_cache_default_record_id(void);

uint8_t csb_v1_skin_cache_get_skin(
    CSB_V1_SkinCache *cache,
    CSB_V1_SkinCacheRecordLookup lookup,
    void *lookup_user,
    int level,
    int level_width,
    int level_height,
    int x,
    int y);

uint8_t csb_v1_skin_cache_get_default_skin(
    CSB_V1_SkinCache *cache,
    CSB_V1_SkinCacheRecordLookup lookup,
    void *lookup_user,
    int level);

int csb_v1_skin_cache_set_skin(
    CSB_V1_SkinCache *cache,
    int level,
    int level_width,
    int level_height,
    int x,
    int y,
    uint8_t skin_num);

const char *csb_v1_skin_cache_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
