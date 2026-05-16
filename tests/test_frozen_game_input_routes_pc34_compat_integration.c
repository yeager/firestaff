#include <stdio.h>
#include "frozen_game_input_routes_pc34_compat.h"
int main(void) { unsigned int ok = frozen_game_input_routes_GetInvariant(); printf("probe=firestaff_frozen_game_input_routes\n"); printf("sourceEvidence=%s\n", frozen_game_input_routes_GetEvidence()); printf("frozenGameInputRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
