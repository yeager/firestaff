#include <stdio.h>
#include "endgame_credits_path_pc34_compat.h"
int main(void) { unsigned int ok = endgame_credits_path_GetInvariant(); printf("probe=firestaff_endgame_credits_path\n"); printf("sourceEvidence=%s\n", endgame_credits_path_GetEvidence()); printf("endgameCreditsPathInvariantOk=%u\n", ok); return ok ? 0 : 1; }
