#include "dm1_v1_p0401_p0450_parameter_owner_audit.h"

#include <stddef.h>

/* ReDMCSB P0401-P0450: verified raw-PC34 GROUP/PROJECTILE contracts only. */
static const DM1V1P0401P0450Audit kAudit[] = {
    {401, 202}, {402, 202}, {403, 202}, {404, 202}, {405, 203},
    {406, 203}, {407, 203}, {408, 203}, {409, 204}, {410, 204},
    {411, 204}, {412, 204}, {413, 205}, {414, 205}, {415, 205},
    {416, 205}, {417, 206}, {418, 206}, {419, 206}, {420, 206},
    {421, 207}, {422, 207}, {423, 207}, {424, 207}, {425, 208},
    {426, 208}, {427, 209}, {428, 209}, {429, 209}, {430, 209},
    {431, 210}, {432, 211}, {433, 212}, {434, 212}, {435, 212},
    {436, 212}, {437, 212}, {438, 212}, {439, 212}, {440, 212},
    {441, 213}, {442, 213}, {443, 213}, {444, 213}, {445, 213},
    {446, 214}, {447, 215}, {448, 215}, {449, 215}, {450, 215}
};

const DM1V1P0401P0450Audit *
dm1_v1_p0401_p0450_parameter_owner_audit(uint16_t parameter)
{
    size_t index;

    for (index = 0; index < sizeof(kAudit) / sizeof(kAudit[0]); ++index) {
        if (kAudit[index].parameter == parameter) {
            return &kAudit[index];
        }
    }
    return NULL;
}
