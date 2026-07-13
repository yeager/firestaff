/* CSBWin post-palette EXPOOL save-policy regression.
 * Source: CSBWin SaveGame.cpp:1972-2034 and CSB.h EDT_Database records. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    POLICY_RECORDS = 5,
    POLICY_RECORD_WORDS = 1,
    POLICY_NODE_WORDS = POLICY_RECORD_WORDS + 2
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

static void add_record(uint8_t *tail, uint32_t index, uint32_t record_id,
                       uint32_t value)
{
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    const uint32_t block_base = index * 64u;
    const uint32_t node = block_base + 1u;
    const uint32_t prior = (uint32_t)tail[(size_t)bucket * 4u] |
        ((uint32_t)tail[(size_t)bucket * 4u + 1u] << 8) |
        ((uint32_t)tail[(size_t)bucket * 4u + 2u] << 16) |
        ((uint32_t)tail[(size_t)bucket * 4u + 3u] << 24);

    put_le16(tail, (size_t)block_base * 4u + 2u, POLICY_NODE_WORDS);
    put_le32(tail, (size_t)node * 4u, prior);
    put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
    put_le32(tail, (size_t)(node + 2u) * 4u, value);
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
        DATABASE = 5u << 24,
        DELETE_DUPLICATE = DATABASE | (1u << 16),
        RUNTIME_SIGNATURE = DATABASE | (2u << 16),
        DEBUGGING = DATABASE | (3u << 16),
        DISABLE_SAVES = DATABASE | (5u << 16)
    };
    uint8_t tail[POLICY_RECORDS * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    CSB_V1_RuntimeProfile profile;
    uint32_t delete_duplicate = 0u;
    uint32_t debugging = 0u;
    uint32_t csbgraphics = 0u;
    uint32_t graphics = 0u;
    uint32_t version = 0u;

    memset(tail, 0, sizeof(tail));
    add_record(tail, 0u, DELETE_DUPLICATE, 0u);
    add_record(tail, 1u, RUNTIME_SIGNATURE, 0x11223344u);
    add_record(tail, 2u, RUNTIME_SIGNATURE | 1u, 0x55667788u);
    add_record(tail, 3u, RUNTIME_SIGNATURE | 2u, 0x99aabbccu);
    add_record(tail, 4u, DISABLE_SAVES, 1u);
    prepare_profile(&profile, tail, sizeof(tail));

    check(csb_v1_runtime_restore_csbwin_save_policy(&profile) == 0,
          "CSBWin restores verified post-palette EXPOOL policy records");
    check(csb_v1_runtime_get_csbwin_save_policy(
              &profile, &delete_duplicate, &debugging, &csbgraphics,
              &graphics, &version) == 0 && delete_duplicate == 0u &&
              debugging == 0u && csbgraphics == 0x11223344u &&
              graphics == 0x55667788u && version == 0x99aabbccu,
          "CSBWin preserves delete-duplicate and runtime signatures");
    check(csb_v1_runtime_csbwin_saves_disabled(&profile) == 1,
          "CSBWin retains EDBT_DisableSaves from the same save tail");

    add_record(tail, 3u, DEBUGGING, 1u);
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_restore_csbwin_save_policy(&profile) == 0 &&
              csb_v1_runtime_get_csbwin_save_policy(
                  &profile, &delete_duplicate, &debugging, &csbgraphics,
                  &graphics, &version) == 0 && debugging == 1u &&
              csbgraphics == 0u && graphics == 0u && version == 0u,
          "CSBWin suppresses runtime signatures when Debuging is enabled");

    put_le16(profile.csbwin_appended_tail, 2u, 2u);
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    check(csb_v1_runtime_restore_csbwin_save_policy(&profile) == -1 &&
              profile.csbwin_debugging_data == 1u,
          "short present policy records leave the live policy unchanged");

    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
