#include "dm2_v1_champion_portrait_gdat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", message); \
        ++failures; \
    } \
} while (0)

static void test_direct_source_route(void)
{
    DM2_V1_ChampionPortraitInput input;
    DM2_V1_ChampionPortraitGdatRoute route;

    memset(&input, 0, sizeof(input));
    input.player_index = 2u;
    input.hero_type = 0x4du;
    input.hero_type_source_bound = 1;

    CHECK(dm2_v1_champion_portrait_gdat_route(&input, &route) == 1,
          "save-bound HeroType resolves a portrait route");
    CHECK(route.category == DM2_GDAT_CATEGORY_CHAMPIONS &&
          route.hero_type == 0x4du &&
          route.field == DM2_V1_CHAMPION_PORTRAIT_GDAT_FIELD &&
          route.rectno == DM2_V1_CHAMPION_PORTRAIT_RECT_BASE + 2u,
          "route matches SKProject DRAW_CHAMPION_PICTURE arguments");
}

static void test_rejections_are_fail_closed(void)
{
    DM2_V1_ChampionPortraitInput input;
    DM2_V1_ChampionPortraitGdatRoute route;
    DM2_V1_ChampionPortraitGdatReceipt receipt;

    memset(&input, 0, sizeof(input));
    input.player_index = 1u;
    input.hero_type = 0x37u;
    CHECK(dm2_v1_champion_portrait_gdat_route(&input, &route) == 0,
          "Firestaff-only HeroType cannot select a GDAT portrait");
    CHECK(route.category == 0u && route.rectno == 0u,
          "rejected route leaves no drawable address");

    input.hero_type_source_bound = 1;
    input.player_index = DM2_V1_CHAMPION_PORTRAIT_PLAYER_COUNT;
    CHECK(dm2_v1_champion_portrait_gdat_route(&input, &route) == 0,
          "out-of-range player cannot select a portrait panel");
    CHECK(dm2_v1_champion_portrait_gdat_receipt(NULL, &input, &receipt) == 0,
          "missing GDAT loader cannot fabricate portrait material");
}

static void test_source_material_receipt(void)
{
    uint8_t raw[34] = {0};
    uint32_t raw_offsets[1] = {0u};
    uint32_t raw_sizes[1] = {sizeof(raw)};
    DM2_V1_GdatEntry entry;
    DM2_V1_AssetLoader loader;
    DM2_V1_ChampionPortraitInput input;
    DM2_V1_ChampionPortraitGdatReceipt receipt;
    int i;

    /* A 4x4 U4 IMG3: the last 16 bytes are its source-owned local palette. */
    raw[0] = 4u;
    raw[2] = 4u;
    raw[3] = 0x80u; /* IMG3 OffsetY == -32: unpacked U4. */
    raw[4] = 4u;
    for (i = 0; i < 8; ++i) raw[10 + i] = (uint8_t)(0x10u + i);
    for (i = 0; i < 16; ++i) raw[18 + i] = (uint8_t)(0x80u + i);

    memset(&entry, 0, sizeof(entry));
    entry.cls1 = DM2_GDAT_CATEGORY_CHAMPIONS;
    entry.cls2 = 0x4du;
    entry.cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entry.cls4 = DM2_V1_CHAMPION_PORTRAIT_GDAT_FIELD;
    memset(&loader, 0, sizeof(loader));
    loader.data = raw;
    loader.data_size = sizeof(raw);
    loader.loaded = 1;
    loader.raw_data_count = 1u;
    loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes;
    loader.entries = &entry;
    loader.entry_count = 1u;

    memset(&input, 0, sizeof(input));
    input.player_index = 3u;
    input.hero_type = 0x4du;
    input.hero_type_source_bound = 1;
    CHECK(dm2_v1_champion_portrait_gdat_receipt(&loader, &input, &receipt) == 1,
          "typed CHAMPIONS IMG3 and local palette form one receipt");
    CHECK(receipt.valid && receipt.route.rectno == 176u &&
          receipt.decoded_width == 4u && receipt.decoded_height == 4u &&
          receipt.decoded_format == DM2_IMG_FMT_U4 &&
          receipt.decoded_pixel_count == 16u &&
          receipt.local_palette16[0] == 0x80u &&
          receipt.local_palette16[15] == 0x8fu &&
          receipt.local_palette_hash != 0u &&
          receipt.decoded_pixels_hash != 0u && receipt.material_hash != 0u,
          "receipt retains the decoded image and exact local-palette evidence");

    entry.cls2 = 0x4eu;
    CHECK(dm2_v1_champion_portrait_gdat_receipt(&loader, &input, &receipt) == 0,
          "a different HeroType cannot borrow another champion portrait");
}

int main(void)
{
    test_direct_source_route();
    test_rejections_are_fail_closed();
    test_source_material_receipt();
    if (failures != 0) {
        fprintf(stderr, "%d test(s) failed\\n", failures);
        return 1;
    }
    puts("DM2 champion portrait GDAT tests passed");
    return 0;
}
