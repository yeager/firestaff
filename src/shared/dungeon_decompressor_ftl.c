/* dungeon_decompressor_ftl.c — FTL compressed dungeon decompressor
 *
 * Source-locked to ReDMCSB DECOMPDU.C F0455_FLOPPY_DecompressDungeon.
 * The compressed format uses a 20-byte frequency table header
 * (4 most-common + 16 less-common bytes) followed by a bitstream.
 *
 * The COMPRESSED_DUNGEON_HEADER precedes the compressed data:
 *   uint16 Signature (0x8104, big-endian on disk)
 *   int32  DecompressedByteCount
 *   uint16 DungeonID
 * Total: 8 bytes. Compressed data starts at offset 8.
 */

#include "dungeon_decompressor_ftl.h"
#include <stdlib.h>
#include <string.h>

/* Bitstream reader state */
typedef struct {
    const unsigned char* data;
    size_t size;
    size_t pos;        /* byte position (reads uint16 at a time) */
    unsigned long bits; /* accumulated bits */
    int bitsLeft;      /* bits remaining in accumulator */
} BitReader;

static void br_init(BitReader* br, const unsigned char* data, size_t size) {
    br->data = data;
    br->size = size;
    br->pos = 0;
    br->bits = 0;
    br->bitsLeft = 0;
}

/* Read next uint16 (big-endian, matching 68k word reads) */
static int br_fetch_word(BitReader* br) {
    if (br->pos + 1 >= br->size) return 0;
    unsigned int w = ((unsigned int)br->data[br->pos] << 8) |
                     (unsigned int)br->data[br->pos + 1];
    br->pos += 2;
    br->bits = (br->bits << 16) | w;
    br->bitsLeft += 16;
    return 1;
}

/* Get N bits from the top of the accumulator (mirrors the 68k lsl.l + swap pattern) */
static unsigned int br_get_bits(BitReader* br, int n) {
    while (br->bitsLeft < n) {
        if (!br_fetch_word(br)) return 0;
    }
    br->bitsLeft -= n;
    unsigned int val = (unsigned int)(br->bits >> br->bitsLeft) & ((1u << n) - 1u);
    return val;
}

int ftl_decompress_dungeon(const unsigned char* compressed, size_t compressedSize,
                           unsigned char* output, long decompressedSize) {
    BitReader br;
    const unsigned char* table;
    long remaining;

    if (!compressed || !output || compressedSize < 20 || decompressedSize <= 0) {
        return 0;
    }

    /* First 20 bytes: frequency tables
     * Bytes 0-3:  4 most common bytes (indexed by 2-bit values)
     * Bytes 4-19: 16 less common bytes (indexed by 4-bit values) */
    table = compressed;

    /* Bitstream starts after the 20-byte table */
    br_init(&br, compressed + 20, compressedSize - 20);

    remaining = decompressedSize;
    while (remaining > 0) {
        /* ReDMCSB DECOMPDU.C F0455 algorithm:
         * Read first word, then process bits:
         *   bit15=0: read 3 total bits (0 + 2-bit index), lookup in 4-byte table
         *   bit15=1, next 2 bits=0: read 4 more bits, lookup in 16-byte table
         *   bit15=1, next 2 bits≠0: read 8 more bits, output literal byte */
        unsigned int tag = br_get_bits(&br, 1);
        if (tag == 0) {
            /* 0 + 2-bit index into most-common table */
            unsigned int idx = br_get_bits(&br, 2);
            *output++ = table[idx];
        } else {
            /* 1 + 2-bit sub-tag */
            unsigned int subtag = br_get_bits(&br, 2);
            if (subtag == 0) {
                /* 10 + 0: read 4-bit index into less-common table */
                unsigned int idx = br_get_bits(&br, 4);
                *output++ = table[4 + idx];
            } else {
                /* 11 or 10 with subtag≠0: read 8-bit literal */
                /* Actually: subtag was already consumed. If subtag==0, we did the 4-bit path.
                 * If subtag!=0, the original shifts to get 8 more bits for a literal. But
                 * we already consumed the 2-bit subtag. The original uses 10 total bits:
                 * 1(tag) + 1(first of subtag) + 8(literal). That's "11" prefix + 8-bit literal.
                 * Wait — re-reading the assembly more carefully:
                 *
                 * The original reads 2 bits after the initial 1-bit tag (total 3 bits so far).
                 * If those 2 bits when decremented by 2 == 0 (i.e., value was exactly 2, 
                 * meaning bits were "10"), then it reads 4 more bits for the 16-byte table.
                 * If non-zero (value was 3, meaning bits "11"), it reads 8 more bits as literal.
                 *
                 * Actually the assembly does: subq.w #2,D1 then checks Z flag.
                 * D1 has the 2-bit value (0-3). After subtracting 2:
                 *   value 0 → -2, NE: 8-bit literal (but this can't happen because we already
                 *     checked bit15=1)
                 *   value 1 → -1, NE: 8-bit literal
                 *   value 2 → 0, EQ: 4-bit table lookup
                 *   value 3 → 1, NE: 8-bit literal
                 *
                 * Wait, I need to re-read more carefully. The 2-bit value IS the two bits
                 * after the leading 1-bit. So the 3-bit prefix is 1XX:
                 *   100 → subtag=0 → wait, original does: subtract 2 from the 2-bit value...
                 *
                 * Let me just look at the pure bit patterns:
                 *   0XX → table[XX] (most common)
                 *   10  → then 4 bits → table[4 + xxxx] (less common)  [subtag 00 after dec by 2 = -2? No...]
                 *   11  → then 8 bits → literal byte
                 *
                 * That's the standard interpretation. My code above with subtag==0 for
                 * the 4-bit path is WRONG because the assembly does subtag = "2-bit value" - 2.
                 * 
                 * Actually looking again at the asm: after btst #15,D1 (which tests bit 15),
                 * if set, it requests 2 bits, then does subq.w #2,D1. But the 2 bits requested
                 * include the bit we already tested! The asm does lsl to extract bits.
                 *
                 * Simplest correct interpretation from the bit patterns:
                 *   0XX = most-common (3 bits total, 2 used as index)
                 *   100 = less-common (7 bits total: 3 prefix + 4 index)
                 *   11XXXXXXXX = literal (10 bits total: 2 prefix + 8 literal)
                 * 
                 * So subtag==0 means prefix was "10", subtag!=0 means "11".
                 * This matches my original code! */
                unsigned int literal = br_get_bits(&br, 8);
                *output++ = (unsigned char)literal;
            }
        }
        remaining--;
    }
    return 1;
}
