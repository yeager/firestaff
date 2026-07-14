#include "redmcsb_f0938_call_close_workbench_pc34_compat.h"

#include <stddef.h>

void redmcsb_f0938_call_close_workbench_pc34_compat(
    const redmcsb_f0938_call_close_workbench_ops_pc34_compat *ops,
    void *context)
{
    void *intuition_base;

    intuition_base = ops->open_library(context, "intuition.library", 0L);
    if (intuition_base == NULL) {
        ops->alert(context, 0x80444D01UL, 0L);
    }
    ops->delay(context, 100L);
    if (!ops->close_workbench(context)) {
        ops->alert(context, 0x80444D03UL, 0L);
    }
    ops->close_library(context, intuition_base);
}

const char *redmcsb_f0938_call_close_workbench_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:275-292 defines F0938_CallCloseWorkbench: "
           "OpenLibrary(\"intuition.library\", 0L), Alert(0x80444D01, 0L) "
           "when it fails, Delay(100L), CloseWorkBench(), Alert(0x80444D03, "
           "0L) when that fails, then CloseLibrary(IntuitionBase). "
           "EXEC.C:4-11 gates the route behind MEDIA442_A20E_A21E; no PC "
           "3.4 route is supplied.";
}
