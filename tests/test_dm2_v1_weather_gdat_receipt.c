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
    static const uint8_t raw[] =
        "CD=6000;FW=8\0"
        "CD=6001;FW=2\0"
        "CD=6002;FW=64\0"
        "CD=6004;FW=32\0"
        "CD=6005;FW=0\0"
        "CD=6006;FW=8\0";
    uint32_t offsets[6];
    uint32_t sizes[6];
    DM2_V1_GdatEntry entries[7];
    DM2_V1_AssetLoader loader;
    DM2_V1_WeatherGdatReceipt receipt;
    DM2_V1_WeatherCommandReceipt command;
    DM2_V1_WeatherOverlayPlan plan;
    static const uint8_t source_commands[6] = {
        0x67u, 0x68u, 0x69u, 0x6au, 0x6bu, 0x6cu
    };
    int i;

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    offsets[0] = 0u;
    for (i = 0; i < 6; ++i) {
        sizes[i] = (uint32_t)strlen((const char *)raw + offsets[i]) + 1u;
        if (i != 5) offsets[i + 1] = offsets[i] + sizes[i];
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
              receipt.command_mask == 0x3fu && receipt.material_mask == 0x3fu &&
              receipt.receipt_hash != 0u,
          "weather receipt binds all six source dtText commands");
    check(receipt.commands[3].command == 0x6au &&
              receipt.commands[3].raw_text == raw + offsets[3] &&
              receipt.commands[3].byte_count == sizes[3] &&
              receipt.commands[3].raw_hash != 0u &&
              receipt.commands[3].material_valid &&
              receipt.commands[3].rect_number == 6004u &&
              receipt.commands[3].flip_mode == 32u,
          "weather receipt decodes original CD/FW material command");
    check(dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x6cu,
                                               &command) &&
              command.raw_text == raw + offsets[5] &&
              command.byte_count == sizes[5] &&
              !dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x70u,
                                                    &command),
          "only c_weather source command range is accepted");
    {
        static const uint8_t repeated[] = "CD=6;noise;CD=42;FW=-2\0";
        int found = 0;
        int32_t value = 0;
        check(dm2_v1_weather_cmdstr_query(repeated, sizeof(repeated), "CD",
                                          &found, &value) && found &&
                  value == 642,
              "CMDSTR parser follows source repeated-digit accumulation");
        check(dm2_v1_weather_cmdstr_query(repeated, sizeof(repeated), "FW",
                                          &found, &value) && found &&
                  value == -2,
              "CMDSTR parser follows source signed decimal branch");
    }
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
    check(dm2_v1_weather_gdat_overlay_plan(&receipt, 0x40u, 0x80u, &plan) &&
              plan.valid && plan.command_count == 2u &&
              plan.required_mask ==
                  (DM2_V1_WEATHER_COMMAND_MASK(0x68u) |
                   DM2_V1_WEATHER_COMMAND_MASK(0x6bu)) &&
              plan.material_mask == plan.required_mask &&
              plan.commands[0].command == 0x68u &&
              plan.commands[0].slot_index == 0u &&
              plan.commands[0].rect_number == 6001u &&
              plan.commands[0].source_offset_x == 0 &&
              plan.commands[0].source_offset_y == 0 &&
              plan.commands[0].source_scale_x == 0x40u &&
              plan.commands[0].source_scale_y == 0x40u &&
              plan.commands[1].command == 0x6bu &&
              plan.commands[1].slot_index == 1u &&
              plan.commands[1].rect_number == 6005u &&
              plan.plan_hash != 0u,
          "weather plan preserves source cloud then rain material order");
    check(dm2_v1_weather_gdat_overlay_plan(&receipt, 0u, 0u, &plan) &&
              plan.valid && plan.command_count == 0u &&
              plan.required_mask == 0u && plan.material_mask == 0u,
          "clear weather has an explicit empty source command plan");

    entries[4].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    check(!dm2_v1_weather_gdat_receipt(&loader, 3u, &receipt),
          "image record cannot masquerade as weather command text");
    check(!dm2_v1_weather_gdat_overlay_plan(&receipt, 0u, 0x80u, &plan),
          "weather plan refuses a receipt invalidated by wrong GDAT type");

    fprintf(stderr, "DM2 weather GDAT command receipt: %d failure(s)\n",
            failures);
    return failures != 0;
}
