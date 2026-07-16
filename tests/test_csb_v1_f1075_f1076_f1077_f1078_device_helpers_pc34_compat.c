#include "csb_v1_f1075_f1076_f1077_f1078_device_helpers_pc34_compat.h"
#include "redmcsb_f1075_open_layers_library_pc34_compat.h"
#include "redmcsb_f1076_close_layers_library_pc34_compat.h"
#include "redmcsb_f1077_open_console_device_pc34_compat.h"
#include "redmcsb_f1078_close_console_device_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static void test_device_boundaries_are_noop(void)
{
    int sentinel = 0x1075;

    csb_v1_f1075_open_layers_library_pc34_compat();
    redmcsb_f1075_open_layers_library_pc34_compat();
    F1075_OpenLayersLibrary();

    csb_v1_f1076_close_layers_library_pc34_compat();
    redmcsb_f1076_close_layers_library_pc34_compat();
    F1076_CloseLayersLibrary();

    csb_v1_f1077_open_console_device_pc34_compat();
    redmcsb_f1077_open_console_device_pc34_compat();
    F1077_OpenConsoleDevice();

    csb_v1_f1078_close_console_device_pc34_compat();
    redmcsb_f1078_close_console_device_pc34_compat();
    F1078_CloseConsoleDevice();

    CHECK(sentinel == 0x1075);
}

static void test_f1075_evidence(void)
{
    const char *csb =
        csb_v1_f1075_open_layers_library_source_evidence_pc34();
    const char *redmcsb =
        redmcsb_f1075_open_layers_library_source_evidence_pc34();

    CHECK(csb == redmcsb);
    check_contains(csb, "AMIGINIT.C:132-143,333-361");
    check_contains(csb, "F1075_OpenLayersLibrary");
    check_contains(csb, "layers.library");
    check_contains(csb, "no PC34 portable host behavior");
}

static void test_f1076_evidence(void)
{
    const char *csb =
        csb_v1_f1076_close_layers_library_source_evidence_pc34();
    const char *redmcsb =
        redmcsb_f1076_close_layers_library_source_evidence_pc34();

    CHECK(csb == redmcsb);
    check_contains(csb, "AMIGINIT.C:4,145-152,363-378");
    check_contains(csb, "F1076_CloseLayersLibrary");
    check_contains(csb, "Layers-library");
    check_contains(csb, "no PC34 portable host behavior");
}

static void test_f1077_evidence(void)
{
    const char *csb =
        csb_v1_f1077_open_console_device_source_evidence_pc34();
    const char *redmcsb =
        redmcsb_f1077_open_console_device_source_evidence_pc34();

    CHECK(csb == redmcsb);
    check_contains(csb, "AMIGINIT.C:4,132-180,333-361");
    check_contains(csb, "F1077_OpenConsoleDevice");
    check_contains(csb, "console.device");
    check_contains(csb, "no PC34 portable host behavior");
}

static void test_f1078_evidence(void)
{
    const char *csb =
        csb_v1_f1078_close_console_device_source_evidence_pc34();
    const char *redmcsb =
        redmcsb_f1078_close_console_device_source_evidence_pc34();

    CHECK(csb == redmcsb);
    check_contains(csb, "AMIGINIT.C:4,182-198,363-379");
    check_contains(csb, "F1078_CloseConsoleDevice");
    check_contains(csb, "console.device");
    check_contains(csb, "no PC34 portable host behavior");
}

int main(void)
{
    test_device_boundaries_are_noop();
    test_f1075_evidence();
    test_f1076_evidence();
    test_f1077_evidence();
    test_f1078_evidence();
    return 0;
}
