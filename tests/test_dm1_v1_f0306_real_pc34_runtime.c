#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_champion_stamina_adjusted_pc34_compat.h"

#include <stdio.h>

static int open_game(const char* dataDir,
                     M12_StartupMenuState* menu,
                     M11_GameViewState* game)
{
    M12_StartupMenu_InitWithDataDir(menu, dataDir, NULL);
    M11_GameView_Init(game);
    return M11_GameView_OpenSelectedMenuEntry(game, menu);
}

int main(int argc, char** argv)
{
    M12_StartupMenuState menu;
    M11_GameViewState game;
    struct ChampionState_Compat* champion;
    int baseValue;
    int halfMaximum;
    int loweredStamina;
    int expected;
    int result;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    if (!open_game(argv[1], &menu, &game) ||
        !M11_GameView_RecruitChampionByMirrorOrdinal(&game, 0) ||
        game.world.party.championCount != 1) {
        fprintf(stderr, "FAIL could not materialize a PC34 mirror champion\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    champion = &game.world.party.champions[0];
    halfMaximum = (int)champion->stamina.maximum >> 1;
    if (!champion->present || champion->stamina.maximum < 4 || halfMaximum <= 0) {
        fprintf(stderr, "FAIL PC34 mirror champion has no usable stamina record\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* The champion record and its original portrait come from installed
     * PC34 GRAPHICS.DAT/DUNGEON.DAT. Lower only the live runtime stamina to
     * exercise the F0306 branch that a real low-stamina save would reach. */
    loweredStamina = halfMaximum - 1;
    champion->stamina.current = (unsigned short)loweredStamina;
    baseValue = ((int)champion->attributes[CHAMPION_ATTR_STRENGTH] << 3) + 100;
    expected = (baseValue >> 1) + (baseValue * loweredStamina) / halfMaximum;

    csb_v1_stamina_compiler_order_set(1);
    result = F0306_CHAMPION_GetStaminaAdjustedValuePc34_Compat(
        (int)champion->stamina.current, halfMaximum, baseValue);
    csb_v1_stamina_compiler_order_set(0);

    if (!champion->portraitBitmapValid || result != expected || result <= (baseValue >> 1)) {
        fprintf(stderr,
                "FAIL PC34 F0306 runtime result=%d expected=%d base=%d current=%d half=%d\n",
                result, expected, baseValue, loweredStamina, halfMaximum);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("ok: PC34 mirror champion F0306 uses Turbo C++ second-operand order\n");
    M11_GameView_Shutdown(&game);
    return 0;
}
