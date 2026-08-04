#include "dm2_v1_util_pc34_compat.h"

int16_t dm2_v1_calc_square_distance(int16_t x1, int16_t y1,
                                     int16_t x2, int16_t y2)
{
    return dm2_v1_abs_i16((int16_t)(x1 - x2))
         + dm2_v1_abs_i16((int16_t)(y1 - y2));
}

int16_t dm2_v1_calc_vector_dir(int16_t a, int16_t d,
                                int16_t b, int16_t c,
                                int rand_bit)
{
    int16_t dx = (int16_t)(a - b);
    int16_t adx = dm2_v1_abs_i16(dx);
    int16_t dy = (int16_t)(d - c);
    int16_t ady = dm2_v1_abs_i16(dy);

    if (adx == ady) {
        if (rand_bit == 0)
            ady++;
        else
            adx++;
    }

    if (adx >= ady)
        return (dx <= 0) ? 1 : 3;
    return (dy <= 0) ? 2 : 0;
}

void dm2_v1_calc_vector_w_dir(int16_t dir, int16_t front, int16_t side,
                               const int16_t dir_dx[4],
                               const int16_t dir_dy[4],
                               int16_t *out_x, int16_t *out_y)
{
    int16_t idx = dir;
    *out_x += (int16_t)(front * dir_dx[idx]);
    *out_y += (int16_t)(front * dir_dy[idx]);
    idx = (int16_t)((idx + 1) & 0x3);
    *out_x += (int16_t)(side * dir_dx[idx]);
    *out_y += (int16_t)(side * dir_dy[idx]);
}

int32_t dm2_v1_compute_power_4_within(int16_t a, int16_t d)
{
    int32_t result = 1;

    for (int16_t n = 0; n < 32; n++) {
        if ((result & (int32_t)a) != 0) {
            if (--d == 0)
                return result;
        }
        result <<= 1;
    }
    return result;
}

void dm2_v1_fill_i16_table(int16_t *table, int16_t value, uint16_t entries)
{
    for (uint16_t i = 0; i < entries; i++)
        table[i] = value;
}
