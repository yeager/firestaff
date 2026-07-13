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
    static const char *const source_text[6] = {
        "CD=6000;FW=8", "CD=6001;FW=2", "CD=6002;FW=64",
        "CD=6004;FW=32", "CD=6005;FW=0", "CD=6006;FW=8"
    };
    uint8_t raw[256];
    uint32_t offsets[12];
    uint32_t sizes[12];
    DM2_V1_GdatEntry entries[20];
    DM2_V1_AssetLoader loader;
    DM2_V1_WeatherGdatReceipt receipt;
    DM2_V1_WeatherCommandReceipt command;
    DM2_V1_WeatherOverlayPlan plan;
    DM2_V1_WeatherDrawContext draw_context;
    DM2_V1_WeatherDrawPlan draw_plan;
    static const uint8_t source_commands[6] = {
        0x67u, 0x68u, 0x69u, 0x6au, 0x6bu, 0x6cu
    };
    int i;
    size_t cursor = 0u;

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    memset(raw, 0, sizeof(raw));
    for (i = 0; i < 6; ++i) {
        sizes[i] = (uint32_t)strlen(source_text[i]) + 1u;
        offsets[i] = (uint32_t)cursor;
        memcpy(raw + cursor, source_text[i], sizes[i]);
        cursor += sizes[i];
        entries[i].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
        entries[i].cls2 = 3u;
        entries[i].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
        entries[i].cls4 = source_commands[i];
        entries[i].data_index = (uint16_t)i;
    }
    for (i = 0; i < 6; ++i) {
        uint16_t width = (uint16_t)(32 + i);
        uint16_t height = (uint16_t)(48 + i);
        offsets[6 + i] = (uint32_t)cursor;
        sizes[6 + i] = 26u;
        raw[cursor + 0u] = (uint8_t)width;
        raw[cursor + 1u] = (uint8_t)(width >> 8);
        raw[cursor + 2u] = (uint8_t)height;
        raw[cursor + 3u] = (uint8_t)(height >> 8);
        raw[cursor + 4u] = 4u;
        for (int palette = 0; palette < 16; ++palette) {
            raw[cursor + 10u + (size_t)palette] =
                (uint8_t)(0x20u + (unsigned int)i + (unsigned int)palette);
        }
        cursor += 26u;
        entries[6 + i].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
        entries[6 + i].cls2 = 3u;
        entries[6 + i].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
        entries[6 + i].cls4 = source_commands[i];
        entries[6 + i].data_index = (uint16_t)(6u + (unsigned int)i);
    }
    entries[12].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[12].cls2 = 3u;
    entries[12].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[12].cls4 = DM2_GDAT_GFXSET_MISTY_MAP;
    entries[12].data_index = 0x0042u;
    entries[13].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
    entries[13].cls2 = 3u;
    entries[13].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
    entries[13].cls4 = 0xfeu;
    entries[13].data_index = 0x02fdu; /* +2, -3 */
    for (i = 0; i < 6; ++i) {
        entries[14 + i].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
        entries[14 + i].cls2 = 3u;
        entries[14 + i].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
        entries[14 + i].cls4 = source_commands[i];
        entries[14 + i].data_index = (uint16_t)(0xff00u | (unsigned int)i);
    }
    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 20u;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 12u;
    loader.data = raw;
    loader.data_size = cursor;

    check(dm2_v1_weather_gdat_receipt(&loader, 3u, &receipt) &&
              receipt.valid && receipt.graphicsset == 3u &&
              receipt.misty_map == 0x0042u &&
              receipt.command_mask == 0x3fu && receipt.material_mask == 0x3fu &&
              receipt.receipt_hash != 0u,
          "weather receipt binds all six source dtText commands");
    check(receipt.graphicsset == 3u && receipt.receipt_hash != 0u &&
              receipt.material_mask == 0x3fu &&
              receipt.commands[0].local_palette_valid &&
              receipt.commands[5].local_palette_valid,
          "weather receipt exposes the bounded live-handoff identity");
    check(receipt.commands[3].command == 0x6au &&
              receipt.commands[3].raw_text == raw + offsets[3] &&
              receipt.commands[3].byte_count == sizes[3] &&
              receipt.commands[3].raw_hash != 0u &&
              receipt.commands[3].material_valid &&
              receipt.commands[3].rect_number == 6004u &&
              receipt.commands[3].flip_mode == 32u &&
              receipt.commands[3].image_present &&
              receipt.commands[3].image_field == 0x6au &&
              receipt.commands[3].query_metadata_valid &&
              receipt.commands[3].query_metadata.width == 35u &&
              receipt.commands[3].query_metadata.height == 51u &&
              receipt.commands[3].query_metadata.graphicsset_offset_present &&
              receipt.commands[3].query_metadata.image_offset_present &&
              receipt.commands[3].query_metadata.query_offset_x == 1 &&
              receipt.commands[3].query_metadata.query_offset_y == 0 &&
              receipt.commands[3].local_palette_valid &&
              receipt.commands[3].local_palette16[0] == 0x23u &&
              receipt.commands[3].local_palette16[15] == 0x32u &&
              receipt.commands[3].local_palette_hash != 0u,
          "weather receipt binds CMDSTR, IMG3 bounds, and local palette");
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
              plan.commands[0].image_width == 33u &&
              plan.commands[0].image_height == 49u &&
              plan.commands[0].query_offset_x == 1 &&
              plan.commands[0].query_offset_y == -2 &&
              plan.commands[1].command == 0x6bu &&
              plan.commands[1].slot_index == 1u &&
              plan.commands[1].rect_number == 6005u &&
              plan.commands[1].image_width == 36u &&
              plan.commands[1].query_offset_y == 1 &&
              plan.plan_hash != 0u,
          "weather plan preserves source cloud then rain material order");
    check(dm2_v1_weather_gdat_overlay_plan(&receipt, 0u, 0u, &plan) &&
              plan.valid && plan.command_count == 0u &&
              plan.required_mask == 0u && plan.material_mask == 0u,
          "clear weather has an explicit empty source command plan");

    memset(&draw_context, 0, sizeof(draw_context));
    draw_context.direction = 3u;
    draw_context.map_x = 1;
    draw_context.map_y = 2;
    draw_context.map_offset_x = 4;
    draw_context.map_offset_y = 5;
    draw_context.map_level = 6;
    draw_context.scene_flags = 2u;
    draw_context.player_moving = 1;
    draw_context.movement_offset_x = 7;
    draw_context.movement_offset_y = 9;
    draw_context.moving_horizon_offset_y = 11;
    check(dm2_v1_weather_gdat_draw_plan(&receipt.commands[1], &draw_context,
                                        &draw_plan) &&
              draw_plan.valid && draw_plan.command == 0x68u &&
              draw_plan.rect_number == 6001u &&
              draw_plan.image_field == 0x68u && !draw_plan.mirror_flip &&
              draw_plan.scale_x == 0x34u && draw_plan.scale_y == 0x34u &&
              draw_plan.draw_offset_x == 7 && draw_plan.draw_offset_y == 11 &&
              draw_plan.source_bounds_valid &&
              draw_plan.source_left == 1 && draw_plan.source_top == -2 &&
              draw_plan.source_right == 34 && draw_plan.source_bottom == 47 &&
              draw_plan.material_hash == receipt.commands[1].material_hash,
          "weather draw plan retains source IMG3 bounds and moving horizon transform");
    draw_context.map_x = 2;
    check(dm2_v1_weather_gdat_draw_plan(&receipt.commands[1], &draw_context,
                                        &draw_plan) && draw_plan.mirror_flip,
          "weather draw plan follows source FW=2 parity flip");
    draw_context.player_moving = 0;
    draw_context.scene_flags = 0x20u;
    draw_context.player_direction = 1u;
    check(dm2_v1_weather_gdat_draw_plan(&receipt.commands[3], &draw_context,
                                        &draw_plan) && draw_plan.mirror_flip &&
              draw_plan.rect_number == 6004u &&
              draw_plan.scale_x == 0x40u && draw_plan.scale_y == 0x40u &&
              draw_plan.draw_offset_x == 0 && draw_plan.draw_offset_y == 0,
          "weather draw plan follows source FW=32 direction flip");
    receipt.commands[3].material_valid = 0;
    check(!dm2_v1_weather_gdat_draw_plan(&receipt.commands[3], &draw_context,
                                         &draw_plan),
          "weather draw plan refuses an unverified image material");
    receipt.commands[3].material_valid = 1;

    sizes[9] = 10u;
    check(dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x6au,
                                               &command) &&
              command.image_present && command.query_metadata_valid &&
              !command.local_palette_valid && !command.material_valid,
          "weather command rejects an IMG3 without its local palette tail");
    sizes[9] = 26u;

    entries[4].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    check(!dm2_v1_weather_gdat_receipt(&loader, 3u, &receipt),
          "image record cannot masquerade as weather command text");
    check(!dm2_v1_weather_gdat_overlay_plan(&receipt, 0u, 0x80u, &plan),
          "weather plan refuses a receipt invalidated by wrong GDAT type");

    entries[4].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
    entries[17].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    check(dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x6au,
                                               &command) &&
              command.material_valid && command.query_metadata_valid &&
              !command.query_metadata.image_offset_present &&
              command.query_metadata.query_offset_x == 2 &&
              command.query_metadata.query_offset_y == -3,
          "weather command preserves source zero offset when field is absent");
    entries[17].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
    for (i = 0; i < 6; ++i) entries[6 + i].cls2 = 4u;
    check(dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x6au,
                                               &command) &&
              !command.material_valid &&
              dm2_v1_weather_gdat_receipt(&loader, 3u, &receipt) &&
              receipt.material_mask == 0u,
          "weather commands refuse images from a different graphics set");

    fprintf(stderr, "DM2 weather GDAT command receipt: %d failure(s)\n",
            failures);
    return failures != 0;
}
