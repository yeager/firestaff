/* CSBWin CSBCode.cpp SubstituteGlobalText EXPOOL recovery regression. */

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
                       uint32_t record_id, const uint8_t *payload,
                       size_t payload_size)
{
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    const uint32_t block_base = block_index * 64u;
    const uint32_t node = block_base + 1u;
    const uint32_t node_words = 2u + (uint32_t)(payload_size / 4u);
    const uint32_t prior = (uint32_t)tail[(size_t)bucket * 4u] |
        ((uint32_t)tail[(size_t)bucket * 4u + 1u] << 8) |
        ((uint32_t)tail[(size_t)bucket * 4u + 2u] << 16) |
        ((uint32_t)tail[(size_t)bucket * 4u + 3u] << 24);

    put_le16(tail, (size_t)block_base * 4u + 2u, (uint16_t)node_words);
    put_le32(tail, (size_t)node * 4u, prior);
    put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
    memcpy(tail + (size_t)(node + 2u) * 4u, payload, payload_size);
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
    enum { RECORD_ID = (10u << 24) | 0x42u };
    static const uint8_t text[] = { 'T', 'H', 'E', ' ', 'R', 'E', 'A', 'L', 'M', '\0', 0u, 0u };
    uint8_t overlong[104];
    uint8_t tail[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    CSB_V1_RuntimeProfile profile;
    char recovered[100];

    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, RECORD_ID, text, sizeof(text));
    prepare_profile(&profile, tail, sizeof(tail));

    check(csb_v1_runtime_recover_csbwin_global_text(
              &profile, 0x42u, recovered, sizeof(recovered)) == 1 &&
              strcmp(recovered, "THE REALM") == 0,
          "CSBWin recovers the original GlobalText value");
    check(csb_v1_runtime_recover_csbwin_global_text(
              &profile, 0x42u, recovered, 5u) == 0 && recovered[0] == '\0',
          "CSBWin rejects insufficient destination capacity without fallback");

    profile.csbwin_appended_tail[0] ^= 1u;
    check(csb_v1_runtime_recover_csbwin_global_text(
              &profile, 0x42u, recovered, sizeof(recovered)) == 0 &&
              recovered[0] == '\0',
          "CSBWin rejects a drifted GlobalText EXPOOL receipt");
    profile.csbwin_appended_tail[0] ^= 1u;
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    add_record(profile.csbwin_appended_tail, 1u, RECORD_ID, text, sizeof(text));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    check(csb_v1_runtime_recover_csbwin_global_text(
              &profile, 0x42u, recovered, sizeof(recovered)) == 0 &&
              recovered[0] == '\0',
          "CSBWin rejects duplicate live GlobalText owners");
    csb_v1_runtime_cleanup(&profile);

    memset(overlong, 'A', sizeof(overlong));
    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, RECORD_ID, overlong, sizeof(overlong));
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_recover_csbwin_global_text(
              &profile, 0x42u, recovered, sizeof(recovered)) == 0 &&
              recovered[0] == '\0',
          "CSBWin rejects overlong or unterminated GlobalText source bytes");

    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
