#include "dm1_v1_l0001_l0050_local_owner_audit.h"

#include <stddef.h>

/* ReDMCSB L0001-L0050 are automatic locals, never standalone PC34 inputs. */
static const DM1V1L0001L0050Audit kAudit[] = {
    {1, 31, 1}, {2, 31, 1}, {3, 31, 1}, {4, 32, 1}, {5, 33, 1},
    {6, 33, 1}, {7, 34, 1}, {8, 34, 1}, {9, 34, 1}, {10, 34, 1},
    {11, 36, 1}, {12, 36, 1}, {13, 37, 1}, {14, 37, 1}, {15, 38, 1},
    {16, 38, 1}, {17, 38, 1}, {18, 38, 1}, {19, 38, 1}, {20, 38, 1},
    {21, 41, 1}, {22, 41, 1}, {23, 43, 1}, {24, 43, 1}, {25, 44, 1},
    {26, 44, 1}, {27, 44, 1}, {28, 44, 1}, {29, 45, 1}, {30, 46, 1},
    {31, 47, 1}, {32, 47, 1}, {33, 47, 1}, {34, 48, 1}, {35, 49, 1},
    {36, 49, 1}, {37, 54, 1}, {38, 60, 1}, {39, 60, 1}, {40, 60, 1},
    {41, 60, 1}, {42, 61, 1}, {43, 63, 1}, {44, 64, 1}, {45, 64, 1},
    {46, 64, 1}, {47, 64, 1}, {48, 65, 1}, {49, 66, 1}, {50, 66, 1}
};

const DM1V1L0001L0050Audit *
dm1_v1_l0001_l0050_local_owner_audit(uint16_t label, uint16_t enclosingRoutine)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].label == label &&
            kAudit[index].enclosingRoutine == enclosingRoutine) {
            return &kAudit[index];
        }
    }
    return NULL;
}
