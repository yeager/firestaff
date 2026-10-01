#include "theron_v1_track02_skill_rank_raw.h"

#include <string.h>

/* Authentic raw records at US UD 0x1C9B6B and JP Rev. 1 UD 0x89333.
 * The two hash-verified editions have the same bytes. Prefix values 0x60-0x65
 * are retained as source bytes only; their font/icon rendering is unbound. */
static const char *const g_skill_level_source_records[
    THERON_TRACK02_SKILL_LEVEL_COUNT] = {
    "NEOPHYTE", "NOVICE", "APPRENTICE", "JOURNEYMAN", "CRAFTSMAN",
    "ARTISAN", "ADEPT", "EXPERT", "\x60 MASTER", "\x61 MASTER",
    "\x62 MASTER", "\x63 MASTER", "\x64 MASTER", "\x65 MASTER",
    "ARCHMASTER",
};

static int skill_level_source_record(unsigned int index,
                                     const uint8_t **out_bytes,
                                     size_t *out_size) {
    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (index >= THERON_TRACK02_SKILL_LEVEL_COUNT || !out_bytes || !out_size)
        return 0;
    *out_bytes = (const uint8_t *)(const void *)
        g_skill_level_source_records[index];
    *out_size = strlen(g_skill_level_source_records[index]);
    return 1;
}

int theron_v1_track02_us_skill_level_source_record(
    unsigned int index, const uint8_t **out_bytes, size_t *out_size) {
    return skill_level_source_record(index, out_bytes, out_size);
}

int theron_v1_track02_jp_skill_level_source_record(
    unsigned int index, const uint8_t **out_bytes, size_t *out_size) {
    return skill_level_source_record(index, out_bytes, out_size);
}
