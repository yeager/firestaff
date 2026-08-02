#include "theron_v1_sector_alloc.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_sector_alloc_count() == 42);

    const Theron_SectorAlloc *r0 = theron_v1_sector_alloc(0);
    (void)r0;
    assert(r0 != NULL);
    assert(r0->data_size == 0xC800);
    assert(r0->sector_start == 0x080B);
    assert(r0->sector_count == 0x0100);

    const Theron_SectorAlloc *r8 = theron_v1_sector_alloc(8);
    (void)r8;
    assert(r8 != NULL);
    assert(r8->data_size == 0xE000);

    const Theron_SectorAlloc *r41 = theron_v1_sector_alloc(41);
    (void)r41;
    assert(r41 != NULL);
    assert(r41->data_size == 0x11E2);
    assert(r41->sector_start == 0xFF00);

    assert(theron_v1_sector_alloc(42) == NULL);

    for (unsigned int i = 0; i < 42; i++) {
        const Theron_SectorAlloc *r = theron_v1_sector_alloc(i);
        assert(r != NULL);
        assert(r->data_size > 0);
    }

    printf("PASS: theron_v1_sector_alloc\n");
    return 0;
}
