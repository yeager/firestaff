#include "csb_v1_f1085_f1086_f1087_intuition_vector_helpers_pc34_compat.h"
#include "redmcsb_f1085_intuition_vector_replacement_pc34_compat.h"
#include "redmcsb_f1086_replace_intuition_vectors_pc34_compat.h"
#include "redmcsb_f1087_restore_intuition_vectors_pc34_compat.h"

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

static void test_f1085_zero_callback(void)
{
    const char *csb =
        csb_v1_f1085_intuition_vector_replacement_source_evidence_pc34();
    const char *redmcsb =
        redmcsb_f1085_intuition_vector_replacement_source_evidence_pc34();

    CHECK(csb_v1_f1085_intuition_vector_replacement_pc34_compat() == 0);
    CHECK(redmcsb_f1085_intuition_vector_replacement_pc34_compat() == 0);
    CHECK(F1085_IntuitionVectorReplacement() == 0);
    CHECK(csb == redmcsb);
    check_contains(csb, "AMIGINIT.C:277");
    check_contains(csb, "F1085_IntuitionVectorReplacement");
    check_contains(csb, "zero callback");
}

static void test_f1086_f1087_are_noop_boundaries(void)
{
    int sentinel = 0x1086;

    csb_v1_f1086_replace_intuition_vectors_pc34_compat();
    redmcsb_f1086_replace_intuition_vectors_pc34_compat();
    F1086_ReplaceIntuitionVectors();

    csb_v1_f1087_restore_intuition_vectors_pc34_compat();
    redmcsb_f1087_restore_intuition_vectors_pc34_compat();
    F1087_RestoreIntuitionVectors();

    CHECK(sentinel == 0x1086);
}

static void test_f1086_evidence(void)
{
    const char *csb =
        csb_v1_f1086_replace_intuition_vectors_source_evidence_pc34();
    const char *redmcsb =
        redmcsb_f1086_replace_intuition_vectors_source_evidence_pc34();

    CHECK(csb == redmcsb);
    check_contains(csb, "AMIGINIT.C:283");
    check_contains(csb, "F1086_ReplaceIntuitionVectors");
    check_contains(csb, "Amiga-only Intuition vector");
    check_contains(csb, "no PC34 portable host vector route");
}

static void test_f1087_evidence(void)
{
    const char *csb =
        csb_v1_f1087_restore_intuition_vectors_source_evidence_pc34();
    const char *redmcsb =
        redmcsb_f1087_restore_intuition_vectors_source_evidence_pc34();

    CHECK(csb == redmcsb);
    check_contains(csb, "AMIGINIT.C:293");
    check_contains(csb, "F1087_RestoreIntuitionVectors");
    check_contains(csb, "Amiga-only Intuition vector");
    check_contains(csb, "no PC34 portable host vector route");
}

int main(void)
{
    test_f1085_zero_callback();
    test_f1086_f1087_are_noop_boundaries();
    test_f1086_evidence();
    test_f1087_evidence();
    return 0;
}
