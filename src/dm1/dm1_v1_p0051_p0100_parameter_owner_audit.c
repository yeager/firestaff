#include "dm1_v1_p0051_p0100_parameter_owner_audit.h"

#include <stddef.h>

/* ReDMCSB P0051-P0100 are enclosing-routine contracts, never PC34 inputs. */
static const DM1V1P0051P0100Audit kAudit[] = {
    {51, 40, 1}, {52, 40, 1}, {53, 40, 1}, {54, 40, 1}, {55, 40, 1},
    {56, 40, 1}, {57, 41, 1}, {57, 768, 1}, {58, 41, 1}, {58, 768, 1},
    {59, 41, 1}, {60, 41, 1}, {61, 41, 1}, {61, 768, 1}, {62, 41, 1},
    {62, 768, 1}, {63, 41, 1}, {63, 768, 1}, {64, 41, 1}, {64, 768, 1},
    {65, 42, 1}, {66, 42, 1}, {67, 46, 1}, {68, 46, 1}, {69, 47, 1},
    {70, 47, 1}, {71, 48, 1}, {72, 48, 1}, {73, 49, 1}, {74, 49, 1},
    {75, 52, 1}, {76, 52, 1}, {77, 52, 1}, {78, 52, 1}, {79, 53, 1},
    {80, 53, 1}, {81, 53, 1}, {82, 53, 1}, {83, 53, 1}, {84, 60, 1},
    {85, 60, 1}, {86, 60, 1}, {87, 61, 1}, {88, 64, 1}, {89, 64, 1},
    {90, 64, 1}, {91, 64, 1}, {92, 67, 1}, {93, 68, 1}, {94, 70, 1},
    {95, 79, 1}, {96, 93, 1}, {97, 93, 1}, {98, 94, 1}, {99, 95, 1},
    {100, 97, 1}
};

const DM1V1P0051P0100Audit *
dm1_v1_p0051_p0100_parameter_owner_audit(
    uint16_t parameter,
    uint16_t enclosingRoutine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].parameter == parameter &&
            kAudit[index].enclosingRoutine == enclosingRoutine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
