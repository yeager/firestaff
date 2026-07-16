#include "csb_v1_f1168_f1170_f1171_f1305_f1307_usio_fio1_boundaries_pc34_compat.h"

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

static void test_boundaries_are_noops(void)
{
    volatile uint32_t sentinel = 0x11681307U;

    csb_v1_f1168_usio_18_empty_pc34_compat();
    F1168_USIO_18_Empty();
    csb_v1_f1170_usio_03_expunge_pc34_compat();
    F1170_USIO_03_Expunge();
    csb_v1_f1171_usio_19_lock_df0_pc34_compat();
    F1171_USIO_19_LockDF0();
    csb_v1_f1305_open_ftl_library_pc34_compat();
    F1305_OpenFTLLibrary();
    csb_v1_f1307_fio1_03_expunge_pc34_compat();
    F1307_FIO1_03_Expunge();

    CHECK(sentinel == 0x11681307U);
}

static void test_evidence_strings(void)
{
    const char *f1168 = csb_v1_f1168_usio_18_empty_source_evidence_pc34();
    const char *f1170 = csb_v1_f1170_usio_03_expunge_source_evidence_pc34();
    const char *f1171 = csb_v1_f1171_usio_19_lock_df0_source_evidence_pc34();
    const char *f1305 = csb_v1_f1305_open_ftl_library_source_evidence_pc34();
    const char *f1307 = csb_v1_f1307_fio1_03_expunge_source_evidence_pc34();

    check_contains(f1168, "USIO2.C:116");
    check_contains(f1168, "F1168_USIO_18_Empty");
    check_contains(f1168, "no portable PC34 side effect");

    check_contains(f1170, "USIO2.C:176");
    check_contains(f1170, "F1170_USIO_03_Expunge");
    check_contains(f1170, "no PC34 library teardown route");

    check_contains(f1171, "USIO2.C:205");
    check_contains(f1171, "F1171_USIO_19_LockDF0");
    check_contains(f1171, "no PC34 floppy lock route");

    check_contains(f1305, "FIO1MAIN.C:34");
    check_contains(f1305, "F1305_OpenFTLLibrary");
    check_contains(f1305, "no PC34 host library route");

    check_contains(f1307, "FIO1MAIN.C:31");
    check_contains(f1307, "F1307_FIO1_03_Expunge");
    check_contains(f1307, "no PC34 host library teardown route");
}

int main(void)
{
    test_boundaries_are_noops();
    test_evidence_strings();
    return 0;
}
