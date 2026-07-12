/* Opt-in real-data C127 -> C040 rename -> wall return probe. */
#include "m11_game_view.h"
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv) {
    M11_GameViewState s; M11_BootProbeReceipt b; M11_Dm1FloorItemHostPresentationReceipt f;
    unsigned char fb[320 * 200]; const char *d=argc>1?argv[1]:NULL;
    if(!d||!d[0]) { puts("SKIP DM1 HoC C127/C040 rename: no data directory"); return 0; }
    M11_GameView_Init(&s);
    if(!M11_GameView_StartDm1(&s,d)) { puts("SKIP DM1 HoC C127/C040 rename: data unavailable"); M11_GameView_Shutdown(&s); return 0; }
    memset(fb,0,sizeof(fb)); M11_GameView_Draw(&s,fb,320,200); M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
    if(!M11_GameView_GetBootProbeReceipt(&s,&b)||!b.dm1HoCLiveC127MaterialRequest||!b.dm1HoCHallMirrorOverlay||!s.candidateMirrorPanelActive) {
        puts("SKIP DM1 HoC C127/C040 rename: no naturally restored candidate panel"); M11_GameView_Shutdown(&s); return 0;
    }
    if(f.valid||b.dm1HoCLiveF0115MaterialRequest||!M11_GameView_BeginMirrorCandidateReincarnateRename(&s)||!s.candidateMirrorRenameActive) {
        puts("FAIL DM1 HoC C127/C040 rename entered floor/projectile lane"); M11_GameView_Shutdown(&s); return 1;
    }
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
    if(f.valid||!M11_GameView_CancelMirrorCandidate(&s)||s.candidateMirrorPanelActive||s.candidateMirrorRenameActive) {
        puts("FAIL DM1 HoC C040 rename cancel ownership"); M11_GameView_Shutdown(&s); return 1;
    }
    M11_GameView_Draw(&s,fb,320,200); M11_GameView_GetDm1FloorItemHostPresentationReceipt(&f);
    if(!M11_GameView_GetBootProbeReceipt(&s,&b)||f.valid||b.dm1HoCLiveF0115MaterialRequest) {
        puts("FAIL DM1 HoC C127/C040 return leaked F0115 lane"); M11_GameView_Shutdown(&s); return 1;
    }
    puts("ok: DM1 HoC C127 -> C040 rename -> wall return stays lane-disjoint"); M11_GameView_Shutdown(&s); return 0;
}
