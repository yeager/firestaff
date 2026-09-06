/* Authentic I34E graphics/font; bounded RAM party, names and rune buffers.
 * Pixel oracle follows SPELDRAW.C F0393:87-94 / MENUDRAW F0397,F0398,
 * not the plan. */
#include "m11_game_view.h"
#include "asset_find_by_hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e); goto fail; } } while(0)

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
int main(void) {
    static const int modes[] = { M12_PRESENTATION_V1_ORIGINAL,
        M12_PRESENTATION_V20_FILTERED, M12_PRESENTATION_V21_UPSCALED };
    static const char* names[]={"HALK","TIGGY","WUUF","ZED"};
    static const int tabX[4][4]={{233,280,294,308},{233,247,294,308},
                               {233,247,261,308},{233,247,261,275}};
    const char* archive=getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    M11_GameViewState* state;
    const M11_AssetSlot* background;
    unsigned char actual[64000], expected[64000];
    FILE* media;
    int mode,caster,sparse,i,x,y;
    if(!archive || !(media=fopen(archive,"rb"))) return 77;
    fclose(media);
    if(!asset_file_matches_md5(archive,"ee7b83cdb88c39c441a319f9610e97d6")) return 1;
    state=calloc(1,sizeof(*state)); if(!state) return 1;
    M11_GameView_Init(state);
    CHECK(M11_GameView_StartDm1(state,archive));
    CHECK(state->originalFontAvailable);
    background = M11_AssetLoader_Load(&state->assetLoader, 9);
    CHECK(background && background->loaded && background->pixels &&
          background->width == 87 && background->height == 25);
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
            memcpy(expected+(50+y)*320+233,background->pixels+y*87,87);
        /* COORD.C C221 clear; caster-dependent C224+5*c slots. */
        for(i=0;i<4;++i) if(!sparse||i==caster)
            for(y=42;y<(i==caster?50:49);++y)
                for(x=tabX[caster][i];x<tabX[caster][i]+(i==caster?45:12);++x)
                    expected[y*320+x]=4; /* I34 VGA F8154 XOR4 after black clear. */
        for(i=0;names[caster][i];++i)
            glyph(expected,&state->originalFont,235+14*caster+6*i,48,names[caster][i],0,4);
        CHECK(equal_box(actual,expected,233,42,87,8));
        /* Selected buffer differs from the leader and each other caster. */
        for(i=0;i<6;++i) {
            glyph(expected,&state->originalFont,239+14*i,58,96+6*caster+i,4,0);
            CHECK(equal_box(actual,expected,239+14*i,54,6,6));
        }
        for(i=0;i<4;++i) {
            glyph(expected,&state->originalFont,241+9*i,70,
                  i<2?state->dm1SpellCasting.input[caster].symbols[i]:' ',4,0);
            CHECK(equal_box(actual,expected,241+9*i,66,6,6));
        }
        CHECK(equal_box(actual,expected,233,42,87,33));
        CHECK(state->world.party.activeChampionIndex==(sparse?caster:(caster+1)%4));
        CHECK(state->dm1SpellCasting.magicCasterIndex==caster);
    }
    puts("PASS authentic spell panel: 24 Original/V2.0/V2.1 caster/sparse cases");
    M11_GameView_Shutdown(state); free(state); return 0;
fail: M11_GameView_Shutdown(state); free(state); return 1;
}
