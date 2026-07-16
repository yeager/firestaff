#include "csb_v1_f0089_strncpy_pc34_compat.h"
#include "redmcsb_f0089_strncpy.h"

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

static void test_copies_until_count(void)
{
    char buffer[8];

    memset(buffer, 'x', sizeof(buffer));
    CHECK(F0089_strncpy(buffer, "abcdef", 3) == buffer);
    CHECK(memcmp(buffer, "abcxxxxx", sizeof(buffer)) == 0);
}

static void test_copies_nul_and_does_not_pad(void)
{
    char buffer[8];

    memset(buffer, 'x', sizeof(buffer));
    CHECK(csb_v1_f0089_strncpy_pc34_compat(buffer, "ab", 6) == buffer);
    CHECK(buffer[0] == 'a');
    CHECK(buffer[1] == 'b');
    CHECK(buffer[2] == '\0');
    CHECK(buffer[3] == 'x');
    CHECK(buffer[7] == 'x');
}

static void test_invalid_or_nonpositive_count_is_noop(void)
{
    char buffer[4] = {'a', 'b', 'c', '\0'};

    CHECK(redmcsb_f0089_strncpy(buffer, "z", 0) == buffer);
    CHECK(strcmp(buffer, "abc") == 0);
    CHECK(csb_v1_f0089_strncpy_pc34_compat(buffer, "z", -4) == buffer);
    CHECK(strcmp(buffer, "abc") == 0);
    CHECK(csb_v1_f0089_strncpy_pc34_compat(NULL, "z", 1) == NULL);
}

static void test_source_evidence(void)
{
    const char *evidence = csb_v1_f0089_strncpy_source_evidence_pc34();

    CHECK(evidence != NULL);
    CHECK(strstr(evidence, "F0089_strncpy") != NULL);
    CHECK(strstr(evidence, "DEFS.H:3085") != NULL);
    CHECK(strstr(evidence, "do not pad") != NULL);
}

int main(void)
{
    test_copies_until_count();
    test_copies_nul_and_does_not_pad();
    test_invalid_or_nonpositive_count_is_noop();
    test_source_evidence();
    return 0;
}
