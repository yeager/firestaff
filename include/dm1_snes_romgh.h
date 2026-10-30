#ifndef DM1_SNES_ROMGH_H
#define DM1_SNES_ROMGH_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t source_offset;
    size_t source_size;
    size_t table_length;
    size_t bank;
    size_t bank_offset;
    uint8_t compression;
} Dm1SnesRomghEntry;

/*
 * Resolve an entry in an SCK-format ROMGH table to its stored ROM span.
 *
 * The ROMGH table contains a little-endian entry count, that many
 * little-endian 16-bit stored lengths, then one compression byte per entry.
 * SCK's SNES graphics stream starts in bank 9 with a 512-byte copier-header
 * bias. Before reading an item that crosses the 32 KiB bank window, SCK moves
 * the cursor to the next bank and then reads the full table length. The
 * skipped bytes appear in SCK's resmap span length, not in the ROMGH length.
 *
 * romghOffset and romghHeaderSize are physical ROM file offsets/sizes.
 * Returns 1 on success, 0 for malformed metadata, invalid arguments,
 * arithmetic overflow, or a source span outside the ROM.
 */
int dm1_snes_romgh_resolve(const uint8_t *rom, size_t romSize,
                           size_t romghOffset, size_t romghHeaderSize,
                           size_t itemIndex, Dm1SnesRomghEntry *entry);

#endif
