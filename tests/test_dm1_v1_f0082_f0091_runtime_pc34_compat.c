#include "dm1_v1_f0082_f0091_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;
#define CHECK(expression) do { ++assertions; if (!(expression)) { ++failures; \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); } } while (0)

int main(void)
{
    int32_t quotient = 0;
    int16_t length = -1;
    char overlap[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', '\0' };
    char string[12] = "DM";
    char copy[8] = { 0 };
    CHECK(dm1_v1_f0082_ldiv_pc34(-21, 4, &quotient) && quotient == -5);
    CHECK(!dm1_v1_f0082_ldiv_pc34(1, 0, &quotient));
    CHECK(dm1_v1_f0083_lmul_pc34(0x10000, 0x10001) == 0x10000);
    CHECK(dm1_v1_f0084_blockmv_pc34(overlap + 1, 7, overlap, 8, 6));
    CHECK(memcmp(overlap, "aabcdef", 7) == 0);
    CHECK(!dm1_v1_f0084_blockmv_pc34(overlap, 2, overlap + 1, 7, 3));
    CHECK(dm1_v1_f0086_strcat_pc34(string, sizeof(string), "1") == string &&
          strcmp(string, "DM1") == 0);
    CHECK(!dm1_v1_f0086_strcat_pc34(string, 4, "XYZ"));
    CHECK(dm1_v1_f0087_strcmp_pc34("abc", "abd") < 0);
    CHECK(dm1_v1_f0088_strcpy_pc34(copy, sizeof(copy), "CSB") == copy &&
          strcmp(copy, "CSB") == 0);
    CHECK(!dm1_v1_f0088_strcpy_pc34(copy, 3, "CSB"));
    CHECK(dm1_v1_f0090_strlen_pc34("Nexus", 6, &length) && length == 5);
    CHECK(!dm1_v1_f0090_strlen_pc34("Nexus", 5, &length));
    CHECK(dm1_v1_f0091_strchr_pc34(copy, 'S') == copy + 1);
    CHECK(!dm1_v1_f0091_strchr_pc34(copy, '\0'));
    CHECK(strstr(dm1_v1_f0082_f0091_runtime_source_evidence_pc34(),
                 "STRING.C:6-70") != NULL);
    printf("test_dm1_v1_f0082_f0091_runtime_pc34_compat: %d assertions, %d failures\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
