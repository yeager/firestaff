/* Source-bound F0115 material ordering inside one F0128 square. */
#ifndef DM1_V1_F0115_SQUARE_MATERIAL_SCHEDULER_PC34_COMPAT_H
#define DM1_V1_F0115_SQUARE_MATERIAL_SCHEDULER_PC34_COMPAT_H

#include "dm1_v1_f0128_per_square_scheduler_pc34_compat.h"

#include <stdint.h>

enum {
    DM1_V1_F0115_MATERIAL_NORMAL_OBJECT_PC34 = 1,
    DM1_V1_F0115_MATERIAL_PROJECTILE_PC34,
    DM1_V1_F0115_MATERIAL_EXPLOSION_PC34
};

typedef struct {
    int square;
    int kind;
    int graphicIndex;
    const uint8_t *pixels;
    int width;
    int height;
    int transparentColor;
} DM1_V1_F0115SquareMaterialPc34;

#define DM1_V1_F0115_SQUARE_MATERIAL_MAX_PC34 64

typedef struct {
    int valid;
    int materialCount;
    DM1_V1_F0115SquareMaterialPc34
        materials[DM1_V1_F0115_SQUARE_MATERIAL_MAX_PC34];
} DM1_V1_F0115SquareMaterialSchedulePc34;

/* Builds an F0115-only material sequence from an already admitted F0128
 * plan. Every source surface must be real decoded PC34 data. A malformed or
 * absent surface rejects the whole sequence before any renderer sees it. */
int dm1_v1_f0115_square_material_schedule_pc34(
    const DM1_V1_F0128SchedulerPlanPc34 *plan,
    const DM1_V1_F0115SquareMaterialPc34 *materials,
    int materialCount,
    DM1_V1_F0115SquareMaterialSchedulePc34 *outSchedule);

const char *dm1_v1_f0115_square_material_scheduler_source_pc34(void);

#endif
