/* skproject c_weather.cpp provenance: weather selects environment dtText
 * commands 0x64..0x6c by MapGraphicsStyle (bolt 100+RAND16(3), cloud
 * 0x67..0x69, rain 0x6a..0x6c).  They are never image fields. */
#include "dm2_v1_weather_gdat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void put16le(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

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
    static const char *const source_text[9] = {
        "cd6002", "cd6002", "cd6002",
        "cd6000fw8", "cd6001fw2", "cd6002fw64",
        "cd6004fw32", "cd6005fw0", "cd6006fw8"
    };
    uint8_t raw[512];
    uint32_t offsets[18];
    uint32_t sizes[18];
    DM2_V1_GdatEntry entries[29];
    DM2_V1_AssetLoader loader;
    DM2_V1_WeatherGdatReceipt receipt;
    DM2_V1_DistantEnvironmentReceipt distant;
    DM2_V1_WeatherCommandReceipt command;
    DM2_V1_WeatherOverlayPlan plan;
    DM2_V1_WeatherDrawContext draw_context;
    DM2_V1_WeatherDrawPlan draw_plan;
    DM2_V1_WeatherDestinationClip destination_clip;
    DM2_V1_WeatherState restored_weather;
    DM2_V1_WeatherRestoredStateReceipt restored_state;
    DM2_V1_WeatherRendererReceipt renderer_receipt;
    uint8_t rect_table[76];
    static const uint8_t source_commands[9] = {
        0x64u, 0x65u, 0x66u, 0x67u, 0x68u, 0x69u, 0x6au, 0x6bu, 0x6cu
    };
    int i;
    size_t cursor = 0u;

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    memset(raw, 0, sizeof(raw));
    for (i = 0; i < 9; ++i) {
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
    for (i = 0; i < 9; ++i) {
        /* Minimal source-shaped IMG9 mode-2: 2x1, one literal flag byte and
         * two indexed pixels. The verified PC weather command family uses
         * this global-palette format. */
        offsets[9 + i] = (uint32_t)cursor;
        sizes[9 + i] = 11u;
        raw[cursor + 0u] = 2u;
        raw[cursor + 2u] = 1u;
        raw[cursor + 3u] = 0x7cu;
        raw[cursor + 4u] = 8u;
        raw[cursor + 6u] = 2u;
        raw[cursor + 8u] = 0xffu;
        raw[cursor + 9u] = (uint8_t)(0x12u + (unsigned int)i);
        raw[cursor + 10u] = (uint8_t)(0x22u + (unsigned int)i);
        cursor += 11u;
        entries[9 + i].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
        entries[9 + i].cls2 = 3u;
        entries[9 + i].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
        entries[9 + i].cls4 = source_commands[i];
        entries[9 + i].data_index = (uint16_t)(9u + (unsigned int)i);
    }
    entries[18].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[18].cls2 = 3u;
    entries[18].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[18].cls4 = DM2_GDAT_GFXSET_MISTY_MAP;
    entries[18].data_index = 0x0042u;
    entries[19].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
    entries[19].cls2 = 3u;
    entries[19].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
    entries[19].cls4 = 0xfeu;
    entries[19].data_index = 0x02fdu; /* +2, -3 */
    for (i = 0; i < 9; ++i) {
        entries[20 + i].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
        entries[20 + i].cls2 = 3u;
        entries[20 + i].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
        entries[20 + i].cls4 = source_commands[i];
        entries[20 + i].data_index = (uint16_t)(0xff00u | (unsigned int)i);
    }
    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 29u;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 18u;
    loader.data = raw;
    loader.data_size = cursor;

    check(dm2_v1_weather_gdat_receipt(&loader, 3u, &receipt) &&
              receipt.valid && receipt.graphicsset == 3u &&
              receipt.misty_map == 0x0042u &&
              receipt.command_mask == 0x1ffu &&
              receipt.material_mask == 0x1ffu &&
              receipt.receipt_hash != 0u,
          "weather receipt binds all nine source dtText commands");
    check(receipt.graphicsset == 3u && receipt.receipt_hash != 0u &&
              receipt.material_mask == 0x1ffu &&
              !receipt.commands[0].local_palette_valid &&
              receipt.commands[0].global_palette_identity_valid &&
              receipt.commands[8].global_palette_identity_valid,
          "weather receipt exposes the bounded live-handoff identity");
    check(receipt.commands[6].command == 0x6au &&
              receipt.commands[6].raw_text == raw + offsets[6] &&
              receipt.commands[6].byte_count == sizes[6] &&
              receipt.commands[6].raw_hash != 0u &&
              receipt.commands[6].material_valid &&
              receipt.commands[6].rect_number == 6004u &&
              receipt.commands[6].flip_mode == 32u &&
              receipt.commands[6].image_present &&
              receipt.commands[6].image_field == 0x6au &&
              receipt.commands[6].query_metadata_valid &&
              receipt.commands[6].query_metadata.width == 2u &&
              receipt.commands[6].query_metadata.height == 1u &&
              receipt.commands[6].query_metadata.graphicsset_offset_present &&
              receipt.commands[6].query_metadata.image_offset_present &&
              receipt.commands[6].query_metadata.query_offset_x == 1 &&
              receipt.commands[6].query_metadata.query_offset_y == 3 &&
              !receipt.commands[6].local_palette_valid &&
              receipt.commands[6].global_palette_identity_valid &&
              receipt.commands[6].palette_translation_count == 256u &&
              receipt.commands[6].global_palette_identity_hash != 0u &&
              receipt.commands[6].decoded_pixels_valid &&
              receipt.commands[6].decoded_width == 2u &&
              receipt.commands[6].decoded_height == 1u &&
              receipt.commands[6].decoded_format == DM2_IMG_FMT_IMG9 &&
              receipt.commands[6].decoded_pixel_count == 2u &&
              receipt.commands[6].decoded_pixels_hash != 0u,
          "weather receipt binds CMDSTR, decoded IMG9 pixels, and global palette");
    check(receipt.commands[0].command == 0x64u &&
              receipt.commands[0].material_valid &&
              receipt.commands[0].rect_number == 6002u &&
              receipt.commands[0].flip_mode == 0u &&
              receipt.commands[0].image_field == 0x64u &&
              receipt.commands[2].command == 0x66u &&
              receipt.commands[2].rect_number == 6002u,
          "lightning bolt commands bind their cd6002 material without FW");
    check(dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x6cu,
                                               &command) &&
              command.raw_text == raw + offsets[8] &&
              command.byte_count == sizes[8] &&
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
              plan.commands[0].image_width == 2u &&
              plan.commands[0].image_height == 1u &&
              plan.commands[0].query_offset_x == 1 &&
              plan.commands[0].query_offset_y == 1 &&
              plan.commands[1].command == 0x6bu &&
              plan.commands[1].slot_index == 1u &&
              plan.commands[1].rect_number == 6005u &&
              plan.commands[1].image_width == 2u &&
              plan.commands[1].query_offset_y == 4 &&
              plan.plan_hash != 0u,
          "weather plan preserves source cloud then rain material order");
    {
        const uint8_t live_slot[DM2_V1_DISTANT_ENVIRONMENT_BYTES] =
            { 0x68u, 2u, 3u, 4u, 0u, 0u, 0u, 0u, 0x40u, 0x40u };
        check(dm2_v1_weather_distant_environment_receipt(
                  &receipt, 0x68u, 0u, live_slot, &distant) &&
                  distant.valid && distant.command == 0x68u &&
                  distant.slot_index == 0u && distant.raw_hash != 0u,
              "live DistantEnvironment slot is retained only for source material");
        check(!dm2_v1_weather_distant_environment_receipt(
                  &receipt, 0x70u, 0u, live_slot, &distant),
              "unknown live weather command stays unavailable");
    }
    check(dm2_v1_weather_gdat_overlay_plan(&receipt, 0u, 0u, &plan) &&
              plan.valid && plan.command_count == 0u &&
              plan.required_mask == 0u && plan.material_mask == 0u,
          "clear weather has an explicit empty source command plan");

    memset(rect_table, 0, sizeof(rect_table));
    put16le(rect_table, 0xfc0du);
    put16le(rect_table + 2u, 2u);
    put16le(rect_table + 4u, 6000u);
    put16le(rect_table + 6u, 6006u);
    put16le(rect_table + 8u, 6100u);
    put16le(rect_table + 10u, 6100u);
    /* Rect 6001: bottom-centred anchor -> helper rect 6100. */
    rect_table[12u + 8u] = 7u;
    put16le(rect_table + 12u + 8u + 2u, 6100u);
    put16le(rect_table + 12u + 8u + 4u, 20u);
    put16le(rect_table + 12u + 8u + 6u, 60u);
    /* Rect 6005: rain shares the proven helper rectangle form. */
    rect_table[12u + 5u * 8u] = 7u;
    put16le(rect_table + 12u + 5u * 8u + 2u, 6100u);
    put16le(rect_table + 12u + 5u * 8u + 4u, 20u);
    put16le(rect_table + 12u + 5u * 8u + 6u, 60u);
    /* Rect 6002: lightning bolt shares the proven helper rectangle form. */
    rect_table[12u + 2u * 8u] = 7u;
    put16le(rect_table + 12u + 2u * 8u + 2u, 6100u);
    put16le(rect_table + 12u + 2u * 8u + 4u, 20u);
    put16le(rect_table + 12u + 2u * 8u + 6u, 60u);
    rect_table[68u] = 9u;
    put16le(rect_table + 68u + 4u, 40u);
    put16le(rect_table + 68u + 6u, 20u);
    check(dm2_v1_weather_gdat_destination_clip(
              rect_table, sizeof(rect_table), &receipt.commands[4],
              &destination_clip) && destination_clip.valid &&
              destination_clip.x == 0 && destination_clip.y == 82 &&
              destination_clip.w == 40 && destination_clip.h == 20 &&
              destination_clip.table_hash != 0u,
          "weather CD resolves through the source dt04 destination clip");
    receipt.commands[4].material_valid = 0;
    check(!dm2_v1_weather_gdat_destination_clip(
              rect_table, sizeof(rect_table), &receipt.commands[4],
              &destination_clip),
          "weather destination clip rejects an unverified image/palette pair");
    receipt.commands[4].material_valid = 1;

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
    check(dm2_v1_weather_gdat_draw_plan(&receipt.commands[4], &draw_context,
                                        &draw_plan) &&
              draw_plan.valid && draw_plan.command == 0x68u &&
              draw_plan.rect_number == 6001u &&
              draw_plan.image_field == 0x68u && !draw_plan.mirror_flip &&
              draw_plan.scale_x == 0x34u && draw_plan.scale_y == 0x34u &&
              draw_plan.draw_offset_x == 7 && draw_plan.draw_offset_y == 11 &&
              draw_plan.source_bounds_valid &&
              draw_plan.source_left == 1 && draw_plan.source_top == 1 &&
              draw_plan.source_right == 3 && draw_plan.source_bottom == 2 &&
              draw_plan.material_hash == receipt.commands[4].material_hash,
          "weather draw plan retains source IMG9 bounds and moving horizon transform");
    draw_context.map_x = 2;
    check(dm2_v1_weather_gdat_draw_plan(&receipt.commands[4], &draw_context,
                                        &draw_plan) && draw_plan.mirror_flip,
          "weather draw plan follows source FW=2 parity flip");
    {
        const uint8_t dynamic_slot[DM2_V1_DISTANT_ENVIRONMENT_BYTES] = {
            0x68u, 2u, 0x71u, 0x17u, 20u, 0u, 10u, 0u, 0x20u, 0x30u
        };

        draw_context.map_x = 1;
        draw_context.moving_other_offset_y = 13;
        check(dm2_v1_weather_distant_environment_receipt(
                  &receipt, 0x68u, 0u, dynamic_slot, &distant) &&
                  dm2_v1_weather_gdat_draw_plan_from_distant_environment(
                      &receipt.commands[4], &distant, &draw_context,
                      &draw_plan) && draw_plan.valid && !draw_plan.mirror_flip &&
                  draw_plan.scale_x == 26u && draw_plan.scale_y == 39u &&
                  draw_plan.draw_offset_x == 16 && draw_plan.draw_offset_y == 21,
              "live DistantEnvironment w4/w6/b8/b9 control the weather blit");
        distant.raw[1] = 8u;
        check(!dm2_v1_weather_gdat_draw_plan_from_distant_environment(
                  &receipt.commands[4], &distant, &draw_context, &draw_plan),
              "weather draw rejects a live slot with mismatched cmFW");
    }
    draw_context.player_moving = 0;
    draw_context.scene_flags = 0x20u;
    draw_context.player_direction = 1u;
    check(dm2_v1_weather_gdat_draw_plan(&receipt.commands[6], &draw_context,
                                        &draw_plan) && draw_plan.mirror_flip &&
              draw_plan.rect_number == 6004u &&
              draw_plan.scale_x == 0x40u && draw_plan.scale_y == 0x40u &&
              draw_plan.draw_offset_x == 0 && draw_plan.draw_offset_y == 0,
          "weather draw plan follows source FW=32 direction flip");
    receipt.commands[6].material_valid = 0;
    check(!dm2_v1_weather_gdat_draw_plan(&receipt.commands[6], &draw_context,
                                         &draw_plan),
          "weather draw plan refuses an unverified image material");
    receipt.commands[6].material_valid = 1;

    dm2_v1_weather_init(&restored_weather);
    check(!dm2_v1_weather_restored_state_receipt(&restored_weather,
                                                 &restored_state),
          "fresh weather state rejects an invented environment clock");
    check(dm2_v1_weather_sky_color(&restored_weather) == -1,
          "fresh weather state cannot manufacture a sky colour");
    dm2_v1_weather_advance_time(&restored_weather, 1);
    check(restored_weather.time_of_day == DM2_TIME_UNKNOWN,
          "unknown environment clock cannot advance from a fixture value");
    restored_weather.weather = DM2_WEATHER_RAIN;
    restored_weather.weather_intensity = 128;
    restored_weather.weather_seed = 0x4a3d7f01u;
    restored_weather.time_of_day = 721;
    restored_weather.time_fraction = 721.0f / 1440.0f;
    check(dm2_v1_weather_restored_state_receipt(&restored_weather,
                                                &restored_state) &&
              restored_state.valid && restored_state.weather == DM2_WEATHER_RAIN &&
              restored_state.intensity == 128u && restored_state.time_of_day == 721u &&
              restored_state.weather_seed == 0x4a3d7f01u &&
              restored_state.state_hash != 0u,
          "restored weather state carries only validated runtime fields");
    check(dm2_v1_weather_sky_color(&restored_weather) == -1,
          "a restored clock cannot replace GDAT-backed outdoor material");
    check(dm2_v1_weather_particle_count(&restored_weather) == 0,
          "a restored weather enum cannot manufacture particle multiplicity");
    {
        /* c_weather.cpp:441-474 — the bolt slot's byte 1 is the live
         * RANDDIR value the source writes after a successful retrieve,
         * not the GDAT FW key (bolt text "cd6002" has none). */
        uint8_t bolt_slot[DM2_V1_DISTANT_ENVIRONMENT_BYTES] = {
            0x64u, 2u, 0x72u, 0x17u, 0u, 0u, 0u, 0u, 0x40u, 0x40u
        };

        check(dm2_v1_weather_distant_environment_receipt(
                  &receipt, 0x64u, 0u, bolt_slot, &distant) &&
                  dm2_v1_weather_gdat_draw_plan_from_distant_environment(
                      &receipt.commands[0], &distant, &draw_context,
                      &draw_plan) && draw_plan.valid &&
                  draw_plan.command == 0x64u &&
                  draw_plan.rect_number == 6002u &&
                  draw_plan.mirror_flip &&
                  draw_plan.material_hash ==
                      receipt.commands[0].material_hash,
              "lightning bolt slot binds RANDDIR byte to cd6002 material");
        bolt_slot[1] = 3u;
        check(dm2_v1_weather_distant_environment_receipt(
                  &receipt, 0x64u, 0u, bolt_slot, &distant) &&
                  dm2_v1_weather_gdat_draw_plan_from_distant_environment(
                      &receipt.commands[0], &distant, &draw_context,
                      &draw_plan) && draw_plan.valid && !draw_plan.mirror_flip,
              "bolt RANDDIR 3 draws unflipped like the source");
        bolt_slot[1] = 4u;
        check(dm2_v1_weather_distant_environment_receipt(
                  &receipt, 0x64u, 0u, bolt_slot, &distant) &&
                  !dm2_v1_weather_gdat_draw_plan_from_distant_environment(
                      &receipt.commands[0], &distant, &draw_context, &draw_plan),
              "bolt slot rejects a byte 1 outside the RANDDIR range");
    }
    {
        const uint8_t source_slots[3][DM2_V1_DISTANT_ENVIRONMENT_BYTES] = {
            { 0x68u, 2u, 0x71u, 0x17u, 0u, 0u, 0u, 0u, 0x40u, 0x40u },
            { 0x6bu, 0u, 0x75u, 0x17u, 0u, 0u, 0u, 0u, 0x40u, 0x40u },
            { 0x65u, 1u, 0x72u, 0x17u, 0u, 0u, 0u, 0u, 0x40u, 0x40u }
        };
        DM2_V1_DistantEnvironmentReceipt source_receipts[3];

        check(dm2_v1_weather_distant_environment_receipt(
                  &receipt, 0x68u, 0u, source_slots[0], &source_receipts[0]) &&
                  dm2_v1_weather_distant_environment_receipt(
                  &receipt, 0x6bu, 1u, source_slots[1], &source_receipts[1]) &&
                  dm2_v1_weather_distant_environment_receipt(
                  &receipt, 0x65u, 2u, source_slots[2], &source_receipts[2]) &&
                  dm2_v1_weather_gdat_renderer_receipt(
                  &restored_state, &receipt, source_receipts, 3u, &draw_context,
                  rect_table, sizeof(rect_table), &renderer_receipt) &&
                  renderer_receipt.valid &&
                  renderer_receipt.command_count == 3u &&
                  renderer_receipt.draws[0].command == 0x68u &&
                  renderer_receipt.draws[1].command == 0x6bu &&
                  renderer_receipt.draws[2].command == 0x65u &&
                  renderer_receipt.clips[0].valid &&
                  renderer_receipt.clips[1].valid &&
                  renderer_receipt.clips[2].valid &&
                  renderer_receipt.renderer_hash != 0u,
              "restored weather binds cloud, rain, and bolt slots to renderer receipts");
        source_receipts[1].raw[0] = 0x6au;
        check(!dm2_v1_weather_gdat_renderer_receipt(
                  &restored_state, &receipt, source_receipts, 3u, &draw_context,
                  rect_table, sizeof(rect_table), &renderer_receipt),
              "renderer receipt rejects a mismatched source command byte");
    }
    restored_weather.weather = DM2_WEATHER_COUNT;
    check(!dm2_v1_weather_restored_state_receipt(&restored_weather,
                                                 &restored_state),
          "invalid restored weather state cannot reach renderer receipt");

    {
        /* SkWinCore::QUERY_GDAT_TEXT (2636:0377): with dtWordValue(0,0,0)
         * bit 3 set, command text decodes as (b ^ 0xFF) - i.  Encode the
         * real bolt text "cd6002" and prove the receipt parses the decoded
         * form while keeping the raw-byte identity. */
        static const char plain[] = "cd6002";
        uint8_t enc_raw[64];
        uint32_t enc_offsets[2];
        uint32_t enc_sizes[2];
        DM2_V1_GdatEntry enc_entries[3];
        DM2_V1_AssetLoader enc_loader;
        size_t plain_size = sizeof(plain);
        size_t k;

        memset(enc_raw, 0, sizeof(enc_raw));
        for (k = 0u; k < plain_size; ++k) {
            enc_raw[k] = (uint8_t)(((uint8_t)plain[k] + (uint8_t)k) ^ 0xFFu);
        }
        /* Minimal IMG9 mode-2 record, same source format as above. */
        enc_offsets[1] = 32u;
        enc_sizes[1] = 11u;
        enc_raw[32u] = 2u;
        enc_raw[34u] = 1u;
        enc_raw[35u] = 0x7cu;
        enc_raw[36u] = 8u;
        enc_raw[38u] = 2u;
        enc_raw[40u] = 0xffu;
        enc_raw[41u] = 0x55u;
        enc_raw[42u] = 0x65u;
        enc_offsets[0] = 0u;
        enc_sizes[0] = (uint32_t)plain_size;
        memset(enc_entries, 0, sizeof(enc_entries));
        enc_entries[0].cls1 = 0u; /* GDAT header word: text encoded */
        enc_entries[0].cls2 = 0u;
        enc_entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
        enc_entries[0].cls4 = 0u;
        enc_entries[0].data_index = 0x8u;
        enc_entries[1].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
        enc_entries[1].cls2 = 3u;
        enc_entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
        enc_entries[1].cls4 = 0x64u;
        enc_entries[1].data_index = 0u;
        enc_entries[2].cls1 = DM2_GDAT_CATEGORY_ENVIRONMENT;
        enc_entries[2].cls2 = 3u;
        enc_entries[2].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
        enc_entries[2].cls4 = 0x64u;
        enc_entries[2].data_index = 1u;
        memset(&enc_loader, 0, sizeof(enc_loader));
        enc_loader.loaded = 1;
        enc_loader.entries = enc_entries;
        enc_loader.entry_count = 3u;
        enc_loader.raw_offsets = enc_offsets;
        enc_loader.raw_sizes = enc_sizes;
        enc_loader.raw_data_count = 2u;
        enc_loader.data = enc_raw;
        enc_loader.data_size = sizeof(enc_raw);
        check(dm2_v1_weather_gdat_command_receipt(&enc_loader, 3u, 0x64u,
                                                   &command) &&
                  command.raw_text == enc_raw &&
                  command.byte_count == plain_size &&
                  command.decoded_text_size == plain_size &&
                  command.decoded_text[0] == 'c' &&
                  command.decoded_text[1] == 'd' &&
                  command.decoded_text_hash != 0u &&
                  command.material_valid &&
                  command.rect_number == 6002u && command.flip_mode == 0u,
              "encoded GDAT command text decodes to its cd6002 material");
    }

    sizes[15] = 10u;
    check(dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x6au,
                                               &command) &&
              command.image_present && command.query_metadata_valid &&
              !command.global_palette_identity_valid && !command.material_valid,
          "weather command rejects a truncated IMG9 payload");
    sizes[15] = 11u;

    raw[offsets[15] + 0u] = 100u;
    check(dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x6au,
                                               &command) &&
              command.image_present && command.query_metadata_valid &&
              !command.decoded_pixels_valid &&
              !command.material_valid,
          "weather command rejects an image whose IMG9 pixels cannot decode");
    raw[offsets[15] + 0u] = 2u;

    entries[7].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    check(!dm2_v1_weather_gdat_receipt(&loader, 3u, &receipt),
          "image record cannot masquerade as weather command text");
    check(!dm2_v1_weather_gdat_overlay_plan(&receipt, 0u, 0x80u, &plan),
          "weather plan refuses a receipt invalidated by wrong GDAT type");

    entries[7].cls3 = DM2_GDAT_ENTRY_TYPE_TEXT;
    entries[26].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    check(dm2_v1_weather_gdat_command_receipt(&loader, 3u, 0x6au,
                                               &command) &&
              command.material_valid && command.query_metadata_valid &&
              !command.query_metadata.image_offset_present &&
              command.query_metadata.query_offset_x == 2 &&
              command.query_metadata.query_offset_y == -3,
          "weather command preserves source zero offset when field is absent");
    entries[26].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
    for (i = 0; i < 9; ++i) entries[9 + i].cls2 = 4u;
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
