/* dungeon_decompressor_ftl.c — FTL compressed dungeon decompressor
 *
 * Source-locked to ReDMCSB DECOMPDU.C F0455_FLOPPY_DecompressDungeon
 * and verified against Christophe Fontanel's DungeonDecompress1.1.vbs.
 *
 * Compressed dungeon format:
 *   Offset 0: uint16 signature (0x8104 LE / 0x0481 BE)
 *   Offset 2: uint32 decompressed byte count (endianness matches signature)
 *   Offset 6: uint16 dungeon ID (unused by decompressor)
 *   Offset 8: 20 bytes lookup table (4 common + 16 less common)
 *   Offset 28: compressed bitstream
 *
 * Bit encoding (MSB-first):
 *   0 + 2 bits  = index into table[0..3]  (most common bytes)
 *   10 + 4 bits = index into table[4..19] (less common bytes)
 *   11 + 8 bits = literal byte
 */

#include "dungeon_decompressor_ftl.h"
#include <string.h>

typedef struct {
    const unsigned char* data;
    size_t size;
    size_t pos;
    unsigned long buffer;
    int bits;
} FTL_BitReader;

static void ftl_br_init(FTL_BitReader* br, const unsigned char* data, size_t size) {
    br->data = data;
    br->size = size;
    br->pos = 0;
    br->buffer = 0;
    br->bits = 0;
}

static unsigned int ftl_get_bits(FTL_BitReader* br, int count) {
    unsigned int result;
    /* Ensure enough bits in buffer (read bytes MSB-first) */
    while (br->bits < count) {
        if (br->pos < br->size) {
            br->buffer = (br->buffer << 8) | br->data[br->pos++];
            br->bits += 8;
        } else {
            br->buffer <<= 8;
            br->bits += 8;
        }
    }
    /* Extract top 'count' bits */
    br->bits -= count;
    result = (unsigned int)(br->buffer >> br->bits) & ((1u << count) - 1u);
    br->buffer &= (1ul << br->bits) - 1ul;
    return result;
}

int ftl_decompress_dungeon(const unsigned char* compressed, size_t compressedSize,
                           unsigned char* output, long decompressedSize) {
    const unsigned char* table;
    FTL_BitReader br;
    long remaining;

    if (!compressed || !output || compressedSize < 20 || decompressedSize <= 0) {
        return 0;
    }

    /* First 20 bytes = lookup table */
    table = compressed;

    /* Bitstream starts after table */
    ftl_br_init(&br, compressed + 20, compressedSize - 20);

    remaining = decompressedSize;
    while (remaining > 0) {
        if (ftl_get_bits(&br, 1) == 0) {
            /* 0 + 2-bit index → most common table */
            unsigned int idx = ftl_get_bits(&br, 2);
            *output++ = table[idx];
        } else {
            if (ftl_get_bits(&br, 1) == 0) {
                /* 10 + 4-bit index → less common table */
                unsigned int idx = ftl_get_bits(&br, 4);
                *output++ = table[4 + idx];
            } else {
                /* 11 + 8-bit literal */
                unsigned int literal = ftl_get_bits(&br, 8);
                *output++ = (unsigned char)literal;
            }
        }
        remaining--;
    }
    return 1;
}
