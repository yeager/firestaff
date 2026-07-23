/* CSBWin Statistics.cpp EDBT_MonsterNames recovery regression.
 * Source: Statistics.cpp::GetMonsterName and data.cpp::EXPOOL::Locate. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void put_le16(uint8_t *bytes, size_t offset, uint16_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
}

static void put_le32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
    bytes[offset + 2u] = (uint8_t)(value >> 16);
    bytes[offset + 3u] = (uint8_t)(value >> 24);
}

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static void add_record(uint8_t *tail,
                       uint32_t block_index,
                       uint32_t record_id,
                       const char *text)
{
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    const uint32_t block_base = block_index * 64u;
    const uint32_t node = block_base + 1u;
    const size_t payload_size = strlen(text) + 1u;
    const uint16_t node_words = (uint16_t)(
        2u + (payload_size + sizeof(uint32_t) - 1u) / sizeof(uint32_t));
    const uint32_t prior = (uint32_t)tail[(size_t)bucket * 4u] |
        ((uint32_t)tail[(size_t)bucket * 4u + 1u] << 8) |
        ((uint32_t)tail[(size_t)bucket * 4u + 2u] << 16) |
        ((uint32_t)tail[(size_t)bucket * 4u + 3u] << 24);

    put_le16(tail, (size_t)block_base * 4u + 2u, node_words);
    put_le32(tail, (size_t)node * 4u, prior);
    put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
    memcpy(tail + (size_t)(node + 2u) * 4u, text, payload_size);
    put_le32(tail, (size_t)bucket * 4u, node);
}

static void prepare_profile(CSB_V1_RuntimeProfile *profile,
                            const uint8_t *tail,
                            size_t size)
{
    csb_v1_runtime_init(profile, NULL);
    memcpy(profile->csbwin_appended_tail, tail, size);
    profile->csbwin_appended_tail_valid = 1;
    profile->csbwin_appended_tail_size = size;
    profile->csbwin_appended_tail_preserved_size = size;
    profile->csbwin_appended_tail_fnv1a = fnv1a32(tail, size);
}

int main(void)
{
    enum { RECORD_ID = (5u << 24) | (6u << 16) | 3u };
    uint8_t tail[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    CSB_V1_RuntimeProfile profile;
    char name[32];

    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, RECORD_ID, "Flying Eye|Evil Eye");
    prepare_profile(&profile, tail, sizeof(tail));

    check(csb_v1_runtime_recover_csbwin_monster_name(
              &profile, 3u, 0, name, sizeof(name)) == 1 &&
              strcmp(name, "Flying Eye") == 0,
          "CSBWin recovers the first source monster-name variant");
    check(csb_v1_runtime_recover_csbwin_monster_name(
              &profile, 3u, 1, name, sizeof(name)) == 1 &&
              strcmp(name, "Evil Eye") == 0,
          "CSBWin recovers the selected pipe-separated name variant");
    check(csb_v1_runtime_recover_csbwin_monster_name(
              &profile, 3u, 2, name, sizeof(name)) == 0 && name[0] == '\0',
          "CSBWin rejects a missing source name variant without fallback");

    profile.csbwin_appended_tail[0] ^= 1u;
    check(csb_v1_runtime_recover_csbwin_monster_name(
              &profile, 3u, 0, name, sizeof(name)) == 0 && name[0] == '\0',
          "CSBWin rejects a drifted EXPOOL monster-name receipt");
    profile.csbwin_appended_tail[0] ^= 1u;
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    add_record(profile.csbwin_appended_tail, 1u, RECORD_ID, "Duplicate");
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    check(csb_v1_runtime_recover_csbwin_monster_name(
              &profile, 3u, 0, name, sizeof(name)) == 0 && name[0] == '\0',
          "CSBWin rejects duplicate live EXPOOL name owners");

    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
