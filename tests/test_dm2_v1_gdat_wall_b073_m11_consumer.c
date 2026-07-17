#include "dm2_v1_gdat_wall_b073_m11_consumer.h"

#include <stdio.h>
#include <string.h>

static uint32_t hash_bytes(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    while (count-- != 0u) { hash ^= *bytes++; hash *= 16777619u; }
    return hash;
}

int main(void)
{
    DM2_V1_GdatB073InputReceipt input = { 0 };
    DM2_V1_GdatWallM11CommandPlan wall = { 0 };
    DM2_V1_GdatWallB073InterpreterReceipt interpreter = { 0 };
    DM2_V1_GdatWallB073OutputReceipt output = { 0 };
    DM2_V1_GdatWallTrimReceipt trim = { 0 };
    DM2_V1_GdatWallTrimM11Receipt trim_m11 = { 0 };
    DM2_V1_Dm2ViewportM11CompositionReceipt composition = { 0 };
    DM2_V1_GdatWallB073M11Receipt receipt;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t pixels[4] = { 1u, 2u, 3u, 4u };
    uint8_t palette[256];
    uint16_t i;
    int positive, hflip, flip_reject, cache_drift, surface_drift, composition_drift;

    memset(framebuffer, 0xa5, sizeof(framebuffer));
    for (i = 0u; i < 256u; ++i) palette[i] = (uint8_t)(i ^ 0x5au);
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    input.valid = input.no_draw = 1; input.identity_hash = 1u;
    input.input.alpha_mask = 2u; input.input.cache_owned = 1;
    wall.valid = 1; wall.command_count = 1u; wall.command_hash = 2u;
    wall.commands[0].pixels = pixels; wall.commands[0].width = wall.commands[0].height = 2u;
    wall.commands[0].source_width = wall.commands[0].source_height = 2u;
    wall.commands[0].destination_width = wall.commands[0].destination_height = 2u;
    wall.commands[0].destination_x = 10u; wall.commands[0].destination_y = 20u;
    wall.commands[0].view_square = 1u; wall.commands[0].rect_number = 0x2bfu;
    wall.commands[0].raw_hash = wall.commands[0].decoded_hash = 3u;
    wall.commands[0].palette_hash = hash_bytes(wall.commands[0].palette16, 16u);
    wall.commands[0].material_receipt_hash = wall.commands[0].geometry_hash = 4u;
    input.input.palette_identity = wall.commands[0].palette_hash;
    interpreter.valid = interpreter.no_draw = 1; interpreter.command_index = 0u;
    interpreter.wall_hash = 2u; interpreter.cache_palette_bytes = palette;
    interpreter.cache_palette_bytes_count = 256u; interpreter.cache_allocation = 9u;
    interpreter.cache_identity = 7u; interpreter.output_cache_hash = hash_bytes(palette, 256u);
    interpreter.identity_hash = 5u;
    output.valid = output.no_draw = 1; output.command_index = 0u; output.wall_hash = 2u;
    output.cache_palette_bytes = palette; output.cache_palette_bytes_count = 256u;
    output.cache_allocation = 9u; output.cache_identity = 7u; output.identity_hash = 6u;
    trim.valid = trim.no_draw = 1; trim.identity_hash = 7u; trim.left = 5u; trim.top = 6u;
    trim.surface = viewport.surface_snapshot;
    trim_m11.valid = trim_m11.no_draw = 1; trim_m11.normal_scale = 1u;
    trim_m11.view_square = 1u; trim_m11.rect_number = 0x2bfu;
    trim_m11.wall_material_hash = 2u; trim_m11.composition_identity_hash = 8u;
    trim_m11.identity_hash = 9u;
    composition.valid = composition.no_draw = 1; composition.identity_hash = 8u;
    composition.session_identity = composition.data_epoch = 1u;
    composition.surface_before = composition.surface_after = viewport.surface_snapshot;
    positive = dm2_v1_gdat_wall_b073_m11_receipt_build(&input, &wall, 0u,
        &interpreter, &output, &trim, &trim_m11, &composition, &viewport, &receipt) &&
        dm2_v1_gdat_wall_b073_m11_consume(&receipt, &input, &wall, &interpreter,
            &output, &trim, &trim_m11, &composition, &viewport) &&
        framebuffer[20u * DM2_VP_WIDTH + 10u] == palette[1u] &&
        framebuffer[20u * DM2_VP_WIDTH + 11u] == 0xa5u &&
        framebuffer[21u * DM2_VP_WIDTH + 10u] == palette[3u] &&
        framebuffer[21u * DM2_VP_WIDTH + 11u] == palette[4u];
    memset(framebuffer, 0xa5, sizeof(framebuffer));
    wall.commands[0].mirror_flip = 1u;
    trim_m11.source_flip = 1u;
    hflip = dm2_v1_gdat_wall_b073_m11_receipt_build(&input, &wall, 0u,
        &interpreter, &output, &trim, &trim_m11, &composition, &viewport, &receipt) &&
        receipt.source_flip == 1u &&
        dm2_v1_gdat_wall_b073_m11_consume(&receipt, &input, &wall, &interpreter,
            &output, &trim, &trim_m11, &composition, &viewport) &&
        framebuffer[20u * DM2_VP_WIDTH + 10u] == 0xa5u &&
        framebuffer[20u * DM2_VP_WIDTH + 11u] == palette[1u] &&
        framebuffer[21u * DM2_VP_WIDTH + 10u] == palette[4u] &&
        framebuffer[21u * DM2_VP_WIDTH + 11u] == palette[3u];
    wall.commands[0].mirror_flip = 2u;
    flip_reject = !dm2_v1_gdat_wall_b073_m11_receipt_build(&input, &wall, 0u,
        &interpreter, &output, &trim, &trim_m11, &composition, &viewport, &receipt);
    wall.commands[0].mirror_flip = 1u;
    ++palette[1u];
    cache_drift = !dm2_v1_gdat_wall_b073_m11_consume(&receipt, &input, &wall,
        &interpreter, &output, &trim, &trim_m11, &composition, &viewport) &&
        framebuffer[20u * DM2_VP_WIDTH + 10u] != palette[1u];
    --palette[1u];
    ++viewport.surface_snapshot.generation;
    surface_drift = !dm2_v1_gdat_wall_b073_m11_consume(&receipt, &input, &wall,
        &interpreter, &output, &trim, &trim_m11, &composition, &viewport);
    --viewport.surface_snapshot.generation;
    ++composition.identity_hash;
    composition_drift = !dm2_v1_gdat_wall_b073_m11_consume(&receipt, &input,
        &wall, &interpreter, &output, &trim, &trim_m11, &composition, &viewport);
    printf("positive=%d hflip=%d flip_reject=%d cache=%d surface=%d composition=%d\n",
        positive, hflip, flip_reject, cache_drift, surface_drift, composition_drift);
    return positive && hflip && flip_reject && cache_drift && surface_drift &&
        composition_drift ? 0 : 1;
}
