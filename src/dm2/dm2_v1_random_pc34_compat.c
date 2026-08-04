#include "dm2_v1_random_pc34_compat.h"

#define RANDOM_MAGIC 0xbb40e62dUL

void dm2_v1_random_init(DM2_V1_RandomState *r)
{
    r->state = 0;
}

void dm2_v1_random_seed(DM2_V1_RandomState *r, uint32_t seed)
{
    r->state = seed;
}

int32_t dm2_v1_rand(DM2_V1_RandomState *r)
{
    uint32_t s = r->state * RANDOM_MAGIC + 11;
    r->state = s;
    s >>= 8;
    return (int32_t)s;
}

int16_t dm2_v1_rand16(DM2_V1_RandomState *r, int16_t max)
{
    if (max == 0)
        return 0;
    return (int16_t)((uint16_t)dm2_v1_rand(r) % (uint32_t)(uint16_t)max);
}

int dm2_v1_randbit(DM2_V1_RandomState *r)
{
    uint32_t s = r->state * RANDOM_MAGIC + 11;
    r->state = s;
    return (int)((s >> 8) & 1);
}

int8_t dm2_v1_randdir(DM2_V1_RandomState *r)
{
    uint32_t s = r->state * RANDOM_MAGIC + 11;
    r->state = s;
    return (int8_t)((s >> 8) & 3);
}
