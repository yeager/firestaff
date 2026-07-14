#ifndef FIRESTAFF_REDMCSB_F1034_WRITE_CURRENT_TASK_DIAGNOSTIC_H
#define FIRESTAFF_REDMCSB_F1034_WRITE_CURRENT_TASK_DIAGNOSTIC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*redmcsb_f1034_find_task_fn)(void *context);
typedef int16_t (*redmcsb_f1034_string_length_fn)(void *context,
                                                   const char *string);
typedef void (*redmcsb_f1034_write_fn)(void *context,
                                       void *file_handle,
                                       const char *string,
                                       long byte_count);

/*
 * ReDMCSB F1034 writes to the NIL: handle only from the task captured at
 * startup. The dependencies expose the original Amiga calls for a host.
 */
void redmcsb_f1034_write_current_task_diagnostic(
    redmcsb_f1034_find_task_fn find_task,
    redmcsb_f1034_string_length_fn string_length,
    redmcsb_f1034_write_fn write,
    void *context,
    void *current_task,
    void *nil_handle,
    const char *string);

const char *redmcsb_f1034_write_current_task_diagnostic_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1034_WRITE_CURRENT_TASK_DIAGNOSTIC_H */
