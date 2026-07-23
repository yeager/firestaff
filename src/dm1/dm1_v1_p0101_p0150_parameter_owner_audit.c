#include "dm1_v1_p0101_p0150_parameter_owner_audit.h"

#include <stddef.h>

/* ReDMCSB P0101-P0150 are DUNVIEW enclosing-routine contracts only. */
static const DM1V1P0101P0150Audit kAudit[] = {
    {101, 99, 1}, {101, 791, 1}, {102, 99, 1}, {102, 791, 1},
    {103, 99, 1}, {104, 99, 1}, {105, 100, 1}, {106, 100, 1},
    {107, 101, 1}, {108, 101, 1}, {109, 102, 1}, {110, 103, 1},
    {111, 103, 1}, {112, 104, 1}, {113, 104, 1}, {114, 105, 1},
    {115, 105, 1}, {116, 107, 1}, {117, 107, 1}, {118, 108, 1},
    {119, 108, 1}, {120, 109, 1}, {121, 109, 1}, {122, 110, 1},
    {123, 110, 1}, {124, 111, 1}, {125, 111, 1}, {126, 111, 1},
    {127, 111, 1}, {128, 111, 1}, {129, 111, 1}, {130, 112, 1},
    {131, 112, 1}, {132, 112, 1}, {133, 112, 1}, {134, 112, 1},
    {135, 113, 1}, {136, 113, 1}, {137, 114, 1}, {138, 114, 1},
    {139, 114, 1}, {140, 114, 1}, {141, 115, 1}, {142, 115, 1},
    {143, 115, 1}, {144, 115, 1}, {145, 115, 1}, {146, 115, 1},
    {147, 116, 1}, {148, 116, 1}, {149, 116, 1}, {150, 117, 1}
};

const DM1V1P0101P0150Audit *
dm1_v1_p0101_p0150_parameter_owner_audit(
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
