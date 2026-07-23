#include "dm1_v1_p0301_p0350_parameter_owner_audit.h"

#include <stddef.h>

/* ReDMCSB P0301-P0350: verified raw-PC34 DUNGEON/GROUP contracts only. */
static const DM1V1P0301P0350Audit kAudit[] = {
    {301, 169}, {302, 169}, {303, 169}, {304, 170}, {305, 170},
    {306, 170}, {307, 170}, {308, 170}, {309, 171}, {310, 171},
    {311, 171}, {312, 171}, {313, 171}, {314, 171}, {315, 171},
    {316, 171}, {317, 172}, {318, 172}, {319, 172}, {320, 172},
    {321, 173}, {322, 174}, {323, 175}, {324, 175}, {325, 176},
    {326, 176}, {327, 177}, {328, 177}, {329, 177}, {330, 177},
    {331, 177}, {332, 178}, {333, 178}, {334, 178}, {335, 179},
    {336, 179}, {337, 179}, {338, 180}, {339, 180}, {340, 181},
    {341, 181}, {342, 182}, {343, 182}, {344, 182}, {345, 183},
    {346, 183}, {347, 183}, {348, 184}, {349, 185}, {350, 185}
};

const DM1V1P0301P0350Audit *
dm1_v1_p0301_p0350_parameter_owner_audit(uint16_t parameter)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].parameter == parameter) {
            return &kAudit[index];
        }
    }
    return NULL;
}
