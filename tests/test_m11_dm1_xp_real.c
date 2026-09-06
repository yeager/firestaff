/* Original I34E archive + bounded RAM party fixture; no savegame or capture.
 * ReDMCSB MENU.C:50-83,1824-1826 and CHAMPION.C:868-891 own the oracle. */
#include "m11_game_view.h"
#include "asset_find_by_hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); goto fail; \
} } while (0)

int main(void) {
    static const int difficulty[] = {0,1,1,2,2,2,3,3,3,4,5,5,6,6};
    static const int runes[] = {5,2,3,4}; /* Mon Oh Ir Ra */
    static const int modes[] = { M12_PRESENTATION_V1_ORIGINAL,
        M12_PRESENTATION_V20_FILTERED, M12_PRESENTATION_V21_UPSCALED };
    const char* archive = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    M11_GameViewState* state;
    struct SpellDefinition_Compat spell;
    FILE* media;
    int recent, i, mode;
    unsigned char rendered[64000], reference[64000];
    if (!archive || !(media = fopen(archive, "rb"))) {
        puts("skip: original I34E archive unavailable"); return 77;
    }
    fclose(media);
    /* Pin the supplied original package, not just its display filename. */
    if (!asset_file_matches_md5(archive, "ee7b83cdb88c39c441a319f9610e97d6")) {
        fputs("FAIL: archive identity is not canonical I34E\n", stderr); return 1;
    }
    state = calloc(1, sizeof(*state));
    if (!state) return 1;
    M11_GameView_Init(state);
    CHECK(M11_GameView_StartDm1(state, archive));
    CHECK(state->world.dungeon && state->world.dungeon->header.mapCount == 14);
    for (i = 0; i < 14; ++i)
        CHECK(state->world.dungeon->maps[i].difficulty == difficulty[i]);
    CHECK(F0752b_MAGIC_LookupSpellByTableIndex_Compat(6, &spell));
    CHECK(spell.symbolsPacked == 0x00686f76 && spell.baseRequiredSkillLevel == 4 &&
          spell.skillIndex == 17);
    for (mode = 0; mode < 3; ++mode) for (recent = 0; recent < 2; ++recent) {
        struct ChampionState_Compat* champion = &state->world.party.champions[0];
        struct ChampionLifecycleState_Compat* life = &state->world.lifecycle.champions[0];
        const uint32_t initialXp = 1000000;
        /* BASE.C:1695: seed1 -> BB40E638; RANDOM(8)=6. MENU.C:1826:
         * 6 + (4+6)*16 + (6-1)*4*8 + (4+6)^2 = 426.
         * This constant oracle never calls the production XP/RNG helper. */
        const uint32_t expected = 426u * (recent ? 12u : 6u);
        int manaBefore;
        state->presentationMode = modes[mode];
        F0600_CHAMPION_InitEmpty_Compat(champion);
        memset(life, 0, sizeof(*life));
        state->world.party.championCount = 1;
        state->world.party.activeChampionIndex = 0;
        state->world.party.mapIndex = 12; /* Original difficulty6; dungeon unchanged. */
        champion->present = 1;
        champion->hp.current = champion->hp.maximum = 100;
        champion->stamina.current = champion->stamina.maximum = 1000;
        champion->mana.current = champion->mana.maximum = 900;
        champion->attributes[CHAMPION_ATTR_WISDOM] = 100;
        life->skills20[3].experience = initialXp;
        life->skills20[17].experience = initialXp;
        /* Separate casts are distinct source frames, not two conflicting
         * presentation serials injected into the same frame tick. */
        state->world.gameTick = 1000 + mode * 2 + recent;
        state->world.lifecycle.lastCreatureAttackTime =
            state->world.gameTick - (recent ? 1 : 200);
        state->world.magic.magicalLightAmount = 0;
        state->inventoryPanelActive = 0;
        CHECK(M11_GameView_OpenSpellPanel(state));
        for (i = 0; i < 4; ++i) CHECK(M11_GameView_EnterRune(state, runes[i]));
        manaBefore = champion->mana.current;
        state->world.masterRng.seed = 1; /* After rune entry, before F0412 RANDOM(8). */
        CHECK(M11_GameView_CastSpell(state));
        CHECK(life->skills20[17].experience == initialXp + expected);
        CHECK(life->skills20[3].experience == initialXp + expected);
        CHECK(state->world.magic.magicalLightAmount > 0);
        CHECK(champion->mana.current == manaBefore); /* F0399 already paid. */
        {
            const int liveCount = state->dm1LiveActionEffects.count;
            int row, nonblack = 0;
            CHECK(liveCount > 0);
            /* Render the actual F0412 effect, then compare its C013 output
             * with the source panel at identical world state. A rejected
             * live receipt clears this region, unlike the original C009. */
            memset(rendered, 0, sizeof(rendered));
            M11_GameView_Draw(state, rendered, 320, 200);
            state->dm1LiveActionEffects.count = 0;
            memset(reference, 0, sizeof(reference));
            M11_GameView_Draw(state, reference, 320, 200);
            state->dm1LiveActionEffects.count = liveCount;
            for (row = 42; row < 75; ++row) {
                int col;
                CHECK(memcmp(rendered + row * 320 + 233,
                             reference + row * 320 + 233, 87) == 0);
                for (col = 233; col < 320; ++col)
                    nonblack += rendered[row * 320 + col] != 0;
            }
            CHECK(nonblack > 0);
        }
        printf("I34E Mon Light mode=%d recent=%d award=%u: passed\n",
               mode, recent, expected);
    }
    for (mode = 0; mode < 3; ++mode) {
        struct ChampionState_Compat *champion = &state->world.party.champions[0];
        struct ChampionLifecycleState_Compat *life = &state->world.lifecycle.champions[0];
        struct ChampionLifecycleState_Compat afterLife;
        struct ChampionState_Compat afterChampion;
        uint32_t afterSeed, afterTick;
        int manaBefore;
        /* Original spell and original map, RAM-only mastery threshold fixture.
         * High hidden Air XP guarantees sufficient F0303 mastery while base
         * Wizard XP crosses 500 once. No original object/dungeon bytes change. */
        state->presentationMode = modes[mode];
        F0600_CHAMPION_InitEmpty_Compat(champion);
        memset(life, 0, sizeof(*life));
        state->world.party.championCount = 1;
        state->world.party.activeChampionIndex = 0;
        state->world.party.mapIndex = 0; /* Actual map difficulty zero. */
        champion->present = 1;
        champion->hp.current = 50; champion->hp.maximum = 123;
        champion->stamina.current = 400; champion->stamina.maximum = 987;
        champion->mana.current = 800; champion->mana.maximum = 800;
        for (i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
            champion->attributes[i] = 40;
            champion->attributeMaximums[i] = 40;
        }
        life->skills20[3].experience = 499;
        life->skills20[17].experience = 1000000;
        state->world.gameTick = 2000 + mode;
        state->world.lifecycle.lastCreatureAttackTime = state->world.gameTick - 200;
        state->world.magic.magicalLightAmount = 0;
        state->inventoryPanelActive = 0;
        CHECK(M11_GameView_OpenSpellPanel(state));
        for (i = 0; i < 4; ++i) CHECK(M11_GameView_EnterRune(state, runes[i]));
        manaBefore = champion->mana.current;
        state->world.masterRng.seed = 1;
        CHECK(M11_GameView_CastSpell(state));
        CHECK(state->world.magic.magicalLightAmount > 0);
        CHECK(life->skills20[3].experience == 925);
        CHECK(life->skills20[17].experience == 1000426);
        CHECK(champion->skillExperience[3] == 925 && champion->skillLevels[3] == 2);
        CHECK(champion->hp.maximum > 123 && champion->hp.maximum == life->maxHealth);
        CHECK(champion->stamina.maximum > 987 && champion->stamina.maximum == life->maxStamina);
        CHECK(champion->mana.maximum > 800 && champion->mana.maximum == life->maxMana);
        CHECK(champion->hp.current == 50 && champion->stamina.current == 400 &&
              champion->mana.current == manaBefore);
        for (i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
            CHECK(champion->attributes[i] == 40);
            CHECK(champion->attributeMaximums[i] ==
                  life->statistics[i + 1][LIFECYCLE_STAT_MAXIMUM]);
        }
        afterLife = *life;
        afterChampion = *champion;
        afterSeed = state->world.masterRng.seed;
        afterTick = state->world.gameTick;
        /* Presentation-only tick avoids timed regeneration while proving the
         * UI does not replay the already-committed spell XP/level-up effect. */
        M11_GameView_TickAnimation(state);
        M11_GameView_Draw(state, rendered, 320, 200);
        M11_GameView_Draw(state, reference, 320, 200);
        CHECK(state->world.gameTick == afterTick && state->world.masterRng.seed == afterSeed);
        CHECK(memcmp(life, &afterLife, sizeof(afterLife)) == 0);
        CHECK(memcmp(champion, &afterChampion, sizeof(afterChampion)) == 0);
        printf("I34E public Mon Light level-up mode=%d: one award, published maxima, no UI replay\n", mode);
    }
    M11_GameView_Shutdown(state); free(state); return 0;
fail:
    M11_GameView_Shutdown(state); free(state); return 1;
}
