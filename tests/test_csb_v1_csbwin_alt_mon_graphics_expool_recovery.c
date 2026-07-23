/* CSBWin Code51a4.cpp AltGraphicMapping EXPOOL recovery regression. */

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

static void add_record(uint8_t *tail, uint32_t block_index,
                       uint32_t record_id, const uint32_t values[4])
{
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    const uint32_t block_base = block_index * 64u;
    const uint32_t node = block_base + 1u;
    const uint32_t prior = (uint32_t)tail[(size_t)bucket * 4u] |
        ((uint32_t)tail[(size_t)bucket * 4u + 1u] << 8) |
        ((uint32_t)tail[(size_t)bucket * 4u + 2u] << 16) |
        ((uint32_t)tail[(size_t)bucket * 4u + 3u] << 24);
    unsigned int i;

    put_le16(tail, (size_t)block_base * 4u + 2u, 6u);
    put_le32(tail, (size_t)node * 4u, prior);
    put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
    for (i = 0u; i < 4u; ++i) {
        put_le32(tail, (size_t)(node + 2u + i) * 4u, values[i]);
    }
    put_le32(tail, (size_t)bucket * 4u, node);
}

static void prepare_profile(CSB_V1_RuntimeProfile *profile,
                            const uint8_t *tail, size_t size)
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
    enum { RECORD_ID = (5u << 24) | (8u << 16) | (4u << 8) | 12u };
    const uint32_t mapping[4] = { 101u, 202u, 0xfffffffeu, 404u };
    uint8_t tail[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    CSB_V1_RuntimeProfile profile;
    int32_t graphic_id = -1;

    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, RECORD_ID, mapping);
    prepare_profile(&profile, tail, sizeof(tail));

    check(csb_v1_runtime_recover_csbwin_alt_mon_graphic(
              &profile, 4u, 12u, 1u, &graphic_id) == 1 &&
              graphic_id == 202,
          "CSBWin recovers the selected original alternate graphic mapping");
    check(csb_v1_runtime_recover_csbwin_alt_mon_graphic(
              &profile, 4u, 12u, 2u, &graphic_id) == 1 &&
              graphic_id == -2,
          "CSBWin preserves source negative alternate-graphic sentinels");
    check(csb_v1_runtime_recover_csbwin_alt_mon_graphic(
              &profile, 4u, 12u, 4u, &graphic_id) == 0 && graphic_id == -1,
          "CSBWin rejects an out-of-range alternate graphic without fallback");

    profile.csbwin_appended_tail[0] ^= 1u;
    check(csb_v1_runtime_recover_csbwin_alt_mon_graphic(
              &profile, 4u, 12u, 1u, &graphic_id) == 0 && graphic_id == -1,
          "CSBWin rejects a drifted alternate-graphic EXPOOL receipt");
    profile.csbwin_appended_tail[0] ^= 1u;
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    add_record(profile.csbwin_appended_tail, 1u, RECORD_ID, mapping);
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    check(csb_v1_runtime_recover_csbwin_alt_mon_graphic(
              &profile, 4u, 12u, 1u, &graphic_id) == 0 && graphic_id == -1,
          "CSBWin rejects duplicate live alternate-graphic owners");

    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
