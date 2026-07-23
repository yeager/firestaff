#include "dm1_v1_p0701_p0750_parameter_owner_audit.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_P0701P0750ParameterOwnerAudit *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_p0701_p0750_parameter_owner_audit_pc34(&count);
    CHECK(entries && count == 52u);
    for (index = 0u; index < count; ++index) {
        CHECK(entries[index].parameter_number >= 701u &&
              entries[index].parameter_number <= 750u);
        CHECK(entries[index].owner_function >= 336u &&
              entries[index].owner_function <= 798u);
        CHECK(entries[index].source_anchor && entries[index].source_anchor[0]);
        CHECK(entries[index].standalone_port_forbidden);
    }
    CHECK(dm1_v1_p0701_p0750_parameter_owner_find_pc34(701u, 0u)->owner_function == 336u);
    CHECK(dm1_v1_p0701_p0750_parameter_owner_find_pc34(720u, 0u)->owner_function == 356u);
    CHECK(dm1_v1_p0701_p0750_parameter_owner_find_pc34(720u, 1u)->owner_function == 356u);
    CHECK(dm1_v1_p0701_p0750_parameter_owner_find_pc34(749u, 1u)->owner_function == 798u);
    CHECK(dm1_v1_p0701_p0750_parameter_owner_find_pc34(750u, 0u)->owner_function == 376u);
    CHECK(!dm1_v1_p0701_p0750_parameter_owner_find_pc34(700u, 0u));
    CHECK(!dm1_v1_p0701_p0750_parameter_owner_find_pc34(750u, 1u));
    CHECK(strstr(dm1_v1_p0701_p0750_parameter_owner_evidence_pc34(),
                 "not independent") != 0);
    return 0;
}
