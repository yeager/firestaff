/* Real PC-DOS archive::SKSave resume -> M11 source-admitted spell regression. */

#include "m11_game_view.h"
#include "dm2_v1_spell.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int select_authentic_cast(uint8_t out_runes[4], int *out_count,
                                 int *out_hero, int *out_spell)
{
    DM2_V1_RuntimeSpellTable table;
    int hero;

    if (!out_runes || !out_count || !out_hero || !out_spell) return 0;
    memset(&table, 0, sizeof(table));
    dm2_v1_spell_cast_player_build_table(NULL, &table);
    for (hero = 0; hero < DM2_MAX_HEROES; ++hero) {
        DM2_V1_RuntimeSourceHeroStateReceipt state;
        memset(&state, 0, sizeof(state));
        if (!dm2_v1_runtime_get_source_hero_state((uint8_t)hero, &state) ||
            state.cur_hp <= 0) continue;
        for (int i = 0; i < table.count; ++i) {
            DM2_V1_SpellCastPlayerReceipt cast;
            int count = 1;
            uint32_t key = table.records[i].key;
            memset(out_runes, 0, 4u);
            out_runes[0] = DM2_RUNE_YA;
            for (int shift = 16; shift >= 0; shift -= 8) {
                uint8_t rune = (uint8_t)(key >> shift);
                if (rune != 0u && count < 4) out_runes[count++] = rune;
            }
            memset(&cast, 0, sizeof(cast));
            cast = dm2_v1_spell_cast_player(
                &table, out_runes, state.wizardry_skill, state.cur_mp, 0);
            if (cast.valid && cast.cast_success) {
                *out_count = count;
                *out_hero = hero;
                *out_spell = cast.spell_index;
                return 1;
            }
        }
    }
    return 0;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    char save_path[1024];
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    uint8_t runes[4];
    int rune_count;
    int hero;
    int spell;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not set");
        return 77;
    }
    if (snprintf(save_path, sizeof(save_path), "%s::data/sksave1.dat", archive) >=
            (int)sizeof(save_path)) {
        fputs("FAIL: DOS save path is too long\n", stderr);
        return 1;
    }
    memset(&view, 0, sizeof(view));
    memset(&spec, 0, sizeof(spec));
    spec.title = "Dungeon Master II PC-DOS SKSave";
    spec.gameId = "dm2";
    spec.dataDir = archive;
    spec.savePath = save_path;
    spec.sourceId = "dos-sksave-real";
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.launcherOptionsBound = 1;

    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec) ||
        !view.dm2BootProfile || !view.dm2State.level_loaded) {
        fprintf(stderr, "FAIL: DOS SKSave did not publish an M11 DM2 session\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    if (!M11_GameView_OpenSpellPanel(&view)) {
        fputs("FAIL: DOS M11 did not open the DM2 spell panel\n", stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    /* SKSAVE1's real active hero cannot admit Fireball.  The native owner
     * must still consume that rejected input and apply the original rune-tail
     * rule; prior M11 code falsely reported it as an unavailable owner. */
    view.spellBuffer.runes[0] = DM2_RUNE_YA;
    view.spellBuffer.runes[1] = DM2_RUNE_FUL;
    view.spellBuffer.runes[2] = DM2_RUNE_IR;
    view.spellBuffer.runeCount = 3;
    if (!M11_GameView_CastSpell(&view) || view.spellPanelOpen ||
        strcmp(view.inspectTitle, "DM2 SPELL REJECTED") != 0) {
        fputs("FAIL: DOS M11 did not consume the authentic Fireball rejection\n",
              stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    if (!M11_GameView_OpenSpellPanel(&view)) {
        fputs("FAIL: DOS M11 did not reopen its spell panel after rejection\n",
              stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    if (!select_authentic_cast(runes, &rune_count, &hero, &spell) ||
        !dm2_v1_runtime_activate_action_hand(hero, 0)) {
        fputs("FAIL: DOS SKSave has no source-admitted spell cast\n", stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    view.dm1SpellCasting.magicCasterIndex = hero;
    memcpy(view.spellBuffer.runes, runes, sizeof(runes));
    view.spellBuffer.runeCount = rune_count;
    if (!M11_GameView_CastSpell(&view) || view.spellPanelOpen) {
        fputs("FAIL: DOS M11 DM2 spell did not commit through the source owner\n",
              stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    printf("PASS: authentic DOS SKSave resume commits M11 spell %d for hero %d\n",
           spell, hero);
    M11_GameView_Shutdown(&view);
    return 0;
}
