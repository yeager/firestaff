#include "redmcsb_f1034_write_current_task_diagnostic.h"

void redmcsb_f1034_write_current_task_diagnostic(
    redmcsb_f1034_find_task_fn find_task,
    redmcsb_f1034_string_length_fn string_length,
    redmcsb_f1034_write_fn write,
    void *context,
    void *current_task,
    void *nil_handle,
    const char *string)
{
    if (find_task(context) == current_task) {
        write(context, nil_handle, string, (long)string_length(context, string));
    }
}

const char *redmcsb_f1034_write_current_task_diagnostic_source_evidence(void)
{
    return "ReDMCSB IO2.C:219-225 defines F1034_ under "
           "MEDIA746_A36M_A31E_A31M_A33M_A35E_A35M: FindTask(0L) must "
           "equal G3165_ps_CurrentTask before one Write of G3159_ps_NIL1, "
           "the supplied string, and (long)F0090_strlen(string). "
           "AMIGINIT.C:239 opens G3159_ps_NIL1 as NIL:, and "
           "AMIGINIT.C:341 captures G3165_ps_CurrentTask with FindTask(0L).";
}
