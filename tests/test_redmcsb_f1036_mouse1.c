#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1036_mouse1.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    enum { WORDS_PER_PLANE = 4 };
    int16_t bitmap_storage[2 + (4 * WORDS_PER_PLANE)] = {
        17, 2,
        0x0101, 0x0102, 0x0103, 0x0104,
        0x0201, 0x0202, 0x0203, 0x0204,
        0x0301, 0x0302, 0x0303, 0x0304,
        0x0401, 0x0402, 0x0403, 0x0404
    };
    uint16_t sprite_images[REDMCSB_F1036_POINTER_BANKS *
                           REDMCSB_F1036_POINTER_BANK_WORDS];
    uint16_t *bank;
    (void)bank;
    const char *evidence;
    (void)evidence;

    memset(sprite_images, 0xa5, sizeof(sprite_images));
    redmcsb_f1036_mouse1(sprite_images,
                          (const uint8_t *)(const void *)&bitmap_storage[2], 2);

    bank = sprite_images + (3 * REDMCSB_F1036_POINTER_BANK_WORDS);
    assert(bank[0] == 0xa5a5U);
    assert(bank[1] == 0xa5a5U);
    assert(bank[2] == 0x0101U);
    assert(bank[3] == 0x0201U);
    assert(bank[4] == 0x0103U);
    assert(bank[5] == 0x0203U);
    assert(bank[40 + 2] == 0x0301U);
    assert(bank[40 + 3] == 0x0401U);
    assert(bank[40 + 4] == 0x0303U);
    assert(bank[40 + 5] == 0x0403U);
    assert(bank[80 + 2] == 0x0102U);
    assert(bank[80 + 3] == 0x0202U);
    assert(bank[80 + 4] == 0x0104U);
    assert(bank[80 + 5] == 0x0204U);
    assert(bank[120 + 2] == 0x0302U);
    assert(bank[120 + 3] == 0x0402U);
    assert(bank[120 + 4] == 0x0304U);
    assert(bank[120 + 5] == 0x0404U);
    assert(bank[2 + REDMCSB_F1036_SPRITE_DATA_WORDS] == 0xa5a5U);
    assert(sprite_images[0] == 0xa5a5U);

    evidence = redmcsb_f1036_mouse1_source_evidence();
    assert(strstr(evidence, "IO.C:1863-2019") != NULL);
    assert(strstr(evidence, "IO.C:1956-1961") != NULL);
    assert(strstr(evidence, "IO.C:1962-2017") != NULL);
    assert(strstr(evidence, "AMIGA.H:91-97") != NULL);
    assert(strstr(evidence, "DEFS.H:3444-3445") != NULL);
    puts("ok: ReDMCSB F1036 Amiga mouse-sprite layout");
    return 0;
}
