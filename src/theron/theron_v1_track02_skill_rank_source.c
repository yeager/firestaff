#include "theron_v1_track02_skill_rank_source.h"
#include "theron_v1_track02_champion_strings.h"

#include <string.h>

/* Authentic JP Rev. 1 Track 02 source records at UD 0x89333. The media
 * regression binds each record to the hash-verified JP source bytes
 * (see parity-evidence/pass215_theron_track02_binary_analysis.md). These are
 * raw source bytes, not a claim about rank-icon rendering. */
static const char *const g_jp_skill_level_source_records[
    THERON_TRACK02_SKILL_LEVEL_COUNT] = {
    "NEOPHYTE", "NOVICE", "APPRENTICE", "JOURNEYMAN", "CRAFTSMAN",
    "ARTISAN", "ADEPT", "EXPERT", "\x60 MASTER", "\x61 MASTER",
    "\x62 MASTER", "\x63 MASTER", "\x64 MASTER", "\x65 MASTER",
    "ARCHMASTER",
};

int theron_v1_track02_jp_skill_level_source_record(
    unsigned int index, const uint8_t **out_bytes, size_t *out_size)
{
    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (index >= THERON_TRACK02_SKILL_LEVEL_COUNT || !out_bytes || !out_size)
        return 0;
    *out_bytes = (const uint8_t *)(const void *)
        g_jp_skill_level_source_records[index];
    *out_size = strlen(g_jp_skill_level_source_records[index]);
    return 1;
}
