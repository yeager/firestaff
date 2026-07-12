/* Opt-in real-data HoC C127 -> C040 reincarnate/rename -> party HUD probe. */
#include "m11_game_view.h"
#include "firestaff/dm1/v1/resurrection_rename_ui_gate_pc34_compat.h"
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv) {
    M11_GameViewState s; M11_BootProbeReceipt b; M11_Dm1FloorItemHostPresentationReceipt f;
    unsigned char fb[320*200]; const char *d=argc>1?argv[1]:NULL; int party_index;
    if(!d||!d[0]) { puts("SKIP DM1 HoC C127 party HUD: no data directory"); return 0; }
    M11_GameView_Init(&s);
    if(!M11_GameView_StartDm1(&s,d)) { puts("SKIP DM1 HoC C127 party HUD: data unavailable"); M11_GameView_Shutdown(&s); return 0; }
    memset(fb,0,sizeof(fb)); M11_GameView_Draw(&s,fb,320,200); M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
    if(!M11_GameView_GetBootProbeReceipt(&s,&b)||!b.dm1HoCLiveC127MaterialRequest||!b.dm1HoCHallMirrorOverlay||!s.candidateMirrorPanelActive) {
        puts("SKIP DM1 HoC C127 party HUD: no naturally restored mirror candidate"); M11_GameView_Shutdown(&s); return 0;
    }
    party_index=s.candidateMirrorPartyIndex;
    if(f.valid||b.dm1HoCLiveF0115MaterialRequest||party_index<0||party_index>=s.world.party.championCount||
       !M11_GameView_BeginMirrorCandidateReincarnateRename(&s)||!s.candidateMirrorRenameActive ||
       !M11_GameView_ApplyMirrorCandidateRenameAscii(&s,'A') ||
       !M11_GameView_ApplyMirrorCandidateRenameCommand(&s,DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT)) {
        puts("FAIL DM1 HoC C127 reincarnate/rename route"); M11_GameView_Shutdown(&s); return 1;
    }
    M11_GameView_Draw(&s,fb,320,200); M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
    if(s.candidateMirrorPanelActive||s.candidateMirrorRenameActive||party_index>=s.world.party.championCount||
       !s.world.party.champions[party_index].present||s.world.party.champions[party_index].name[0]=='\0'||
       f.valid||!M11_GameView_GetBootProbeReceipt(&s,&b)||b.dm1HoCLiveF0115MaterialRequest) {
        puts("FAIL DM1 HoC party/HUD handoff leaked mirror or F0115 lane"); M11_GameView_Shutdown(&s); return 1;
    }
    printf("ok: DM1 HoC C127 reincarnate/rename joined party slot=%d HUD redraw F0115=0\n",party_index);
    M11_GameView_Shutdown(&s); return 0;
}
