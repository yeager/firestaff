#include "theron_v1_track02_champion_roster.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(theron_v1_track02_us_champion_count() == 8);
    assert(theron_v1_track02_us_champion(8) == NULL);

    const Theron_ChampionRecord *theron = theron_v1_track02_us_champion(0);
    assert(theron != NULL);
    assert(strcmp(theron->name, "THERON") == 0);
    assert(theron->title == NULL);
    assert(theron->sex == 'M');
    assert(theron->hp == 175);
    assert(theron->stamina == 1500);
    assert(theron->mana == 50);
    assert(theron->strength == 50);

    const Theron_ChampionRecord *mara = theron_v1_track02_us_champion(1);
    assert(strcmp(mara->name, "MARA") == 0);
    assert(strcmp(mara->title, "GUARDIAN OF WISDOM") == 0);
    assert(mara->sex == 'F');
    assert(mara->wisdom == 70);
    assert(mara->mana == 200);

    const Theron_ChampionRecord *hexa = theron_v1_track02_us_champion(3);
    assert(strcmp(hexa->name, "HEXA") == 0);
    assert(hexa->strength == 50 && hexa->dexterity == 50 &&
           hexa->wisdom == 50 && hexa->vitality == 50);

    const Theron_ChampionRecord *tiran = theron_v1_track02_us_champion(5);
    assert(strcmp(tiran->name, "TIRAN") == 0);
    assert(tiran->mana == 0);
    assert(tiran->strength == 70);

    const Theron_ChampionRecord *pentai = theron_v1_track02_us_champion(7);
    assert(strcmp(pentai->name, "PENTAI") == 0);
    assert(pentai->sex == 'F');
    assert(pentai->hp == 550);
    assert(pentai->anti_fire == 70);

    for (unsigned i = 0; i < 8; i++) {
        const Theron_ChampionRecord *c = theron_v1_track02_us_champion(i);
        assert(c->name != NULL);
        assert(c->sex == 'M' || c->sex == 'F');
        assert(c->hp > 0);
        assert(c->stamina > 0);
    }

    printf("PASS: theron_v1_track02_champion_roster\n");
    return 0;
}
