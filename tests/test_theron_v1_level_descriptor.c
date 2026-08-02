#include "theron_v1_level_descriptor.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_level_descriptor_count() == 53);

    const Theron_LevelDescriptor *d0 = theron_v1_level_descriptor(0);
    (void)d0;
    assert(d0 != NULL);
    assert(d0->flags == 1);
    assert(d0->map_width == 2);
    assert(d0->data_size == 0x0876);
    assert(d0->cumulative_column_index == 2);

    const Theron_LevelDescriptor *d16 = theron_v1_level_descriptor(16);
    (void)d16;
    assert(d16 != NULL);
    assert(d16->map_width == 28);
    assert(d16->data_size == 0xE000);

    const Theron_LevelDescriptor *d42 = theron_v1_level_descriptor(42);
    (void)d42;
    assert(d42 != NULL);
    assert(d42->map_width == 1);
    assert(d42->data_size == 0x0280);
    assert(d42->cumulative_column_index == 232);

    const Theron_LevelDescriptor *d52 = theron_v1_level_descriptor(52);
    (void)d52;
    assert(d52 != NULL);
    assert(d52->map_width == 1);
    assert(d52->data_size == 0x023D);
    assert(d52->cumulative_column_index == 2);

    assert(theron_v1_level_descriptor(53) == NULL);

    for (unsigned int i = 0; i < 53; i++) {
        const Theron_LevelDescriptor *d = theron_v1_level_descriptor(i);
        assert(d != NULL);
        assert(d->flags == 1);
        assert(d->map_width >= 1);
        assert(d->data_size > 0);
    }

    printf("PASS: theron_v1_level_descriptor\n");
    return 0;
}
