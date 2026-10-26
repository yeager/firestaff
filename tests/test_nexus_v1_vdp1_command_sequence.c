#include "nexus_v1_vdp1_command_sequence.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void wl16(uint8_t *p, unsigned value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
}

static int read_external_snapshot(const char *path, uint8_t *buffer)
{
    FILE *file;
    size_t read_count;
    if (!path || !buffer) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    read_count = fread(buffer, 1U, NEXUS_V1_VDP1_VRAM_BYTES, file);
    fclose(file);
    return read_count == NEXUS_V1_VDP1_VRAM_BYTES;
}

int main(void)
{
    uint8_t *vram = (uint8_t *)calloc(1U, NEXUS_V1_VDP1_VRAM_BYTES);
    Nexus_V1_Vdp1CommandSequenceInput input;
    Nexus_V1_Vdp1CommandSequenceReceipt receipt;
    uint32_t start = 0x100U;
    const char *external_path = getenv("FIRESTAFF_NEXUS_VDP1_VRAM");
    const char *external_copr = getenv("FIRESTAFF_NEXUS_VDP1_COPR");
    const char *trace_path = getenv("FIRESTAFF_NEXUS_VDP1_TRACE");

    if (trace_path) {
        uint8_t commands[178U * NEXUS_V1_VDP1_COMMAND_BYTES];
        uint32_t offsets[178];
        Nexus_V1_Vdp1CommandSequenceReceipt traced;
        FILE *file = fopen(trace_path, "rb");
        int i;
        if (!file) { free(vram); return 1; }
        for (i = 0; i < 178; ++i) {
            uint8_t address[4];
            if (fread(address, 1U, 4U, file) != 4U ||
                fread(commands + (size_t)i * NEXUS_V1_VDP1_COMMAND_BYTES,
                      1U, NEXUS_V1_VDP1_COMMAND_BYTES, file) != NEXUS_V1_VDP1_COMMAND_BYTES) {
                fclose(file); free(vram); return 1;
            }
            offsets[i] = (uint32_t)address[0] | ((uint32_t)address[1] << 8U) |
                ((uint32_t)address[2] << 16U) | ((uint32_t)address[3] << 24U);
        }
        if (fgetc(file) != EOF) { fclose(file); free(vram); return 1; }
        fclose(file);
        if (!nexus_v1_vdp1_command_sequence_trace(commands, offsets, 178, &traced) ||
            traced.start_byte_offset != 0U || traced.end_byte_offset != 0xd9a0U ||
            !traced.semantic_admission_blocked) {
            fprintf(stderr, "FAIL: authentic Ymir VDP1 command trace\n");
            free(vram); return 1;
        }
        free(vram);
        puts("test_nexus_v1_vdp1_command_sequence: AUTHENTIC TRACE PASS");
        return 0;
    }

    {
        uint8_t commands[5U * NEXUS_V1_VDP1_COMMAND_BYTES] = {0};
        uint32_t offsets[5] = {0x100U, 0x120U, 0x200U, 0x140U, 0x160U};
        Nexus_V1_Vdp1CommandSequenceReceipt traced;
        wl16(commands, 0x000aU);
        wl16(commands + 12U, 160U);
        wl16(commands + 14U, 112U);
        wl16(commands + 32U, 0x2002U);
        wl16(commands + 34U, 0x200U >> 3U);
        wl16(commands + 64U, 0x3002U);
        wl16(commands + 96U, 0x400aU);
        wl16(commands + 108U, 7U);
        wl16(commands + 128U, 0x8000U);
        if (!nexus_v1_vdp1_command_sequence_trace(commands, offsets, 5, &traced) ||
            traced.command_count != 5 || traced.draw_count != 2 ||
            traced.local_coordinate_count != 1 || traced.command_origin_x[3] != 160 ||
            traced.command_origin_y[3] != 112 || !traced.semantic_admission_blocked) {
            fprintf(stderr, "FAIL: traced VDP1 call, return or skipped local command\n");
            free(vram);
            return 1;
        }
        offsets[2] = 0x220U;
        if (nexus_v1_vdp1_command_sequence_trace(commands, offsets, 5, &traced) ||
            traced.valid) {
            fprintf(stderr, "FAIL: traced VDP1 accepted invalid jump\n");
            free(vram);
            return 1;
        }
        /* VDP1 retains one return address; a nested CALL does not replace it. */
        offsets[2] = 0x200U;
        offsets[3] = 0x300U;
        offsets[4] = 0x140U;
        wl16(commands + 64U, 0x2002U);
        wl16(commands + 66U, (0x300U >> 3U) | 3U);
        wl16(commands + 96U, 0x3002U);
        if (!nexus_v1_vdp1_command_sequence_trace(commands, offsets, 5, &traced)) {
            fprintf(stderr, "FAIL: VDP1 nested call return-address semantics\n");
            free(vram); return 1;
        }
        /* A RETURN without a saved address advances to the next command. */
        wl16(commands + 32U, 0x3002U);
        wl16(commands + 64U, 0x8000U);
        offsets[2] = 0x140U;
        if (!nexus_v1_vdp1_command_sequence_trace(commands, offsets, 3, &traced)) {
            fprintf(stderr, "FAIL: VDP1 return without call\n");
            free(vram); return 1;
        }
        wl16(commands + 12U, 0x0400U);
        wl16(commands + 14U, 0x1fffU);
        if (!nexus_v1_vdp1_command_sequence_trace(commands, offsets, 3, &traced) ||
            traced.display_origin_x != 1024 || traced.display_origin_y != -1) {
            fprintf(stderr, "FAIL: VDP1 local coordinates are not signed 13-bit\n");
            free(vram); return 1;
        }
    }

    if (!vram) return 1;
    if (external_path) {
        Nexus_V1_Vdp1CommandSequenceInput external_input;
        Nexus_V1_Vdp1CommandSequenceReceipt external_receipt;
        unsigned long copr = external_copr ? strtoul(external_copr, NULL, 0) : 0UL;
        if (!read_external_snapshot(external_path, vram)) {
            fprintf(stderr, "FAIL: external VDP1 snapshot could not be read\n");
            free(vram);
            return 1;
        }
        memset(&external_input, 0, sizeof(external_input));
        external_input.vdp1_vram = vram;
        external_input.vdp1_vram_size = NEXUS_V1_VDP1_VRAM_BYTES;
        external_input.copr_word = (uint32_t)copr;
        if (!nexus_v1_vdp1_command_sequence_frame(
                &external_input, &external_receipt) ||
            external_receipt.command_count != 220 ||
            external_receipt.draw_count != 215 ||
            external_receipt.user_clip_count != 2 ||
            external_receipt.local_coordinate_count != 2 ||
            external_receipt.display_origin_x != 160 ||
            external_receipt.display_origin_y != 112 ||
            !external_receipt.semantic_admission_blocked) {
            fprintf(stderr, "FAIL: authentic VDP1 command sequence receipt\n");
            free(vram);
            return 1;
        }
        free(vram);
        puts("test_nexus_v1_vdp1_command_sequence: AUTHENTIC PASS");
        return 0;
    }
    /* System clip -> local coordinate -> mode-1 draw -> local coordinate
     * update -> mode-1 draw -> END. */
    wl16(vram + start, 0x0009U);
    wl16(vram + start + 32U, 0x000aU);
    wl16(vram + start + 32U + 12U, 160U);
    wl16(vram + start + 32U + 14U, 112U);
    wl16(vram + start + 64U, 0x0002U);
    wl16(vram + start + 96U, 0x000aU);
    wl16(vram + start + 96U + 12U, 12U);
    wl16(vram + start + 96U + 14U, 24U);
    wl16(vram + start + 128U, 0x0002U);
    wl16(vram + start + 160U, 0x8000U);
    memset(&input, 0, sizeof(input));
    input.vdp1_vram = vram;
    input.vdp1_vram_size = NEXUS_V1_VDP1_VRAM_BYTES;
    input.copr_word = (start + 64U) >> 3U;
    if (!nexus_v1_vdp1_command_sequence_frame(&input, &receipt) ||
        !receipt.valid || receipt.command_count != 6 ||
        receipt.draw_count != 2 || receipt.system_clip_count != 1 ||
        receipt.local_coordinate_count != 2 ||
        receipt.display_origin_x != 160 || receipt.display_origin_y != 112 ||
        !receipt.command_origin_verified[2] ||
        receipt.command_origin_x[2] != 160 ||
        receipt.command_origin_y[2] != 112 ||
        !receipt.command_origin_verified[4] ||
        receipt.command_origin_x[4] != 12 ||
        receipt.command_origin_y[4] != 24 ||
        !receipt.semantic_admission_blocked) {
        fprintf(stderr, "FAIL: bounded VDP1 command sequence receipt\n");
        free(vram);
        return 1;
    }
    /* A longer inferred chain must not win if it draws before LOCAL. */
    wl16(vram + 0x300U, 0x1002U);
    wl16(vram + 0x302U, (start + 64U) >> 3U);
    if (!nexus_v1_vdp1_command_sequence_frame(&input, &receipt) ||
        receipt.start_byte_offset != start) {
        fprintf(stderr, "FAIL: inferred chain drew before verified origin\n");
        free(vram); return 1;
    }
    wl16(vram + 0x300U, 0U);
    wl16(vram + 0x302U, 0U);
    /* A skipped local command must not change display-space origin. */
    wl16(vram + start + 96U, 0x400aU);
    if (!nexus_v1_vdp1_command_sequence_frame(&input, &receipt) ||
        receipt.local_coordinate_count != 1 || receipt.draw_count != 2 ||
        receipt.command_origin_x[4] != 160 || receipt.command_origin_y[4] != 112) {
        fprintf(stderr, "FAIL: skipped VDP1 local command changed origin\n");
        free(vram);
        return 1;
    }
    /* A chain without a local-coordinate record cannot invent an origin. */
    wl16(vram + start + 32U, 0x0002U);
    wl16(vram + start + 96U, 0x0002U);
    if (nexus_v1_vdp1_command_sequence_frame(&input, &receipt) != 0 ||
        receipt.valid) {
        fprintf(stderr, "FAIL: VDP1 sequence invented display origin\n");
        free(vram);
        return 1;
    }
    free(vram);
    puts("test_nexus_v1_vdp1_command_sequence: PASS");
    return 0;
}
