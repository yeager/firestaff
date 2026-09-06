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
    const char* archive = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    M11_GameViewState* state;
    struct SpellDefinition_Compat spell;
    FILE* media;
    int recent, i;
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
    for (recent = 0; recent < 2; ++recent) {
        struct ChampionState_Compat* champion = &state->world.party.champions[0];
        struct ChampionLifecycleState_Compat* life = &state->world.lifecycle.champions[0];
        const uint32_t initialXp = 1000000;
        /* BASE.C:1695: seed1 -> BB40E638; RANDOM(8)=6. MENU.C:1826:
         * 6 + (4+6)*16 + (6-1)*4*8 + (4+6)^2 = 426.
         * This constant oracle never calls the production XP/RNG helper. */
        const uint32_t expected = 426u * (recent ? 12u : 6u);
        int manaBefore;
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
        state->world.gameTick = 1000;
        state->world.lifecycle.lastCreatureAttackTime = recent ? 999 : 800;
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
        printf("I34E Mon Light recent=%d award=%u: passed\n", recent, expected);
    }
    M11_GameView_Shutdown(state); free(state); return 0;
fail:
    M11_GameView_Shutdown(state); free(state); return 1;
}
