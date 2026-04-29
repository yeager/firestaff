#include "endgame_restart_quit_pc34_compat.h"

static const EndgameRestartQuitCompat kRestartQuit = {
    215u, 216u, 437u, 438u, 900u, 1u, 1u,
    "ENDGAME.C:485-543 draws restart/quit buttons at C437/C438; ENDGAME.C:559-568 clears restart flag then waits 900 VBlanks; COMMAND.C:2463-2468 maps C215 to G0523_B_RestartGameRequested and C216 to quit"
};

const EndgameRestartQuitCompat* ENDGAME_Compat_GetRestartQuitSchedule(void) { return &kRestartQuit; }

const char* ENDGAME_Compat_GetRestartQuitEvidence(void) {
    return "ReDMCSB source lock: DEFS.H defines C215_COMMAND_RESTART_GAME/C216_COMMAND_QUIT and C437/C438 endgame zones; ENDGAME.C draws restart and quit controls, installs G0446 restart-game mouse input, clears G0523, waits 900 VBlank/delay ticks for restart, and branches if requested; COMMAND.C dispatches restart/quit commands.";
}
