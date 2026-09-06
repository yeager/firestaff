/* Authentic late-edition graphics/font; bounded RAM party, names and rune buffers.
 * Pixel oracle follows SPELDRAW.C F0393:87-94 / MENUDRAW F0397,F0398,
 * not the plan. */
#include "m11_game_view.h"
#include "asset_find_by_hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef DM1_LATE_SPELL_PANEL_REAL_CHECK_H
#define DM1_LATE_SPELL_PANEL_REAL_CHECK_H
#define LATE_SPELL_CHECK(e) do { if (!(e)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e); return 0; } } while(0)

static void glyph(unsigned char* pixels, const M11_FontState* font,
                  int x, int baseline, unsigned char ch, int fg, int bg) {
    int row, col;
    /* TEXT2.C F0644: 6 pixels at cell bit offset3, 6 rows, baseline-4.
     * Read packed original M653 bytes directly, not host glyph routines. */
    for(row=0;row<6;++row) for(col=0;col<6;++col) {
        int bit=row*1024+ch*8+3+col;
        if(x+col<320)
            pixels[(baseline-4+row)*320+x+col]=
                font->bitmap[bit/8] & (0x80u>>(bit%8)) ? fg : bg;
    }
}
static int equal_box(const unsigned char* a,const unsigned char* b,int x,int y,int w,int h) {
    int row,col;
    for(row=y;row<y+h;++row) for(col=x;col<x+w;++col)
        if(a[row*320+col]!=b[row*320+col]) {
            fprintf(stderr,"pixel %d,%d actual%u expected%u\n",col,row,a[row*320+col],b[row*320+col]); return 0;
        }
    return 1;
}
/* Fresh disposable session; ASCII names and runes only, not Japanese text parity. */
static int check_late_spell_panel_real(M11_GameViewState *state) {
    static const int modes[] = { M12_PRESENTATION_V1_ORIGINAL,
        M12_PRESENTATION_V20_FILTERED, M12_PRESENTATION_V21_UPSCALED };
    static const char* names[]={"HALK","TIGGY","WUUF","ZED"};
    static const int tabX[4][4]={{233,280,294,308},{233,247,294,308},
                               {233,247,261,308},{233,247,261,275}};
    const M11_AssetSlot* background;
    unsigned char actual[64000], expected[64000];
    int mode,caster,sparse,i,x,y;
    const int towns = state->assetLoader.legacyDm1 &&
        !state->assetLoader.legacyBigEndian;
    /* Independent original registry: JDM root C013=(3,12,319,82),
     * EDM/DOS C013=(3,12,319,74). Children220..264 match byte-for-byte. */
    const int yOffset = towns && state->dm1FmtownsStartupReceipt.language ==
        DM1_FMTOWNS_LANG_JP ? 8 : 0;
    LATE_SPELL_CHECK(state->originalFontAvailable);
    background = M11_AssetLoader_Load(&state->assetLoader, 9);
    if (background) printf("late spell C009: loaded=%d size=%dx%d\n",
        background->loaded, background->width, background->height);
    LATE_SPELL_CHECK(background && background->loaded && background->pixels &&
          background->width == (towns ? 96 : 87) && background->height == 25);
    for(mode=0;mode<3;++mode) for(caster=0;caster<4;++caster) for(sparse=0;sparse<2;++sparse) {
        printf("checking mode=%d caster=%d sparse=%d\n", mode, caster, sparse);
        state->presentationMode=modes[mode];
        state->world.party.championCount=4;
        state->world.party.activeChampionIndex=sparse?caster:(caster+1)%4;
        state->inventoryPanelActive=0; state->showDebugHUD=0;
        state->spellPanelOpen=1;
        state->dm1SpellCasting.magicCasterIndex=caster;
        for(i=0;i<4;++i) {
            struct ChampionState_Compat* c=&state->world.party.champions[i];
            F0600_CHAMPION_InitEmpty_Compat(c);
            c->present=1; c->hp.current=(!sparse||i==caster)?100:0; c->hp.maximum=100;
            memcpy(c->name,names[i],strlen(names[i]));
            state->dm1SpellCasting.input[i].symbolStep=i;
            memset(state->dm1SpellCasting.input[i].symbols,0,5);
            state->dm1SpellCasting.input[i].symbols[0]=(char)(96+i);
            state->dm1SpellCasting.input[i].symbols[1]=(char)(102+i);
        }
        memset(actual,0,sizeof(actual)); memset(expected,0,sizeof(expected));
        M11_GameView_Draw(state,actual,320,200);
        /* I34 CASTER.C:90: C009 alone, bottom-right anchored in C013.
         * Authentic item696 C012 is87x33; C013 ends at319,74, placing
         * this87x25 source bitmap at233,50. No legacy C011 strip blits. */
        for (y=0;y<25;++y)
            /* F20 F0635 anchors96pixels at224 then clips through C012,
             * yielding sourceX9; I34's87pixels need no source clipping. */
            memcpy(expected+(50+yOffset+y)*320+233,
                   background->pixels+y*background->width+(towns ? 9 : 0),87);
        /* COORD.C C221 clear; caster-dependent C224+5*c slots. */
        for(i=0;i<4;++i) if(!sparse||i==caster)
            for(y=42;y<(i==caster?50:49);++y)
                for(x=tabX[caster][i];x<tabX[caster][i]+(i==caster?45:12);++x)
                    expected[(y+yOffset)*320+x]=4; /* Source XOR4 after black clear. */
        for(i=0;names[caster][i];++i)
            glyph(expected,&state->originalFont,235+14*caster+6*i,48+yOffset,names[caster][i],0,4);
        LATE_SPELL_CHECK(equal_box(actual,expected,233,42+yOffset,87,8));
        /* Selected buffer differs from the leader and each other caster. */
        for(i=0;i<6;++i) {
            glyph(expected,&state->originalFont,239+14*i,58+yOffset,96+6*caster+i,4,0);
            LATE_SPELL_CHECK(equal_box(actual,expected,239+14*i,54+yOffset,6,6));
        }
        for(i=0;i<4;++i) {
            glyph(expected,&state->originalFont,241+9*i,70+yOffset,
                  i<2?state->dm1SpellCasting.input[caster].symbols[i]:' ',4,0);
            LATE_SPELL_CHECK(equal_box(actual,expected,241+9*i,66+yOffset,6,6));
        }
        LATE_SPELL_CHECK(equal_box(actual,expected,233,42+yOffset,87,33));
        LATE_SPELL_CHECK(state->world.party.activeChampionIndex==(sparse?caster:(caster+1)%4));
        LATE_SPELL_CHECK(state->dm1SpellCasting.magicCasterIndex==caster);
    }
    /* COMMAND.C C100/C109 and SYMBOL.C F0399/F0400: use public mouse
     * dispatch, not the geometry planner, to prove the drawn targets work.
     * The original JP C013 is eight rows below its English counterpart. */
    for (mode=0; mode<3; ++mode) {
        state->presentationMode=modes[mode];
        state->world.party.championCount=4;
        state->world.party.activeChampionIndex=0;
        state->inventoryPanelActive=0;
        state->showDebugHUD=0;
        state->spellPanelOpen=0;
        state->dm1SpellCasting.magicCasterIndex=0;
        memset(&state->spellBuffer,0,sizeof(state->spellBuffer));
        state->spellRuneRow=0;
        for (i=0;i<4;++i) {
            struct ChampionState_Compat *champion=&state->world.party.champions[i];
            F0600_CHAMPION_InitEmpty_Compat(champion);
            champion->present=1;
            champion->hp.current=(i==0 || i==3)?100:0;
            champion->hp.maximum=100;
            champion->mana.current=champion->mana.maximum=50;
            memcpy(champion->name,names[i],strlen(names[i]));
            memset(&state->dm1SpellCasting.input[i],0,
                   sizeof(state->dm1SpellCasting.input[i]));
        }
        /* Bottom-right inclusive parent point: JP82, English74. Opening
         * the parent must not also execute a child command on that click. */
        (void)M11_GameView_HandlePointer(state,319,74+yOffset,1);
        (void)M11_GameView_HandlePointer(state,319,74+yOffset,0);
        LATE_SPELL_CHECK(state->spellPanelOpen && state->spellBuffer.runeCount==0);
        LATE_SPELL_CHECK(state->world.party.champions[0].mana.current==50);
        /* Slots1/2 are dead: slot3 remains selectable beyond tab_count2. */
        (void)M11_GameView_HandlePointer(state,309,43+yOffset,1);
        (void)M11_GameView_HandlePointer(state,309,43+yOffset,0);
        LATE_SPELL_CHECK(state->dm1SpellCasting.magicCasterIndex==3);
        LATE_SPELL_CHECK(state->world.party.activeChampionIndex==0);
        (void)M11_GameView_HandlePointer(state,241,56+yOffset,1);
        (void)M11_GameView_HandlePointer(state,241,56+yOffset,0);
        LATE_SPELL_CHECK(state->world.party.champions[3].mana.current==49);
        LATE_SPELL_CHECK(state->world.party.champions[0].mana.current==50);
        LATE_SPELL_CHECK(state->dm1SpellCasting.input[3].symbols[0]==96 &&
                         state->dm1SpellCasting.input[3].symbolStep==1 &&
                         state->spellBuffer.runeCount==1);
        /* F0400 removes the symbol without refunding its mana cost. */
        (void)M11_GameView_HandlePointer(state,305,63+yOffset,1);
        (void)M11_GameView_HandlePointer(state,305,63+yOffset,0);
        LATE_SPELL_CHECK(state->dm1SpellCasting.input[3].symbols[0]==0 &&
                         state->dm1SpellCasting.input[3].symbolStep==0 &&
                         state->spellBuffer.runeCount==0 &&
                         state->world.party.champions[3].mana.current==49);
    }
    if (towns) {
        const M11_AssetSlot *actionMenu=M11_AssetLoader_Load(&state->assetLoader,10);
        LATE_SPELL_CHECK(actionMenu && actionMenu->loaded && actionMenu->pixels);
        printf("Original F20 %s action C010: %dx%d\n",
               yOffset ? "JP" : "EN", actionMenu->width, actionMenu->height);
        const M11_AssetSlot *movement=M11_AssetLoader_Load(&state->assetLoader,13);
        LATE_SPELL_CHECK(movement && movement->loaded && movement->pixels);
        printf("Original F20 %s movement C013: %dx%d\n",
               yOffset ? "JP" : "EN", movement->width, movement->height);
        const M11_AssetSlot *atlas=M11_AssetLoader_Load(&state->assetLoader,48);
        const int cellY=yOffset?94:86, cellH=yOffset?62:35;
        const int iconY=yOffset?117:96;
        /* OBJECT.C F0036:318-342: icon201 is block48, local9 =>144,0.
         * ACTIDRAW.C F0386:249-281 remaps palette12 to4, fills cyan,
         * then centers that original16x16 icon in C089+slot. */
        LATE_SPELL_CHECK(atlas && atlas->loaded && atlas->pixels &&
                         atlas->width>=160 && atlas->height>=16);
        state->actingChampionOrdinal=0;
        state->candidateMirrorOrdinal=0;
        state->candidateMirrorPanelActive=0;
        state->resting=0;
        for (mode=0;mode<3;++mode) for (int deadParity=0;deadParity<2;++deadParity) {
            state->presentationMode=modes[mode];
            for (i=0;i<4;++i) {
                struct ChampionState_Compat *champion=&state->world.party.champions[i];
                F0600_CHAMPION_InitEmpty_Compat(champion);
                champion->present=1;
                champion->hp.maximum=100;
                champion->hp.current=((i&1)==deadParity)?100:0;
            }
            state->world.party.activeChampionIndex=deadParity;
            memset(actual,0,sizeof(actual)); memset(expected,0,sizeof(expected));
            M11_GameView_Draw(state,actual,320,200);
            for (i=0;i<4;++i) {
                int cellX=233+22*i;
                if ((i&1)==deadParity) {
                    for (y=cellY;y<cellY+cellH;++y)
                        memset(expected+y*320+cellX,4,20);
                    for (y=0;y<16;++y) for (x=0;x<16;++x) {
                        unsigned char pixel=atlas->pixels[y*atlas->width+144+x];
                        expected[(iconY+y)*320+cellX+2+x]=pixel==12?4:pixel;
                    }
                }
                LATE_SPELL_CHECK(equal_box(actual,expected,cellX,cellY,20,cellH));
            }
            if (yOffset) {
                LATE_SPELL_CHECK(movement->width==96 && movement->height==41);
                for(y=0;y<41;++y)
                    memcpy(expected+(159+y)*320+233,
                           movement->pixels+y*96+9,87);
                LATE_SPELL_CHECK(equal_box(actual,expected,233,159,87,41));
            }
        }
        puts("PASS: original FM Towns idle action cells, all slots/alive-dead, three modes");
        {
            const M11_AssetSlot *menu=M11_AssetLoader_Load(&state->assetLoader,10);
            const int menuY=yOffset?85:77, menuH=yOffset?72:45;
            LATE_SPELL_CHECK(menu && menu->loaded && menu->pixels &&
                             menu->width==(yOffset?96:87) && menu->height==menuH);
            /* ACTIDRAW.C:333-355 selects C079/C077/C011 by action count.
             * Rightmost column is outside label glyphs; compare authentic
             * C010 pixels rather than asserting host-generated colours. */
            for (mode=0;mode<3;++mode) for(i=0;i<4;++i) {
                unsigned char actions[3];
                int rows, cropH;
                state->presentationMode=modes[mode];
                state->world.party.champions[i].hp.current=100;
                state->actionDisabledTicks[i]=0;
                M11_GameView_ClearActingChampion(state);
                LATE_SPELL_CHECK(M11_GameView_SetActingChampion(state,i));
                LATE_SPELL_CHECK(M11_GameView_GetActingActionIndices(state,actions));
                rows=actions[0]==255?0:actions[1]==255?1:actions[2]==255?2:3;
                LATE_SPELL_CHECK(rows>0);
                cropH=yOffset?9+21*rows:9+12*rows;
                memset(actual,15,sizeof(actual));
                M11_GameView_Draw(state,actual,320,200);
                for(y=0;y<menuH;++y) {
                    unsigned char pixel=y<cropH?
                        menu->pixels[y*menu->width+menu->width-1]:0;
                    if(actual[(menuY+y)*320+319]!=pixel) {
                        fprintf(stderr,"FAIL: FMT menu border mode%d actor%d rows%d y%d actual%u expected%u\n",
                            mode,i,rows,menuY+y,actual[(menuY+y)*320+319],pixel);
                        return 0;
                    }
                }
                M11_GameView_ClearActingChampion(state);
            }
            puts("PASS: original FM Towns active-menu C010 border, all actors and modes (not text parity)");
            /* Original C080 Pass is narrower in JDM. Test public pointer
             * routing at both inclusive corners without executing an action. */
            for(mode=0;mode<3;++mode) for(int edge=0;edge<2;++edge) {
                const int passX=yOffset?295:285;
                const int passY=yOffset?85:77;
                int staminaBefore[4];
                state->presentationMode=modes[mode];
                state->world.party.activeChampionIndex=0;
                state->world.party.champions[3].hp.current=100;
                for(i=0;i<4;++i) {
                    state->actionDisabledTicks[i]=0;
                    staminaBefore[i]=state->world.party.champions[i].stamina.current;
                }
                M11_GameView_ClearActingChampion(state);
                LATE_SPELL_CHECK(M11_GameView_SetActingChampion(state,3));
                if(yOffset) {
                    (void)M11_GameView_HandlePointer(state,294,85,1);
                    (void)M11_GameView_HandlePointer(state,294,85,0);
                    LATE_SPELL_CHECK(state->actingChampionOrdinal==4);
                }
                (void)M11_GameView_HandlePointer(state,edge?319:passX,passY+(edge?6:0),1);
                (void)M11_GameView_HandlePointer(state,edge?319:passX,passY+(edge?6:0),0);
                LATE_SPELL_CHECK(state->actingChampionOrdinal==0 &&
                                 state->world.party.activeChampionIndex==0);
                for(i=0;i<4;++i)
                    LATE_SPELL_CHECK(state->world.party.champions[i].stamina.current==staminaBefore[i] &&
                                     state->actionDisabledTicks[i]==0);
                M11_GameView_ClearActingChampion(state);
            }
            puts("PASS: original FM Towns Pass mouse corners close without action cost or leader change");
        }
        if (yOffset) {
            /* JDM C089..C092: inclusive cell edges, independent of the
             * centered icon. F0389 selects an actor, never a new leader. */
            for (mode=0;mode<3;++mode) for (i=0;i<4;++i) {
                const int cellX=233+22*i;
                const int leader=(i+1)%4;
                state->presentationMode=modes[mode];
                state->world.party.activeChampionIndex=leader;
                state->inventoryPanelActive=0;
                state->resting=0;
                for (int champion=0;champion<4;++champion) {
                    state->world.party.champions[champion].hp.current=100;
                    state->actionDisabledTicks[champion]=0;
                }
                for (int edge=0;edge<2;++edge) {
                    int clickX=cellX+(edge?19:0);
                    int clickY=edge?155:94;
                    M11_GameView_ClearActingChampion(state);
                    (void)M11_GameView_HandlePointer(state,clickX,clickY,1);
                    (void)M11_GameView_HandlePointer(state,clickX,clickY,0);
                    LATE_SPELL_CHECK(state->actingChampionOrdinal==(unsigned int)(i+1));
                    LATE_SPELL_CHECK(state->world.party.activeChampionIndex==leader);
                    M11_GameView_ClearActingChampion(state);
                    state->world.party.champions[i].hp.current=0;
                    (void)M11_GameView_HandlePointer(state,clickX,clickY,1);
                    (void)M11_GameView_HandlePointer(state,clickX,clickY,0);
                    LATE_SPELL_CHECK(state->actingChampionOrdinal==0);
                    LATE_SPELL_CHECK(state->world.party.activeChampionIndex==leader);
                    state->world.party.champions[i].hp.current=100;
                    M11_GameView_ClearActingChampion(state);
                }
            }
            puts("PASS: original JDM action cell inclusive edges/dead rejection, all slots and modes");
            /* Original JDM movement zones C068/C069 occupy the top row
             * y160..178. Rotate only: no invented traversable dungeon. */
            for (mode=0;mode<3;++mode) {
                int beforeDirection, beforeX, beforeY;
                state->presentationMode=modes[mode];
                state->world.party.activeChampionIndex=0;
                state->inventoryPanelActive=0;
                state->resting=0;
                for (i=0;i<4;++i) {
                    state->world.party.champions[i].hp.current=100;
                    state->world.party.champions[i].stamina.current=1000;
                    state->world.party.champions[i].stamina.maximum=1000;
                }
                beforeDirection=state->world.party.direction;
                beforeX=state->world.party.mapX; beforeY=state->world.party.mapY;
                (void)M11_GameView_HandlePointer(state,248,169,1);
                (void)M11_GameView_HandlePointer(state,248,169,0);
                LATE_SPELL_CHECK(state->world.party.direction==((beforeDirection+3)&3));
                LATE_SPELL_CHECK(state->world.party.mapX==beforeX && state->world.party.mapY==beforeY);
                (void)M11_GameView_HandlePointer(state,305,169,1);
                (void)M11_GameView_HandlePointer(state,305,169,0);
                LATE_SPELL_CHECK(state->world.party.direction==beforeDirection);
                LATE_SPELL_CHECK(state->world.party.mapX==beforeX && state->world.party.mapY==beforeY);
            }
            puts("PASS: original JDM movement arrow mouse rotation in all three modes");
        }
    }
    {
        /* Exercise real empty-hand actions in the loaded dungeon, without
         * inventing a target or assuming attack success. F0407 records the
         * chosen action even when the authentic front square is empty. */
        for(mode=0;mode<3;++mode) for(int row=0;row<3;++row) {
            unsigned char actions[3];
            const int rowY=(yOffset?94:86)+row*(yOffset?21:12);
            int staminaBefore;
            state->presentationMode=modes[mode];
            state->world.party.activeChampionIndex=0;
            state->inventoryPanelActive=0;
            state->resting=0;
            for(i=0;i<4;++i) {
                state->world.party.champions[i].hp.current=100;
                state->world.party.champions[i].stamina.current=1000;
                state->world.party.champions[i].stamina.maximum=1000;
                state->actionDisabledTicks[i]=0;
            }
            state->world.party.champions[3].actionIndex=255;
            M11_GameView_ClearActingChampion(state);
            LATE_SPELL_CHECK(M11_GameView_SetActingChampion(state,3));
            LATE_SPELL_CHECK(M11_GameView_GetActingActionIndices(state,actions));
            LATE_SPELL_CHECK(actions[0]==6 && actions[1]==7 && actions[2]==8);
            staminaBefore=state->world.party.champions[3].stamina.current;
            /* The one-pixel gap beneath each source row is not an action. */
            (void)M11_GameView_HandlePointer(state,250,rowY+(yOffset?20:11),1);
            (void)M11_GameView_HandlePointer(state,250,rowY+(yOffset?20:11),0);
            LATE_SPELL_CHECK(state->actingChampionOrdinal==4 &&
                             state->world.party.champions[3].actionIndex==255 &&
                             state->world.party.champions[3].stamina.current==staminaBefore);
            (void)M11_GameView_HandlePointer(state,250,rowY+5,1);
            (void)M11_GameView_HandlePointer(state,250,rowY+5,0);
            fprintf(stderr,"FMT action pointer lang%s mode%d row%d y%d actor%u index%u expected%u leader%d stamina%d->%u cooldown%u active%d tick%u\n",
                yOffset?"JP":"EN",mode,row,rowY+5,state->actingChampionOrdinal,
                state->world.party.champions[3].actionIndex,actions[row],
                state->world.party.activeChampionIndex,staminaBefore,
                (unsigned int)state->world.party.champions[3].stamina.current,
                (unsigned int)state->actionDisabledTicks[3],state->active,
                (unsigned int)state->world.gameTick);
            LATE_SPELL_CHECK(state->actingChampionOrdinal==0 &&
                             state->world.party.champions[3].actionIndex==actions[row] &&
                             state->world.party.activeChampionIndex==0);
            LATE_SPELL_CHECK(state->world.party.champions[3].stamina.current<staminaBefore);
            M11_GameView_ClearActingChampion(state);
        }
        puts("PASS: original late-edition empty-hand action rows and gaps, all three modes");
    }
    puts("PASS authentic late spell panel: 24 pixel cases and three mode-specific mouse input cases");
    return 1;
}
#undef LATE_SPELL_CHECK
#endif
