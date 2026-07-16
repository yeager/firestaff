/* Real-data DM1 F0115 floor-item material receipt.
 * ReDMCSB DUNVIEW.C F0115:4547-4581/4820 draws objects before creatures;
 * C10 is the object transparency key.  This probe finds, rather than makes,
 * an open-square item in the loaded dungeon and drives it through D1C. */
#include "dm1_v1_probe_assets.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"
#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum { FB_W=320, FB_H=200, C10_TRANSPARENT=10 };

static int is_item(int t) { return dm1_v1_thing_type_is_floor_item_pc34(t); }
static unsigned short next_thing(const struct DungeonThings_Compat* things, unsigned short thing) {
    int t=(int)THING_GET_TYPE(thing), i=(int)THING_GET_INDEX(thing);
    const unsigned char* raw;
    if (!things || thing==THING_NONE || thing==THING_ENDOFLIST || t<0 || t>=16 ||
        !s_thingDataByteCount[t] || !things->rawThingData[t] || i<0 || i>=things->thingCounts[t]) return THING_ENDOFLIST;
    raw=things->rawThingData[t]+i*(int)s_thingDataByteCount[t];
    return (unsigned short)(raw[0]|((unsigned short)raw[1]<<8));
}
static int subtype(const struct DungeonThings_Compat* t, unsigned short thing) {
    int i=(int)THING_GET_INDEX(thing);
    switch (THING_GET_TYPE(thing)) {
    case THING_TYPE_WEAPON: return t->weapons && i<t->weaponCount ? t->weapons[i].type : -1;
    case THING_TYPE_ARMOUR: return t->armours && i<t->armourCount ? t->armours[i].type : -1;
    case THING_TYPE_POTION: return t->potions && i<t->potionCount ? t->potions[i].type : -1;
    case THING_TYPE_JUNK: return t->junks && i<t->junkCount ? t->junks[i].type : -1;
    case THING_TYPE_SCROLL: return 0;
    case THING_TYPE_CONTAINER: return 0;
    default: return -1;
    }
}
static int open_square(const M11_GameViewState* g,int m,int x,int y) {
    const struct DungeonMapDesc_Compat* map=&g->world.dungeon->maps[m];
    unsigned char s;
    if(x<0||y<0||x>=map->width||y>=map->height) return 0;
    s=g->world.dungeon->tiles[m].squareData[x*(int)map->height+y];
    return ((s&DUNGEON_SQUARE_MASK_TYPE)>>5)!=DUNGEON_ELEMENT_WALL;
}
int main(int argc,char** argv) {
    const char* data; M12_StartupMenuInitOptions o; M12_StartupMenuState menu; M11_GameViewState g;
    int map=-1,ix=-1,iy=-1,dir=-1,type=-1,sub=-1,cell=-1, mi,x,y,d, ok=1;
    unsigned char fb[FB_W*FB_H]; const M11_AssetSlot* asset; DM1_ItemSpriteBlitPlan plan;
    if(argc<2||!argv[1]||!argv[1][0]) { puts("SKIP dm1 F0115 item material: no DATA_DIR supplied"); return 0; }
    data=argv[1]; memset(&o,0,sizeof(o)); o.skipScreenshotGalleryScan=1;
    M12_StartupMenu_InitWithOptions(&menu,data,"dm1",&o); M11_GameView_Init(&g);
    if(!M11_GameView_OpenSelectedMenuEntry(&g,&menu)||!g.assetsAvailable) { printf("SKIP dm1 F0115 item material: data unavailable at %s\n",data); M11_GameView_Shutdown(&g); return 0; }
    for(mi=0;mi<(int)g.world.dungeon->header.mapCount&&map<0;++mi) {
        const struct DungeonMapDesc_Compat* md=&g.world.dungeon->maps[mi];
        for(y=0;y<(int)md->height&&map<0;++y) for(x=0;x<(int)md->width&&map<0;++x) {
            unsigned short q=F0511_DUNGEON_GetSquareFirstThing_Compat(g.world.dungeon,g.world.things,mi,x,y); int guard=0;
            if(!open_square(&g,mi,x,y)) continue;
            while(q!=THING_NONE&&q!=THING_ENDOFLIST&&guard++<64) {
                if(is_item(THING_GET_TYPE(q)) && subtype(g.world.things,q)>=0) {
                    static const int dx[4]={0,-1,0,1},dy[4]={1,0,-1,0};
                    for(d=0;d<4;++d) if(open_square(&g,mi,x+dx[d],y+dy[d])) { map=mi;ix=x;iy=y;dir=d;type=THING_GET_TYPE(q);sub=subtype(g.world.things,q);cell=THING_GET_CELL(q);break; }
                    break;
                } q=next_thing(g.world.things,q);
            }
        }
    }
    if(map<0) { puts("SKIP dm1 F0115 item material: no source floor item route"); M11_GameView_Shutdown(&g); return 0; }
    { static const int dx[4]={0,-1,0,1},dy[4]={1,0,-1,0}; g.world.party.mapIndex=map; g.world.party.mapX=ix+dx[dir]; g.world.party.mapY=iy+dy[dir]; g.world.party.direction=dir; }
    asset=M11_AssetLoader_Load(&g.assetLoader,dm1_item_sprite_index(type,sub));
    if(!asset||!asset->loaded||!asset->pixels) { puts("SKIP dm1 F0115 item material: original object atlas unavailable"); M11_GameView_Shutdown(&g); return 0; }
    ok &= dm1_item_sprite_blit_plan(&plan,type,sub,cell,0,0,0,0,33,16,46,192,110,(int)asset->width,(int)asset->height);
    memset(fb,0,sizeof(fb)); M11_GameView_Draw(&g,fb,FB_W,FB_H);
    { int sx,sy,opaque=0,seen=0; for(sy=0;sy<(int)asset->height;++sy) for(sx=0;sx<(int)asset->width;++sx) if((asset->pixels[sy*(int)asset->width+sx]&15)!=C10_TRANSPARENT) ++opaque;
      for(sy=plan.draw_y;sy<plan.draw_y+plan.draw_h;++sy) for(sx=plan.draw_x;sx<plan.draw_x+plan.draw_w;++sx) if((fb[sy*FB_W+sx]&15)!=C10_TRANSPARENT) ++seen;
      printf("INFO F0115 map=%d item=(%d,%d) type=%d subtype=%d atlas=%u %ux%u palette=C10 dst=(%d,%d,%d,%d) opaque=%d dst-non-C10=%d\n",map,ix,iy,type,sub,plan.graphic_index,asset->width,asset->height,plan.draw_x,plan.draw_y,plan.draw_w,plan.draw_h,opaque,seen);
      ok &= opaque>0 && seen>0 && plan.transparent_color==C10_TRANSPARENT;
    }
    M11_GameView_Shutdown(&g); return ok?0:1;
}
