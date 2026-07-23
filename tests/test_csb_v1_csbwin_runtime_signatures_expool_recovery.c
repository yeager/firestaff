/* CSBWin SaveGame.cpp RuntimeFileSignatures recovery regression. */

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

static void add_record(uint8_t *tail, uint32_t block_index,
                       uint32_t record_id, uint32_t value,
                       uint16_t node_words)
{
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    const uint32_t block_base = block_index * 64u;
    const uint32_t node = block_base + 1u;
    const uint32_t prior = read_le32(tail, (size_t)bucket * 4u);

    put_le16(tail, (size_t)block_base * 4u + 2u, node_words);
    put_le32(tail, (size_t)node * 4u, prior);
    put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
    put_le32(tail, (size_t)(node + 2u) * 4u, value);
    put_le32(tail, (size_t)bucket * 4u, node);
}

static void add_signature_bundle(uint8_t *tail, uint32_t blocks,
                                 uint32_t record_count)
{
    const uint32_t base = (5u << 24) | (2u << 16);
    uint32_t index;

    (void)blocks;
    for (index = 0u; index < record_count; ++index) {
        add_record(tail, index, base | index,
                   0x10203040u + index, 3u);
    }
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
    enum { block_bytes = CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES };
    uint8_t tail[4u * block_bytes];
    CSB_V1_RuntimeProfile profile;
    uint32_t csb_signature = 0u;
    uint32_t graphics_signature = 0u;
    uint32_t version = 0u;

    memset(tail, 0, sizeof(tail));
    add_signature_bundle(tail, 3u, 3u);
    prepare_profile(&profile, tail, 3u * block_bytes);
    check(csb_v1_runtime_recover_csbwin_runtime_file_signatures(
              &profile, &csb_signature, &graphics_signature, &version) == 1 &&
              csb_signature == 0x10203040u &&
              graphics_signature == 0x10203041u && version == 0x10203042u,
          "CSBWin recovers the complete raw runtime-signature bundle");

    profile.csbwin_appended_tail[0] ^= 1u;
    check(csb_v1_runtime_recover_csbwin_runtime_file_signatures(
              &profile, &csb_signature, &graphics_signature, &version) == 0 &&
              csb_signature == 0u && graphics_signature == 0u && version == 0u,
          "CSBWin rejects a drifted runtime-signature receipt");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    add_signature_bundle(tail, 2u, 2u);
    prepare_profile(&profile, tail, 2u * block_bytes);
    check(csb_v1_runtime_recover_csbwin_runtime_file_signatures(
              &profile, &csb_signature, &graphics_signature, &version) == 0 &&
              csb_signature == 0u && graphics_signature == 0u && version == 0u,
          "CSBWin rejects a partial runtime-signature bundle");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    add_signature_bundle(tail, 3u, 3u);
    add_record(tail, 3u, (5u << 24) | (2u << 16), 0x10203040u, 3u);
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_recover_csbwin_runtime_file_signatures(
              &profile, &csb_signature, &graphics_signature, &version) == 0 &&
              csb_signature == 0u && graphics_signature == 0u && version == 0u,
          "CSBWin rejects duplicate live runtime-signature owners");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    add_signature_bundle(tail, 3u, 3u);
    put_le16(tail, 64u * 4u + 2u, 4u);
    prepare_profile(&profile, tail, 3u * block_bytes);
    check(csb_v1_runtime_recover_csbwin_runtime_file_signatures(
              &profile, &csb_signature, &graphics_signature, &version) == 0 &&
              csb_signature == 0u && graphics_signature == 0u && version == 0u,
          "CSBWin rejects a malformed runtime-signature record size");
    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
