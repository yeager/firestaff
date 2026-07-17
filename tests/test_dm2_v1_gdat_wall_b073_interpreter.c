#include "dm2_v1_gdat_wall_b073_interpreter.h"

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
    uint8_t raw7[518];
    uint8_t cache[256];
    uint8_t saved[256];
    uint32_t offset = 0u, size = sizeof(raw7);
    DM2_V1_GdatEntry entry = { 1u, 0u, 7u, 2u, 0u, 0u, 0u };
    DM2_V1_AssetLoader loader = { 0 };
    DM2_V1_GdatB073InputReceipt input = { 0 };
    DM2_V1_GdatWallM11CommandPlan wall = { 0 };
    DM2_V1_GdatWallB073Raw7LoaderReceipt raw7_receipt;
    DM2_V1_GdatWallB073InterpreterReceipt interpreter;
    DM2_V1_GdatWallB073OutputReceipt output;
    uint8_t pixel = 0u;
    uint16_t i;
    int ok;

    memset(raw7, 0, sizeof(raw7));
    raw7[0] = 1u; raw7[1] = 2u;
    raw7[2] = 0u; raw7[3] = 63u;
    raw7[4] = 0x20u; raw7[5] = 0x21u;
    for (i = 0u; i < 256u; ++i) {
        raw7[6u + (size_t)i * 2u] = 0u;
        raw7[6u + (size_t)i * 2u + 1u] = 0u;
        cache[i] = (uint8_t)i;
    }
    loader.loaded = 1; loader.data = raw7; loader.data_size = sizeof(raw7);
    loader.entries = &entry; loader.entry_count = 1u;
    loader.raw_offsets = &offset; loader.raw_sizes = &size; loader.raw_data_count = 1u;
    input.valid = input.no_draw = 1; input.identity_hash = 1u;
    input.input.palette_identity = hash_bytes(cache, 16u);
    input.input.raw7_identity = hash_bytes(raw7, sizeof(raw7));
    input.input.lookup_identity = 1u; input.input.traversal_identity = 1u;
    input.input.alpha_mask = 0xffu; input.input.colors = 16u;
    input.input.light = 0u; input.input.cache_owned = 1;
    input.input.cache_allocation = 9u;
    wall.valid = 1; wall.command_count = 1u; wall.command_hash = 2u;
    wall.commands[0].pixels = &pixel; wall.commands[0].palette_hash = input.input.palette_identity;
    memcpy(wall.commands[0].palette16, cache, 16u);
    wall.commands[0].raw_hash = wall.commands[0].decoded_hash = 3u;
    wall.commands[0].material_receipt_hash = 4u;
    ok = dm2_v1_gdat_wall_b073_raw7_loader_receipt_build(
        &loader, &input, &wall, 0u, 9u, 7u, &raw7_receipt);
    ok &= dm2_v1_gdat_wall_b073_interpreter_build(&input, &wall, 0u,
        &raw7_receipt, cache, sizeof(cache), 9u, 7u, &interpreter, &output) &&
        interpreter.valid && interpreter.no_draw && output.valid && output.no_draw &&
        interpreter.output_cache_hash == hash_bytes(cache, sizeof(cache));
    for (i = 0u; i < 256u; ++i) ok &= cache[i] == 0x20u;

    /* The alpha-special case walks to the adjacent output entry. */
    raw7[4] = 0x20u; raw7[5] = 0x21u;
    input.input.alpha_mask = 0x20u;
    input.input.raw7_identity = hash_bytes(raw7, sizeof(raw7));
    memcpy(cache, wall.commands[0].palette16, 16u);
    for (i = 16u; i < 256u; ++i) cache[i] = (uint8_t)i;
    ok &= dm2_v1_gdat_wall_b073_raw7_loader_receipt_build(
        &loader, &input, &wall, 0u, 9u, 7u, &raw7_receipt) &&
        dm2_v1_gdat_wall_b073_interpreter_build(&input, &wall, 0u,
            &raw7_receipt, cache, sizeof(cache), 9u, 7u, &interpreter, &output);
    for (i = 0u; i < 256u; ++i) ok &= cache[i] == 0x21u;

    memcpy(saved, cache, sizeof(saved));
    ++raw7_receipt.raw7_hash;
    ok &= !dm2_v1_gdat_wall_b073_interpreter_build(&input, &wall, 0u,
        &raw7_receipt, cache, sizeof(cache), 9u, 7u, &interpreter, &output) &&
        memcmp(cache, saved, sizeof(cache)) == 0;
    --raw7_receipt.raw7_hash;
    ok &= !dm2_v1_gdat_wall_b073_interpreter_build(&input, &wall, 0u,
        &raw7_receipt, cache, 16u, 9u, 7u, &interpreter, &output);
    ++wall.commands[0].palette16[0];
    ok &= !dm2_v1_gdat_wall_b073_interpreter_build(&input, &wall, 0u,
        &raw7_receipt, cache, sizeof(cache), 9u, 7u, &interpreter, &output);

    printf("%s dm2_v1_gdat_wall_b073_interpreter\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
