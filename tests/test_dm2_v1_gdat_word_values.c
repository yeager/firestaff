/*
 * test_dm2_v1_gdat_word_values.c
 *
 * Focused DM2 GDAT typed-data gate. This covers skproject dtWordValue
 * access for item metadata used by HUD/dungeon item rendering.
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *f;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out_data || !out_size) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(f);
        return 0;
    }
    if (fread(data, 1u, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return 0;
    }
    fclose(f);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int candidate_path(char *out, size_t out_size, const char *suffix)
{
    const char *data = getenv("FIRESTAFF_DATA");
    const char *home = getenv("HOME");

    if (!out || out_size == 0u || !suffix) return 0;
    if (data && data[0]) {
        snprintf(out, out_size, "%s/%s", data, suffix);
        return 1;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/%s", home, suffix);
        return 1;
    }
    return 0;
}

static int load_graphics(uint8_t **out_data, size_t *out_size, char *path, size_t path_size)
{
    static const char *suffixes[] = {
        "dm2/GRAPHICS.DAT",
        "dm2/graphics.dat",
        "dm2/DM2GRAPHICS.DAT",
        "dm2/DM2GRA.DAT"
    };
    size_t i;
    const char *dm2_data = getenv("FIRESTAFF_DM2_DATA_DIR");

    /* Keep real-corpus checks usable with the same direct DM2 data root as
     * the boot-profile tests.  This accepts user-supplied files in place and
     * never extracts or copies them. */
    if (dm2_data && dm2_data[0]) {
        static const char *names[] = {
            "GRAPHICS.DAT", "graphics.dat", "DM2GRAPHICS.DAT", "DM2GRA.DAT"
        };
        for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
            snprintf(path, path_size, "%s/%s", dm2_data, names[i]);
            if (read_file(path, out_data, out_size)) return 1;
        }
    }

    for (i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, path_size, suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

static void test_item_word_values_real_data(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    char path[1024];
    DM2_V1_AssetLoader loader;
    int category_count = 0;
    int weight_count = 0;
    int money_count = 0;
    int flags_count = 0;
    int equip_count = 0;
    uint32_t receipt_hash = 0x32495756u;

    memset(&loader, 0, sizeof(loader));
    if (!load_graphics(&graphics, &graphics_size, path, sizeof(path))) {
        printf("  SKIP: optional real DM2 GRAPHICS.DAT not present\n");
        return;
    }

    CHECK(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0,
          "real DM2 GRAPHICS.DAT initializes typed GDAT loader");
    if (!loader.loaded) {
        free(graphics);
        return;
    }
    CHECK(dm2_v1_asset_loader_validate_typed_graph(&loader) == 1,
          "real typed ENT1 entries resolve to bounded GDAT raw payloads");

    for (int category = DM2_GDAT_CATEGORY_WEAPONS;
         category <= DM2_GDAT_CATEGORY_MISCELLANEOUS;
         ++category) {
        int category_hit = 0;
        for (int index = 0; index < 0x100; ++index) {
            uint16_t value;
            if (dm2_v1_asset_load_word_value(&loader, category, index, 0x00, &value)) {
                ++flags_count;
                category_hit = 1;
                receipt_hash = (receipt_hash * 16777619u) ^ value;
            }
            if (dm2_v1_asset_load_word_value(&loader, category, index, 0x01, &value)) {
                ++weight_count;
                category_hit = 1;
                receipt_hash = (receipt_hash * 16777619u) ^ value;
            }
            if (dm2_v1_asset_load_word_value(&loader, category, index, 0x02, &value)) {
                ++money_count;
                category_hit = 1;
                receipt_hash = (receipt_hash * 16777619u) ^ value;
            }
            if (dm2_v1_asset_load_word_value(&loader, category, index, 0x04, &value)) {
                ++equip_count;
                category_hit = 1;
                receipt_hash = (receipt_hash * 16777619u) ^ value;
            }
        }
        if (category_hit) ++category_count;
    }

    CHECK(category_count >= 4,
          "skproject item categories expose dtWordValue metadata");
    CHECK(flags_count > 0 && weight_count > 0 && money_count > 0,
          "item flags, weight and money word-values are available");
    CHECK(equip_count > 0,
          "item equipment word-values are available where present");
    CHECK(receipt_hash != 0u,
          "item word-value receipt hash is nonzero");

    dm2_v1_asset_loader_free(&loader);
    free(graphics);
}

static void test_carried_item_selector(void)
{
    uint8_t field = 0u;

    CHECK(dm2_v1_viewport_select_carried_item_image_field(
              0u, 7u, 91u, 2, &field) && field == 0x18u,
          "missing dtWordValue(6) retains SKProject image field 0x18");
    CHECK(dm2_v1_viewport_select_carried_item_image_field(
              0x0003u, 7u, 8u, 0, &field) && field == 0x1au,
          "mode-0 hand selector follows the source game tick");
    CHECK(dm2_v1_viewport_select_carried_item_image_field(
              0x0503u, 7u, 8u, 0, &field) && field == 0x18u,
          "mode-5 hand selector includes the source record index");
    CHECK(dm2_v1_viewport_select_carried_item_image_field(
              0x0204u, 7u, 8u, 3, &field) && field == 0x1bu,
          "mode-2 hand selector follows the source party direction");
    CHECK(!dm2_v1_viewport_select_carried_item_image_field(
               0x0102u, 7u, 8u, 0, &field) &&
              !dm2_v1_viewport_select_carried_item_image_field(
               0x8002u, 7u, 8u, 0, &field),
          "random and equipment-dependent selectors fail closed");
}

static void test_interface_palette_real_data(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    char path[1024];
    DM2_V1_AssetLoader loader;
    DM2_V1_InterfacePalette palette;

    memset(&loader, 0, sizeof(loader));
    if (!load_graphics(&graphics, &graphics_size, path, sizeof(path))) {
        printf("  SKIP: optional real DM2 GRAPHICS.DAT not present\n");
        return;
    }
    CHECK(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0,
          "real DM2 GRAPHICS.DAT initializes palette decoder");
    if (loader.loaded) {
        CHECK(dm2_v1_asset_load_interface_palette(
                  &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
                  DM2_GDAT_INTERFACE_PALETTE_FIELD, &palette) == 1,
              "real INTERFACE_GENERAL dtPalIRGB/dtPalette16 pair decodes");
        CHECK(palette.hash != 0u,
              "real interface palette exposes a nonzero semantic receipt");
        /* Greatstone's dm2_pc10_en GRAPHICS.DAT raw 0206 (P8B1
         * "Interface - Main Screen 0", system palette) documents the same
         * global palette used by raw 0174/0175 IMG9 credits/menu.  The GDAT
         * rows are RGB888 and the renderer consumes RGB6. */
        CHECK(palette.rgb6[0][0] == 0u && palette.rgb6[0][1] == 0u &&
                  palette.rgb6[0][2] == 0u &&
                  palette.rgb6[1][0] == 3u && palette.rgb6[1][1] == 2u &&
                  palette.rgb6[1][2] == 0u &&
                  palette.rgb6[4][0] == 14u && palette.rgb6[4][1] == 8u &&
                  palette.rgb6[4][2] == 1u,
              "real DM2 system palette matches documented IMG9 RGB6 anchors");
    }
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
}

static void test_interface_palette_decoder_fixture(void)
{
    uint8_t irgb[256 * 4];
    uint8_t pal16[16];
    uint32_t offsets[2] = { 0u, 0u };
    uint32_t sizes[2] = { sizeof(irgb), sizeof(pal16) };
    DM2_V1_GdatEntry entries[2];
    DM2_V1_AssetLoader loader;
    DM2_V1_InterfacePalette palette;

    for (int i = 0; i < 256; ++i) {
        irgb[i * 4 + 0] = 0xffu;
        irgb[i * 4 + 1] = (uint8_t)i;
        irgb[i * 4 + 2] = (uint8_t)(255 - i);
        irgb[i * 4 + 3] = (uint8_t)(i ^ 0xa5);
    }
    for (int i = 0; i < 16; ++i) pal16[i] = (uint8_t)(15 - i);
    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    loader.data = irgb;
    loader.data_size = sizeof(irgb);
    loader.loaded = 1;
    loader.raw_data_count = 2;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.entries = entries;
    loader.entry_count = 2;
    entries[0].cls1 = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_PAL_IRGB;
    entries[0].cls4 = DM2_GDAT_INTERFACE_PALETTE_FIELD;
    entries[0].data_index = 0;
    entries[1] = entries[0];
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_PAL_16;
    entries[1].data_index = 1;

    /* Keep both typed payloads contiguous so the fixture uses the normal
     * raw-offset lookup rather than a test-only data path. */
    offsets[1] = sizeof(irgb);
    {
        uint8_t combined[sizeof(irgb) + sizeof(pal16)];
        memcpy(combined, irgb, sizeof(irgb));
        memcpy(combined + sizeof(irgb), pal16, sizeof(pal16));
        loader.data = combined;
        loader.data_size = sizeof(combined);
        CHECK(dm2_v1_asset_load_interface_palette(
                  &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
                  DM2_GDAT_INTERFACE_PALETTE_FIELD, &palette) == 1,
              "typed dtPalIRGB/dtPalette16 fixture decodes");
        CHECK(palette.rgb6[0][0] == 0u && palette.rgb6[0][1] == 63u &&
                  palette.rgb6[0][2] == (0xa5u >> 2),
              "IRGB decoder skips byte zero and converts RGB to VGA 6-bit");
        CHECK(palette.palette16[0] == 15u && palette.palette16[15] == 0u &&
                  palette.hash != 0u,
              "dtPalette16 remains a 16-byte logical colour-index table");
        sizes[1] = 15u;
        CHECK(dm2_v1_asset_load_interface_palette(
                  &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
                  DM2_GDAT_INTERFACE_PALETTE_FIELD, &palette) == 0,
              "short dtPalette16 payload is rejected");
    }
}

static void test_immediate_typed_entries_do_not_alias_raw_payloads(void)
{
    uint8_t raw_payload[4] = { 0xdeu, 0xadu, 0xbeu, 0xefu };
    uint32_t offsets[1] = { 0u };
    uint32_t sizes[1] = { sizeof(raw_payload) };
    DM2_V1_GdatEntry entries[2];
    DM2_V1_AssetLoader loader;
    const uint8_t *raw;
    size_t raw_size;
    uint16_t value;

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    loader.data = raw_payload;
    loader.data_size = sizeof(raw_payload);
    loader.loaded = 1;
    loader.raw_data_count = 1;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.entries = entries;
    loader.entry_count = 2;

    entries[0].cls1 = DM2_GDAT_CATEGORY_WEAPONS;
    entries[0].cls2 = 7;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[0].cls4 = 1;
    entries[0].data_index = 0x1234u;
    entries[1].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[1].cls2 = 2;
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
    entries[1].cls4 = 0x22;
    entries[1].data_index = 0x0042u;

    raw_size = 99u;
    raw = dm2_v1_asset_load_sized(
        &loader, DM2_GDAT_CATEGORY_WEAPONS, 7, 1, &raw_size);
    CHECK(raw == NULL && raw_size == 0u,
          "dtWordValue immediate entry does not alias a raw payload slot");
    CHECK(dm2_v1_asset_load_word_value(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 7, 1, &value) == 1 &&
              value == 0x1234u,
          "dtWordValue remains available through exact typed lookup");

    raw_size = 99u;
    raw = dm2_v1_asset_load_sized(
        &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 2, 0x22, &raw_size);
    CHECK(raw == NULL && raw_size == 0u,
          "dtImageOffset immediate entry does not alias a raw payload slot");
    CHECK(dm2_v1_asset_load_image_offset(
              &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 2, 0x22, &value) == 1 &&
              value == 0x0042u,
          "dtImageOffset remains available through exact typed lookup");
}

static void test_img3_local_palette_fixture(void)
{
    uint8_t raw_payload[4] = { 0xdeu, 0xadu, 0xbeu, 0xefu };
    uint32_t offsets[1] = { 0u };
    uint32_t sizes[1] = { sizeof(raw_payload) };
    DM2_V1_GdatEntry entries[2];
    DM2_V1_AssetLoader loader;
    const uint8_t *raw;
    size_t raw_size;
    uint16_t value;

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    loader.data = raw_payload;
    loader.data_size = sizeof(raw_payload);
    loader.loaded = 1;
    loader.raw_data_count = 1;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.entries = entries;
    loader.entry_count = 2;

    entries[0].cls1 = DM2_GDAT_CATEGORY_WEAPONS;
    entries[0].cls2 = 7;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[0].cls4 = 1;
    entries[0].data_index = 0x1234u;
    entries[1].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[1].cls2 = 2;
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
    entries[1].cls4 = 0x22;
    entries[1].data_index = 0x0042u;

    raw_size = 99u;
    raw = dm2_v1_asset_load_sized(
        &loader, DM2_GDAT_CATEGORY_WEAPONS, 7, 1, &raw_size);
    CHECK(raw == NULL && raw_size == 0u,
          "dtWordValue immediate entry does not alias a raw payload slot");
    CHECK(dm2_v1_asset_load_word_value(
              &loader, DM2_GDAT_CATEGORY_WEAPONS, 7, 1, &value) == 1 &&
              value == 0x1234u,
          "dtWordValue remains available through exact typed lookup");

    raw_size = 99u;
    raw = dm2_v1_asset_load_sized(
        &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 2, 0x22, &raw_size);
    CHECK(raw == NULL && raw_size == 0u,
          "dtImageOffset immediate entry does not alias a raw payload slot");
    CHECK(dm2_v1_asset_load_image_offset(
              &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, 2, 0x22, &value) == 1 &&
              value == 0x0042u,
          "dtImageOffset remains available through exact typed lookup");
}

static void test_door_light_palette_darkness(void)
{
    uint8_t darkness = 0u;

    /* SKProject _32cb_0804 combines c_light with _4976_4226.  D3 retry
     * uses its distance index (3), whose original table value is 28. */
    CHECK(dm2_v1_gdat_door_light_palette_darkness(20u, 3u, &darkness) &&
              darkness == 40u,
          "D3 light palette combines source c_light with distance darkness");
    CHECK(dm2_v1_gdat_door_light_palette_darkness(0u, 3u, &darkness) &&
              darkness == 28u,
          "D3 light palette preserves original zero-light darkness");
    CHECK(!dm2_v1_gdat_door_light_palette_darkness(65u, 3u, &darkness) &&
              !dm2_v1_gdat_door_light_palette_darkness(20u, 5u, &darkness),
          "door light palette rejects out-of-range source values");
}

static void test_ornate_animation_frame(void)
{
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntry entries[1];
    uint32_t offsets[1] = { 0u };
    uint32_t sizes[1] = { 3u };
    uint8_t text[] = { '2', 'A', 0u };
    uint16_t frame = 0u;
    uint32_t receipt = 0u;

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    loader.data = text;
    loader.data_size = sizeof(text);
    loader.loaded = 1;
    loader.raw_data_count = 1u;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.entries = entries;
    loader.entry_count = 1u;
    entries[0].cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
    entries[0].cls2 = 7u;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[0].cls4 = 0x0du;
    entries[0].data_index = 0x8003u;
    CHECK(dm2_v1_asset_query_ornate_animation_frame(
              &loader, DM2_GDAT_CATEGORY_WALL_GFX, 7, 4u, 0u,
              &frame, &receipt) && frame == 2u && receipt != 0u,
          "ornate word animation uses source length and high-bit frame base");

    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
    entries[0].data_index = 0u;
    CHECK(dm2_v1_asset_query_ornate_animation_frame(
              &loader, DM2_GDAT_CATEGORY_WALL_GFX, 7, 1u, 0u,
              &frame, &receipt) && frame == 10u && receipt != 0u,
          "ornate text animation decodes SKProject base-36 frame bytes");
    text[1] = '!';
    CHECK(!dm2_v1_asset_query_ornate_animation_frame(
              &loader, DM2_GDAT_CATEGORY_WALL_GFX, 7, 1u, 0u,
              &frame, &receipt),
          "ornate animation rejects unsupported source sequence bytes");
}

int main(void)
{
    printf("=== DM2 V1 GDAT Word-Value Test ===\n");
    test_carried_item_selector();
    test_interface_palette_decoder_fixture();
    test_immediate_typed_entries_do_not_alias_raw_payloads();
    test_img3_local_palette_fixture();
    test_door_light_palette_darkness();
    test_ornate_animation_frame();
    test_interface_palette_real_data();
    test_item_word_values_real_data();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
