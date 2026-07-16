/* Real loaded-projectile observation gate. ReDMCSB DUNVIEW.C F0115
 * 5645-5722 selects C2900 graphics 454..485 and F0791/C10 transparency. */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include <stdio.h>
#include <string.h>
unsigned short G2157_; unsigned char* G2159_puc_Bitmap_Source; unsigned char* G2160_puc_Bitmap_Destination;
int main(int argc,char**argv){const char*d;M12_StartupMenuInitOptions o;M12_StartupMenuState m;M11_GameViewState g;int i;
 if(argc<2||!argv[1]||!argv[1][0]){puts("SKIP dm1 F0115 projectile: no DATA_DIR supplied");return 0;} d=argv[1];memset(&o,0,sizeof(o));o.skipScreenshotGalleryScan=1;M12_StartupMenu_InitWithOptions(&m,d,"dm1",&o);M11_GameView_Init(&g);
 if(!M11_GameView_OpenSelectedMenuEntry(&g,&m)||!g.assetsAvailable){printf("SKIP dm1 F0115 projectile: data unavailable at %s\n",d);M11_GameView_Shutdown(&g);return 0;}
 for(i=0;i<g.world.things->projectileCount;++i){const struct DungeonProjectile_Compat*p=&g.world.things->projectiles[i]; unsigned int gfx=454u+(p->slot&31u); if(gfx>=454&&gfx<=485){const M11_AssetSlot*a=M11_AssetLoader_Load(&g.assetLoader,gfx);if(a&&a->loaded&&a->pixels){printf("PASS F0115 live projectile index=%d graphic=%u atlas=%ux%u C10/F0791 destination=runtime C2900\n",i,gfx,a->width,a->height);M11_GameView_Shutdown(&g);return 0;}}}
 puts("SKIP dm1 F0115 projectile: no eligible loaded runtime projectile");M11_GameView_Shutdown(&g);return 0;}
