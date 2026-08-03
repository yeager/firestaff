#include "theron_v1_track02_champion_roster.h"
#include "theron_v1_champions.h"
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
    assert(theron->luck == 80);
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

    /* Skill levels: Theron has Apprentice in all 4 classes */
    assert(theron->fighter_skills[0] == 3); /* Swing 3 */
    assert(theron->ninja_skills[3]   == 3); /* Shoot 3 */
    assert(theron->priest_skills[1]  == 3); /* Heal 3 */
    assert(theron->wizard_skills[3]  == 3); /* Water 3 */

    /* Tiran is pure Fighter (Master), no other skills */
    assert(tiran->fighter_skills[0] == 9); /* Swing 9 */
    assert(tiran->ninja_skills[0] == 0 && tiran->ninja_skills[1] == 0);
    assert(tiran->priest_skills[0] == 0 && tiran->priest_skills[1] == 0);
    assert(tiran->wizard_skills[0] == 0 && tiran->wizard_skills[1] == 0);

    /* Hexa is perfectly balanced: all 4s */
    for (int i = 0; i < 4; i++) {
        assert(hexa->fighter_skills[i] == 4);
        assert(hexa->ninja_skills[i]   == 4);
        assert(hexa->priest_skills[i]  == 4);
        assert(hexa->wizard_skills[i]  == 4);
    }

    /* Starting equipment from DMWeb */
    assert(theron->start_equip_count == 3);
    assert(theron->start_equip_item[0] == 20); /* LEATHER JERKIN */
    assert(theron->start_equip_item[1] == 29); /* GHI TROUSERS */
    assert(theron->start_equip_item[2] == 37); /* LEATHER BOOTS */

    assert(tiran->start_equip_count == 5);
    assert(tiran->start_equip_item[0] == 33); /* BASINET */
    assert(tiran->start_equip_item[4] == 9);  /* MORNINGSTAR */

    assert(pentai->start_equip_count == 10);
    assert(pentai->start_equip_item[0] == 1);  /* ILLUMULET */
    assert(pentai->start_equip_item[8] == 4);  /* DAGGER */
    assert(pentai->start_equip_item[9] == 41); /* ROPE */

    /* Verify party init wires equipment into champion slots */
    {
        Theron_V1_Party p;
        theron_v1_party_init(&p, 0);
        /* Theron: Leather Jerkin in armor slot */
        assert(p.champions[0].slots[1] == 20); /* ESLOT_ARMOR = LEATHER JERKIN */
        assert(p.champions[0].slots[4] == 37); /* ESLOT_BOOTS = LEATHER BOOTS */
        assert(p.champions[0].inventory[0] == 20);
        assert(p.champions[0].inventory[1] == 29);
        assert(p.champions[0].inventory[2] == 37);
        assert(p.champions[0].load == 3);
    }

    /* Party init uses real roster data */
    {
        Theron_V1_Party party;
        theron_v1_party_init(&party, 0);
        assert(party.champion_count == 4);
        assert(strcmp(party.champions[0].name, "THERON") == 0);
        assert(party.champions[0].health == 175);
        assert(party.champions[0].max_health == 175);
        assert(party.champions[0].stamina == 1500);
        assert(party.champions[0].strength == 50);
        assert(party.champions[0].primary_class == THERON_CLASS_FIGHTER);
        assert(party.champions[0].fighter_level == 3);
        assert(strcmp(party.champions[1].name, "MARA") == 0);
        assert(party.champions[1].wisdom == 70);
        assert(party.champions[1].primary_class == THERON_CLASS_PRIEST);
        assert(strcmp(party.champions[2].name, "LINOS") == 0);
        assert(party.champions[2].primary_class == THERON_CLASS_NINJA);
        assert(strcmp(party.champions[3].name, "HEXA") == 0);
        assert(party.champions[3].primary_class == THERON_CLASS_FIGHTER);
    }

    /* Soul Room companion selection */
    {
        Theron_V1_Party party;
        theron_v1_party_init(&party, 0);
        assert(theron_v1_party_set_companion(&party, 1, 4, 2) == 0);
        assert(strcmp(party.champions[1].name, "HAKAR") == 0);
        assert(party.champions[1].strength == 60);
        assert(theron_v1_party_set_companion(&party, 2, 7, 2) == 0);
        assert(strcmp(party.champions[2].name, "PENTAI") == 0);
        assert(party.champions[2].anti_fire == 70);
        assert(theron_v1_party_set_companion(&party, 0, 1, 2) == -1);
        assert(theron_v1_party_set_companion(&party, 4, 1, 2) == -1);
        assert(theron_v1_party_set_companion(&party, 1, 0, 2) == -1);
        assert(theron_v1_party_set_companion(&party, 1, 99, 2) == -1);
    }

    /* DOTAN per-dungeon availability */
    {
        assert(theron_v1_companion_available_in_dungeon(6, 1) == 0);
        assert(theron_v1_companion_available_in_dungeon(6, 2) == 1);
        assert(theron_v1_companion_available_in_dungeon(6, 7) == 1);
        for (unsigned int r = 1; r <= 7; r++) {
            if (r == 6) continue;
            assert(theron_v1_companion_available_in_dungeon(r, 1) == 1);
        }
        Theron_V1_Party party2;
        theron_v1_party_init(&party2, 0);
        assert(theron_v1_party_set_companion(&party2, 1, 6, 1) == -1);
        assert(theron_v1_party_set_companion(&party2, 1, 6, 2) == 0);
        assert(strcmp(party2.champions[1].name, "DOTAN") == 0);
    }

    printf("PASS: theron_v1_track02_champion_roster\n");
    return 0;
}
