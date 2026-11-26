#include "csb_v1_atari_f0115_alcove_shift.h"

unsigned int csb_v1_atari_f0115_alcove_shift_advance(
    unsigned int shift_index, int alcove_y)
{
    if (alcove_y == 0) return shift_index;
    ++shift_index;
    return shift_index >= 14u ? 2u : shift_index;
}
