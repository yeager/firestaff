/* Opt-in real-data HoC C127/C040 reincarnate -> party HUD action/spell redraw probe. */
#include "m11_game_view.h"
#include "firestaff/dm1/v1/resurrection_rename_ui_gate_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){
 M11_GameViewState s; M11_BootProbeReceipt b; M11_Dm1FloorItemHostPresentationReceipt f; unsigned char fb[320*200]; const char*d=argc>1?argv[1]:0; int p;
 if(!d||!*d){puts("SKIP DM1 HoC party action HUD: no data directory");return 0;} M11_GameView_Init(&s);
 if(!M11_GameView_StartDm1(&s,d)){puts("SKIP DM1 HoC party action HUD: data unavailable");M11_GameView_Shutdown(&s);return 0;}
 memset(fb,0,sizeof(fb));M11_GameView_Draw(&s,fb,320,200);M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
 if(!M11_GameView_GetBootProbeReceipt(&s,&b)||!b.dm1HoCLiveC127MaterialRequest||!b.dm1HoCHallMirrorOverlay||!s.candidateMirrorPanelActive){puts("SKIP DM1 HoC party action HUD: no natural candidate panel");M11_GameView_Shutdown(&s);return 0;}
 p=s.candidateMirrorPartyIndex;
 if(f.valid||b.dm1HoCLiveF0115MaterialRequest||p<0||!M11_GameView_BeginMirrorCandidateReincarnateRename(&s)||!M11_GameView_ApplyMirrorCandidateRenameAscii(&s,'A')||!M11_GameView_ApplyMirrorCandidateRenameCommand(&s,DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT)){puts("FAIL DM1 HoC reincarnate route");M11_GameView_Shutdown(&s);return 1;}
 memset(fb,0,sizeof(fb));M11_GameView_Draw(&s,fb,320,200);M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
 if(p>=s.world.party.championCount||!s.world.party.champions[p].present||s.candidateMirrorPanelActive||s.candidateMirrorRenameActive||f.valid||!M11_GameView_GetBootProbeReceipt(&s,&b)||b.dm1HoCLiveF0115MaterialRequest){puts("FAIL DM1 HoC HUD action/spell redraw contaminated mirror/object lane");M11_GameView_Shutdown(&s);return 1;}
 printf("ok: DM1 HoC party slot=%d HUD action/spell redraw with C127/F0115 lanes clear\n",p);M11_GameView_Shutdown(&s);return 0;
}
