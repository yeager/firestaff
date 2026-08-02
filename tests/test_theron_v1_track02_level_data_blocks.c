#include "theron_v1_track02_level_data_blocks.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* 7 levels */
    assert(THERON_TRACK02_LEVEL_COUNT == 7);

    /* Level 1 */
    const Theron_LevelDataBlockDesc *b0 = theron_v1_track02_level_data_block(0);
    assert(b0 != NULL);
    assert(b0->ud_offset == 0x09F000);
    assert(b0->per_level_meta[0] == 0x07);
    assert(b0->per_level_meta[1] == 0x87);

    /* Level 2 — non-aligned UD offset */
    const Theron_LevelDataBlockDesc *b1 = theron_v1_track02_level_data_block(1);
    assert(b1->ud_offset == 0x0DF342);

    /* Level 5 — has 0xFF in metadata */
    const Theron_LevelDataBlockDesc *b4 = theron_v1_track02_level_data_block(4);
    assert(b4->per_level_meta[5] == 0xFF);

    /* Level 7 */
    const Theron_LevelDataBlockDesc *b6 = theron_v1_track02_level_data_block(6);
    assert(b6->ud_offset == 0x21F000);
    assert(b6->per_level_meta[1] == 0x86);

    /* Out of bounds */
    assert(theron_v1_track02_level_data_block(7) == NULL);

    /* All blocks have non-zero UD offsets */
    for (unsigned i = 0; i < THERON_TRACK02_LEVEL_COUNT; i++) {
        const Theron_LevelDataBlockDesc *b = theron_v1_track02_level_data_block(i);
        assert(b->ud_offset > 0);
    }

    printf("PASS: theron_v1_track02_level_data_blocks\n");
    return 0;
}
