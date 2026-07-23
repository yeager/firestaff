#include "dm1_v1_p0651_p0700_parameter_owner_audit.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_P0651P0700ParameterOwnerAudit *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_p0651_p0700_parameter_owner_audit_pc34(&count);
    CHECK(entries && count == 50u);
    for (index = 0u; index < count; ++index) {
        CHECK(entries[index].parameter_number >= 651u &&
              entries[index].parameter_number <= 700u);
        CHECK(entries[index].owner_function == 312u ||
              (entries[index].owner_function >= 313u &&
               entries[index].owner_function <= 336u));
        CHECK(entries[index].source_anchor && entries[index].source_anchor[0]);
        CHECK(entries[index].standalone_port_forbidden);
    }
    CHECK(dm1_v1_p0651_p0700_parameter_owner_find_pc34(651u, 0u)->owner_function == 312u);
    CHECK(dm1_v1_p0651_p0700_parameter_owner_find_pc34(662u, 0u)->owner_function == 321u);
    CHECK(dm1_v1_p0651_p0700_parameter_owner_find_pc34(678u, 0u)->owner_function == 326u);
    CHECK(dm1_v1_p0651_p0700_parameter_owner_find_pc34(694u, 0u)->owner_function == 333u);
    CHECK(dm1_v1_p0651_p0700_parameter_owner_find_pc34(700u, 0u)->owner_function == 336u);
    CHECK(!dm1_v1_p0651_p0700_parameter_owner_find_pc34(650u, 0u));
    CHECK(!dm1_v1_p0651_p0700_parameter_owner_find_pc34(700u, 1u));
    CHECK(strstr(dm1_v1_p0651_p0700_parameter_owner_evidence_pc34(),
                 "not independent") != 0);
    return 0;
}
