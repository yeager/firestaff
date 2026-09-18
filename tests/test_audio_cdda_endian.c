#include "audio_sdl_m11.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const uint8_t red_book[8] = { 0x12, 0x34, 0xab, 0xcd,
                                  0x80, 0x00, 0x7f, 0xff };
    uint8_t s16le[8] = { 0 };
    const uint8_t expected[8] = { 0x34, 0x12, 0xcd, 0xab,
                                  0x00, 0x80, 0xff, 0x7f };

    assert(M11_Audio_ConvertRedBookPcmToS16Le(
        red_book, s16le, sizeof(red_book)) == 1);
    for (size_t index = 0u; index < sizeof(expected); ++index) {
        assert(s16le[index] == expected[index]);
    }
    assert(M11_Audio_ConvertRedBookPcmToS16Le(NULL, s16le,
                                               sizeof(red_book)) == 0);
    assert(M11_Audio_ConvertRedBookPcmToS16Le(red_book, NULL,
                                               sizeof(red_book)) == 0);
    assert(M11_Audio_ConvertRedBookPcmToS16Le(red_book, s16le, 6u) == 0);

    puts("PASS: Red Book CD-DA samples are converted to SDL S16LE");
    return 0;
}
