/* CSBWin Character.cpp::GetFromWings text-identity recovery regression. */

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
                       uint32_t record_id, const uint8_t *payload)
{
    enum { record_bytes = 100, node_words = 27 };
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    const uint32_t block_base = block_index * 64u;
    const uint32_t node = block_base + 1u;
    const uint32_t prior = read_le32(tail, (size_t)bucket * 4u);

    put_le16(tail, (size_t)block_base * 4u + 2u, node_words);
    put_le32(tail, (size_t)node * 4u, prior);
    put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
    memcpy(tail + (size_t)(node + 2u) * 4u, payload, record_bytes);
    put_le32(tail, (size_t)bucket * 4u, node);
}

static void build_wing_tail(uint8_t *tail, uint16_t fingerprint,
                            uint32_t record_count)
{
    enum { record_bytes = 100 };
    uint8_t record[record_bytes];
    uint32_t record_index;

    for (record_index = 0u; record_index < record_count; ++record_index) {
        memset(record, 0, sizeof(record));
        if (record_index == 0u) {
            memcpy(record, "GOTHMOG", 7u);
            memcpy(record + 8u, "THE CRUEL", 9u);
        }
        if (record_index == 2u) put_le16(record, 80u, fingerprint);
        add_record(tail, record_index,
                   (8u << 24) | (record_index << 16) | fingerprint,
                   record);
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
    enum { fingerprint = 0x4a3cu, block_bytes = CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES };
    uint8_t tail[9u * block_bytes];
    uint8_t duplicate[100];
    CSB_V1_RuntimeProfile profile;
    char name[9];
    char title[17];

    memset(tail, 0, sizeof(tail));
    build_wing_tail(tail, fingerprint, 8u);
    prepare_profile(&profile, tail, 8u * block_bytes);
    check(csb_v1_runtime_recover_csbwin_wing_identity(
              &profile, fingerprint, name, sizeof(name), title, sizeof(title)) == 1 &&
              strcmp(name, "GOTHMOG") == 0 && strcmp(title, "THE CRUEL") == 0,
          "CSBWin recovers a complete raw wing name and title");
    check(csb_v1_runtime_recover_csbwin_wing_identity(
              &profile, fingerprint, name, 7u, title, sizeof(title)) == 0 &&
              name[0] == '\0' && title[0] == '\0',
          "CSBWin rejects an undersized wing identity destination");

    profile.csbwin_appended_tail[0] ^= 1u;
    check(csb_v1_runtime_recover_csbwin_wing_identity(
              &profile, fingerprint, name, sizeof(name), title, sizeof(title)) == 0 &&
              name[0] == '\0' && title[0] == '\0',
          "CSBWin rejects a drifted wing identity receipt");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    build_wing_tail(tail, fingerprint, 8u);
    memset(duplicate, 0, sizeof(duplicate));
    memcpy(duplicate, "GOTHMOG", 7u);
    memcpy(duplicate + 8u, "THE CRUEL", 9u);
    add_record(tail, 8u, (8u << 24) | fingerprint, duplicate);
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_recover_csbwin_wing_identity(
              &profile, fingerprint, name, sizeof(name), title, sizeof(title)) == 0 &&
              name[0] == '\0' && title[0] == '\0',
          "CSBWin rejects duplicate live wing record owners");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    build_wing_tail(tail, fingerprint, 7u);
    prepare_profile(&profile, tail, 7u * block_bytes);
    check(csb_v1_runtime_recover_csbwin_wing_identity(
              &profile, fingerprint, name, sizeof(name), title, sizeof(title)) == 0 &&
              name[0] == '\0' && title[0] == '\0',
          "CSBWin rejects a partial eight-record wing bundle");
    csb_v1_runtime_cleanup(&profile);

    memset(tail, 0, sizeof(tail));
    build_wing_tail(tail, fingerprint, 8u);
    memset(tail + 20u, 'X', 16u);
    prepare_profile(&profile, tail, 8u * block_bytes);
    check(csb_v1_runtime_recover_csbwin_wing_identity(
              &profile, fingerprint, name, sizeof(name), title, sizeof(title)) == 0 &&
              name[0] == '\0' && title[0] == '\0',
          "CSBWin rejects unterminated raw wing title bytes");
    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
