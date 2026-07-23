#include "dm1_v1_p0451_p0500_parameter_owner_audit.h"

#include <stddef.h>

/* ReDMCSB P0451-P0500: verified raw-PC34 PROJECTILE/GROUP contracts only. */
static const DM1V1P0451P0500Audit kAudit[] = {
    {451, 216}, {452, 216}, {453, 217}, {454, 217}, {455, 217},
    {456, 217}, {457, 217}, {458, 218}, {459, 218}, {460, 218},
    {461, 218}, {462, 219}, {463, 220}, {464, 221}, {465, 221},
    {466, 222}, {467, 222}, {468, 223}, {469, 223}, {470, 224},
    {471, 224}, {472, 225}, {473, 225}, {474, 226}, {475, 226},
    {476, 226}, {477, 226}, {478, 227}, {479, 227}, {480, 227},
    {481, 227}, {482, 227}, {483, 228}, {484, 228}, {485, 228},
    {486, 228}, {487, 229}, {488, 229}, {489, 229}, {490, 229},
    {491, 229}, {492, 229}, {493, 230}, {494, 230}, {495, 231},
    {496, 231}, {497, 231}, {498, 231}, {499, 231}, {500, 231}
};

const DM1V1P0451P0500Audit *
dm1_v1_p0451_p0500_parameter_owner_audit(uint16_t parameter)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].parameter == parameter) {
            return &kAudit[index];
        }
    }
    return NULL;
}
