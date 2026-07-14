#include "redmcsb_f0780_file_is_handle_invalid_pc34_compat.h"

#include <stdint.h>

int main(void)
{
    if (redmcsb_f0780_file_is_handle_invalid_pc34_compat(INT16_C(-1)) ==
        false) {
        return 1;
    }
    if (redmcsb_f0780_file_is_handle_invalid_pc34_compat(INT16_MIN) ==
        false) {
        return 2;
    }
    if (redmcsb_f0780_file_is_handle_invalid_pc34_compat(INT16_C(0)) !=
        false) {
        return 3;
    }
    if (redmcsb_f0780_file_is_handle_invalid_pc34_compat(INT16_MAX) !=
        false) {
        return 4;
    }
    return 0;
}
