#include "dm1_v1_p0501_p0550_parameter_owner_audit.h"

#include <stddef.h>

/* ReDMCSB P0501-P0550: verified raw-PC34 MELEE/TIMELINE/MOVE contracts only. */
static const DM1V1P0501P0550Audit kAudit[] = {
    {501, 231}, {502, 231}, {503, 231}, {504, 232}, {505, 232},
    {506, 232}, {507, 232}, {508, 232}, {509, 234}, {510, 234},
    {511, 235}, {512, 236}, {513, 237}, {514, 238}, {515, 239},
    {516, 241}, {517, 242}, {518, 243}, {519, 244}, {520, 245},
    {521, 246}, {522, 247}, {523, 247}, {524, 248}, {525, 249},
    {526, 249}, {527, 250}, {528, 251}, {529, 252}, {530, 253},
    {531, 254}, {532, 255}, {534, 257}, {535, 258}, {536, 258},
    {537, 258}, {538, 258}, {539, 259}, {540, 259}, {541, 262},
    {542, 262}, {543, 262}, {544, 263}, {545, 263}, {546, 264},
    {547, 265}, {548, 265}, {549, 265}, {550, 265}
};

const DM1V1P0501P0550Audit *
dm1_v1_p0501_p0550_parameter_owner_audit(uint16_t parameter)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].parameter == parameter) {
            return &kAudit[index];
        }
    }
    return NULL;
}
