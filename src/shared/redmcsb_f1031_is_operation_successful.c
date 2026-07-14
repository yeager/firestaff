#include "redmcsb_f1031_is_operation_successful.h"

bool redmcsb_f1031_is_operation_successful(int16_t *error_count)
{
    if (*error_count != 0) {
        *error_count = 0;
        return false;
    }

    return true;
}

const char *redmcsb_f1031_is_operation_successful_source_evidence(void)
{
    return "ReDMCSB FILE.C:754-792 encloses F1031_IsOperationSuccessful "
           "in MEDIA607_X30J_X31J. FILE.C:763-792 tests "
           "G3091_i_ErrorCount; a nonzero count is cleared and returns "
           "C0_FALSE, while zero returns C1_TRUE. FILE.C:806-830 installs "
           "the X68000 TRAP 14 error handler that increments "
           "G3091_i_ErrorCount.";
}
