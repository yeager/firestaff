#include "csb_v1_f1070_f1071_f1072_f1074_library_helpers_pc34_compat.h"
#include "redmcsb_f1070_close_dos_library_pc34_compat.h"
#include "redmcsb_f1071_open_graphics_library_pc34_compat.h"
#include "redmcsb_f1072_close_graphics_library_pc34_compat.h"
#include "redmcsb_f1074_close_intuition_library_pc34_compat.h"

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

static void test_library_boundaries_are_noop(void)
{
    int sentinel = 0x1070;

    redmcsb_f1070_close_dos_library_pc34_compat();
    F1070_CloseDosLibrary();
    redmcsb_f1071_open_graphics_library_pc34_compat();
    F1071_OpenGraphicsLibrary();
    redmcsb_f1072_close_graphics_library_pc34_compat();
    F1072_CloseGraphicsLibrary();
    redmcsb_f1074_close_intuition_library_pc34_compat();
    F1074_CloseIntuitionLibrary();

    CHECK(sentinel == 0x1070);
}

static void test_f1070_evidence(void)
{
    const char *evidence =
        redmcsb_f1070_close_dos_library_source_evidence_pc34();

    check_contains(evidence, "AMIGINIT.C:4");
    check_contains(evidence, "AMIGINIT.C:79-87");
    check_contains(evidence, "F1070_CloseDosLibrary");
    check_contains(evidence, "DOSBase");
    check_contains(evidence, "No PC 3.4 branch");
}

static void test_f1071_evidence(void)
{
    const char *evidence =
        redmcsb_f1071_open_graphics_library_source_evidence_pc34();

    check_contains(evidence, "AMIGINIT.C:4");
    check_contains(evidence, "AMIGINIT.C:89-99");
    check_contains(evidence, "F1071_OpenGraphicsLibrary");
    check_contains(evidence, "graphics.library");
    check_contains(evidence, "version 31");
    check_contains(evidence, "GfxBase");
    check_contains(evidence, "No PC 3.4 branch");
}

static void test_f1072_evidence(void)
{
    const char *evidence =
        redmcsb_f1072_close_graphics_library_source_evidence_pc34();

    check_contains(evidence, "AMIGINIT.C:4");
    check_contains(evidence, "AMIGINIT.C:101-108");
    check_contains(evidence, "F1072_CloseGraphicsLibrary");
    check_contains(evidence, "GfxBase");
    check_contains(evidence, "No PC 3.4 branch");
}

static void test_f1074_evidence(void)
{
    const char *evidence =
        redmcsb_f1074_close_intuition_library_source_evidence_pc34();

    check_contains(evidence, "AMIGINIT.C:4");
    check_contains(evidence, "AMIGINIT.C:122-130");
    check_contains(evidence, "F1074_CloseIntuitionLibrary");
    check_contains(evidence, "IntuitionBase");
    check_contains(evidence, "No PC 3.4 branch");
}

int main(void)
{
    test_library_boundaries_are_noop();
    test_f1070_evidence();
    test_f1071_evidence();
    test_f1072_evidence();
    test_f1074_evidence();
    return 0;
}
