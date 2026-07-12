/* Opt-in real-data HoC C127 -> reincarnate -> projectile-lane probe.
 * ReDMCSB DUNVIEW.C F0115 draws objects before separate projectiles (5645+). */
#include "m11_game_view.h"
#include "firestaff/dm1/v1/resurrection_rename_ui_gate_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(int ac,char **av){M11_GameViewState s;M11_BootProbeReceipt b;M11_Dm1FloorItemHostPresentationReceipt f;unsigned char fb[320*200];const char*d=ac>1?av[1]:0;
if(!d||!*d){puts("SKIP DM1 HoC throw: no data directory");return 0;}M11_GameView_Init(&s);if(!M11_GameView_StartDm1(&s,d)){puts("SKIP DM1 HoC throw: data unavailable");M11_GameView_Shutdown(&s);return 0;}memset(fb,0,sizeof(fb));M11_GameView_Draw(&s,fb,320,200);M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
if(!M11_GameView_GetBootProbeReceipt(&s,&b)||!b.dm1HoCLiveC127MaterialRequest||!b.dm1HoCHallMirrorOverlay||!s.candidateMirrorPanelActive||f.valid||b.dm1HoCLiveF0115MaterialRequest){puts("SKIP DM1 HoC throw: no natural C127 candidate frame");M11_GameView_Shutdown(&s);return 0;}
if(!M11_GameView_BeginMirrorCandidateReincarnateRename(&s)||!M11_GameView_ApplyMirrorCandidateRenameAscii(&s,'A')||!M11_GameView_ApplyMirrorCandidateRenameCommand(&s,DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT)){puts("FAIL DM1 HoC throw resurrection route");M11_GameView_Shutdown(&s);return 1;}
M11_GameView_Draw(&s,fb,320,200);M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);if(f.valid||!M11_GameView_ProbeDrawDm1ProjectileForFloorItemReceipt(&s,fb,320,200)){puts("SKIP DM1 HoC throw: no source projectile lane");M11_GameView_Shutdown(&s);return 0;}M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
if(f.valid||M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1)){puts("FAIL DM1 HoC projectile contaminated F0115 floor lane");M11_GameView_Shutdown(&s);return 1;}puts("ok: DM1 HoC C127/HUD projectile lane stays separate from F0115");M11_GameView_Shutdown(&s);return 0;}
