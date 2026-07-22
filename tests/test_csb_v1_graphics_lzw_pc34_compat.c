#include "csb_v1_graphics_lzw_pc34_compat.h"

#include <stdint.h>
#include <string.h>

static int pack_codes(const uint16_t *codes, size_t count, uint8_t *out,
                      size_t out_size)
{
    size_t bit = 0U;
    size_t index;

    if (!codes || !out || count == 0U || out_size == 0U) return 0;
    memset(out, 0, out_size);
    for (index = 0U; index < count; ++index) {
        unsigned int shift;
        if (codes[index] > 0x1ffU || bit + 9U > out_size * 8U) return 0;
        for (shift = 0U; shift < 9U; ++shift) {
            if ((codes[index] & (uint16_t)(1U << shift)) != 0U)
                out[(bit + shift) >> 3U] |=
                    (uint8_t)(1U << ((bit + shift) & 7U));
        }
        bit += 9U;
    }
    return 1;
}

int main(void)
{
    const uint16_t codes[] = { 65U, 0x90U, 4U };
    uint8_t packed[4];
    uint8_t decoded[8];
    size_t decoded_size = 0U;

    if (!pack_codes(codes, sizeof(codes) / sizeof(codes[0]), packed,
                    sizeof(packed)) ||
        csb_v1_graphics_lzw_decode_pc34_compat(
            packed, sizeof(packed), decoded, sizeof(decoded),
            &decoded_size) != 0 ||
        decoded_size != 4U || memcmp(decoded, "AAAA", 4U) != 0)
        return 1;
    return 0;
}
