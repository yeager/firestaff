/* CSBWin data.cpp ExtendedCellFlags raw recovery regression. */

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
                       uint32_t record_id, const uint32_t *words,
                       uint16_t node_words)
{
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    const uint32_t block_base = block_index * 64u;
    const uint32_t node = block_base + 1u;
    const uint32_t prior = read_le32(tail, (size_t)bucket * 4u);
    unsigned int word;

    put_le16(tail, (size_t)block_base * 4u + 2u, node_words);
    put_le32(tail, (size_t)node * 4u, prior);
    put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
    for (word = 0u; words && word + 2u < node_words; ++word) {
        put_le32(tail, (size_t)(node + 2u + word) * 4u, words[word]);
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
    enum {
        level = 12u,
        x = 21u,
        record_id = (2u << 24) | (level << 5) | x,
        block_bytes = CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES
    };
    const uint32_t source_words[8] = {
        0x01234567u, 0x89abcdefu, 0x0badc0deu, 0xfeedfaceu,
        0x76543210u, 0x55aa55aau, 0x80000001u, 0x13579bdfu
    };
    uint8_t tail[2u * block_bytes];
    uint32_t recovered[8] = { 0u };
    CSB_V1_RuntimeProfile profile;
    uint32_t original_receipt;

    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, record_id, source_words, 10u);
    prepare_profile(&profile, tail, sizeof(tail));
    original_receipt = profile.csbwin_appended_tail_fnv1a;
    check(csb_v1_runtime_recover_csbwin_extended_cell_flags(
              &profile, level, x, recovered) == 1 &&
              memcmp(recovered, source_words, sizeof(recovered)) == 0 &&
              profile.csbwin_appended_tail_fnv1a == original_receipt,
          "CSBWin recovers one raw eight-word ExtendedCellFlags record without mutation");
    check(csb_v1_runtime_recover_csbwin_extended_cell_flags(
              &profile, 64u, x, recovered) == 0 &&
              csb_v1_runtime_recover_csbwin_extended_cell_flags(
                  &profile, level, 32u, recovered) == 0,
          "CSBWin rejects out-of-range ExtendedCellFlags coordinates");
    profile.csbwin_appended_tail[0] ^= 1u;
    check(csb_v1_runtime_recover_csbwin_extended_cell_flags(
              &profile, level, x, recovered) == 0,
          "CSBWin rejects a drifted ExtendedCellFlags receipt");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, record_id, source_words, 10u);
    add_record(tail, 1u, record_id, source_words, 10u);
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_recover_csbwin_extended_cell_flags(
              &profile, level, x, recovered) == 0,
          "CSBWin rejects duplicate live ExtendedCellFlags records");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, record_id, source_words, 9u);
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_recover_csbwin_extended_cell_flags(
              &profile, level, x, recovered) == 0,
          "CSBWin rejects a short ExtendedCellFlags record");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_recover_csbwin_extended_cell_flags(
              &profile, level, x, recovered) == 0,
          "CSBWin rejects an absent ExtendedCellFlags record without defaulting");
    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
