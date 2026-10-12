#include "dungeon_decompressor_ftl.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    unsigned char compressed[21];
    unsigned char output[3] = {0};

    memset(compressed, 0x5a, sizeof(compressed));

    /* A complete common-byte token still decodes normally. */
    compressed[20] = 0x00;
    assert(ftl_decompress_dungeon(compressed, sizeof(compressed), output, 1));
    assert(output[0] == 0x5a);

    /* Literal prefix 11 is incomplete without its following eight bits. */
    compressed[20] = 0xc0;
    assert(!ftl_decompress_dungeon(compressed, sizeof(compressed), output, 1));

    /* Two complete common tokens fit in one byte; a third does not. */
    compressed[20] = 0x00;
    assert(!ftl_decompress_dungeon(compressed, sizeof(compressed), output, 3));

    assert(!ftl_decompress_dungeon(compressed, 20, output, 1));
    return 0;
}
