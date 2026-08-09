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
    /* System clip -> local coordinate -> mode-1 draw -> END. */
    wl16(vram + start, 0x0009U);
    wl16(vram + start + 32U, 0x000aU);
    wl16(vram + start + 32U + 12U, 160U);
    wl16(vram + start + 32U + 14U, 112U);
    wl16(vram + start + 64U, 0x0002U);
    wl16(vram + start + 96U, 0x8000U);
    memset(&input, 0, sizeof(input));
    input.vdp1_vram = vram;
    input.vdp1_vram_size = NEXUS_V1_VDP1_VRAM_BYTES;
    input.copr_word = (start + 64U) >> 3U;
    if (!nexus_v1_vdp1_command_sequence_frame(&input, &receipt) ||
        !receipt.valid || receipt.command_count != 4 ||
        receipt.draw_count != 1 || receipt.system_clip_count != 1 ||
        receipt.local_coordinate_count != 1 ||
        receipt.display_origin_x != 160 || receipt.display_origin_y != 112 ||
        !receipt.semantic_admission_blocked) {
        fprintf(stderr, "FAIL: bounded VDP1 command sequence receipt\n");
        free(vram);
        return 1;
    }
    /* A chain without a local-coordinate record cannot invent an origin. */
    wl16(vram + start + 32U, 0x0002U);
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
