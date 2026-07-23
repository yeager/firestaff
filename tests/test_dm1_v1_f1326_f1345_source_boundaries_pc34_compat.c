#include "dm1_v1_f1326_f1345_source_boundaries_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    static const unsigned int numbers[] = {1326U, 1327U, 1337U, 1340U,
                                           1343U, 1344U, 1345U};
    unsigned int index;
    const DM1_V1_F1326F1345BoundaryPc34 *entry;

    for (index = 0; index < sizeof(numbers) / sizeof(numbers[0]); ++index) {
        entry = dm1_v1_f1326_f1345_source_boundary_pc34(numbers[index]);
        CHECK(entry != 0 && entry->number == numbers[index]);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_f1326_f1345_admits_authentic_route_pc34(numbers[index]));
        CHECK(!dm1_v1_f1326_f1345_has_synthetic_route_pc34(numbers[index]));
    }
    CHECK(!dm1_v1_f1326_f1345_source_boundary_pc34(1328U));
    CHECK(!dm1_v1_f1326_f1345_source_boundary_pc34(1342U));
    CHECK(strstr(dm1_v1_f1326_f1345_source_boundary_evidence_pc34(),
                 "Existing F1328") != 0);
    return 0;
}
