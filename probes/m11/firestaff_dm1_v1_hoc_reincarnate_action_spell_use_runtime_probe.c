/* Opt-in real-data HoC resurrection -> HUD action/spell use lane probe. */
#include "m11_game_view.h"
#include "firestaff/dm1/v1/resurrection_rename_ui_gate_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(int ac,char **av){M11_GameViewState s;M11_BootProbeReceipt b;M11_Dm1FloorItemHostPresentationReceipt f;unsigned char fb[320*200];const char*d=ac>1?av[1]:0;int p;
if(!d||!*d){puts("SKIP DM1 HoC action use: no data directory");return 0;}M11_GameView_Init(&s);if(!M11_GameView_StartDm1(&s,d)){puts("SKIP DM1 HoC action use: data unavailable");M11_GameView_Shutdown(&s);return 0;}memset(fb,0,sizeof(fb));M11_GameView_Draw(&s,fb,320,200);M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
if(!M11_GameView_GetBootProbeReceipt(&s,&b)||!b.dm1HoCLiveC127MaterialRequest||!s.candidateMirrorPanelActive){puts("SKIP DM1 HoC action use: no natural mirror candidate");M11_GameView_Shutdown(&s);return 0;}p=s.candidateMirrorPartyIndex;
if(f.valid||b.dm1HoCLiveF0115MaterialRequest||p<0||!M11_GameView_BeginMirrorCandidateReincarnateRename(&s)||!M11_GameView_ApplyMirrorCandidateRenameAscii(&s,'A')||!M11_GameView_ApplyMirrorCandidateRenameCommand(&s,DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT)){puts("FAIL DM1 HoC resurrection route");M11_GameView_Shutdown(&s);return 1;}
s.world.party.activeChampionIndex=p;if(!M11_GameView_UseItem(&s)){puts("SKIP DM1 HoC action use: resurrected champion has no usable source action/spell item");M11_GameView_Shutdown(&s);return 0;}M11_GameView_Draw(&s,fb,320,200);M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
if(f.valid||!M11_GameView_GetBootProbeReceipt(&s,&b)||b.dm1HoCLiveF0115MaterialRequest||s.candidateMirrorPanelActive){puts("FAIL DM1 HoC action/spell lane leaked F0115/object material");M11_GameView_Shutdown(&s);return 1;}puts("ok: DM1 HoC resurrected champion action/spell HUD use stays lane-disjoint");M11_GameView_Shutdown(&s);return 0;}
