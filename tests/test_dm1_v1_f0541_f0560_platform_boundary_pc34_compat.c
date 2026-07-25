#include "dm1_v1_f0541_f0560_platform_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    unsigned int i;

    assert(dm1_v1_f0541_f0560_platform_boundary_pc34_count() == 13U);
    for (i = 0; i < dm1_v1_f0541_f0560_platform_boundary_pc34_count(); ++i) {
        const DM1_V1_F0541F0560PlatformBoundaryPc34 *boundary =
            dm1_v1_f0541_f0560_platform_boundary_pc34_at(i);
        (void)boundary;
        assert(boundary != 0);
        assert(strncmp(boundary->symbol, "F05", 3) == 0);
        assert(boundary->source_anchor[0] != '\0');
        assert(boundary->platform_partition[0] != '\0');
        assert(boundary->pc34_route_or_rationale[0] != '\0');
        assert(!dm1_v1_f0541_f0560_platform_boundary_has_pc34_route(boundary));
    }
    assert(dm1_v1_f0541_f0560_platform_boundary_pc34_at(13) == 0);
    assert(!dm1_v1_f0541_f0560_platform_boundary_has_pc34_route(0));

    {
        const char *evidence =
            dm1_v1_f0541_f0560_platform_boundary_source_evidence_pc34();
        (void)evidence;
        assert(strstr(evidence, "INPUT.C") != 0);
        assert(strstr(evidence, "F0550") != 0);
        assert(strstr(evidence, "F0550's generic helper") != 0);
    }
    return 0;
}
