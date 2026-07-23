#include "dm1_v1_p0251_p0300_parameter_owner_audit.h"

#include <stddef.h>

/* ReDMCSB P0251-P0300: verified raw-PC34 DUNGEON enclosing contracts only. */
static const DM1V1P0251P0300Audit kAudit[] = {
    {251, 148}, {252, 149}, {253, 150}, {254, 150}, {255, 150},
    {256, 150}, {257, 150}, {258, 151}, {259, 151}, {260, 152},
    {261, 152}, {262, 152}, {263, 152}, {264, 152}, {265, 153},
    {266, 153}, {267, 153}, {268, 153}, {269, 153}, {270, 154},
    {271, 154}, {272, 154}, {273, 154}, {274, 155}, {275, 155},
    {276, 156}, {277, 157}, {278, 157}, {279, 158}, {280, 159},
    {281, 160}, {282, 160}, {283, 161}, {284, 161}, {285, 162},
    {286, 162}, {287, 163}, {288, 163}, {289, 163}, {290, 163},
    {291, 164}, {292, 164}, {293, 164}, {294, 164}, {295, 165},
    {296, 166}, {297, 167}, {298, 168}, {299, 168}, {300, 168}
};

const DM1V1P0251P0300Audit *
dm1_v1_p0251_p0300_parameter_owner_audit(uint16_t parameter)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].parameter == parameter) {
            return &kAudit[index];
        }
    }
    return NULL;
}
