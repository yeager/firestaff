/*
 * CSBWin DSA SETSKIN saved-EXPOOL writeback regression.
 *
 * Source: CSBWin DSA.cpp:3122-3135; data.cpp:2130-2167
 * The fixture is structurally valid DB11/EXPOOL data, not a real save.
 */

#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

static void check(int condition, const char *message)
{
    if (condition) {
        printf("PASS: %s\n", message);
    } else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
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

static void build_skin_column_record(uint8_t *tail, const uint8_t skins[4])
{
    const uint32_t record_id = CSB_V1_SKIN_CACHE_EDT_SKINS << 24;
    const uint32_t bucket = 32u +
        ((record_id * 0xbb40e62du) >> 27);

    memset(tail, 0, CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES);
    put_le16(tail, 2u, 3u);
    put_le32(tail, (size_t)bucket * 4u, 1u);
    put_le32(tail, 1u * 4u, 0u);
    put_le32(tail, 2u * 4u, record_id);
    memcpy(tail + 3u * 4u, skins, 4u);
}

static void prepare_profile(CSB_V1_RuntimeProfile *profile,
                            const uint8_t skins[4])
{
    csb_v1_runtime_init(profile, NULL);
    build_skin_column_record(profile->csbwin_appended_tail, skins);
    profile->csbwin_appended_tail_valid = 1;
    profile->csbwin_appended_tail_size = CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES;
    profile->csbwin_appended_tail_preserved_size =
        CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES;
    profile->csbwin_appended_tail_fnv1a = fnv1a32(
        profile->csbwin_appended_tail,
        profile->csbwin_appended_tail_preserved_size);
}

static void prepare_profile_with_larger_free_node(
    CSB_V1_RuntimeProfile *profile,
    const uint8_t skins[4])
{
    prepare_profile(profile, skins);
    /* CSBWin EXPOOL::enlarge has already supplied one source-owned four-word
     * DB11 node in the second block. SETSKIN may consume it after Read frees
     * the old three-word node, exactly as data.cpp:2145-2166 does. */
    put_le16(profile->csbwin_appended_tail,
             CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES + 2u, 4u);
    put_le32(profile->csbwin_appended_tail, 4u * 4u, 65u);
    put_le32(profile->csbwin_appended_tail, 65u * 4u, 0u);
    profile->csbwin_appended_tail_size =
        2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES;
    profile->csbwin_appended_tail_preserved_size =
        2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES;
    profile->csbwin_appended_tail_fnv1a = fnv1a32(
        profile->csbwin_appended_tail,
        profile->csbwin_appended_tail_preserved_size);
}

int main(void)
{
    static const uint8_t skins[4] = { 1u, 2u, 3u, 4u };
    static const uint8_t single_skin[4] = { 0u, 0u, 0u, 5u };
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t before[2u * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    uint8_t grid[4];
    uint32_t original_fnv;
    const uint8_t *payload = NULL;
    size_t payload_size = 0u;

    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 2;
    prepare_profile(&profile, skins);
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    original_fnv = profile.csbwin_appended_tail_fnv1a;
    check(csb_v1_runtime_set_csbwin_saved_skin(&profile, 0, 1, 1, 42u) == 1 &&
              csb_v1_runtime_locate_csbwin_appended_expool_record(
                  &profile, CSB_V1_SKIN_CACHE_EDT_SKINS << 24,
                  &payload, &payload_size) == 1 &&
              payload_size == 4u && payload[0] == 1u && payload[1] == 2u &&
              payload[2] == 3u && payload[3] == 42u &&
              profile.csbwin_appended_tail_fnv1a != original_fnv,
          "CSBWin SETSKIN writes an existing complete saved EDT_Skins column");
    check(csb_v1_runtime_custom_background_skin_grid(
              &profile, grid, (int)sizeof(grid), NULL, NULL, NULL, NULL) == 1 &&
              grid[0] == 1u && grid[1] == 2u && grid[2] == 3u && grid[3] == 42u,
          "SETSKIN invalidates the HUD cache and presents the saved source byte");

    memcpy(before, profile.csbwin_appended_tail,
           CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES);
    check(csb_v1_runtime_set_csbwin_saved_skin(&profile, 0, 0, 2, 9u) == 0 &&
              memcmp(before, profile.csbwin_appended_tail,
                     CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES) == 0,
          "SETSKIN rejects a write that would require a new DB11 payload extent");

    profile.csbwin_appended_tail_fnv1a ^= 1u;
    memcpy(before, profile.csbwin_appended_tail, sizeof(before));
    check(csb_v1_runtime_set_csbwin_saved_skin(&profile, 0, 0, 0, 8u) == 0 &&
              memcmp(before, profile.csbwin_appended_tail, sizeof(before)) == 0,
          "SETSKIN rejects an altered saved EXPOOL receipt before mutation");
    profile.dungeon_handle = NULL;
    csb_v1_runtime_cleanup(&profile);

    prepare_profile_with_larger_free_node(&profile, skins);
    check(csb_v1_runtime_set_csbwin_saved_skin(&profile, 0, 0, 2, 9u) == 1 &&
              csb_v1_runtime_locate_csbwin_appended_expool_record(
                  &profile, CSB_V1_SKIN_CACHE_EDT_SKINS << 24,
                  &payload, &payload_size) == 1 &&
              payload_size == 8u && payload[0] == 1u && payload[3] == 4u &&
              payload[4] == 9u,
          "SETSKIN consumes an original larger DB11 free node for expansion");
    csb_v1_runtime_cleanup(&profile);

    prepare_profile(&profile, single_skin);
    check(csb_v1_runtime_set_csbwin_saved_skin(&profile, 0, 1, 1, 0u) == 1 &&
              csb_v1_runtime_locate_csbwin_appended_expool_record(
                  &profile, CSB_V1_SKIN_CACHE_EDT_SKINS << 24,
                  &payload, &payload_size) == 0,
          "SETSKIN deletes an all-zero column through source EXPOOL Read");
    csb_v1_runtime_cleanup(&profile);

    return g_failures == 0 ? 0 : 1;
}
