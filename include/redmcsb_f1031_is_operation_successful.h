#ifndef FIRESTAFF_REDMCSB_F1031_IS_OPERATION_SUCCESSFUL_H
#define FIRESTAFF_REDMCSB_F1031_IS_OPERATION_SUCCESSFUL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB F1031 tests the X68000 TRAP 14 error counter. A nonzero count is
 * consumed by clearing it and reports failure; zero reports success.
 */
bool redmcsb_f1031_is_operation_successful(int16_t *error_count);

const char *redmcsb_f1031_is_operation_successful_source_evidence(void);

/* ReDMCSB source-named alias for
 * redmcsb_f1031_is_operation_successful. */
bool F1031_IsOperationSuccessful(int16_t *error_count);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1031_IS_OPERATION_SUCCESSFUL_H */
