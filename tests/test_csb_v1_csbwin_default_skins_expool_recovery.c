/* CSBWin data.cpp SKIN_CACHE default record raw recovery regression. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
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

static uint32_t read_le32(const uint8_t *bytes, size_t offset)
{
    return (uint32_t)bytes[offset] | ((uint32_t)bytes[offset + 1u] << 8) |
        ((uint32_t)bytes[offset + 2u] << 16) |
        ((uint32_t)bytes[offset + 3u] << 24);
}

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static void add_record(uint8_t *tail, uint32_t block, uint32_t record_id,
                       const uint8_t *payload, uint16_t node_words)
{
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    const uint32_t base = block * 64u;
    const uint32_t node = base + 1u;
    const uint32_t prior = read_le32(tail, (size_t)bucket * 4u);
    put_le16(tail, (size_t)base * 4u + 2u, node_words);
    put_le32(tail, (size_t)node * 4u, prior);
    put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
    if (payload && node_words > 2u) {
        memcpy(tail + (size_t)(node + 2u) * 4u, payload,
               (size_t)(node_words - 2u) * 4u);
    }
    put_le32(tail, (size_t)bucket * 4u, node);
}

static void prepare(CSB_V1_RuntimeProfile *profile,
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
    enum { record_id = (4u << 24) | 0x00800000u,
           block_bytes = CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES };
    const uint8_t source[8] = { 3u, 7u, 1u, 4u, 9u, 2u, 6u, 5u };
    uint8_t tail[2u * block_bytes];
    uint8_t recovered[64] = { 0u };
    CSB_V1_RuntimeProfile profile;
    size_t size = 0u;
    uint32_t receipt;

    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, record_id, source, 4u);
    prepare(&profile, tail, sizeof(tail));
    receipt = profile.csbwin_appended_tail_fnv1a;
    check(csb_v1_runtime_recover_csbwin_default_skins(
              &profile, recovered, sizeof(recovered), &size) == 1 && size == sizeof(source) &&
              memcmp(recovered, source, sizeof(source)) == 0 &&
              profile.csbwin_appended_tail_fnv1a == receipt,
          "CSBWin recovers only the raw default-skin payload without zero-fill");
    check(csb_v1_runtime_recover_csbwin_default_skins(
              &profile, recovered, sizeof(source) - 1u, &size) == 0 && size == 0u,
          "CSBWin rejects an insufficient default-skin output buffer");
    profile.csbwin_appended_tail[0] ^= 1u;
    check(csb_v1_runtime_recover_csbwin_default_skins(
              &profile, recovered, sizeof(recovered), &size) == 0,
          "CSBWin rejects a drifted default-skin receipt");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, record_id, source, 4u);
    add_record(tail, 1u, record_id, source, 4u);
    prepare(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_recover_csbwin_default_skins(
              &profile, recovered, sizeof(recovered), &size) == 0,
          "CSBWin rejects duplicate default-skin records");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    prepare(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_recover_csbwin_default_skins(
              &profile, recovered, sizeof(recovered), &size) == 0,
          "CSBWin rejects an absent default-skin record without defaulting");
    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
