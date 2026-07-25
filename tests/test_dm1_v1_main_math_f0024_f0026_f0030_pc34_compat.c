#include "dm1_v1_main_math_f0024_f0026_f0030_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    const char* source;
    (void)source;

    assert(F0024_MAIN_GetMinimumValue(4, 9) == 4);
    assert(F0024_MAIN_GetMinimumValue(9, 4) == 4);
    assert(F0024_MAIN_GetMinimumValue(-3, 4) == -3);
    assert(dm1_v1_main_get_minimum_value_f0024_pc34(7, 7) == 7);

    assert(F0026_MAIN_GetBoundedValue(0, -5, 100) == 0);
    assert(F0026_MAIN_GetBoundedValue(0, 42, 100) == 42);
    assert(F0026_MAIN_GetBoundedValue(0, 142, 100) == 100);
    assert(dm1_v1_main_get_bounded_value_f0026_pc34(21, 17, 255) == 21);
    assert(dm1_v1_main_get_bounded_value_f0026_pc34(21, 300, 255) == 255);

    assert(F0030_MAIN_GetScaledProduct(80, 3, 7) == 70);
    assert(F0030_MAIN_GetScaledProduct(65, 6, 130) == 132);
    assert(F0030_MAIN_GetScaledProduct(120, 7, 64) == 60);
    assert(dm1_v1_main_get_scaled_product_f0030_pc34(57, 3, 11) == 78);

    source = dm1_v1_main_math_f0024_f0026_f0030_source_pc34();
    assert(source != 0);
    assert(strstr(source, "F0024_MAIN_GetMinimumValue") != 0);
    assert(strstr(source, "F0026_MAIN_GetBoundedValue") != 0);
    assert(strstr(source, "F0030_MAIN_GetScaledProduct") != 0);
    return 0;
}
