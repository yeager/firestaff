#include "theron_v1_track02_dungeon_descriptor.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_track02_dungeon_descriptor_count() == 7);

    /* Pointer table */
    const Theron_DungeonPointerRecord *p0 = theron_v1_track02_dungeon_pointer(0);
    assert(p0 != NULL);
    assert(p0->constant_278a == 0x278A);
    assert(p0->desc_offset == 0x0058);

    const Theron_DungeonPointerRecord *p5 = theron_v1_track02_dungeon_pointer(5);
    assert(p5 != NULL);
    assert(p5->desc_offset == 0x0000);

    assert(theron_v1_track02_dungeon_pointer(8) == NULL);

    /* AKUTUBA descriptor */
    const Theron_DungeonDescriptor *d0 = theron_v1_track02_dungeon_descriptor(0);
    assert(d0 != NULL);
    assert(d0->has_descriptor == 1);
    assert(d0->field0 == 47);
    assert(d0->field1 == 44);
    assert(d0->field2 == 3);
    assert(d0->field5 == 2);

    /* FORMIC and SARMON identical */
    const Theron_DungeonDescriptor *d2 = theron_v1_track02_dungeon_descriptor(2);
    const Theron_DungeonDescriptor *d3 = theron_v1_track02_dungeon_descriptor(3);
    assert(d2->field0 == d3->field0);
    assert(d2->field1 == d3->field1);

    /* THIEF has no descriptor */
    const Theron_DungeonDescriptor *d5 = theron_v1_track02_dungeon_descriptor(5);
    assert(d5->has_descriptor == 0);

    /* DEMON has no descriptor */
    const Theron_DungeonDescriptor *d6 = theron_v1_track02_dungeon_descriptor(6);
    assert(d6->has_descriptor == 0);

    assert(theron_v1_track02_dungeon_descriptor(7) == NULL);

    /* All entries with descriptors have field5 == 2 */
    for (unsigned i = 0; i < 5; i++) {
        const Theron_DungeonDescriptor *d = theron_v1_track02_dungeon_descriptor(i);
        assert(d->field5 == 2);
    }

    /* All pointer records share constant_278a */
    for (unsigned i = 0; i < 8; i++) {
        const Theron_DungeonPointerRecord *p = theron_v1_track02_dungeon_pointer(i);
        assert(p->constant_278a == 0x278A);
    }

    printf("PASS: theron_v1_track02_dungeon_descriptor\n");
    return 0;
}
