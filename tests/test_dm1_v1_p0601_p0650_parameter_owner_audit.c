#include "dm1_v1_p0601_p0650_parameter_owner_audit.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_P0601P0650ParameterOwnerAudit *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_p0601_p0650_parameter_owner_audit_pc34(&count);
    CHECK(entries && count == 55u);
    for (index = 0u; index < count; ++index) {
        CHECK(entries[index].parameter_number >= 601u &&
              entries[index].parameter_number <= 650u);
        CHECK(entries[index].owner_function == 2u ||
              (entries[index].owner_function >= 285u &&
               entries[index].owner_function <= 312u) ||
              entries[index].owner_function == 7020u);
        CHECK(entries[index].source_anchor && entries[index].source_anchor[0]);
        CHECK(entries[index].standalone_port_forbidden);
    }
    CHECK(dm1_v1_p0601_p0650_parameter_owner_find_pc34(601u, 0u)->owner_function == 285u);
    CHECK(dm1_v1_p0601_p0650_parameter_owner_find_pc34(609u, 1u)->owner_function == 289u);
    CHECK(dm1_v1_p0601_p0650_parameter_owner_find_pc34(621u, 1u)->owner_function == 297u);
    CHECK(dm1_v1_p0601_p0650_parameter_owner_find_pc34(624u, 1u)->owner_function == 7020u);
    CHECK(!dm1_v1_p0601_p0650_parameter_owner_find_pc34(624u, 2u));
    CHECK(dm1_v1_p0601_p0650_parameter_owner_find_pc34(650u, 0u)->owner_function == 312u);
    CHECK(!dm1_v1_p0601_p0650_parameter_owner_find_pc34(600u, 0u));
    CHECK(strstr(dm1_v1_p0601_p0650_parameter_owner_evidence_pc34(),
                 "not independent") != 0);
    return 0;
}
