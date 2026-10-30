#include "dm1_snes_romgh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL:%d: %s\n", __LINE__, #expr); \
        return 0; \
    } \
} while (0)

static void put_u16le(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xffu);
    p[1] = (uint8_t)(value >> 8);
}

static int test_bank_boundary(void)
{
    const size_t header_offset = 512u;
    const size_t header_size = 11u;
    const size_t rom_size = 512u + 11u * 32768u;
    uint8_t *rom = (uint8_t *)calloc(rom_size, 1u);
    Dm1SnesRomghEntry entry;

    CHECK(rom != NULL);
    put_u16le(rom + header_offset, 3u);
    put_u16le(rom + header_offset + 2u, 4u);
    put_u16le(rom + header_offset + 4u, 32760u);
    put_u16le(rom + header_offset + 6u, 10u);
    rom[header_offset + 8u] = 0u;
    rom[header_offset + 9u] = 1u;
    rom[header_offset + 10u] = 3u;

    CHECK(dm1_snes_romgh_resolve(rom, rom_size, header_offset, header_size,
                                  0u, &entry));
    CHECK(entry.source_offset == 512u + 9u * 32768u);
    CHECK(entry.source_size == 4u && entry.table_length == 4u);
    CHECK(entry.bank == 9u && entry.bank_offset == 0u && entry.compression == 0u);

    CHECK(dm1_snes_romgh_resolve(rom, rom_size, header_offset, header_size,
                                  2u, &entry));
    CHECK(entry.source_offset == 512u + 10u * 32768u);
    CHECK(entry.source_size == 10u && entry.table_length == 10u);
    CHECK(entry.bank == 10u && entry.bank_offset == 0u && entry.compression == 3u);

    free(rom);
    return 1;
}

static int test_invalid_metadata_and_ranges(void)
{
    uint8_t rom[512u + 512u] = {0};
    Dm1SnesRomghEntry entry;

    put_u16le(rom + 512u, 1u);
    put_u16le(rom + 514u, 4u);
    rom[516u] = 0u;
    CHECK(!dm1_snes_romgh_resolve(rom, sizeof(rom), 512u, 4u, 0u, &entry));
    CHECK(!dm1_snes_romgh_resolve(rom, sizeof(rom), 512u, 5u, 1u, &entry));
    CHECK(!dm1_snes_romgh_resolve(rom, sizeof(rom), 512u, 5u, 0u, &entry));
    CHECK(!dm1_snes_romgh_resolve(rom, sizeof(rom), sizeof(rom), 5u, 0u, &entry));
    CHECK(entry.source_offset == 0u && entry.source_size == 0u);
    return 1;
}

int main(void)
{
    if (!test_bank_boundary() || !test_invalid_metadata_and_ranges()) return 1;
    puts("PASS: DM1 SNES ROMGH resolver bounds and bank crossing");
    return 0;
}
