#include "nexus_v1_saturn_tile_transform.h"

static int16_t sign_extend_word(uint32_t value) {
    return (int16_t)(value & 0xffffU);
}

/* SH-2: sts macl,r0; shlr8 r0; exts.w r0,r0. */
static int16_t sh2_macl_shift8(int32_t product) {
    return sign_extend_word(((uint32_t)product) >> 8U);
}

int nexus_v1_saturn_expand_tile_8x48(
    const uint8_t *input,
    size_t input_size,
    uint16_t *output,
    size_t output_words,
    size_t output_stride_words,
    const uint16_t table_words[32],
    int16_t coefficient_r9,
    int16_t coefficient_r10) {
    int16_t r9 = coefficient_r9;
    int16_t r10 = coefficient_r10;
    unsigned row;

    if (!input || !output || !table_words || output_stride_words < 48U ||
        input_size < 7U * 0x80U + 16U + 47U * 4U + (47U >> 1U) +
            (7U >> 1U) + 1U ||
        output_words < 7U * output_stride_words + 48U) {
        return 0;
    }

    for (row = 0U; row < 8U; ++row) {
        const uint8_t *row_input = input + (size_t)row * 0x80U;
        uint16_t *row_output = output + (size_t)row * output_stride_words;
        const uint8_t selector = (uint8_t)(row_input[4] & 0x0fU);
        const unsigned table_offset = (unsigned)(row_input[4] & 0xf0U) >> 2U;
        const int16_t table_r5 = (int16_t)table_words[table_offset >> 1U];
        const int16_t table_r4 = (int16_t)table_words[(table_offset >> 1U) + 1U];
        unsigned pixel;

        for (pixel = 0U; pixel < 48U; ++pixel) {
            const size_t source_offset = 16U + (size_t)pixel * 4U +
                (size_t)(pixel >> 1U) + (size_t)(row >> 1U);
            uint32_t pixel_value;
            int32_t value;
            int32_t product;
            int shift = 0;

            if ((size_t)row * 0x80U + source_offset >= input_size) {
                return 0;
            }
            pixel_value = (uint32_t)row_input[source_offset] <<
                ((row & 1U) ? 8U : 16U);
            pixel_value &= 0xf000U;
            if (selector & 0x01U) shift += 1;
            if (selector & 0x02U) shift += 2;
            if (selector & 0x04U) shift += 4;
            if (selector & 0x08U) shift += 8;
            if (shift > 0) pixel_value >>= (unsigned)shift;

            product = (int32_t)r9 * (int32_t)table_r5;
            value = (int32_t)pixel_value + sh2_macl_shift8(product);
            product = (int32_t)r10 * (int32_t)table_r4;
            value += sh2_macl_shift8(product);
            r10 = r9;
            r9 = (int16_t)value;
            row_output[pixel] = (uint16_t)value;
        }
    }
    return 1;
}
