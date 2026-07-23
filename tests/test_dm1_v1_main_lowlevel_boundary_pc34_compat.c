#include "dm1_v1_main_lowlevel_boundary_pc34_compat.h"
#include "byteops_pc34_compat.h"
#include "redmcsb_f0009_f0010_spaced_writes_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

int main(void)
{
    const DM1_V1_MainLowLevelAuditPc34Compat *entry;
    DM1_V1_F0018ExceptionVectorReceiptPc34Compat receipt;
    char bytes[] = { 'a', 'b', 'c', 'd', 'e', 'f' };
    int16_t words[] = { 0, 0, 0, 0, 0 };
    uint16_t id;

    for (id = 3u; id <= 10u; ++id) {
        entry = dm1_v1_main_lowlevel_audit_pc34(id);
        CHECK(entry != NULL && entry->functionNumber == id &&
              entry->sourceAnchor != NULL);
    }
    entry = dm1_v1_main_lowlevel_audit_pc34(18u);
    CHECK(entry != NULL &&
          entry->kind == DM1_V1_MAIN_LOWLEVEL_EXCEPTION_VECTOR_BOUNDARY &&
          entry->hostMutationForbidden && entry->hasFirestaffOwner);
    CHECK(dm1_v1_main_lowlevel_audit_pc34(11u) == NULL);
    CHECK(strstr(dm1_v1_main_lowlevel_source_evidence_pc34(), "F0018") != NULL);

    CHECK(!dm1_v1_f0018_set_exception_vectors_host_boundary_pc34(&receipt));
    CHECK(!receipt.accepted && receipt.hostVectorMutationSuppressed &&
          receipt.syntheticInterruptSuppressed &&
          receipt.sourceFingerprint == 0u);
    CHECK(!dm1_v1_f0018_set_exception_vectors_host_boundary_pc34(NULL));

    F0007_MAIN_CopyBytes(bytes, bytes + 1, 5);
    CHECK(memcmp(bytes, "aabcde", 6u) == 0);
    F0008_MAIN_ClearBytes(bytes + 2, 3u);
    CHECK(bytes[0] == 'a' && bytes[1] == 'a' && bytes[2] == 0 &&
          bytes[3] == 0 && bytes[4] == 0 && bytes[5] == 'e');
    F0009_MAIN_WriteSpacedBytes(bytes, 3u, 'x', 2);
    CHECK(bytes[0] == 'x' && bytes[2] == 'x' && bytes[4] == 'x');
    F0010_MAIN_WriteSpacedWords(words, 3u, (int16_t)-7, 4);
    CHECK(words[0] == -7 && words[2] == -7 && words[4] == -7);

    printf("test_dm1_v1_main_lowlevel_boundary_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
