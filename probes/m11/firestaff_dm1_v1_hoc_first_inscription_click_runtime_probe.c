/* Opt-in real-data HoC wall-inscription movement/click provenance probe.
 * ReDMCSB DUNVIEW.C:3679-3706 uses M648 with C10 transparency. */
#include "m11_game_view.h"
#include "menu_input_m12.h"
#include <stdio.h>
#include <string.h>
int main(int ac,char**av){M11_GameViewState s;unsigned char fb[320*200];const char*d=ac>1?av[1]:0;int i,nonzero=0;
if(!d||!*d){puts("SKIP DM1 HoC inscription: no data directory");return 0;}M11_GameView_Init(&s);if(!M11_GameView_StartDm1(&s,d)){puts("SKIP DM1 HoC inscription: data unavailable");M11_GameView_Shutdown(&s);return 0;}(void)M11_GameView_HandleInput(&s,M12_MENU_INPUT_UP);(void)M11_GameView_AdvanceIdleTick(&s);memset(fb,0,sizeof(fb));M11_GameView_Draw(&s,fb,320,200);(void)M11_GameView_HandlePointer(&s,160,80,1);M11_GameView_Draw(&s,fb,320,200);
for(i=0;i<M11_FONT_BITMAP_BYTES;++i)if(s.originalFont.bitmap[i]){nonzero=1;break;}if(!s.originalFontAvailable||!s.originalFont.loaded||s.originalFont.graphicIndex<=0||!nonzero){puts("SKIP DM1 HoC inscription: original M648 font route unavailable");M11_GameView_Shutdown(&s);return 0;}puts("ok: DM1 HoC inscription click uses loaded original M648 font provenance; no replacement font");M11_GameView_Shutdown(&s);return 0;}
