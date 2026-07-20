/* Source-owned DM2 weather ENVIRONMENT selection.
 * skproject/SKWIN/SkWinCore.cpp UPDATE_WEATHER 3df7:0388-040f and
 * RETRIEVE_ENVIRONMENT_CMD_CD_FW 3df7:075f. */

#include "dm2_v1_weather_gdat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    } else {
        fprintf(stderr, "PASS: %s\n", label);
    }
}

static void add_entry(DM2_V1_GdatEntry *entry, uint8_t type,
                      uint8_t field, uint16_t raw_index)
{
    memset(entry, 0, sizeof(*entry));
    entry->cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
    entry->cls2 = 3u;
    entry->cls3 = type;
    entry->cls4 = field;
    entry->data_index = raw_index;
}

int main(void)
{
    static const uint8_t raw[] = {
        'c', 'd', '=', '-', '1', '2', 'f', 'w', '=', '7', 0,
        0xa1, 0xa2, 0xa3, 0xa4,
        'c', 'd', '=', '2', '3', 'f', 'w', '=', '4', 0,
        0xb1, 0xb2, 0xb3
    };
    uint32_t offsets[] = {0u, 11u, 15u, 25u};
    uint32_t sizes[] = {11u, 4u, 10u, 3u};
    DM2_V1_GdatEntry entries[4];
    DM2_V1_AssetLoader loader;
    DM2_V1_EnvironmentWeatherReceipt receipt;

    memset(&loader, 0, sizeof(loader));
    add_entry(&entries[0], 5u, DM2_V1_ENVIRONMENT_SKY_CLOUDS_MEDIUM, 0u);
    add_entry(&entries[1], DM2_GDAT_ENTRY_TYPE_IMAGE,
              DM2_V1_ENVIRONMENT_SKY_CLOUDS_MEDIUM, 1u);
    add_entry(&entries[2], 5u, DM2_V1_ENVIRONMENT_WET_GROUND_MEDIUM, 2u);
    add_entry(&entries[3], DM2_GDAT_ENTRY_TYPE_IMAGE,
              DM2_V1_ENVIRONMENT_WET_GROUND_MEDIUM, 3u);
    loader.loaded = 1;
    loader.data = raw;
    loader.data_size = sizeof(raw);
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 4u;
    loader.entries = entries;
    loader.entry_count = 4u;

    check(dm2_v1_weather_gdat_environment_receipt(
              &loader, 3u, 71u, 1, 1, 0x40u, 0x80u, 1, 1,
              &receipt) == 1 && receipt.valid &&
              receipt.map_load_token == 71u && receipt.material_count == 2u &&
              receipt.materials[0].environment_field ==
                  DM2_V1_ENVIRONMENT_SKY_CLOUDS_MEDIUM &&
              receipt.materials[0].command_cd == -12 &&
              receipt.materials[0].command_fw == 7u &&
              receipt.materials[0].image_byte_count == 4u &&
              receipt.materials[1].environment_field ==
                  DM2_V1_ENVIRONMENT_WET_GROUND_MEDIUM &&
              receipt.materials[1].command_cd == 23 &&
              receipt.materials[1].command_fw == 4u &&
              receipt.materials[1].image_byte_count == 3u &&
              receipt.materials[0].text_hash != 0u &&
              receipt.materials[1].image_hash != 0u,
          "source cloud and wet-ground ENVIRONMENT pairs stay data-backed");

    check(dm2_v1_weather_gdat_environment_receipt(
              &loader, 3u, 72u, 1, 0, 0xffu, 0xffu, 1, 1,
              &receipt) == 1 && receipt.valid && receipt.material_count == 0u,
          "disabled source weather publishes no substitute material");

    entries[3].cls3 = 5u;
    check(dm2_v1_weather_gdat_environment_receipt(
              &loader, 3u, 73u, 1, 1, 0x40u, 0x80u, 1, 1,
              &receipt) == 0 && !receipt.valid,
          "missing selected original image rejects instead of falling back");
    entries[3].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;

    check(dm2_v1_weather_gdat_environment_receipt(
              &loader, 3u, 0u, 1, 1, 0x40u, 0x80u, 1, 1,
              &receipt) == 0 && !receipt.valid,
          "missing source map token rejects environment handoff");

    return failures ? 1 : 0;
}
