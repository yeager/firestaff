#include "csb_v1_f1159_f1160_f1161_f1162_f1163_usio_helpers_pc34_compat.h"

#include <stdint.h>
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

static void test_empty_usio_boundaries_are_noops(void)
{
    volatile uint32_t sentinel = 0x11591161U;

    csb_v1_f1159_empty_pc34_compat();
    F1159_Empty();
    csb_v1_f1160_usio_04_empty_pc34_compat();
    F1160_USIO_04_Empty();
    csb_v1_f1161_usio_05_empty_pc34_compat();
    F1161_USIO_05_Empty();

    CHECK(sentinel == 0x11591161U);
}

static void test_pointer_visibility_boundaries_are_noops(void)
{
    volatile uint32_t sentinel = 0x11621163U;

    csb_v1_f1162_usio_06_hide_pointer_pc34_compat();
    F1162_USIO_06_HidePointer();
    csb_v1_f1163_usio_07_show_pointer_pc34_compat();
    F1163_USIO_07_ShowPointer();

    CHECK(sentinel == 0x11621163U);
}

static void test_evidence_strings(void)
{
    const char *f1159 = csb_v1_f1159_empty_source_evidence_pc34();
    const char *f1160 = csb_v1_f1160_usio_04_empty_source_evidence_pc34();
    const char *f1161 = csb_v1_f1161_usio_05_empty_source_evidence_pc34();
    const char *f1162 =
        csb_v1_f1162_usio_06_hide_pointer_source_evidence_pc34();
    const char *f1163 =
        csb_v1_f1163_usio_07_show_pointer_source_evidence_pc34();

    check_contains(f1159, "USIOMAIN.C:12");
    check_contains(f1159, "F1159_Empty");
    check_contains(f1159, "no portable PC34 side effect");

    check_contains(f1160, "USIO1.C:29");
    check_contains(f1160, "F1160_USIO_04_Empty");
    check_contains(f1160, "no portable PC34 side effect");

    check_contains(f1161, "USIO1.C:36");
    check_contains(f1161, "F1161_USIO_05_Empty");
    check_contains(f1161, "no portable PC34 side effect");

    check_contains(f1162, "USIO1.C:43");
    check_contains(f1162, "F1162_USIO_06_HidePointer");
    check_contains(f1162, "no PC34 cursor route");

    check_contains(f1163, "USIO1.C:61");
    check_contains(f1163, "F1163_USIO_07_ShowPointer");
    check_contains(f1163, "no PC34 cursor route");
}

int main(void)
{
    test_empty_usio_boundaries_are_noops();
    test_pointer_visibility_boundaries_are_noops();
    test_evidence_strings();
    return 0;
}
