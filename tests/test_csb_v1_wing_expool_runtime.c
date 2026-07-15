/* CSBWin Character.cpp CHARDESC::GetFromWings EXPOOL regression.
 * The fixture is the original eight-record EDT_Character shape, not a
 * substitute character layout. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    character_records = 8,
    character_record_bytes = 100,
    character_node_words = 27
};

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

static uint32_t read_le32(const uint8_t *bytes, size_t offset)
{
    return (uint32_t)bytes[offset] |
        ((uint32_t)bytes[offset + 1u] << 8) |
        ((uint32_t)bytes[offset + 2u] << 16) |
        ((uint32_t)bytes[offset + 3u] << 24);
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

static void build_wing_tail(uint8_t *tail, size_t size, uint16_t fingerprint,
                            uint32_t talents)
{
    uint32_t i;

    memset(tail, 0, size);
    for (i = 0u; i < character_records; ++i) {
        const uint32_t record_id = (8u << 24) | (i << 16) | fingerprint;
        const uint32_t bucket = 32u +
            ((record_id * 0xbb40e62du) >> 27);
        const uint32_t block_base = i * 64u;
        const uint32_t node = block_base + 1u;
        const uint32_t prior = read_le32(tail, (size_t)bucket * 4u);
        uint8_t *payload = tail + (size_t)(node + 2u) * 4u;

        put_le16(tail, (size_t)block_base * 4u + 2u,
                 character_node_words);
        put_le32(tail, (size_t)node * 4u, prior);
        put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
        memset(payload, (int)i, character_record_bytes);
        put_le32(tail, (size_t)bucket * 4u, node);
    }
    /* CHARDESC layout offsets from Character.cpp: talents=276,
     * fingerPrint=280. Both live in the third 100-byte record. */
    put_le32(tail, (size_t)(2u * 64u + 3u) * 4u + 76u, talents);
    put_le16(tail, (size_t)(2u * 64u + 3u) * 4u + 80u, fingerprint);
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
    uint8_t tail[character_records * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    CSB_V1_RuntimeProfile profile;
    uint32_t talents = 0u;

    build_wing_tail(tail, sizeof(tail), 0x1234u, 0x89abcdefu);
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_read_csbwin_wing_talents(
              &profile, 0x1234u, &talents) == 1 &&
              talents == 0x89abcdefu,
          "CSBWin restores a complete eight-record EDT_Character wing");
    check(csb_v1_runtime_has_csbwin_wing_character(&profile, 0x1234u) == 1,
          "CSBWin WHEREISCHAR finds its source first EDT_Character record");
    check(csb_v1_runtime_set_csbwin_wing_talents(
              &profile, 0x1234u, 0x10203040u) == 1 &&
              csb_v1_runtime_read_csbwin_wing_talents(
                  &profile, 0x1234u, &talents) == 1 &&
              talents == 0x10203040u,
          "CSBWin SaveToWings rewrites all existing character records");
    check(csb_v1_runtime_read_csbwin_wing_talents(
              &profile, 0x9999u, &talents) == 0 && talents == 0u,
          "CSBWin reports an authenticated absent wing as source zero");

    prepare_profile(&profile, tail,
                    sizeof(tail) - CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES);
    check(csb_v1_runtime_has_csbwin_wing_character(&profile, 0x1234u) == 1,
          "CSBWin WHEREISCHAR keeps the source first-record lookup");
    check(csb_v1_runtime_read_csbwin_wing_talents(
              &profile, 0x1234u, &talents) == -1 && talents == 0u,
          "CSBWin rejects a partial EDT_Character wing bundle");

    prepare_profile(&profile, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a ^= 1u;
    check(csb_v1_runtime_read_csbwin_wing_talents(
              &profile, 0x1234u, &talents) == -1 && talents == 0u,
          "CSBWin rejects an altered wing EXPOOL receipt");

    return failures == 0 ? 0 : 1;
}
