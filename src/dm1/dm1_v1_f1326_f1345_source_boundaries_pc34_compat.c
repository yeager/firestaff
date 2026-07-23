#include "dm1_v1_f1326_f1345_source_boundaries_pc34_compat.h"

static const DM1_V1_F1326F1345BoundaryPc34 kBoundaries[] = {
    {1326, "F1326_FIO1_05_FormatSaveDisk", "FIO1.C:285", "Disk formatting is media-specific; no PC34 format route or generated save medium is admitted."},
    {1327, "F1327_FIO1_23_", "FIO1.C:50", "FIO1 lock/device helper lacks an authenticated PC34 media route."},
    {1337, "F1337_FIO1_13_Lock", "FIO1.C:634", "Source file lock depends on original media/lock state; no host lock synthesis is admitted."},
    {1340, "F1340_FIO1_09_", "FIO1.C:728", "Drive-specific FIO1 route lacks an authenticated PC34 device receipt."},
    {1343, "F1343_FIO1_06_", "FIO1.C:791", "FIO1 device route lacks an authenticated PC34 device receipt."},
    {1344, "F1344_UninitializeData", "FIO1.C:26", "FIO1 platform-data lifecycle has no portable PC34 contract."},
    {1345, "F1345_InitializeData", "FIO1.C:27", "FIO1 platform-data lifecycle has no portable PC34 contract."},
};

const DM1_V1_F1326F1345BoundaryPc34 *
dm1_v1_f1326_f1345_source_boundary_pc34(unsigned int number)
{
    unsigned int index;
    for (index = 0; index < sizeof(kBoundaries) / sizeof(kBoundaries[0]); ++index) {
        if (kBoundaries[index].number == number) return &kBoundaries[index];
    }
    return 0;
}

int dm1_v1_f1326_f1345_admits_authentic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_f1326_f1345_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1326_f1345_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB FIO1.C:26-27,50,285,634,728,791. Existing F1328-F1336, "
           "F1338-F1339, and F1341-F1342 PC34 callback owners remain separate. "
           "No generated disk, lock, save, UI, or device route is admitted.";
}
