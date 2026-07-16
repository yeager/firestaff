#include "dm1_v1_main_math_f0024_f0026_f0030_pc34_compat.h"

int F0024_MAIN_GetMinimumValue(int a, int b) {
    return (a < b) ? a : b;
}

int F0026_MAIN_GetBoundedValue(int minimum, int value, int maximum) {
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

int F0030_MAIN_GetScaledProduct(int value, int shift, int factor) {
    return (int)(((long)value * (long)factor) >> shift);
}

int dm1_v1_main_get_minimum_value_f0024_pc34(int a, int b) {
    return F0024_MAIN_GetMinimumValue(a, b);
}

int dm1_v1_main_get_bounded_value_f0026_pc34(
    int minimum, int value, int maximum) {
    return F0026_MAIN_GetBoundedValue(minimum, value, maximum);
}

int dm1_v1_main_get_scaled_product_f0030_pc34(
    int value, int shift, int factor) {
    return F0030_MAIN_GetScaledProduct(value, shift, factor);
}

const char* dm1_v1_main_math_f0024_f0026_f0030_source_pc34(void) {
    return "ReDMCSB ATARIST.H F0024_MAIN_GetMinimumValue, "
           "F0026_MAIN_GetBoundedValue, and F0030_MAIN_GetScaledProduct; "
           "DM1 runtime consumers include CHAMPION.C skill/stamina math and "
           "combat scaled-product damage paths.";
}
