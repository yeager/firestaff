#include "firestaff/dm1/v1/champion_unpoison_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int s_assertions;
static int s_failures;

#define CHECK(condition) do { \
    ++s_assertions; \
    if (!(condition)) { \
        ++s_failures; \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
    } \
} while (0)

int main(void)
{
    struct {
        unsigned int before;
        DM1_V1_ChampionPoisonStatePc34Compat poison;
        unsigned int after;
    } boundedState = { 0x11223344u, { 65535u }, 0x55667788u };

    CHECK(strstr(F0323_CHAMPION_Unpoison_SourceEvidencePc34Compat(),
                 "F0323_CHAMPION_Unpoison") != NULL);

    F0323_CHAMPION_Unpoison_Compat(NULL);

    F0323_CHAMPION_Unpoison_Compat(&boundedState.poison);
    CHECK(boundedState.poison.poisonDose == 0);
    CHECK(boundedState.before == 0x11223344u);
    CHECK(boundedState.after == 0x55667788u);

    F0323_CHAMPION_Unpoison_Compat(&boundedState.poison);
    CHECK(boundedState.poison.poisonDose == 0);

    printf("test_dm1_v1_champion_unpoison_pc34_compat: %d assertions, %d failures\n",
           s_assertions, s_failures);
    return s_failures == 0 ? 0 : 1;
}
