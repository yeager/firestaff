#include "dm1_v1_early_viewport_family_audit_pc34_compat.h"

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
    const DM1_V1_EarlyViewportAuditPc34Compat *entry;
    uint16_t function_number;

    for (function_number = 100u; function_number <= 120u; ++function_number) {
        entry = dm1_v1_early_viewport_family_audit_pc34(function_number);
        CHECK(entry != NULL && entry->functionNumber == function_number &&
              entry->requiresSourceReceipt && entry->hostFallbackForbidden &&
              entry->sourceAnchor != NULL && entry->firestaffOwner != NULL);
    }
    CHECK(dm1_v1_early_viewport_family_audit_pc34(99u) == NULL);
    CHECK(dm1_v1_early_viewport_family_audit_pc34(121u) == NULL);
    entry = dm1_v1_early_viewport_family_audit_pc34(112u);
    CHECK(entry != NULL && entry->owner == DM1_V1_EARLY_VIEWPORT_OWNER_CEILING_PIT);
    entry = dm1_v1_early_viewport_family_audit_pc34(116u);
    CHECK(entry != NULL && entry->owner == DM1_V1_EARLY_VIEWPORT_OWNER_SQUARE_SCHEDULER);
    CHECK(strstr(dm1_v1_early_viewport_family_source_evidence_pc34(), "F0120") != NULL);

    printf("test_dm1_v1_early_viewport_family_audit_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
