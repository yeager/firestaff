#include "csb_v1_f0089_strncpy_pc34_compat.h"
#include "redmcsb_f0089_strncpy.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_exact_count_copy_without_nul(void)
{
    char dst[] = "xxxxx";

    CHECK(F0089_strncpy(dst, "ABCD", 2) == dst);
    CHECK(dst[0] == 'A');
    CHECK(dst[1] == 'B');
    CHECK(dst[2] == 'x');
    CHECK(dst[3] == 'x');
    CHECK(dst[4] == 'x');
    return 0;
}

static int test_copies_nul_and_does_not_pad(void)
{
    char dst[] = { 'x', 'x', 'x', 'x', 'x', 'x' };

    CHECK(F0089_strncpy(dst, "A", 5) == dst);
    CHECK(dst[0] == 'A');
    CHECK(dst[1] == '\0');
    CHECK(dst[2] == 'x');
    CHECK(dst[3] == 'x');
    CHECK(dst[4] == 'x');
    CHECK(dst[5] == 'x');
    return 0;
}

static int test_zero_and_negative_counts_are_bounded_noops(void)
{
    char zero_dst[] = "zero";
    char negative_dst[] = "neg";

    CHECK(F0089_strncpy(zero_dst, "AB", 0) == zero_dst);
    CHECK(strcmp(zero_dst, "zero") == 0);
    CHECK(F0089_strncpy(negative_dst, "AB", (int16_t)-1) == negative_dst);
    CHECK(strcmp(negative_dst, "neg") == 0);
    return 0;
}

static int test_null_arguments_return_destination_without_write(void)
{
    char dst[] = "safe";

    CHECK(F0089_strncpy(dst, 0, 4) == dst);
    CHECK(strcmp(dst, "safe") == 0);
    CHECK(F0089_strncpy(0, "AB", 2) == 0);
    return 0;
}

static int test_csb_and_shared_names_delegate_to_same_boundary(void)
{
    char csb_dst[] = "xxxx";
    char shared_dst[] = "xxxx";

    CHECK(csb_v1_f0089_strncpy_pc34_compat(csb_dst, "Q", 4) == csb_dst);
    CHECK(redmcsb_f0089_strncpy(shared_dst, "Q", 4) == shared_dst);
    CHECK(memcmp(csb_dst, shared_dst, sizeof(csb_dst)) == 0);
    return 0;
}

static int test_source_evidence_names_f0089(void)
{
    const char *evidence = csb_v1_f0089_strncpy_source_evidence_pc34();

    CHECK(evidence != 0);
    CHECK(strstr(evidence, "F0089_strncpy") != 0);
    CHECK(strstr(evidence, "do not pad") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_exact_count_copy_without_nul() == 0);
    CHECK(test_copies_nul_and_does_not_pad() == 0);
    CHECK(test_zero_and_negative_counts_are_bounded_noops() == 0);
    CHECK(test_null_arguments_return_destination_without_write() == 0);
    CHECK(test_csb_and_shared_names_delegate_to_same_boundary() == 0);
    CHECK(test_source_evidence_names_f0089() == 0);
    return 0;
}
