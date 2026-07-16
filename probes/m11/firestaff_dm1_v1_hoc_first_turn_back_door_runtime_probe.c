/* Opt-in real-data HoC first turn/back/door interaction lane probe.
 * ReDMCSB COMMAND.C routes turn/back; DUNVIEW.C owns the resulting lanes. */
#include "m11_game_view.h"
#include "menu_input_m12.h"
#include <stdio.h>
#include <string.h>
int main(int ac,char**av){M11_GameViewState s;M11_BootProbeReceipt b;M11_Dm1FloorItemHostPresentationReceipt f;unsigned char fb[320*200];const char*d=ac>1?av[1]:0;
if(!d||!*d){puts("SKIP DM1 HoC turn/door: no data directory");return 0;}M11_GameView_Init(&s);if(!M11_GameView_StartDm1(&s,d)){puts("SKIP DM1 HoC turn/door: data unavailable");M11_GameView_Shutdown(&s);return 0;}memset(fb,0,sizeof(fb));M11_GameView_Draw(&s,fb,320,200);if(s.candidateMirrorPanelActive){puts("SKIP DM1 HoC turn/door: candidate panel active");M11_GameView_Shutdown(&s);return 0;}(void)M11_GameView_HandleInput(&s,M12_MENU_INPUT_TURN_LEFT);(void)M11_GameView_HandleInput(&s,M12_MENU_INPUT_DOWN);(void)M11_GameView_HandleInput(&s,M12_MENU_INPUT_ACTION);(void)M11_GameView_AdvanceIdleTick(&s);M11_GameView_Draw(&s,fb,320,200);M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
if(s.candidateMirrorPanelActive||s.candidateMirrorRenameActive||!M11_GameView_GetBootProbeReceipt(&s,&b)||b.dm1HoCLiveF0115MaterialRequest&&!f.valid){puts("FAIL DM1 HoC turn/back/door broke render lane ownership");M11_GameView_Shutdown(&s);return 1;}puts("ok: DM1 HoC turn/back/door interaction preserves source render lanes");M11_GameView_Shutdown(&s);return 0;}
