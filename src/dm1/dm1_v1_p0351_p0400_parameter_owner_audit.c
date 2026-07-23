#include "dm1_v1_p0351_p0400_parameter_owner_audit.h"

#include <stddef.h>

/* ReDMCSB P0351-P0400: verified raw-PC34 GROUP contracts only. */
static const DM1V1P0351P0400Audit kAudit[] = {
    {351, 185}, {352, 185}, {353, 185}, {354, 185}, {355, 186},
    {356, 186}, {357, 186}, {358, 186}, {359, 186}, {360, 187},
    {361, 187}, {362, 187}, {363, 188}, {364, 188}, {365, 188},
    {366, 188}, {367, 189}, {368, 189}, {369, 190}, {370, 190},
    {371, 190}, {372, 190}, {373, 190}, {374, 190}, {375, 191},
    {376, 191}, {377, 191}, {378, 191}, {379, 191}, {380, 192},
    {381, 192}, {382, 193}, {383, 193}, {384, 197}, {385, 197},
    {386, 198}, {387, 198}, {388, 199}, {389, 199}, {390, 199},
    {391, 199}, {392, 199}, {393, 200}, {394, 200}, {395, 200},
    {396, 200}, {397, 201}, {398, 201}, {399, 201}, {400, 202}
};

const DM1V1P0351P0400Audit *
dm1_v1_p0351_p0400_parameter_owner_audit(uint16_t parameter)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].parameter == parameter) {
            return &kAudit[index];
        }
    }
    return NULL;
}
