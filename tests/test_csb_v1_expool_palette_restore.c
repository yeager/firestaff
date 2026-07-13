/*
 * CSBWin EDT_Palette EXPOOL restore regression.
 *
 * Source: CSBWin SaveGame.cpp:1218-1238, 1945-1970 and data.cpp:1575-1600.
 * The fixture is a byte-level DB11/EXPOOL representation, not substitute
 * palette data: every record uses the original key/hash/node contract.
 */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    EXPOOL_BLOCK_BYTES = CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES,
    PALETTE_RECORDS = 24,
    PALETTE_RECORD_BYTES = 64,
    PALETTE_RECORD_WORDS = 16,
    PALETTE_NODE_WORDS = PALETTE_RECORD_WORDS + 2
};

static int failures;

static void check(int condition, const char *message)
{
    if (condition) {
        printf("PASS: %s\n", message);
    } else {
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

static uint32_t read_le32(const uint8_t *bytes, size_t offset)
{
    return (uint32_t)bytes[offset] |
        ((uint32_t)bytes[offset + 1u] << 8) |
        ((uint32_t)bytes[offset + 2u] << 16) |
        ((uint32_t)bytes[offset + 3u] << 24);
}

static void build_palette_tail(uint8_t *tail, size_t size)
{
    uint32_t i;

    memset(tail, 0, size);
    for (i = 0u; i < PALETTE_RECORDS; ++i) {
        const uint32_t record_id = (7u << 24) | i;
        const uint32_t bucket = 32u +
            ((record_id * 0xbb40e62du) >> 27);
        const uint32_t block_base = i * 64u;
        const uint32_t node = block_base + 1u;
        const uint32_t prior = read_le32(tail, (size_t)bucket * 4u);
        uint32_t b;

        put_le16(tail, (size_t)block_base * 4u + 2u,
                 PALETTE_NODE_WORDS);
        put_le32(tail, (size_t)node * 4u, prior);
        put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
        for (b = 0u; b < PALETTE_RECORD_BYTES; ++b) {
            tail[(size_t)(node + 2u) * 4u + b] =
                (uint8_t)((i * 17u + b) & 0xffu);
        }
        put_le32(tail, (size_t)bucket * 4u, node);
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
    uint8_t tail[PALETTE_RECORDS * EXPOOL_BLOCK_BYTES];
    CSB_V1_RuntimeProfile profile;
    const uint8_t *palette = NULL;
    size_t palette_size = 0u;

    build_palette_tail(tail, sizeof(tail));
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_restore_csbwin_expool_overlay_palette(&profile) == 0,
          "CSBWin accepts a complete EDT_Palette EXPOOL bundle");
    check(csb_v1_runtime_get_csbwin_expool_overlay_palette(
              &profile, &palette, &palette_size) == 1 &&
              palette_size == CSB_V1_CSBWIN_OVERLAY_PALETTE_BYTES,
          "CSBWin exposes a complete EDT_Palette runtime receipt");
    check(palette != NULL && palette[0] == 0u &&
              palette[511] == (uint8_t)(7u * 17u + 63u) &&
              palette[512] == (uint8_t)(8u * 17u) &&
              palette[1024] == (uint8_t)(16u * 17u) &&
              palette[1535] == (uint8_t)(23u * 17u + 63u),
          "CSBWin restores EDT_Palette bytes in source RGB order");

    /* SaveGame.cpp requires size >= 16 words for every record. Shortening
     * one DB11 descriptor must leave the live renderer receipt untouched. */
    put_le16(profile.csbwin_appended_tail, 2u, 17u);
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    check(csb_v1_runtime_restore_csbwin_expool_overlay_palette(&profile) == 0 &&
              profile.csbwin_overlay_palette_valid == 0 &&
              csb_v1_runtime_get_csbwin_expool_overlay_palette(
                  &profile, &palette, &palette_size) == 0,
          "short EDT_Palette record never publishes a partial HUD palette");

    prepare_profile(&profile, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a ^= 1u;
    check(csb_v1_runtime_restore_csbwin_expool_overlay_palette(&profile) == -1 &&
              csb_v1_runtime_get_csbwin_expool_overlay_palette(
                  &profile, &palette, &palette_size) == 0,
          "altered appended-tail receipt never exposes palette bytes");

    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
