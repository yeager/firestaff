#ifndef DM1_LEGACY_SPELL_PANEL_REAL_CHECK_H
#define DM1_LEGACY_SPELL_PANEL_REAL_CHECK_H

/* Original bitmap/font oracle; only party state is a bounded RAM fixture.
 * CASTER.C:21-74, MENU.C:844-910 and SPELDRAW.C:22-84. */
static void legacy_spell_glyph(unsigned char *frame, const M11_FontState *font,
                               int x, int baseline, unsigned char ch,
                               int foreground, int background) {
    for (int row = 0; row < 6; ++row) for (int col = 0; col < 6; ++col) {
        int bit = row * 1024 + ch * 8 + 3 + col;
        if (x + col < 320)
            frame[(baseline - 4 + row) * 320 + x + col] =
                (font->bitmap[bit / 8] & (0x80u >> (bit % 8)))
                ? foreground : background;
    }
}

/* Invoke on a fresh, disposable native session: this deliberately changes
 * party fixtures and presentation mode, not original game assets. */
static int check_legacy_spell_panel_real(M11_GameViewState *state) {
    static const int modes[] = { M12_PRESENTATION_V1_ORIGINAL,
        M12_PRESENTATION_V20_FILTERED, M12_PRESENTATION_V21_UPSCALED };
    static const char *names[] = { "HALK", "TIGGY", "WUUF", "ZED" };
    static const int tabs[4][4] = { {233,280,294,308}, {233,247,294,308},
        {233,247,261,308}, {233,247,261,275} };
    const M11_AssetSlot *background = M11_AssetLoader_Load(&state->assetLoader, 9);
    const M11_AssetSlot *lines = M11_AssetLoader_Load(&state->assetLoader, 11);
    unsigned char actual[64000], expected[64000];
    int atari = state->assetLoader.atariStDm1;
    if ((!atari && !(state->assetLoader.legacyDm1 && state->assetLoader.legacyBigEndian)) ||
        !state->originalFontAvailable || !background || !lines ||
        !background->pixels || !lines->pixels || background->width != 96 ||
        background->height != 33 || lines->width != 96 || lines->height != 36) {
        fputs("FAIL: legacy spell original C00996x33/C01196x36/font admission\n", stderr);
        return 0;
    }
    for (int mode=0; mode<3; ++mode) for (int caster=0; caster<4; ++caster)
    for (int sparse=0; sparse<2; ++sparse) {
        state->presentationMode = modes[mode];
        state->world.party.championCount = 4;
        state->world.party.activeChampionIndex = sparse ? caster : (caster+1)%4;
        state->inventoryPanelActive = 0; state->showDebugHUD = 0;
        state->spellPanelOpen = 1; state->dm1SpellCasting.magicCasterIndex = caster;
        for (int i=0; i<4; ++i) {
            struct ChampionState_Compat *champion = &state->world.party.champions[i];
            F0600_CHAMPION_InitEmpty_Compat(champion);
            champion->present=1; champion->hp.current=(!sparse || i==caster)?100:0;
            champion->hp.maximum=100;
            memcpy(champion->name,names[i],strlen(names[i]));
            state->dm1SpellCasting.input[i].symbolStep=i;
            memset(state->dm1SpellCasting.input[i].symbols,0,5);
            state->dm1SpellCasting.input[i].symbols[0]=(char)(96+i);
            state->dm1SpellCasting.input[i].symbols[1]=(char)(102+i);
        }
        memset(expected,0,sizeof(expected)); memset(actual,0,sizeof(actual));
        for (int y=0; y<33; ++y)
            memcpy(expected+(42+y)*320+224,background->pixels+y*96,96);
        /* CASTER Atari loop is y=1; y<12: exactly rows1..11.
         * Amiga copies all12 rows. MENU F0392 uses source offsets12/24. */
        for (int row=0; row<2; ++row) for (int y=atari?1:0; y<12; ++y)
            memcpy(expected+(50+12*row+y)*320+224,
                   lines->pixels+(12+12*row+y)*96,96);
        /* MENU.C:500 G0504 clear, then source F0393 tab highlights. */
        for (int y=42; y<50; ++y) memset(expected+y*320+233,0,87);
        for (int i=0; i<4; ++i) if (!sparse || i==caster)
            for (int y=42; y<(i==caster?50:49); ++y)
                for (int x=tabs[caster][i]; x<tabs[caster][i]+(i==caster?45:12); ++x)
                    expected[y*320+x]=4;
        for (int i=0; names[caster][i]; ++i)
            legacy_spell_glyph(expected,&state->originalFont,235+14*caster+6*i,48,names[caster][i],0,4);
        for (int i=0; i<6; ++i)
            legacy_spell_glyph(expected,&state->originalFont,239+14*i,58,96+6*caster+i,4,0);
        /* F0392 stops at NUL; untouched tail retains authentic C011 bytes. */
        for (int i=0; i<2; ++i)
            legacy_spell_glyph(expected,&state->originalFont,241+9*i,70,
                state->dm1SpellCasting.input[caster].symbols[i],4,0);
        M11_GameView_Draw(state,actual,320,200);
        for (int y=42; y<75; ++y) for (int x=224; x<320; ++x)
            if (actual[y*320+x]!=expected[y*320+x]) {
                fprintf(stderr,"FAIL: legacy spell mode%d caster%d sparse%d pixel%d,%d actual%u expected%u\n",
                    mode,caster,sparse,x,y,actual[y*320+x],expected[y*320+x]);
                return 0;
            }
    }
    state->spellPanelOpen = 0;
    memset(actual, 15, sizeof(actual));
    M11_GameView_Draw(state, actual, 320, 200);
    for (int y=42; y<75; ++y) for (int x=224; x<320; ++x)
        if (actual[y*320+x] != 0) {
            fprintf(stderr,"FAIL: closed legacy spell panel retains pixel%d,%d=%u\n",
                    x,y,actual[y*320+x]);
            return 0;
        }
    puts("PASS: 24 original-media legacy spell panel pixel comparisons and closed-panel clear");
    return 1;
}
#endif
