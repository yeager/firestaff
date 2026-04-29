#include <stdio.h>
#include "champion_name_hand_routes_pc34_compat.h"
int main(void) { unsigned int ok = champion_name_hand_routes_GetInvariant(); printf("probe=firestaff_champion_name_hand_routes\n"); printf("sourceEvidence=%s\n", champion_name_hand_routes_GetEvidence()); printf("championNameHandRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
