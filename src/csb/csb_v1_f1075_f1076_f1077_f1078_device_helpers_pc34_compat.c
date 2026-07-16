#include "csb_v1_f1075_f1076_f1077_f1078_device_helpers_pc34_compat.h"
#include "redmcsb_f1075_open_layers_library_pc34_compat.h"
#include "redmcsb_f1076_close_layers_library_pc34_compat.h"
#include "redmcsb_f1077_open_console_device_pc34_compat.h"
#include "redmcsb_f1078_close_console_device_pc34_compat.h"

void csb_v1_f1075_open_layers_library_pc34_compat(void)
{
}

void redmcsb_f1075_open_layers_library_pc34_compat(void)
{
    csb_v1_f1075_open_layers_library_pc34_compat();
}

void F1075_OpenLayersLibrary(void)
{
    csb_v1_f1075_open_layers_library_pc34_compat();
}

const char *csb_v1_f1075_open_layers_library_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:132-143,333-361 "
           "F1075_OpenLayersLibrary; Amiga-only layers.library open route, "
           "no PC34 portable host behavior";
}

const char *redmcsb_f1075_open_layers_library_source_evidence_pc34(void)
{
    return csb_v1_f1075_open_layers_library_source_evidence_pc34();
}

void csb_v1_f1076_close_layers_library_pc34_compat(void)
{
}

void redmcsb_f1076_close_layers_library_pc34_compat(void)
{
    csb_v1_f1076_close_layers_library_pc34_compat();
}

void F1076_CloseLayersLibrary(void)
{
    csb_v1_f1076_close_layers_library_pc34_compat();
}

const char *csb_v1_f1076_close_layers_library_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:4,145-152,363-378 "
           "F1076_CloseLayersLibrary; Amiga-only Layers-library teardown "
           "route, no PC34 portable host behavior";
}

const char *redmcsb_f1076_close_layers_library_source_evidence_pc34(void)
{
    return csb_v1_f1076_close_layers_library_source_evidence_pc34();
}

void csb_v1_f1077_open_console_device_pc34_compat(void)
{
}

void redmcsb_f1077_open_console_device_pc34_compat(void)
{
    csb_v1_f1077_open_console_device_pc34_compat();
}

void F1077_OpenConsoleDevice(void)
{
    csb_v1_f1077_open_console_device_pc34_compat();
}

const char *csb_v1_f1077_open_console_device_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:4,132-180,333-361 "
           "F1077_OpenConsoleDevice; Amiga-only console.device open route, "
           "no PC34 portable host behavior";
}

const char *redmcsb_f1077_open_console_device_source_evidence_pc34(void)
{
    return csb_v1_f1077_open_console_device_source_evidence_pc34();
}

void csb_v1_f1078_close_console_device_pc34_compat(void)
{
}

void redmcsb_f1078_close_console_device_pc34_compat(void)
{
    csb_v1_f1078_close_console_device_pc34_compat();
}

void F1078_CloseConsoleDevice(void)
{
    csb_v1_f1078_close_console_device_pc34_compat();
}

const char *csb_v1_f1078_close_console_device_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:4,182-198,363-379 "
           "F1078_CloseConsoleDevice; Amiga-only console.device teardown "
           "route, no PC34 portable host behavior";
}

const char *redmcsb_f1078_close_console_device_source_evidence_pc34(void)
{
    return csb_v1_f1078_close_console_device_source_evidence_pc34();
}
