#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int check_string(const uint8_t *data, size_t size, size_t offset,
    const char *text)
{
    size_t length = strlen(text) + 1U;
    return offset + length <= size && memcmp(data + offset, text, length) == 0;
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

    /* DM.BIN's adjacent strings are the retail startup/menu loader's own
     * resource names. The pointer counts are byte-level receipts only; they
     * do not infer menu order or Saturn VDP1/VDP2 presentation. */
    if (!check_string(data, (size_t)file_size, 0x373B4U, "MENU.BPK") ||
        !check_string(data, (size_t)file_size, 0x373C0U, "yam\\menu.c") ||
        !check_string(data, (size_t)file_size, 0x373CCU, "FONT256.S2D") ||
        !check_string(data, (size_t)file_size, 0x373D8U, "STABG.BIN") ||
        count_be32(data, (size_t)file_size, base + UINT32_C(0x373B4)) != 1U ||
        count_be32(data, (size_t)file_size, base + UINT32_C(0x373CC)) != 1U ||
        count_be32(data, (size_t)file_size, base + UINT32_C(0x373D8)) != 1U ||
        count_be32(data, (size_t)file_size, base + UINT32_C(0x373C0)) != 10U) {
        free(data);
        fprintf(stderr, "FAIL: DM.BIN startup/menu resource anchor mismatch\n");
        return 1;
    }
    free(data);
    puts("PASS: real DM.BIN startup/menu resource anchors verified");
    return 0;
}
