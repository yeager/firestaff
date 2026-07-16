/* Opt-in real-data HoC first COMMAND.C movement/HUD lane probe. */
#include "m11_game_view.h"
#include "menu_input_m12.h"
#include <stdio.h>
#include <string.h>
int main(int ac,char**av){M11_GameViewState s;M11_BootProbeReceipt b;M11_Dm1FloorItemHostPresentationReceipt f;unsigned char fb[320*200];const char*d=ac>1?av[1]:0;
if(!d||!*d){puts("SKIP DM1 HoC first move: no data directory");return 0;}M11_GameView_Init(&s);if(!M11_GameView_StartDm1(&s,d)){puts("SKIP DM1 HoC first move: data unavailable");M11_GameView_Shutdown(&s);return 0;}memset(fb,0,sizeof(fb));M11_GameView_Draw(&s,fb,320,200);if(!M11_GameView_GetBootProbeReceipt(&s,&b)||s.candidateMirrorPanelActive){puts("SKIP DM1 HoC first move: not in ordinary player movement state");M11_GameView_Shutdown(&s);return 0;}(void)M11_GameView_HandleInput(&s,M12_MENU_INPUT_UP);(void)M11_GameView_AdvanceIdleTick(&s);M11_GameView_Draw(&s,fb,320,200);M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
if(s.candidateMirrorPanelActive||s.candidateMirrorRenameActive||!M11_GameView_GetBootProbeReceipt(&s,&b)||b.dm1HoCLiveF0115MaterialRequest&& !f.valid){puts("FAIL DM1 HoC first move broke wall/floor/HUD lane ownership");M11_GameView_Shutdown(&s);return 1;}puts("ok: DM1 HoC first movement command keeps wall/door/floor/HUD lanes coherent");M11_GameView_Shutdown(&s);return 0;}
