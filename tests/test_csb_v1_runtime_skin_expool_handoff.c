/*
 * CSB runtime save-to-HUD skin handoff regression.
 *
 * Source:
 *   CSBWin data.cpp:2053-2125 SKIN_CACHE::Load/GetSkin/GetDefaultSkin
 *   CSBWin data.cpp EXPOOL::Locate
 *
 * This uses a local, structurally valid DB11/EXPOOL record fixture only. It
 * is not a real-save corpus claim and it supplies no fallback skin data.
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
    /* One node at word one: p+1 is the key and p+2 begins the four-byte
     * source skin column. CSBWin's block size is p-relative. */
    put_le16(tail, 2u, 3u);
    put_le32(tail, (size_t)bucket * 4u, 1u);
    put_le32(tail, 1u * 4u, 0u);
    put_le32(tail, 2u * 4u, record_id);
    memcpy(tail + 3u * 4u, skins, 4u);
}

int main(void)
{
    static const uint8_t first_skins[4] = { 7u, 8u, 9u, 10u };
    static const uint8_t resumed_skins[4] = { 31u, 32u, 33u, 34u };
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    uint8_t grid[4];
    int width = 0;
    int height = 0;
    int loaded_level = -1;
    int default_skin = -1;

    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 2;

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    build_skin_column_record(profile.csbwin_appended_tail, first_skins);
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES;
    profile.csbwin_appended_tail_preserved_size =
        CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES;
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);

    check(csb_v1_runtime_custom_background_skin_grid(
              &profile, grid, (int)sizeof(grid), &width, &height,
              &loaded_level, &default_skin) == 1 && width == 2 && height == 2 &&
              loaded_level == 0 && default_skin == 0 &&
              memcmp(grid, first_skins, sizeof(grid)) == 0,
          "initial verified EXPOOL skin column reaches the live HUD grid");

    build_skin_column_record(profile.csbwin_appended_tail, resumed_skins);
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    memset(grid, 0, sizeof(grid));
    check(csb_v1_runtime_custom_background_skin_grid(
              &profile, grid, (int)sizeof(grid), &width, &height,
              &loaded_level, &default_skin) == 1 &&
              memcmp(grid, resumed_skins, sizeof(grid)) == 0,
          "changed verified save tail invalidates stale SKIN_CACHE HUD bytes");

    profile.csbwin_appended_tail_valid = 0;
    profile.csbwin_appended_tail_size = 0u;
    profile.csbwin_appended_tail_preserved_size = 0u;
    profile.csbwin_appended_tail_fnv1a = 0u;
    memset(grid, 0xff, sizeof(grid));
    check(csb_v1_runtime_custom_background_skin_grid(
              &profile, grid, (int)sizeof(grid), &width, &height,
              &loaded_level, &default_skin) == 0 &&
              grid[0] == 0u && grid[1] == 0u && grid[2] == 0u && grid[3] == 0u,
          "absent save skin record clears the cache without a synthetic HUD fallback");

    return g_failures == 0 ? 0 : 1;
}
