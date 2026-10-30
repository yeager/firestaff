#include "dm1_snes_romgh.h"

#define SNES_COPIER_HEADER_SIZE 512u
#define SNES_ROM_BANK_SIZE 32768u
#define SNES_ROM_DATA_FIRST_BANK 9u
#define SNES_ROM_BANK_LAST_OFFSET 32767u

static uint16_t read_u16le(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

int dm1_snes_romgh_resolve(const uint8_t *rom, size_t romSize,
                           size_t romghOffset, size_t romghHeaderSize,
                           size_t itemIndex, Dm1SnesRomghEntry *entry)
{
    size_t headerEnd;
    size_t entryCount;
    size_t lengthsOffset = 2u;
    size_t compressionOffset;
    size_t requiredHeaderSize;
    size_t bank = SNES_ROM_DATA_FIRST_BANK;
    size_t bankOffset = 0u;
    size_t i;

    if (entry) {
        entry->source_offset = 0u;
        entry->source_size = 0u;
        entry->table_length = 0u;
        entry->bank = 0u;
        entry->bank_offset = 0u;
        entry->compression = 0u;
    }
    if (!rom || !entry || romghHeaderSize < 2u ||
        romghOffset > romSize || romghHeaderSize > romSize - romghOffset) {
        return 0;
    }

    headerEnd = romghOffset + romghHeaderSize;
    entryCount = read_u16le(rom + romghOffset);
    if (entryCount == 0u || itemIndex >= entryCount) return 0;
    if (entryCount > (SIZE_MAX - lengthsOffset) / 3u) return 0;
    requiredHeaderSize = lengthsOffset + entryCount * 3u;
    if (requiredHeaderSize > romghHeaderSize) return 0;

    compressionOffset = romghOffset + lengthsOffset + entryCount * 2u;
    if (compressionOffset > headerEnd || entryCount > headerEnd - compressionOffset)
        return 0;

    for (i = 0u; i <= itemIndex; ++i) {
        size_t lengthOffset = romghOffset + lengthsOffset + i * 2u;
        size_t tableLength = read_u16le(rom + lengthOffset);
        size_t sourceOffset;
        size_t sourceSize;
        size_t bankBase;

        if (tableLength == 0u) return 0;

        /* SCK advances only when bankOffset + length exceeds 32767. */
        if (tableLength > SIZE_MAX - bankOffset) return 0;
        if (bankOffset + tableLength > SNES_ROM_BANK_LAST_OFFSET) {
            if (bank == SIZE_MAX) return 0;
            ++bank;
            bankOffset = 0u;
        }

        if (bank > (SIZE_MAX - SNES_COPIER_HEADER_SIZE) / SNES_ROM_BANK_SIZE)
            return 0;
        bankBase = SNES_COPIER_HEADER_SIZE + bank * SNES_ROM_BANK_SIZE;
        if (bankBase > SIZE_MAX - bankOffset) return 0;
        sourceOffset = bankBase + bankOffset;
        sourceSize = tableLength;
        if (sourceSize == 0u || sourceSize > SNES_ROM_BANK_SIZE) return 0;
        if (sourceOffset > romSize || sourceSize > romSize - sourceOffset)
            return 0;

        if (i == itemIndex) {
            entry->source_offset = sourceOffset;
            entry->source_size = sourceSize;
            entry->table_length = tableLength;
            entry->bank = bank;
            entry->bank_offset = bankOffset;
            entry->compression = rom[compressionOffset + i];
            return 1;
        }

        if (sourceSize > SNES_ROM_BANK_SIZE - bankOffset) return 0;
        bankOffset += sourceSize;
    }

    return 0;
}
