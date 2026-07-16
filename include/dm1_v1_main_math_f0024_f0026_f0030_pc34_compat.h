#ifndef FIRESTAFF_DM1_V1_MAIN_MATH_F0024_F0026_F0030_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MAIN_MATH_F0024_F0026_F0030_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB MAIN helpers declared from ATARIST.H and consumed by DM1 runtime. */
int F0024_MAIN_GetMinimumValue(int a, int b);
int F0026_MAIN_GetBoundedValue(int minimum, int value, int maximum);
int F0030_MAIN_GetScaledProduct(int value, int shift, int factor);

int dm1_v1_main_get_minimum_value_f0024_pc34(int a, int b);
int dm1_v1_main_get_bounded_value_f0026_pc34(
    int minimum, int value, int maximum);
int dm1_v1_main_get_scaled_product_f0030_pc34(
    int value, int shift, int factor);
const char* dm1_v1_main_math_f0024_f0026_f0030_source_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
