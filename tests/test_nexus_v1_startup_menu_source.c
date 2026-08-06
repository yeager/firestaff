#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asset_find_by_hash.h"

static size_t count_be32(const uint8_t *data, size_t size, uint32_t value)
{
    size_t count = 0U;
    size_t i;
    for (i = 0U; i + 4U <= size; ++i) {
        uint32_t observed = ((uint32_t)data[i] << 24) |
            ((uint32_t)data[i + 1U] << 16) |
            ((uint32_t)data[i + 2U] << 8) | data[i + 3U];
        if (observed == value) ++count;
    }
    return count;
}

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= data[i];
        hash *= UINT64_C(0x100000001b3);
    }
    return hash;
}

static int check_string(const uint8_t *data, size_t size, size_t offset,
    const char *text)
{
    size_t length = strlen(text) + 1U;
    return offset + length <= size && memcmp(data + offset, text, length) == 0;
}

static uint16_t read_be16(const uint8_t *data, size_t offset)
{
    return (uint16_t)(((uint16_t)data[offset] << 8) | data[offset + 1U]);
}

static uint32_t read_be32(const uint8_t *data, size_t offset)
{
    return ((uint32_t)data[offset] << 24) |
        ((uint32_t)data[offset + 1U] << 16) |
        ((uint32_t)data[offset + 2U] << 8) | data[offset + 3U];
}

/* SH-2 MOV.L @(disp,PC),Rn: PC is the address of the instruction and the
 * aligned base is (PC + 4) & ~3. Keep this receipt local to the authenticated
 * retail image; it is not a runtime MMIO or VDP2 interpretation. */
static int check_sh2_mov_l_literal(const uint8_t *data, size_t size,
    size_t instruction_offset, unsigned int register_number,
    size_t literal_offset, uint32_t literal_value)
{
    uint16_t opcode;
    size_t aligned_pc_plus_four;
    size_t observed_literal_offset;

    if (instruction_offset + 2U > size || literal_offset + 4U > size ||
        register_number > 15U) {
        return 0;
    }
    opcode = read_be16(data, instruction_offset);
    if ((opcode & UINT16_C(0xF000)) != UINT16_C(0xD000) ||
        ((opcode >> 8) & 0x0FU) != register_number) {
        return 0;
    }
    aligned_pc_plus_four = (instruction_offset + 4U) & ~(size_t)3U;
    observed_literal_offset = aligned_pc_plus_four +
        ((size_t)(opcode & UINT16_C(0x00FF)) * 4U);
    return observed_literal_offset == literal_offset &&
        read_be32(data, observed_literal_offset) == literal_value;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
    FILE *file;
    long file_size;
    uint8_t *data;
    const uint32_t base = UINT32_C(0x06010000);

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not mounted");
        return 77;
    }
    if (snprintf(path, sizeof(path), "%s/DM.BIN", root) >= (int)sizeof(path) ||
        !(file = fopen(path, "rb")) || fseek(file, 0, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        puts("SKIP: real DM.BIN is unavailable");
        return 77;
    }
    data = (uint8_t *)malloc((size_t)file_size);
    if (!data || fread(data, 1, (size_t)file_size, file) != (size_t)file_size) {
        free(data);
        fclose(file);
        puts("SKIP: real DM.BIN could not be read");
        return 77;
    }
    fclose(file);

    if (!asset_file_matches_md5(path,
                                "e88d60859f65f08fa622e1992b02280f")) {
        free(data);
        fprintf(stderr, "FAIL: DM.BIN is not the authenticated European retail source\n");
        return 1;
    }

    /* DM.BIN's adjacent strings are the retail startup/menu loader's own
     * resource names. The pointer counts are byte-level receipts only; they
     * do not infer menu order or Saturn VDP1/VDP2 presentation. */
    if (!check_string(data, (size_t)file_size, 0x373B4U, "MENU.BPK") ||
        !check_string(data, (size_t)file_size, 0x373C0U, "yam\\menu.c") ||
        !check_string(data, (size_t)file_size, 0x373CCU, "FONT256.S2D") ||
        !check_string(data, (size_t)file_size, 0x373D8U, "STABG.BIN") ||
        !check_string(data, (size_t)file_size, 0x38CF4U, "yam\\vdp2.c") ||
        count_be32(data, (size_t)file_size, base + UINT32_C(0x373B4)) != 1U ||
        count_be32(data, (size_t)file_size, base + UINT32_C(0x373CC)) != 1U ||
        count_be32(data, (size_t)file_size, base + UINT32_C(0x373D8)) != 1U ||
        count_be32(data, (size_t)file_size, base + UINT32_C(0x373C0)) != 10U ||
        count_be32(data, (size_t)file_size, base + UINT32_C(0x38CF4)) != 6U ||
        /* The same startup literal pool retains FONT256.S2D at 0x18BF4.
         * TEXTTABL is an adjacent DM.BIN table marker only; neither receipt
         * proves the Saturn glyph consumer or VDP2 placement. */
        ((uint32_t)data[0x18BF4U] << 24 |
            (uint32_t)data[0x18BF5U] << 16 |
            (uint32_t)data[0x18BF6U] << 8 | data[0x18BF7U]) !=
            base + UINT32_C(0x373CC) ||
        0x294C0U + 8U > (size_t)file_size ||
        memcmp(data + 0x294C0U, "TEXTTABL", 8U) != 0 ||
        /* The SH-2 routine at 0x18B60 is followed by its literal pool. */
        fnv1a64(data + 0x18B60U, 0x90U) !=
            UINT64_C(0xf6d5cc046bab98c7) ||
        ((uint32_t)data[0x18C00U] << 24 |
            (uint32_t)data[0x18C01U] << 16 |
            (uint32_t)data[0x18C02U] << 8 | data[0x18C03U]) !=
            base + UINT32_C(0x373C0) ||
        ((uint32_t)data[0x18C20U] << 24 |
            (uint32_t)data[0x18C21U] << 16 |
            (uint32_t)data[0x18C22U] << 8 | data[0x18C23U]) !=
            base + UINT32_C(0x373D8) ||
        /* Register literals are retained as disassembly receipts only. */
        count_be32(data, (size_t)file_size, UINT32_C(0x25F00006)) != 1U ||
        count_be32(data, (size_t)file_size, UINT32_C(0x25F80000)) != 1U ||
        /* The startup/menu routine is a real SH-2 function: verify its
         * frame/return envelope and the PC-relative references that bind the
         * code to the retail resource literals. These are ownership receipts,
         * not proof that Firestaff has captured or reproduced Saturn output. */
        read_be16(data, 0x18B64U) != UINT16_C(0x4F22) ||
        read_be16(data, 0x18BE4U) != UINT16_C(0x4F26) ||
        read_be16(data, 0x18BE8U) != UINT16_C(0x000B) ||
        !check_sh2_mov_l_literal(data, (size_t)file_size, 0x18B6CU, 4U,
            0x18C20U, base + UINT32_C(0x373D8)) ||
        !check_sh2_mov_l_literal(data, (size_t)file_size, 0x18B7EU, 4U,
            0x18C00U, base + UINT32_C(0x373C0)) ||
        !check_sh2_mov_l_literal(data, (size_t)file_size, 0x18BB8U, 4U,
            0x18C00U, base + UINT32_C(0x373C0)) ||
        !check_sh2_mov_l_literal(data, (size_t)file_size, 0x18B88U, 4U,
            0x18C24U, UINT32_C(0x25E64000))) {
        free(data);
        fprintf(stderr, "FAIL: DM.BIN startup/menu resource anchor mismatch\n");
        return 1;
    }
    free(data);
    puts("PASS: real DM.BIN startup/menu resource anchors verified");
    return 0;
}
