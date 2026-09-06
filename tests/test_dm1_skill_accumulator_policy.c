#include "dm1_v1_skill_experience_pc34_compat.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include <stdio.h>

int main(void)
{
    static const struct {
        uint32_t xp, base;
        uint16_t temporary, baseTemporary;
        int hidden, early, late;
    } cases[] = {
        {0x7fffffff, 0, 1, 0, 0, 1, 24},
        {0xffffffff, 0, 1, 0, 0, 1, 1},
        {0x80000000, 0x80000000, 0, 0, 1, 1, 1},
        {0, 0, 0xffff, 0, 0, 1, 9},
        {0, 0, 0x8000, 0, 0, 1, 8},
        {1000, 1000, 0, 0, 1, 3, 3}
    };
    unsigned i;
    int p;
    for (i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) for (p = 0; p < 2; ++p) {
        DM1_ChampionSkillState skill = {0};
        struct ChampionLifecycleState_Compat life = {0};
        int index = cases[i].hidden ? 4 : 0;
        int expected = p ? cases[i].late : cases[i].early;
        /* Defined bit-pattern conversion, including the signed minimum. */
        skill.skills[0].experience = (int32_t)(cases[i].base <= INT32_MAX
            ? (int64_t)cases[i].base : (int64_t)cases[i].base - INT64_C(4294967296));
        skill.skills[0].temporaryExperience = (int16_t)(cases[i].baseTemporary < 0x8000
            ? (int)cases[i].baseTemporary : (int)cases[i].baseTemporary - 65536);
        skill.skills[index].experience = (int32_t)(cases[i].xp <= INT32_MAX
            ? (int64_t)cases[i].xp : (int64_t)cases[i].xp - INT64_C(4294967296));
        skill.skills[index].temporaryExperience = (int16_t)(cases[i].temporary < 0x8000
            ? (int)cases[i].temporary : (int)cases[i].temporary - 65536);
        life.skills20[0].experience = skill.skills[0].experience;
        life.skills20[0].temporaryExperience = skill.skills[0].temporaryExperience;
        life.skills20[index].experience = skill.skills[index].experience;
        life.skills20[index].temporaryExperience = skill.skills[index].temporaryExperience;
        if (i == 0 && dm1_skill_get_experience(&skill, index, 1) != INT32_MIN)
            return 1;
        if (dm1_skill_get_level_policy(&skill, index, DM1_SKILL_FLAG_IGNORE_OBJECTS,
                0, (enum DM1SkillAccumulatorPolicy)p) != expected ||
            F0848_LIFECYCLE_ComputeSkillLevelWithPolicy_Compat(&life, index, 0,
                (enum DM1SkillAccumulatorPolicy)p) != expected) {
            fprintf(stderr, "skill accumulator mismatch case %u policy %d\n", i, p);
            return 1;
        }
    }
    for (p = 0; p < 2; ++p) {
        struct GameWorld_Compat world = {0};
        struct ChampionLifecycleState_Compat life = {0};
        int before, after;
        world.skillAccumulatorPolicy = (enum DM1SkillAccumulatorPolicy)p;
        world.party.champions[0].present = 1;
        world.lifecycle.champions[0].skills20[0].experience = INT32_MAX;
        if (F0884_WORLD_AwardSkillExperience_Compat(&world, 0, 0, 1, 1, 0) != 0 ||
            world.lifecycle.champions[0].skills20[0].experience != INT32_MIN ||
            F0888_ORCH_GetChampionF0303SkillLevel_Compat(&world, 0, 0) != (p ? 24 : 1))
            return 1;
        life.skills20[0].experience = INT32_MAX;
        if (F0849_LIFECYCLE_AddSkillExperienceWithPolicy_Compat(&life, 0, 1,
                1, 1000, 0, &before, &after, (enum DM1SkillAccumulatorPolicy)p) ||
            life.skills20[0].experience != INT32_MIN || before != 24 ||
            after != (p ? 24 : 1)) return 1;
        life.skills20[0].experience = -1;
        if (F0849_LIFECYCLE_AddSkillExperienceWithPolicy_Compat(&life, 0, 1,
                1, 1000, 0, &before, &after, (enum DM1SkillAccumulatorPolicy)p) ||
            life.skills20[0].experience != 0 || after != 1) return 1;
        life.skills20[0].temporaryExperience = -1; /* Raw 0xffff. */
        (void)F0849_LIFECYCLE_AddSkillExperienceWithPolicy_Compat(&life, 0, 1,
            1, 1000, 0, &before, &after, (enum DM1SkillAccumulatorPolicy)p);
        if (life.skills20[0].temporaryExperience != (p ? -1 : 0)) return 1;
        life.skills20[0].temporaryExperience = INT16_MIN; /* Raw 0x8000. */
        (void)F0849_LIFECYCLE_AddSkillExperienceWithPolicy_Compat(&life, 0, 1,
            1, 1000, 0, &before, &after, (enum DM1SkillAccumulatorPolicy)p);
        if (life.skills20[0].temporaryExperience != (p ? INT16_MIN : INT16_MIN + 1))
            return 1;
    }
    puts("ok: both skill level owners and awards match early/late boundary cases");
    return 0;
}
