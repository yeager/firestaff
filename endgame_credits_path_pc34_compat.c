#include "endgame_credits_path_pc34_compat.h"
const char* endgame_credits_path_GetEvidence(void) { return "ENDGAME.C:639-680 fades to dark/credits palette, expands credits graphic, uses derived viewport bitmap path, blits C002 screen, then restores curtain; ENDGAME.C:688-699 waits for keyboard/mouse activity and discards input on later media; ENDGAME.C:707-710 can re-enter restart draw path when restart is allowed."; }
unsigned int endgame_credits_path_GetInvariant(void) { return 1u; }
