#ifndef REDMCSB_ENDGAME_RESTART_QUIT_PC34_COMPAT_H
#define REDMCSB_ENDGAME_RESTART_QUIT_PC34_COMPAT_H

typedef struct EndgameRestartQuitCompat {
    unsigned int restartCommandId;
    unsigned int quitCommandId;
    unsigned int restartZoneIndex;
    unsigned int quitZoneIndex;
    unsigned int restartWaitVblanks;
    unsigned int restartRequestedFlagSet;
    unsigned int quitDispatchesExit;
    const char* sourceLineEvidence;
} EndgameRestartQuitCompat;

const EndgameRestartQuitCompat* ENDGAME_Compat_GetRestartQuitSchedule(void);
const char* ENDGAME_Compat_GetRestartQuitEvidence(void);

#endif
