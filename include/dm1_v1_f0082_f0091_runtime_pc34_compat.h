#ifndef FIRESTAFF_DM1_V1_F0082_F0091_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0082_F0091_RUNTIME_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int dm1_v1_f0082_ldiv_pc34(int32_t dividend, int32_t divisor, int32_t *out_quotient);
int32_t dm1_v1_f0083_lmul_pc34(int32_t left, int32_t right);
int dm1_v1_f0084_blockmv_pc34(void *destination, size_t destination_size,
                               const void *source, size_t source_size,
                               size_t byte_count);
char *dm1_v1_f0086_strcat_pc34(char *destination, size_t destination_size,
                                const char *source);
int16_t dm1_v1_f0087_strcmp_pc34(const char *left, const char *right);
char *dm1_v1_f0088_strcpy_pc34(char *destination, size_t destination_size,
                                const char *source);
int dm1_v1_f0090_strlen_pc34(const char *string, size_t maximum_length,
                              int16_t *out_length);
char *dm1_v1_f0091_strchr_pc34(char *string, char character);
const char *dm1_v1_f0082_f0091_runtime_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0082_F0091_RUNTIME_PC34_COMPAT_H */
