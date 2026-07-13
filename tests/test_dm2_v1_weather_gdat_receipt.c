/* skproject c_weather.cpp provenance: weather selects environment dtText
 * commands 0x67..0x6c by MapGraphicsStyle.  They are never image fields. */
#include "dm2_v1_weather_gdat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *name)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    } else {
        fprintf(stderr, "PASS: %s\n", name);
    }
}

int main(void)
{
    uint32_t offsets[6] = { 0u, 1u, 3u, 6u, 10u, 15u };
    uint32_t sizes[6] = { 1u, 2u, 3u, 4u, 5u, 6u };
    uint8_t raw[21] = {
        0x11u, 0x22u, 0x23u, 0x31u, 0x32u, 0x33u,
        0x41u, 0x42u, 0x43u, 0x44u, 0x51u, 0x52u,
        0x53u, 0x54u, 0x55u, 0x61u, 0x62u, 0x63u,
        0x64u, 0x65u, 0x66u
    };
    DM2_V1_GdatEntry entries[7];
    DM2_V1_AssetLoader loader;
    DM2_V1_WeatherGdatReceipt receipt;
    DM2_V1_WeatherCommandReceipt command;
    static const uint8_t source_commands[6] = {
        0x67u, 0x68u, 0x69u, 0x6au, 0x6bu, 0x6cu
    };
    int i;

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    for (i = 0; i < 6; ++i) {
        entries[i].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
        entries[i].cls2 = 3u;
        entries[i].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
        entries[i].cls4 = source_commands[i];
        entries[i].data_index = (uint16_t)i;
    }
    entries[6].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[6].cls2 = 3u;
    entries[6].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[6].cls4 = DM2_GDAT_GFXSET_MISTY_MAP;
    entries[6].data_index = 0x0042u;
    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 7u;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 6u;
    loader.data = raw;
    loader.data_size = sizeof(raw);

    check(dm2_v1_weather_gdat_receipt(&loader, 3u, &receipt) &&
              receipt.valid && receipt.graphicsset == 3u &&
              receipt.misty_map == 0x0042u &&
              receipt.command_mask == 0x3fu && receipt.receipt_hash != 0u,
          "weather receipt binds all six source dtText commands");
    check(receipt.commands[3].command == 0x6au &&
              receipt.commands[3].raw_text == raw + offsets[3] &&
              receipt.commands[3].byte_count == sizes[3] &&
              receipt.commands[3].raw_hash != 0u,
          "weather receipt preserves raw source command provenance");
    check(dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x6cu,
                                               &command) &&
              command.raw_text == raw + offsets[5] &&
              command.byte_count == sizes[5] &&
              !dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x70u,
                                                    &command),
          "only c_weather source command range is accepted");
    check(dm2_v1_weather_gdat_cloud_command_for_level(0x0fu) == 0u &&
              dm2_v1_weather_gdat_cloud_command_for_level(0x10u) == 0x67u &&
              dm2_v1_weather_gdat_cloud_command_for_level(0x40u) == 0x68u &&
              dm2_v1_weather_gdat_cloud_command_for_level(0x80u) == 0x69u,
          "cloud thresholds match c_weather branch ordering");
    check(dm2_v1_weather_gdat_rain_command_for_level(0x3fu) == 0u &&
              dm2_v1_weather_gdat_rain_command_for_level(0x40u) == 0x6au &&
              dm2_v1_weather_gdat_rain_command_for_level(0x80u) == 0x6bu &&
              dm2_v1_weather_gdat_rain_command_for_level(0xc0u) == 0x6cu,
          "rain thresholds match c_weather branch ordering");

    entries[4].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    check(!dm2_v1_weather_gdat_receipt(&loader, 3u, &receipt),
          "image record cannot masquerade as weather command text");

    fprintf(stderr, "DM2 weather GDAT command receipt: %d failure(s)\n",
            failures);
    return failures != 0;
}
