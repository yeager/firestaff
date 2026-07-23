#include "dm1_v1_p0551_p0600_parameter_owner_audit.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_P0551P0600ParameterOwnerAudit *entries;
    size_t count;
    size_t index;

    entries = dm1_v1_p0551_p0600_parameter_owner_audit_pc34(&count);
    CHECK(entries && count == 51u);
    for (index = 0u; index < count; ++index) {
        CHECK(entries[index].parameter_number >= 551u &&
              entries[index].parameter_number <= 600u);
        CHECK(entries[index].owner_function >= 265u &&
              entries[index].owner_function <= 284u);
        CHECK(entries[index].source_anchor && entries[index].source_anchor[0]);
        CHECK(entries[index].standalone_port_forbidden);
    }
    CHECK(dm1_v1_p0551_p0600_parameter_owner_find_pc34(551u, 0u)->owner_function == 265u);
    CHECK(dm1_v1_p0551_p0600_parameter_owner_find_pc34(593u, 0u)->owner_function == 277u);
    CHECK(dm1_v1_p0551_p0600_parameter_owner_find_pc34(593u, 1u)->owner_function == 277u);
    CHECK(!dm1_v1_p0551_p0600_parameter_owner_find_pc34(593u, 2u));
    CHECK(dm1_v1_p0551_p0600_parameter_owner_find_pc34(600u, 0u)->owner_function == 284u);
    CHECK(!dm1_v1_p0551_p0600_parameter_owner_find_pc34(550u, 0u));
    CHECK(strstr(dm1_v1_p0551_p0600_parameter_owner_evidence_pc34(),
                 "not independent") != 0);
    return 0;
}
