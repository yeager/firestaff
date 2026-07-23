#include "dm1_v1_p0001_p0050_parameter_owner_audit.h"

#include <stddef.h>

/* ReDMCSB P0001-P0050 are enclosing-routine contracts, never PC34 inputs. */
static const DM1V1P0001P0050Audit kAudit[] = {
    {1, 6, 1}, {2, 6, 1}, {3, 6, 1}, {4, 6, 1}, {5, 7, 1},
    {6, 7, 1}, {7, 7, 1}, {7, 7317, 1}, {8, 8, 1}, {9, 8, 1},
    {10, 9, 1}, {11, 9, 1}, {12, 9, 1}, {13, 9, 1}, {14, 10, 1},
    {15, 10, 1}, {16, 10, 1}, {17, 10, 1}, {18, 19, 1}, {19, 20, 1},
    {20, 20, 1}, {21, 20, 1}, {22, 20, 1}, {23, 21, 1}, {24, 21, 1},
    {25, 21, 1}, {26, 21, 1}, {27, 22, 1}, {28, 23, 1}, {29, 24, 1},
    {30, 24, 1}, {31, 25, 1}, {32, 25, 1}, {33, 26, 1}, {34, 26, 1},
    {35, 26, 1}, {36, 30, 1}, {37, 30, 1}, {38, 30, 1}, {39, 32, 1},
    {40, 33, 1}, {41, 34, 1}, {42, 36, 1}, {43, 36, 1}, {44, 37, 1},
    {45, 37, 1}, {46, 37, 1}, {47, 38, 1}, {48, 38, 1}, {49, 39, 1},
    {50, 40, 1}
};

const DM1V1P0001P0050Audit *
dm1_v1_p0001_p0050_parameter_owner_audit(
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
