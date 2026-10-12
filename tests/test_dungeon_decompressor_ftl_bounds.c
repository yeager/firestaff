#include "dungeon_decompressor_ftl.h"

#include <assert.h>

int main(void)
{
    unsigned char compressed[22];
    unsigned char output[3] = {0};
    unsigned int i;

    for (i = 0; i < 20; ++i) {
        compressed[i] = (unsigned char)i;
    }

    /* A complete common-byte token still decodes normally. */
    compressed[20] = 0x40; /* 0 + index 2 */
    assert(ftl_decompress_dungeon(compressed, sizeof(compressed), output, 1));
    assert(output[0] == 2);

    /* The less-common token selects table[4 + index]. */
    compressed[20] = 0xa8; /* 10 + index 10 */
    assert(ftl_decompress_dungeon(compressed, sizeof(compressed), output, 1));
    assert(output[0] == 14);

    /* The literal token remains MSB-first and decodes all eight payload bits. */
    compressed[20] = 0xd6;
    compressed[21] = 0x80; /* 11 + literal 0x5a */
    assert(ftl_decompress_dungeon(compressed, sizeof(compressed), output, 1));
    assert(output[0] == 0x5a);

    /* Literal prefix 11 is incomplete without its following eight bits. */
    compressed[20] = 0xc0;
    assert(!ftl_decompress_dungeon(compressed, 21, output, 1));

    /* A common token followed by a partial less-common index must fail. */
    compressed[20] = 0x15; /* 000 + 10 + only three of four index bits */
    assert(!ftl_decompress_dungeon(compressed, 21, output, 2));

    /* Two complete common tokens fit in one byte; a third does not. */
    compressed[20] = 0x00;
    assert(!ftl_decompress_dungeon(compressed, 21, output, 3));

    assert(!ftl_decompress_dungeon(compressed, 20, output, 1));
    return 0;
}
