#include <stdio.h>
#include "endgame_restart_quit_pc34_compat.h"
int main(void) {
    const EndgameRestartQuitCompat* s = ENDGAME_Compat_GetRestartQuitSchedule();
    int ok = s && s->restartCommandId == 215u && s->quitCommandId == 216u && s->restartZoneIndex == 437u && s->quitZoneIndex == 438u && s->restartWaitVblanks == 900u && s->restartRequestedFlagSet && s->quitDispatchesExit;
    printf("probe=firestaff_endgame_restart_quit_source_schedule\n");
    printf("restartQuitEvidence=%s\n", ENDGAME_Compat_GetRestartQuitEvidence());
    printf("restartCommand=%u quitCommand=%u restartZone=%u quitZone=%u waitVblanks=%u evidence:%s\n", s->restartCommandId, s->quitCommandId, s->restartZoneIndex, s->quitZoneIndex, s->restartWaitVblanks, s->sourceLineEvidence);
    printf("endgameRestartQuitInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
