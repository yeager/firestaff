#include "redmcsb_f0085_blockcmp_pc34_compat.h"

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

static void test_equal_and_counted_prefix(void)
{
    const uint8_t a[] = {1U, 2U, 3U, 4U};
    const uint8_t b[] = {1U, 2U, 3U, 9U};

    CHECK(F0085_blockcmp(a, a, 4) == 0);
    CHECK(redmcsb_f0085_blockcmp_pc34_compat(a, b, 3) == 0);
    CHECK(redmcsb_f0085_blockcmp_pc34_compat(a, b, 4) < 0);
    CHECK(redmcsb_f0085_blockcmp_pc34_compat(b, a, 4) > 0);
}

static void test_nonpositive_count_and_nulls(void)
{
    const uint8_t a[] = {1U};

    CHECK(redmcsb_f0085_blockcmp_pc34_compat(a, NULL, 0) == 0);
    CHECK(redmcsb_f0085_blockcmp_pc34_compat(a, NULL, -1) == 0);
    CHECK(redmcsb_f0085_blockcmp_pc34_compat(NULL, NULL, 1) == 0);
    CHECK(redmcsb_f0085_blockcmp_pc34_compat(a, NULL, 1) > 0);
    CHECK(redmcsb_f0085_blockcmp_pc34_compat(NULL, a, 1) < 0);
}

static void test_source_evidence(void)
{
    const char *evidence = redmcsb_f0085_blockcmp_source_evidence_pc34();

    CHECK(evidence != NULL);
    CHECK(strstr(evidence, "DEFS.H:6902") != NULL);
    CHECK(strstr(evidence, "_blockcmp") != NULL);
    CHECK(strstr(evidence, "structure bytes") != NULL);
}

int main(void)
{
    test_equal_and_counted_prefix();
    test_nonpositive_count_and_nulls();
    test_source_evidence();
    return 0;
}
